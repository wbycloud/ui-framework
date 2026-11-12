#ifndef UI_CAPABILITY_FIXTURE_CONTROL_H
#define UI_CAPABILITY_FIXTURE_CONTROL_H
/* Test-only named synchronization gates, never module pointers in a message. */
#define CAP_FIXTURE_RESULT 1u
#define CAP_FIXTURE_POLL 2u
#define CAP_FIXTURE_FILLER 3u
#define CAP_FIXTURE_RETRY 4u
#define CAP_FIXTURE_POST 0
#define CAP_FIXTURE_WORKER_EXIT 1
#define CAP_FIXTURE_CALLBACK_EXIT 2
#define CAP_FIXTURE_WORKER_STARTED 3
#define CAP_FIXTURE_CALLBACK_STARTED 4
#define CAP_FIXTURE_POST_OK 5
#define CAP_FIXTURE_POST_LIMIT 6
#define CAP_FIXTURE_GATE_COUNT 7
static void cap_fixture_gate_name(wchar_t name[128],uint64_t instance,int gate)
{(void)swprintf(name,128,L"Local\\UiCapCloseout_%lu_%llu_%d",GetCurrentProcessId(),(unsigned long long)instance,gate);}
#endif
