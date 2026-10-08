from pathlib import Path
import re,json
s='\n'.join(Path(p).read_text(encoding='utf-8') for p in ['src/workspace.c','src/package.c'])
en=sorted(set(re.findall(r'(?:package_error|error_text|windows_error)\([^;]+?"([^"]+)"\)',s)))
zh='''此应用的另一版本已加载
应用 DLL ABI 与清单或运行库不符
应用 DLL 缺少必要的生命周期回调
应用 DLL 缺少 ui_app_query_v1
不支持此应用架构、ABI 或框架 API。
应用仍在关闭
应用清单 API 与加载的 DLL 不符
无法取得应用模块路径
应用包不能替换 ui_framework.dll
应用包超过 256 MiB。
应用返回了非法关闭决定
无法创建应用容器
无法创建应用 host
无法创建应用临时目录
无法创建输出应用包。
无法枚举源码目录。
无法完成包内 DLL 解压
无法加载应用 DLL 或其依赖
无法打开输入文件。
无法读取包内 DLL
无法解析 UTF-8 绝对包路径。
无法卸载应用 DLL
无法写入包内 DLL
DLL 解压路径过长或不可用
DLL 已卸载，但无法移除应用临时文件
请等待当前回调返回后再打开应用
未能完整读取输入文件。
未能完整写入输出应用包。
读取期间输入文件发生变化。
输入文件过大或无法读取。
包头、索引、资源路径或模块项格式错误。
清单字段缺失、重复、未知或非法。
清单为空、过大或不是有效 UTF-8。
必须提供清单、源码目录及输出路径。
模块清理失败；请重启宿主后再打开此应用
模块项缺失或包超过 256 MiB。
枚举源码文件时内存不足。
打开应用包时内存不足。
读取输入文件时内存不足。
读取清单时内存不足。
读取包索引时内存不足。
读取资源名称时内存不足。
存储源码文件时内存不足。
输出必须位于源码目录之外，且不能替换清单。
必须提供应用包路径及输出位置。
源码包含符号链接或非常规文件。
源码包含符号链接、重解析点或设备。
源码包含不安全的资源路径。
源码目录没有应用文件。
源码目录过深或分配失败。
源码目录不可用或为符号链接。
源码目录不可用。
源码枚举失败。
源码文件的资源名称不安全或不受支持。
源码包含忽略大小写后重复或冲突的资源名称。
源码不得包含符号链接、联接或重解析点。
此应用需要 64 位宿主。
源码文件数量过多。
无法构建应用包。
不支持此应用包格式版本。'''.splitlines()
assert len(en)==len(zh)==60
with Path('src/language_catalog.inc').open('a',encoding='utf-8') as f:
 for e,c in zip(en,zh):f.write('UI_TEXT('+','.join(json.dumps(x,ensure_ascii=False) for x in [e,c,e])+')\n')
 f.write('UI_TEXT("Windows error","Windows 错误","Windows error")\n')
p=Path('examples/framework_host/main.c');s=p.read_text(encoding='utf-8');old='if(create_popup(s,4,500,240)){popup_start(s,&json,tr(s,"操作未完成"),text,0);popup_finish(s,&json);}'
new='''char display[1024];const char *translated=tr(s,text),*suffix=strstr(text," (Windows error ");
    if(suffix){char operation[512];size_t bytes=(size_t)(suffix-text);if(bytes<sizeof(operation)){memcpy(operation,text,bytes);operation[bytes]=0;snprintf(display,sizeof(display),"%s (%s %s",tr(s,operation),tr(s,"Windows error"),suffix+16);translated=display;}}
    if(create_popup(s,4,500,240)){popup_start(s,&json,tr(s,"操作未完成"),translated,0);popup_finish(s,&json);}'''
assert old in s;s=s.replace(old,new);p.write_text(s,encoding='utf-8')
