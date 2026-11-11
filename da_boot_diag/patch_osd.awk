# 用法: awk -f patch_osd.awk displayinit.c.orig > displayinit.c
# 诊断用: 在 osd_init() 打开 fb0 所在 OSD 平面后关闭开机动画平面(OSD_PLANE_1), 让 fbcon 文字可见; 插入次数不是 1 则报 PATCH_FAILED。
/^static bool osd_init\(/ { inf=1 }
{ print }
inf && /^\ti4OsdPlaneEnble\(plane, TRUE\);/ { print "\ti4OsdPlaneEnble(OSD_PLANE_1, FALSE); /* DIAG: hide boot-animation plane so fbcon on fb0 is visible */"; inf=0; n++ }
END { if (n!=1) { print "PATCH_FAILED" > "/dev/stderr"; exit 1 } }
