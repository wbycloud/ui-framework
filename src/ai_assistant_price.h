#ifndef UI_AI_ASSISTANT_PRICE_H
#define UI_AI_ASSISTANT_PRICE_H

#define AI_RELEASE_PRICE_PREFIX "release-until-"

/* A release rate expires at 00:00 UTC on the date in its price version.
 * Existing explicit policy versions retain their original semantics. */
static int ai_assistant_price_current(const char *version)
{
    const size_t prefix = sizeof(AI_RELEASE_PRICE_PREFIX) - 1;
    if(strncmp(version,AI_RELEASE_PRICE_PREFIX,prefix))return 1;
    const char *date=version+prefix;
    unsigned part[3]={0};
    for(unsigned i=0;i<8;++i){
        if(date[i]<'0'||date[i]>'9')return 0;
        unsigned index=i<4?0:i<6?1:2;
        part[index]=part[index]*10+(unsigned)(date[i]-'0');
    }
    if(date[8]!='-'||!date[9])return 0;
    SYSTEMTIME expiry={0};expiry.wYear=(WORD)part[0];expiry.wMonth=(WORD)part[1];expiry.wDay=(WORD)part[2];
    FILETIME limit,now;
    if(!SystemTimeToFileTime(&expiry,&limit))return 0;
    GetSystemTimeAsFileTime(&now);
    return CompareFileTime(&now,&limit)<0;
}

#endif
