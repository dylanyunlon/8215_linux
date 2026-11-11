# 用法: awk -f patch_mtz.awk metazone_drv.c.orig > metazone_drv.c
# 诊断用: 在 metazone_probe() 里跳过 MTZ_Init(), 让内置的 metazone 驱动在内核启动阶段完全不访问 NAND 上的 metazone 分区; 替换次数不是 1 则报 PATCH_FAILED。
/^static int metazone_probe\(/ { inf=1 }
inf && /^[[:space:]]*MTZ_Init\(\);[[:space:]]*$/ { print "\t/* DIAG: MTZ_Init() skipped - metazone NAND init/scan disabled for boot test */"; inf=0; n++; next }
inf && /^}/ { inf=0 }
{ print }
END { if (n!=1) { print "PATCH_FAILED" > "/dev/stderr"; exit 1 } }
