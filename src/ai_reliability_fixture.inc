/* Isolated Windows test runtime ONLY. No production VFS or exported API. */
static sqlite3_vfs ar_fault_vfs;static sqlite3_io_methods ar_fault_io;static const sqlite3_io_methods *ar_original_io;
static INIT_ONCE ar_vfs_once=INIT_ONCE_STATIC_INIT;static SRWLOCK ar_io_lock=SRWLOCK_INIT;static volatile LONG ar_disk_fault;
static int ar_write(sqlite3_file *f,const void *p,int n,sqlite3_int64 off)
{LONG code=InterlockedCompareExchange(&ar_disk_fault,0,0);if((code==SQLITE_FULL||code==SQLITE_IOERR_WRITE)&&InterlockedCompareExchange(&ar_disk_fault,0,code)==code)return (int)code;return ar_original_io->xWrite(f,p,n,off);}
static int ar_sync(sqlite3_file *f,int flags)
{if(InterlockedCompareExchange(&ar_disk_fault,0,SQLITE_IOERR_FSYNC)==SQLITE_IOERR_FSYNC)return SQLITE_IOERR_FSYNC;return ar_original_io->xSync(f,flags);}
static int ar_open(sqlite3_vfs *v,const char *name,sqlite3_file *f,int flags,int *out)
{
 sqlite3_vfs *base=v->pAppData;int rc=base->xOpen(base,name,f,flags,out);if(rc!=SQLITE_OK)return rc;
 AcquireSRWLockExclusive(&ar_io_lock);if(!ar_original_io){ar_original_io=f->pMethods;ar_fault_io=*ar_original_io;ar_fault_io.xWrite=ar_write;ar_fault_io.xSync=ar_sync;}
 if(f->pMethods!=ar_original_io){ReleaseSRWLockExclusive(&ar_io_lock);f->pMethods->xClose(f);return SQLITE_CANTOPEN;}f->pMethods=&ar_fault_io;ReleaseSRWLockExclusive(&ar_io_lock);return rc;
}
static BOOL CALLBACK ar_register_vfs(PINIT_ONCE once,PVOID data,PVOID *context)
{sqlite3_vfs *base=sqlite3_vfs_find(NULL);(void)once;(void)data;(void)context;if(!base)return FALSE;ar_fault_vfs=*base;ar_fault_vfs.zName="ui-p6-isolated";ar_fault_vfs.pAppData=base;ar_fault_vfs.pNext=NULL;ar_fault_vfs.xOpen=ar_open;return sqlite3_vfs_register(&ar_fault_vfs,0)==SQLITE_OK;}
static const char *ar_test_vfs(void)
{return InitOnceExecuteOnce(&ar_vfs_once,ar_register_vfs,NULL,NULL)?ar_fault_vfs.zName:NULL;}

static void ar_crash(ui_ai_service_t *s,int point)
{if(InterlockedCompareExchange(&s->fixture_crash,0,0)==point){fprintf(stderr,"P6 controlled process interruption point=%d pid=%lu\n",point,GetCurrentProcessId());fflush(stderr);ExitProcess(86);}}
