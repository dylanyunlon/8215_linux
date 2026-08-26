#!/bin/sh
#
# S00setparam - 投递默认设置参数 JSON 到 /data/set_param/(投递点)
#
# 作用: OTA 升级会更新 rootfs, 本脚本把 rootfs 中自带的默认配置 JSON
#       投递到持久分区上的投递点 /data/set_param/, 供 hcn 中间件迁移逻辑读取.
#       /data/set_param/ 仅作投递点(SRC); 后续迁移逻辑(改用 /config 作 DEST)
#       由 hcn 中间件代码负责, 本脚本只管"按版本投递".
#
# 顺序: /data 由 fstab(/dev/mtkd14)在 rcS "mount -a" 阶段挂载, 早于本脚本;
#       本脚本序号 S00, 排在 S01awtk(demo 启动)之前, 确保投递点先就绪.
#
# 投递规则(按版本, 避免重复投递):
#   - 投递点无 setparam_*.json  -> 拷贝 rootfs 默认
#   - 投递点已有同名(同版本)文件 -> 跳过(已投递)
#   - 投递点有不同版本文件      -> 删旧, 拷贝新版
#
# 注意: 现有 read_src_config() 在 SRC/DEST 同版本时会 rm 掉投递点文件,
#       可能导致下个启动重复投递; 待迁移逻辑改造(/config 作 DEST 并按 DEST
#       版本判重)后即消除. 本脚本行为对投递点本身无害.

ROOTFS_DEFAULT=/usr/share/hcn/set_param/setparam_8215E_v001.json
DROP_DIR=/data/set_param

# 确保 /data 已挂载(fstab 早期挂载; 防御性检查)
ensure_data_mounted() {
	if grep -q ' /data ' /proc/mounts 2>/dev/null; then
		return 0
	fi
	mount /data 2>/dev/null
	if grep -q ' /data ' /proc/mounts 2>/dev/null; then
		return 0
	fi
	return 1
}

start() {
	if ! ensure_data_mounted; then
		echo "[set_param] /data not mounted, skip provisioning"
		return 0
	fi

	if [ ! -f "$ROOTFS_DEFAULT" ]; then
		echo "[set_param] rootfs default missing: $ROOTFS_DEFAULT"
		return 0
	fi

	mkdir -p "$DROP_DIR"

	rootfs_name=$(basename "$ROOTFS_DEFAULT")

	# 投递点现有 setparam_*.json(只取第一个, 规范上投递点只应有一个)
	existing=$(ls "$DROP_DIR"/setparam_*.json 2>/dev/null | head -n 1)

	if [ -n "$existing" ]; then
		existing_name=$(basename "$existing")
		if [ "$existing_name" = "$rootfs_name" ]; then
			# 同版本已投递, 跳过
			return 0
		fi
		# 版本不同, 清掉旧版投递文件
		echo "[set_param] replace $existing_name -> $rootfs_name"
		rm -f "$DROP_DIR"/setparam_*.json
	fi

	if cp "$ROOTFS_DEFAULT" "$DROP_DIR"/; then
		echo "[set_param] delivered $rootfs_name -> $DROP_DIR"
	else
		echo "[set_param] deliver failed"
	fi
}

stop() {
	:
}

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		stop
		start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 1
		;;
esac

exit 0
