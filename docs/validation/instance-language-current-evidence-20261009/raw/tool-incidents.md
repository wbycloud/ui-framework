# 本轮工具/夹具失败（不计作产品失败）

1. 新构建脚本首次以含空格绝对路径经cmd启动，报x86)未识别、未编译；改为相对call并明确CRLF后完成新构建。原过程记录在本地progress.md，正式build.cmd/日志另存。
2. verify-sdk.ps1首轮在HOME任务变量赋值失败，尚未运行程序。verification-launch-failure.log是原工具输出摘录及修正说明；任务目录已创建后保留，使用taskProfileDirectory重新执行。
3. 单独确认观察器首次无宿主HTML资源，create_host断言失败/退出2，confirmation-resource-failure.log保留原输出。补链接时用错res目录，confirmation-link-failure.log保存LNK1181；正确链接当前generated/host.rc.res，重新编译成功后才运行新EXE。

这些问题均不改变产品或断言，不以后续通过覆盖原失败。confirmation-observer.c的相对include对应原build/language-current-20261009目录，实际构建命令与资源路径见confirmation-build.cmd；此处源码副本用于证据阅读。
