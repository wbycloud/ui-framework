# P4-A实施及验收计划

2026-10-26。本轮仅P3评审收口及P4-A，按[P4A-1](xuanshu-p4-controlled-edit-contract.md)实施。P4-B/C不授权。无署名。

## 1. 基线与顺序

实际HEAD e0eca03db25930be55bdb45477027d1793a61904，codex/p1-process-boundaries；独立证据build/xuanshu-p4a-20261026-a61d4e，17427入口文件。原P3 delivery32/source308/binary101入口身份一致，不能将HEAD全差异归本轮。参考工程此前读取被拒，本轮不扩大权限。

1. 冻结合同及独立checkpoint。
2. P3 revision重入受控回归；最小私有方法租约；相称P3回归通过后接编辑。
3. 同目录编辑增量/阶段接口/资源门与专用内存夹具。
4. Service授权、意图确认、事务、回读、结果及UNKNOWN。
5. 隔离备份迁移、重启和故障窗口。
6. Native/Light/ASAN相称回归、自审及范围内整改；保全和清理。

## 2. 文件与接口范围

独立capability_edit.h/ai_edit.h，capability.c私有增量及Service/Store私有增量；必要测试、真实DLL夹具和CMake注册。旧头测试只增加新独立头与前缀检查。编辑描述一次注册同一目录，事务适配不增加业务execute。新编辑接口不接受调用方自报人工授权；正式聊天面板、stateful/EDA及五应用不修改。

所有新API在UI线程；稳定ID在执行前建立。专用控制器持模块/资源/Service租约，后台只复制数据；异步阶段使用call/stage，不存应用裸指针。SQLite仍固定3.54.1，其他依赖不改。资源门与C2共用，busy/UNKNOWN拒绝后续修改，不自动排队。

## 3. 批次验收

P3以新独立回归验证外层continue和revision在栈时destroy PENDING、退栈后销毁、正常继续，并回归只读工具及两种生命周期分支。修复前仅断言失败证据，不预称崩溃复现。

P4-A用实际内存值、opaque revision、事务/撤销回执和数据库事件断言；保留失败时源码及匹配二进制。覆盖P4A-1第6节全部条件，新增路径与受影响公共路径补测，复用仍匹配证据。门槛和超时不放宽。迁移只在临时库/副本，真实输入未验不冒充通过。

P4-B依赖A通过后才接stateful事务；P4-C才接EDA及面板。它们仅为后续依赖，不自动实施。当前终点为P3整改和P4-A联合代码评审。


## 4. 本轮实施收尾与后续停止条件

P3先完成red核实和私有methods租约后才接P4；同一目录阶段接口、后台意图/消费、原生宿主动作及真实DLL夹具已经实现。三个配置使用本轮独立目录；最终结果、逐项用例与首次失败以[独立验收](validation/xuanshu-p4a-validation.md)及其清单为准，不把阶段绿色覆盖首次失败。

实际增量范围为CMake、capability.c/v2私有扩展、ai_service/ai_internal/ai_store、两个独立公共头、四个内部编辑文件、continue及编辑夹具测试、公共头测试和用户授权10份文档加独立验收。未改应用业务实现、P3面板、固定依赖、版本、旧公开布局。专用Light测试DLL运行库匹配原shared目标；不改正式依赖构建。

首批按一次具体调用形成任务范围，结束即失效，不引入多步骤AI编辑任务调度。后续P4-B需另获实现授权并提供真实应用begin/commit/rollback/undo及可跨重开核对的回执；P4-C需正式宿主结构化展示、用户确认、审批失效/错误投影与人工集中验收。前置P4-A通过不自动授权它们。任何来源放宽、业务意图未确认先执行、旧epoch重放、UNKNOWN误清或不安全卸载必须停止；可选视觉/物理验收延期单列。


## 5. 剩余P4授权与实施checkpoint（2026-10-26）

用户附件明确授权剩余P4，原仅A范围保留为历史。本轮基线17442项，P4-A 30/318/131身份匹配，新证据build/xuanshu-p4-final-20261026-c84f12。复审新增continue回调后编辑互斥缺口，red129/2，最小重检后核心30/30；未宣称崩溃。先冻结P4BC-1第10节，再实现应用与面板，保持匹配快照SDK/API/ABI/包、原布局/错误/限额和依赖。

应用文件范围：examples/p2_support.h只转发同一run及应用自有快照；stateful/minimal_eda/generic（features同源分支）/web_counter具体事务适配；原C2收参和草稿合同保留。框架增量为独立注册扩展头及Light绑定头、私有capability/Service/workspace语义接入，正式host主窗/ai_panel/host.html；测试专用真实包和同源隔离接纳，无第二AI执行器或公开自报人工参数。

已授权本轮顺序及门槛：复审收口→stateful真实事务/读回/草稿→EDA固定几何/笔记/清空审批→其余分支全部内存write→同Service正式面板→P3/P4A/C1/A+B/公共头/组件/真DLL相称回归及独立自审→源码与二进制身份/清理。失败自行定位修正、原始证据保留。P5、真实模型、网络/外发执行器、跨实例共享写入、通用事务/undo不做；冻结范围冲突才暂停相关工作，其余继续。

## 6. 剩余P4收尾及停止位置

上述顺序已执行，实际应用接入和正式面板见覆盖矩阵第7节及[P4最终独立验收](validation/xuanshu-p4-validation.md)。本轮先独立核实red129/2并收口，再登记新风险合同、应用事务、Light面板，最后联合回归/自审。后续不得把当前P4范围扩展为P5。

遗留ASAN-Light共享表单脚本50ms中断诊断仍保留；非AI人工对照亦出现，未定位确切热段/根因，不改时限。真实来源/IME/DPI/视觉/辅助技术/长时延期，首轮失败与历史frontend63等独立保留。P4开发阶段结论不等于真实或无条件完整验收。后续真实Provider、Coordinator/子Agent、跨实例共享写入及P5需重新明确授权。
