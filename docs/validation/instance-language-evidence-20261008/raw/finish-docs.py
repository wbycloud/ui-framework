from pathlib import Path
r=Path.cwd()
changes={
 'docs/application-development-standard.md': [('新接口见 [components.h]','新语言接口见 [language.h](../include/ui_framework/language.h)，组件接口见 [components.h]'),('[当前API9验收](validation/component-experience-validation.md)','[当前API9验收](validation/instance-language-validation.md)'),('编译为API8功能样例','编译为API9功能样例'),('本轮列宽/help公开能力升级API8','此前列宽/help公开能力升级API8；本轮实例语言与文本更新升级API9')],
 'docs/generic-web-ui.md':[('当前见[API8验收](validation/component-experience-validation.md)','此前API8见[验收](validation/component-experience-validation.md)，当前见[API9验收](validation/instance-language-validation.md)')],
 'README.md':[('[当前收敛证据](docs/validation/api8-stability-state-validation.md)','[此前API8收敛证据](docs/validation/api8-stability-state-validation.md)')],
 'docs/application-upgrade-prompt.md':[('本轮新增列宽/help能力统一升级API9','此前列宽/help升级API8，本轮新增实例语言及文本更新统一升级API9')],
 'docs/build-and-validation.md':[('API9/ABI1/包格式1，见[迁移](migration-v0.7-to-v0.8.md)及[验收](validation/component-experience-validation.md)','API9/ABI1/包格式1，见[迁移](migration-v0.8-to-v0.9.md)及[验收](validation/instance-language-validation.md)')],
 'docs/handoff.md':[('只维护和验收本轮确认的API8','只维护和验收本轮确认的API9'),('及相关测试。无需重做已完成的实现。','及[0.8→0.9迁移](migration-v0.8-to-v0.9.md)、[实例语言](instance-language.md)和相关测试。无需重做已完成的实现。')]
}
for name,pairs in changes.items():
 p=r/name;s=p.read_text(encoding='utf-8-sig')
 for a,b in pairs:
  assert a in s,(name,a);s=s.replace(a,b,1)
 p.write_text(s,encoding='utf-8')
p=r/'docs/validation/instance-language-validation.md';s=p.read_text(encoding='utf-8')
s=s.replace('第三提交的最终结果见 [full-closure.log](instance-language-evidence-20261008/raw/full-closure.log) 与JUnit；前一提交完整矩阵61通过/0失败/1物理跳过，32轮真实呈现就绪重开也通过，不与最终结果混用','最终产品6d88770：62项中61通过/0失败/1物理跳过，398.63秒；[日志](instance-language-evidence-20261008/raw/full-closure.log)及[JUnit](instance-language-evidence-20261008/raw/full-closure.xml)绑定本轮产物')
s=s.replace('21通过/0失败/1物理条件跳过；当前复验日志单独绑定最终提交，不把有Web构建替代无Web配置','最终产品6d88770重新构建并运行22项：21通过/0失败/1物理跳过，6.52秒；[日志](instance-language-evidence-20261008/raw/native-closure.log)，不把有Web构建替代无Web配置')
s=s.replace('前一提交两个独立Runtime进程各32个呈现就绪周期，末端pending0；句柄394→396、GDI12→12、USER17→17及366→369、GDI12→12、USER15→15。最终提交另外运行独立周期；保留原资源断言','最终产品两个独立Runtime进程各32个真实呈现就绪周期，末端pending0；句柄390→391及391→392，两次GDI12→12、USER17→17。日志runtime-closure32-1/2保留原资源断言；前一提交结果另存，不混用')
p.write_text(s,encoding='utf-8')
p=r/'docs/handoff.md'
with p.open('a',encoding='utf-8') as f:f.write('''
## 19. 2026-10-08 API9 应用拥有的实例语言

接手干净cf0e83a。产品ee6a2a8新增每host简中/英文、系统显示语言来源、同步通知和稳定ID原位文本更新；8db942f补充真实校验叶节点验证；6d88770允许关闭/unmount期间清除回调，继续拒绝新增回调。SDK0.9.0-dev/API9/标准9，ABI1/包格式1不变；新增公共能力不再称API8未变化。应用拥有偏好与存储，宿主跟随活动实例，后台隔离，空宿主查询Windows显示语言。没有全局语言开关或进程locale替代。完整stateful_components应用以A/B稳定档案保存UIL1，不更改UST1/UCW1/ULYT；首次呈现前提交，非法文件保持，错误回退由示例应用明确决定。

最终产品6d88770完整矩阵61通过/0失败/1物理跳过（62项，398.63秒）；重新构建的无Web原生21通过/0失败/1物理跳过（22项，6.52秒）。两个独立实际Runtime各32就绪重开，句柄390→391及391→392，GDI12、USER17、末端pending0。Light与实际Runtime的完整DLL语言套件含10种偏好文件及96/144/192跨进程写入/恢复；真实鼠标、草稿/选择、稳定宽度、HWND/GL、双实例/后台、模态、AI空间回收和关闭卸载通过。保留原生三项真实滚动、完整数据范围/按需查询、旧捕获与轨道修复、原预算和断言。

真实系统首选zh-CN；其他语言映射只是可控输入，未修改机器显示语言。锁屏输入失败、Runtime早期导航/异步清理超时、剪贴板失败、语言初版Light解析/翻译函数遮蔽及关闭回调拒绝均有独立原始日志；后续通过不抹去历史失败或宣称Runtime/持续重绘根治。详见[API9验收](validation/instance-language-validation.md)、[合同](instance-language.md)、[迁移](migration-v0.8-to-v0.9.md)及完整示例；真实前后/双实例/浅深/窄窗/DPI图有运行身份和哈希。当前只维护API9，不重启旧SDK专项。

本轮仅框架与框架自有示例，未访问业务仓库。服务创建访问错误5，当前Session0待权限；严格无登录服务runner仍无，托管CI未授权，真实IME/不同DPI物理屏幕/桌面边缘合成/长期人工待条件。本机与历史不能替代；无远程操作、发布、稳定标签、新会话、子代理或新硬件GL范围。匹配SDK及解包实际调用的外部身份将在归档后补充，避免归档哈希自引用；先保留本地可审查提交。
''')
print('Current entry documents and final product evidence updated; historical sections preserved')
