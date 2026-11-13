# DA 树 NAND-256 板：遗留问题与待办（2026-11-13）

根因与过程见 [`DIAG_LOG_2026-11-12.md`](DIAG_LOG_2026-11-12.md)。下面是还没做完的事。

## 1. 闪屏（DA_G12_full）——推断，未验证
rootfs 里 `S01hcn_application` 和 `S01lvgl_home` 同时存在，都在 `start()` 里把各自的全屏程序放后台（`&`）并 `sleep 1`；`/usr/bin/hcn_application`、`/usr/bin/lvgl_home` 两个二进制都在。两个 LVGL 界面同时画屏，与 `BUILD_ENV_NOTES.md`「已踩过的坑」第 2 条一致。
验证：去掉 `S01lvgl_home`（或 defconfig 里关 `BR2_PACKAGE_LVGL_HOME`），看闪屏是否消失。**还没做，不能当结论。**

## 2. 完整系统下 adb 进不去——原因未知
- `inittab` 里 `::sysinit:/etc/init.d/rcS` 是同步执行；两个 `S01*` 都是后台启动、不会卡住 `rcS`；`S30adb` 只是后台跑 `/etc/android-gadget-setup`。
- 只起 adb 的 inittab 下 adb 是通的（G12），所以 USB 和 adb 本身没问题，问题在完整启动流程里。
- 已打包 `DA_G12_adbfirst`（`mk_adbfirst.sh`：adb 在 `rcS` 之前最先起，`rcS` 跳过后面的 `S30adb`），**待刷机结果**。能进就抓：`ps`、`dmesg`、`/proc/bootprof`、`ls -l /dev/fb*`、`/proc/mounts`、`lsmod`，隔 10 秒再抓一次 `ps` 看进程号有没有在变。
- 仍不通：做对照包，去掉 `S00early_modutils` 和 `S70modutils`，看是不是模块加载搞坏了 USB。

## 3. 正式修复（等 1、2 有结论再做）
```bash
T=/work2/tanyunlong/dylan/AC8215_DA; N=/work2/tanyunlong/dylan/OLD_nand256; B=~/dylan; D=$T/source/kernel/kernel-3.18
# 先确认诊断改动都已还原（应全部显示"原版"）
MZ=$(find $D/drivers/misc/atc/metazone -name metazone_drv.c | grep ac83xx)
cmp $B/bootanidrv.c.orig $D/drivers/misc/atc/bootanimation/bootanidrv.c
cmp $B/ac83xx_kernel_defconfig.orig $D/arch/arm/configs/ac83xx_defconfig
cmp $B/displayinit.c.orig $D/drivers/video/fbdev/atc/ac83xx/displayinit.c
cmp $B/metazone_drv.c.orig $MZ
cmp $B/aic8800_Kconfig.orig $D/drivers/net/wireless/aic/aic8800/Kconfig
# 备份 DA 版库并替换，走正式流程整编
cp -n $T/prebuild/atc_ftl/atc_lib.a $B/atc_lib.a.DA_orig
cp $N/prebuild/atc_ftl/atc_lib.a $T/prebuild/atc_ftl/atc_lib.a
rm -rf $T/out/build/ac83xx/linux-local
cd $T && ./allmake.sh -a nand-256-ddr-256 -d userdebug -m false
```
整编后 `grep -c 'Build error'` 必须为 0，并核对 `Image.bin` 时间戳。

## 4. 要向同事确认
> DA 树 `prebuild/atc_ftl/atc_lib.a`（MD5 `129f1bfe…`，无版本串，9/16）用在 NAND 256MB 的板子上，内核启动约 18 秒后会重启（与 NAND 设备有无无关；挡掉 `mt33xx_mtklib_init` 后重启消失）。换成旧树里的版本（MD5 `21c5372b…`，`[MTKLib]Version 2026020331`，2026-06-10 提交）后，其余完全不变，内核稳定、adb 正常。想确认：DA 这份库是给哪种板子、哪个版本做的？NAND 板上应该用哪一份？
> 另外 `bootanidrv.c` 的 `rsv_free_thread` 里 `wait_event_interruptible(rsv_free_queue, 1)` 条件恒真，内核启动即释放开机动画保留内存（本次实测不是重启原因，但应该是 `g_arm2_vba_exit`）。

## 5. 其他
- DA 树 `prebuild/` 里还有 `multimedia`（14 个文件）、`opengles`、`atcsurface`、`libsettings_atc`、`imgresz`、`audio` 与旧树不同，用户态库，**未验证，不要盲换**。
- `H2`/`H3`（旧 `atcftlcore.c` 配旧/DA 库）已编好未刷，只用于区分"库"和"`atcftlcore.c`"各自的责任，不影响修法。
- 126 上的备份与临时物（`~/dylan/` 即 `/work2/tanyunlong/dylan/`）：`atc_lib.a.DA_orig`、`bootanidrv.c.orig`、`ac83xx_kernel_defconfig.orig`、`displayinit.c.orig`、`metazone_drv.c.orig`、`aic8800_Kconfig.orig`；诊断包 `diag_*`、`DA_G12_full`、`DA_G12_adbfirst`、`release_images_DA_bootanifix_1112`。收尾时清理：`rm -rf $T/out/build/xt_* ~/dylan/root_G*`。
- 确认 DA 树里的 `bootanidrv.c` 已还原（曾被注释过一行，见第 3 节的 `cmp`）。
