# 查证记录

## 当前确认

- HEAD 与参考一致，tracked clean；未跟踪历史材料保留。
- host.html 的 window-button/tab-close/host-tool 存在 hover 与 focus 混用样式，tab focus 使用 border，可能影响几何；尚需实际状态和背景矩形证据，未据此修改。
- Light text_align 默认0，绘制默认DT_LEFT；已有text-align解析。focus统一DrawFocusRect且鼠标按下设focused，当前未看到focus-visible专用状态。只是代码线索，须两后端真实输入核对。
- 浮动手势、多行、提示生命周期、公共布局和End定位已有修复；沿用历史责任边界，不重复列为未实现。
- 旧P1目标默认A三次同栈退出停滞，候选C66进程OS0；本轮不切换候选或重做该专项。

## 待确认

按钮图标实际绘制范围与背景／clip／hit／DPI；鼠标与键盘焦点模态；测量终点、旧探针覆盖；实际CPU／长帧／重复布局责任；独立条件缺口。

## 首次真实基线

before-light-96：50份原桌面BMP；plus图标实际靠左，hover外框无位移，鼠标释放仍DrawFocusRect。F1/鼠标宿主提示走自身300×160 popup，而非已经修复的公共 tooltip；画面挡住表格。这是现存宿主路径，暂不据此扩大修改。

独立探针实际OS exit1，失败为Space后助手未收起；Enter/鼠标检查当时通过。当前尚不归因产品，需KEY消息及焦点／实际状态时序。主返回与自然OS退出分别记录。原BMP、源码和二进制将留存。

KEYTRACE已证实Space正常到达；assistant_expanded=-1是明确收起，assistant_visible()以>0判断。修正探针真值错误，不改产品的三态。Runtime镜像宿主实验presentation持续pending，120秒后诊断终止，未算Runtime产品通过或Mesa失败；产品宿主实际只用Light，后续Runtime通过真实组件及独立公共C样式调用方验证。
