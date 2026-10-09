# P1 默认多线程托管 A/C 对照证据

2026-10-09。对应 [复验报告](../p1-mesa-hosted-control-validation-20261009.md) 和 [Run37917825238](https://github.com/wbycloud/ui-framework/actions/runs/37917825238)，实际测试head `5715a0539b94158e3f18afc32bffbc9a5d6a0168`。

`raw/artifacts/{osmesa,webview2}/` 保存本次目标原始文本、PID／退出事件、CASE_END、JUnit、活对象和逐用例manifest。`raw/analysis/` 是独立后处理与匹配PDB／实际PE解码。`raw/raw/` 保存授权基线、推送／单次dispatch、GitHub元数据、ZIP及提取身份、保护核验。`raw/transfers/` 保留本次下载成功／失败分段元数据，签名URL及凭据未公开。`raw/tools/` 保存本次辅助脚本快照。

`raw/prior-local/` 是上一轮封存本地结果的原字节副本，仅作历史依据；其中report内部链接仍指原保存位置。本次A默认三个超时和C全部相关用例自然退出的记录不能与历史结果混算。

`frames/` 有6张完整真实窗口PNG，来源见 [frame-origins.json](frame-origins.json)，逐像素等同新目标BMP；原135份BMP、13份DMP、2个完整ZIP及匹配PE/PDB在本机新build，不由clone携带。184份重产物路径／SHA见 [local-artifacts.json](local-artifacts.json)，全量776份目标提取清单见 `raw/raw/artifact-{job}-files.json`。GitHub artifact保留至2026-11-08；材料缺失时应报告缺失。

[sha256.json](sha256.json) 封存本目录每个文件，索引自身除外。原A失败、初次传输失败和分析汇总误判保留，原历史目录及SDK没有重收集或补写。未关闭默认worker、提高180／240秒超时、改变25分钟job预算或删除原功能／资源断言。
