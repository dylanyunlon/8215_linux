# DA 树在 NAND 板上起不来：排查记录与工具

板子：AC8215，256M NAND / 256M DDR（原厂 20260921 固件可启动，mtdparts 与旧树 `nand_ext4_256M` 一致，含独立 `boot_misc` 分区）。
DA 树（`/work2/tanyunlong/dylan/AC8215_DA`）用 `-a nand-256-ddr-256` 编的包：开机动画播完约 20 秒后背光熄灭、重启，循环；adb 起不来，串口无输出。

## 结论（截至 2026-11-11）

| 结论 | 依据 |
|---|---|
| 与音乐改动无关 | 纯 DA 基线（`da_pure_baseline.sh`，三处哈希与改动前快照一致）同样重启 |
| 分区、驱动配置链自洽 | preloader 从 `dtb` 分区 0xC000 读 boot_misc（`CONFIG_PRE_DTB_OFFSET` 与 `PRELOADER_DTB_OFFSET=49152` 同源）；csv、内核 `atc_storage_partition.h`、scatter 逐项一致；u-boot 按名找 `system` 得 `root=/dev/mtkd12`；fstab `/data`=`mtkd13`；`MTD_NAND_ATC`/`ATC_ATCLIB`/`EXT4` 均内置；ext4 特性与 AWTK 镜像相同；init、ld-musl、VFP/NEON 都在 |
| preloader、arm2 正常 | 能显示动画（读 boot_misc 失败会在 preloader `while(1)`） |
| **是内核 panic** | bootargs 加 `panic=0` 后不再重启，停在最后一帧（DA 内核 `CONFIG_PANIC_TIMEOUT=10`） |
| **panic 在用户态之前** | inittab 只起 adb + 背光闪烁：不闪、无 adb |
| **panic 在 fb 驱动初始化之前** | fbcon + 在 `osd_init()` 关闭动画平面（`patch_osd.awk`）：画面不变黑、无文字。fb0 在 OSD 第 2 层（`MAINSURFACE_OSD2`），动画在第 1 层 |
| 以下内置驱动不是原因 | 与 AWTK 对齐（`kernel_diag.sh apply`）：COMBO 关、VOUT/VCP/MALI/UMP/BACKLIGHT 改模块、FB1/SCO 关，结果不变（`diag_align`，内核 md5 `ee77371a…`） |

当前头号嫌疑：**NAND 控制器 / FTL（`mtkd`，`prebuild/atc_ftl/atc_lib.a`）这条只有 NAND 板才真正执行的路径**。同事的 NOR 板用同一棵树、同一份内核配置与 DDR256 保留内存布局可以启动。
旁证：dts 里 NAND 节点 `nandflash@f001e400` 的第二段 reg 在 DA 为 `0xf001ec00 0x148`，AWTK（NAND 板可启动）为 `0x200`。

下一步：`diag_nonand`——禁用 `ac83xx-nandflash` 节点 + `panic=10`。不再重启 ⇒ panic 在 NAND/FTL 路径（或挂根之后）；仍约 20 秒重启 ⇒ 与 NAND 无关，再用 `--disable` 分批排除其他外设（注意 metazone 内置，NAND 禁用后若它自身出错也会导致重启，此结果需复核）。

## 踩过的坑

- `allmake.sh` 编译失败后仍会继续打包，`release_images` 里可能是**上一次**的 `Image.bin`。做包前先 `grep 'Build error'` 并核对 `Image.bin` 时间。
- 诊断目录名复用会烧到旧包：`mk_diag_pkg.sh` 每次先删除输出目录再复制，并打印 MD5 供 Windows 端 `certutil` 核对。
- debugfs 1.45.5 的 `write` 不认带目录的目标路径，要先 `cd /etc`；`debugfs -R 'ls …'` 输出到终端会进 less，粘贴的后续命令会被吞，加 `| cat`。
- `FB0_ATC`、`ATC_METAZONE` 不能改成模块：`bootanidrv`（obj-y）直接调用 fb/osd/pmx/lcd 函数，fb 又调用 metazone。
- `ATC_COMBO` 由 `build/setup` 在树里有 WiFi 源码时导出 `ATC_WIFI_CHIP=AIC8800` 后 select 进来；改 setup 关掉它会让 `lvgl_home` 依赖不存在的 `androidauto` 而整编失败。
- 改了内核 defconfig 后必须 `rm -rf out/build/ac83xx/linux-local`，否则 buildroot 不会重读。

## 工具

| 文件 | 作用 |
|---|---|
| `da_pure_baseline.sh` | 备份音乐改动（`~/dylan/music_state_backup_<时间>.tgz`）后把 DA 树还原为纯基线，用全新 `out/` 整编 nand-256 |
| `kernel_diag.sh apply\|restore\|status` | 打上 / 还原内核诊断改动（原件备份在 `~/dylan/ac83xx_kernel_defconfig.orig`、`~/dylan/displayinit.c.orig`） |
| `mk_diag_pkg.sh SRC OUT [--bootargs ..] [--disable PAT].. [--inittab keep\|adb\|udev-adb-fb]` | 从 release_images 生成诊断包并自检 |
| `disable_node.awk` | 按 compatible 把 dts 节点置为 disabled |
| `patch_osd.awk` | 在 `osd_init()` 关闭开机动画平面，让 fbcon 可见 |

例：生成 `diag_nonand`

```bash
T=/work2/tanyunlong/dylan/AC8215_DA
bash da_boot_diag/mk_diag_pkg.sh $T/release_images ~/dylan/diag_nonand \
     --bootargs "panic=10" --disable ac83xx-nandflash --inittab adb
```

## 收尾还原（找到原因后）

```bash
T=/work2/tanyunlong/dylan/AC8215_DA bash da_boot_diag/kernel_diag.sh restore
rm -rf $T/out/build/ac83xx/linux-local
# 音乐改动：解开 ~/dylan/music_state_backup_1110_2140.tgz（tar xzf … -C /），或重跑 hcn_music_lvgl/tools 下的两个接入脚本
```
