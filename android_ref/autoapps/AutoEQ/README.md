## Build Variants 如何选择？

- app、app-language、app-overlay 
  - ac8227、ac8257、mt8163、mt8321、uis8581平台一般都用 mt8163 平台的签名
  - 其它平台可以自行选择是否使用对应平台的签名
  - 选择发布 Release 版本
- UI 模块和 for_build 模块
  - 选择发布 debug 即可

## 子仓库说明
app-language、app-overlay 现在为子仓库模式

更新使用指令：
```shell
git submodule update --init --recursive
```
提交子仓库代码，需要在子仓库目录下提交，再到主仓库目录提交，切记！！！






新增皮肤包支持形态：
hcn_library
gb04
Eq.Utils 配置 getSupportNewSkin()

hcn_libray： 为最小必要控制单位，包含app apk的可复用逻辑和自定义view
注意 hcn_libray 和 app目录下的同名文件需同步更新，由维护新皮肤包的同事负责。

gb04皮肤包： 仅包含layout相关的view， 支持mcc mnc换肤
