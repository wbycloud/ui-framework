from pathlib import Path
files=['README.md','docs/application-development-standard.md','docs/standalone-host.md','docs/generic-web-ui.md','docs/workspace-layout.md','docs/framework-menu-offscreen.md','docs/build-and-validation.md','docs/visual-design.md','docs/version-policy.md','docs/application-upgrade-prompt.md','examples/stateful_components/README.md']
for name in files:
 p=Path(name);s=p.read_text(encoding='utf-8')
 # Current entry statements only; feature-introduction sections and historical evidence retain API8.
 for a,b in [('当前API8','当前API9'),('当前 API8','当前 API9'),('当前维护版本API8','当前维护版本API9'),('当前SDK/API8','当前SDK/API9'),('当前SDK0.8/API8/标准8','当前SDK0.9/API9/标准9'),('只维护API8','只维护API9'),('当前版本为 **SDK0.8.0-dev / API8 / 标准8**','当前版本为 **SDK0.9.0-dev / API9 / 标准9**')]:s=s.replace(a,b)
 if name!='docs/version-policy.md':
  # SDK numerical version in entry docs is the currently consumable build.
  s=s.replace('SDK0.8.0','SDK0.9.0').replace('SDK 0.8.0','SDK 0.9.0').replace('SDK0.8/API8','SDK0.9/API9').replace('SDK 为 0.8.0','SDK 为 0.9.0')
 if name in ['docs/application-upgrade-prompt.md','examples/stateful_components/README.md']:
  s=s.replace('API8','API9').replace('标准修订8','标准修订9').replace('版本0.8.0','版本0.9.0').replace('API_VERSION=8','API_VERSION=9')
  s=s.replace('docs/migration-v0.7-to-v0.8.md','docs/migration-v0.8-to-v0.9.md').replace('docs/validation/component-experience-validation.md','docs/validation/instance-language-validation.md')
 if name=='README.md':
  s=s.replace('`0.8.0`','`0.9.0`').replace('当前工作源码为 0.8.0','当前工作源码为 0.9.0').replace('| 框架 API | `8`','| 框架 API | `9`').replace('| 开发标准修订 | `8`','| 开发标准修订 | `9`')
  s=s.replace('UI_FRAMEWORK_API_VERSION=8','UI_FRAMEWORK_API_VERSION=9').replace('CMake版本0.8.0','CMake版本0.9.0').replace('提供的API8 commit','提供的API9 commit').replace('当前本地API8提交','当前本地API9提交')
  s=s.replace('再读[API8验收](docs/validation/component-experience-validation.md)、[0.7 → 0.8迁移](docs/migration-v0.7-to-v0.8.md)','再读[API9验收](docs/validation/instance-language-validation.md)、[0.8 → 0.9迁移](docs/migration-v0.8-to-v0.9.md)')
  s=s.replace('2. [0.7 → 0.8 迁移](docs/migration-v0.7-to-v0.8.md)：当前API9；','2. [0.8 → 0.9迁移](docs/migration-v0.8-to-v0.9.md)：当前API9；[0.7 → 0.8](docs/migration-v0.7-to-v0.8.md)：此前列宽/help；')
 if name=='docs/application-development-standard.md':
  s=s.replace('修订：**8**','修订：**9**').replace('框架 API 8','框架 API 9').replace('当前升级先读[0.7→0.8迁移](migration-v0.7-to-v0.8.md)','当前升级先读[0.8→0.9迁移](migration-v0.8-to-v0.9.md)')
 if name=='docs/generic-web-ui.md':s=s.replace('/ API8 / 标准修订8','/ API9 / 标准修订9')
 if name in ['docs/workspace-layout.md','docs/framework-menu-offscreen.md']:s=s.replace('框架API8、开发标准修订8','框架API9、开发标准修订9').replace('框架API8、标准修订8','框架API9、标准修订9')
 if name=='docs/version-policy.md':s=s.replace('API1–8','API1–9').replace('实际验证API8应用','实际验证API9应用').replace('本轮API8公开能力','此前API8公开能力')
 if name=='docs/build-and-validation.md':
  s=s.replace('当前升级见[0.7→0.8迁移](migration-v0.7-to-v0.8.md)','当前升级见[0.8→0.9迁移](migration-v0.8-to-v0.9.md)').replace('当前API9接口迁移见[0.7→0.8](migration-v0.7-to-v0.8.md)','当前API9接口迁移见[0.8→0.9](migration-v0.8-to-v0.9.md)')
  s=s.replace('本轮结果见[分页清理验收](validation/component-scroll-only-validation.md)','本轮结果见[实例语言验收](validation/instance-language-validation.md)')
 s=s.replace('当前结果见[API8验收](validation/component-experience-validation.md)','当前结果见[API9验收](validation/instance-language-validation.md)')
 p.write_text(s,encoding='utf-8')
# Handoff: only current summary/table and long-term rules; all numbered historical rounds preserved.
p=Path('docs/handoff.md');s=p.read_text(encoding='utf-8');prefix,rest=s.split('## 3. API4历史实现与定位',1)
prefix=prefix.replace('SDK0.8/API8/标准8','SDK0.9/API9/标准9').replace('当前API8','当前API9').replace('只维护当前API8','只维护当前API9').replace('本轮API8交付','本轮API9交付').replace('`0.8.0` 本地开发版 / `8` / `8`','`0.9.0` 本地开发版 / `9` / `9`').replace('API1–8','API1–9')
prefix=prefix.replace('更新日期：2026-10-08。','更新日期：2026-10-08。本轮接手干净cf0e83a，新增应用拥有的实例语言及原位文本接口；当前[API9验收](validation/instance-language-validation.md)及第19节记录本轮身份与结果。以下API8产品/收敛结果属于历史。')
s=prefix+'## 3. API4历史实现与定位'+rest;p.write_text(s,encoding='utf-8')
