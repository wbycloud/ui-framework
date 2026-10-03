# 冻结 SDK2 公共头文件

头文件复制自实施基准 `3375459ca2d85d0627e3072e3f1e83fc4fa84aae`，不会随当前 include/ 更新。CMake用它构建API2 Web Counter，确保调用方实际使用旧结构布局、枚举和ops尺寸。

这提供可重复的旧调用方编译/加载测试，不能代替原二进制包的兼容证据；本轮另外加载升级前保存的API1/2原包，结果见[验收记录](../../docs/validation/api3-validation.md)。不要将本目录作为新应用 SDK，API3应用使用根 include/。
