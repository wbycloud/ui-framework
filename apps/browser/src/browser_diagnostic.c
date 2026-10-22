#include "browser_diagnostic.h"
#include <stdio.h>
browser_failure_t browser_url_failure(browser_url_result_t result,int redirect)
{
    if(result==BROWSER_URL_SCHEME)return redirect?BROWSER_FAILURE_REDIRECT_PROTOCOL:BROWSER_FAILURE_PROTOCOL;
    if(result==BROWSER_URL_CREDENTIALS)return redirect?BROWSER_FAILURE_REDIRECT_CREDENTIALS:BROWSER_FAILURE_CREDENTIALS;
    if(result==BROWSER_URL_LIMIT)return BROWSER_FAILURE_BUDGET;
    if(result==BROWSER_URL_PORT)return BROWSER_FAILURE_PORT;
    return redirect?BROWSER_FAILURE_REDIRECT_URL:BROWSER_FAILURE_URL;
}
const char *browser_failure_name(browser_failure_t kind)
{
    static const char *names[]={"none","url","protocol","credentials","origin","redirect-url","redirect-protocol","redirect-credentials","redirect-origin","redirect-limit","transport","tls","http","content-type","attachment","budget","encoding","image","document-limit","document-empty","document","queue","cancelled","port"};
    return (unsigned)kind<sizeof(names)/sizeof(names[0])?names[kind]:"unknown";
}
int browser_failure_system_error(browser_failure_t kind,unsigned long code)
{return (kind==BROWSER_FAILURE_TRANSPORT||kind==BROWSER_FAILURE_TLS)&&code!=1460;}
void browser_diagnostic_format(browser_failure_t kind,browser_phase_t phase,unsigned long windows,unsigned long http,unsigned long tls,char *out,size_t capacity)
{
    static const char *phases[]={"未开始","创建会话","解析名称","建立连接/TLS","发送请求","等待响应头","读取响应","转换编码","解码图片"};
    static const char *messages[]={"请求完成","地址无效；请输入完整HTTP/HTTPS地址","协议不受支持；仅允许HTTP/HTTPS","地址包含用户名或密码，已拒绝","来源未允许；请在可信来源设置中明确添加","重定向目标地址无效","重定向切换到不支持的协议","重定向目标包含凭据","重定向目标来源未允许","重定向循环或超过8次限制","网络传输失败；可重试并核对系统网络/代理设置","TLS或证书验证失败；未忽略系统验证错误","HTTP响应失败","内容类型不受支持","拒绝附件/下载","资源大小、数量或呈现预算超限","文本编码无效、不支持或转换后超限","图片损坏或格式不受支持","文档节点、深度、文本或布局预算超限","无可读内容；本版不执行远程脚本","文档解析或呈现失败","资源或结果队列失败","请求已取消"};
    const char *text=kind==BROWSER_FAILURE_PORT?"端口0不受支持；WinHTTP会将0解释为默认端口，已拒绝":(unsigned)kind<sizeof(messages)/sizeof(messages[0])?messages[kind]:"未知失败";
    const char *stage=(unsigned)phase<sizeof(phases)/sizeof(phases[0])?phases[phase]:"未知阶段";
    if(kind==BROWSER_FAILURE_TRANSPORT&&windows==1460)text="应用操作等待期限已到；请求已取消，可重试并检查网络/代理";
    snprintf(out,capacity,"%s [%s]；阶段：%s；%s %lu；HTTP %lu；TLS标志 0x%lx",text,browser_failure_name(kind),stage,browser_failure_system_error(kind,windows)?"Windows/WinHTTP系统码":"应用状态码（非系统返回）",windows,http,tls);
}
