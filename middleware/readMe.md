1.中间件可以编译成静态库lib_mw.a；供ui进程使用。也可以编译成可运行进程，做调试使用，不依赖ui进行编译。

（1）编译成静态库指令。
在根目录下面执行 make DEVICE=ac83xx VARIANT=userdebug lib_mw-rebuild
或者使用以下两条指令
make -C buildroot O=$(pwd)/out lib_mw-dirclean      # 彻底清理 lib_mw 构建目录
make -C buildroot O=$(pwd)/out lib_mw-rebuild       # 重新编译
编译成功后，会生成lib_mw.a静态库
编译信息中有✅[hcn_mw] hcn middleware build success  ->  lib_mw.a

（2）编译成单独进程
在根目录下进入到中间件目录  cd source/vendor/hcn/middleware/
执行：make clean，先清除旧程序
进行编译：make MW_BUILD=exe
在当前目录下面会生成可执行程序：mw_app

编译信息中有✅[hcn_mw] hcn middleware build success  ->  mw_app

在当前文件下面打开cmd或者windows powershell工具
使用以下执行执行mw_app。

# 1. 推送文件到设备
adb push mw_app /tmp/

# 2. 添加可执行权限
adb shell chmod +x /tmp/mw_app

# 3. 运行
adb shell /tmp/mw_app