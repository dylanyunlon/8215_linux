#!/bin/sh
# =====================================================================
#  hcn_app_switch.sh — HCN 构建开关一条命令切换（mode / bt / sdk / carplay / ui）
#
#  修改 hcn_app_config.cmake（HCN_APP_CFG_* 唯一真相源）后自动执行
#  dirclean + rebuild（CMakeCache 缓存旧值，必须强制重配才能生效）。
#
#  用法（在工程根目录执行，或任意目录脚本内自行定位）：
#    ./hcn_app_switch.sh mode full
#    ./hcn_app_switch.sh mode demo
#    ./hcn_app_switch.sh bt on mode full
#    ./source/vendor/hcn/application/hcn_app_switch.sh bt on
#    ./hcn_app_switch.sh bt on sdk off          # 蓝牙开、桩后端
#    ./hcn_app_switch.sh bt on sdk on           # 蓝牙开、厂商 SDK 后端
#    ./hcn_app_switch.sh carplay on             # 仅开 CarPlay
#    ./hcn_app_switch.sh ui auto                # 模板资源存在时用模板，否则回退
#    ./hcn_app_switch.sh ui template            # 优先模板 UI，缺资源时回退
#    ./hcn_app_switch.sh ui fallback            # 只编当前 UI
#    ./hcn_app_switch.sh bt off carplay off     # 全关，回到最小 UI
#    ./hcn_app_switch.sh carplay on --edit-only # 只改配置不编译
#    ./hcn_app_switch.sh                        # 无参数 = 打印当前配置
#
#  demo/full 模式也走本脚本，配置源为 hcn_app_config.cmake 的
#  HCN_APP_CFG_MODE；脚本会自动 dirclean+rebuild。
#
#  板级参数默认 nor-64-ddr-256 / userdebug，可用环境变量覆盖：
#    HCN_MAKE_ARGS="-a nor-64-ddr-512 -d user -m false" ./hcn_app_switch.sh bt on
# =====================================================================
set -e

SELF_DIR=$(cd "$(dirname "$0")" && pwd)
CFG_FILE="$SELF_DIR/hcn_app_config.cmake"

WANT_BT=""
WANT_SDK=""
WANT_CARPLAY=""
WANT_MODE=""
WANT_UI=""
EDIT_ONLY=0

usage() {
    grep -E '^#' "$0" | sed -n '2,32p' | sed 's/^# \{0,2\}//'
}

PENDING=""
for arg in "$@"; do
    case "$arg" in
        --edit-only) EDIT_ONLY=1 ;;
        -h|--help) usage; exit 0 ;;
        on|ON) case "$PENDING" in bt|sdk|carplay) eval "WANT_$(echo "$PENDING" | tr a-z A-Z)=on"; PENDING="" ;;
               *) echo "错误：'on' 前缺少 bt/sdk/carplay"; exit 1 ;; esac ;;
        off|OFF) case "$PENDING" in bt|sdk|carplay) eval "WANT_$(echo "$PENDING" | tr a-z A-Z)=off"; PENDING="" ;;
                 *) echo "错误：'off' 前缺少 bt/sdk/carplay"; exit 1 ;; esac ;;
        bt|sdk|carplay|mode|ui) PENDING=$arg ;;
        full|demo) case "$PENDING" in mode) WANT_MODE=$arg; PENDING="" ;;
                   *) echo "错误：'$arg' 前缺少 mode"; exit 1 ;; esac ;;
        auto|template|fallback) case "$PENDING" in ui) WANT_UI=$arg; PENDING="" ;;
                   *) echo "错误：'$arg' 前缺少 ui"; exit 1 ;; esac ;;
        bt=on)   WANT_BT=on ;;      bt=off)   WANT_BT=off ;;
        sdk=on)  WANT_SDK=on ;;     sdk=off)  WANT_SDK=off ;;
        carplay=on)  WANT_CARPLAY=on ;;  carplay=off)  WANT_CARPLAY=off ;;
        mode=full) WANT_MODE=full ;; mode=demo) WANT_MODE=demo ;;
        ui=auto|ui=template|ui=fallback) WANT_UI=${arg#ui=} ;;
        no-bt)   WANT_BT=off ;;     no-sdk)   WANT_SDK=off ;;
        no-carplay) WANT_CARPLAY=off ;;
        *) echo "错误：未知参数 '$arg'（-h 看用法）"; exit 1 ;;
    esac
done
if [ -n "$PENDING" ]; then
    if [ "$PENDING" = "mode" ]; then
        echo "错误：'mode' 后缺少 full/demo"
    elif [ "$PENDING" = "ui" ]; then
        echo "错误：'ui' 后缺少 auto/template/fallback"
    else
        echo "错误：'$PENDING' 后缺少 on/off"
    fi
    exit 1
fi

if [ -z "$WANT_BT" ] && [ -z "$WANT_SDK" ] && [ -z "$WANT_CARPLAY" ] && [ -z "$WANT_MODE" ] && [ -z "$WANT_UI" ]; then
    echo "当前配置（$CFG_FILE）："
    grep -E '^set\(HCN_APP_CFG' "$CFG_FILE"
    exit 0
fi

# sdk 依赖 bt：bt 显式关时 sdk 开关无效果
if [ "$WANT_BT" = "off" ] && [ "$WANT_SDK" = "on" ]; then
    echo "警告：bt=off 时适配层不编译，sdk=on 无效果，已忽略"
    WANT_SDK=""
fi

# ---- sed 改配置并校验 ----
apply() {  # $1=sym  $2=on|off
    sym="HCN_APP_CFG_$1"
    val=$(echo "$2" | tr 'a-z' 'A-Z')
    if ! grep -qE "^set\($sym[[:space:]]+(ON|OFF)\)" "$CFG_FILE"; then
        echo "错误：$CFG_FILE 中找不到 set($sym ON|OFF)，文件格式可能已变更"
        exit 1
    fi
    sed -i -E "s/^set\($sym[[:space:]]+(ON|OFF)\)/set($sym $val)/" "$CFG_FILE"
    echo "  $sym -> $val"
}

apply_mode() {  # $1=demo|full
    if ! grep -qE '^set\(HCN_APP_CFG_MODE[[:space:]]+"?(demo|full)"?[[:space:]]*\)' "$CFG_FILE"; then
        echo "错误：$CFG_FILE 中找不到 set(HCN_APP_CFG_MODE demo|full)，文件格式可能已变更"
        exit 1
    fi
    sed -i -E "s/^set\(HCN_APP_CFG_MODE[[:space:]]+\"?(demo|full)\"?[[:space:]]*\)/set(HCN_APP_CFG_MODE $1)/" "$CFG_FILE"
    echo "  HCN_APP_CFG_MODE -> $1"
}

apply_ui_variant() {  # $1=auto|template|fallback
    if ! grep -qE '^set\(HCN_APP_CFG_UI_VARIANT[[:space:]]+(auto|template|fallback)[[:space:]]*\)' "$CFG_FILE"; then
        echo "错误：$CFG_FILE 中找不到 set(HCN_APP_CFG_UI_VARIANT auto|template|fallback)，文件格式可能已变更"
        exit 1
    fi
    sed -i -E "s/^set\(HCN_APP_CFG_UI_VARIANT[[:space:]]+(auto|template|fallback)[[:space:]]*\)/set(HCN_APP_CFG_UI_VARIANT $1)/" "$CFG_FILE"
    echo "  HCN_APP_CFG_UI_VARIANT -> $1"
}

echo "修改 $CFG_FILE："
[ -n "$WANT_BT" ]      && apply BT "$WANT_BT"
[ -n "$WANT_SDK" ]     && apply BT_USE_SDK "$WANT_SDK"
[ -n "$WANT_CARPLAY" ] && apply CARPLAY "$WANT_CARPLAY"
if [ -n "$WANT_MODE" ]; then
    apply_mode "$WANT_MODE"
fi
if [ -n "$WANT_UI" ]; then
    apply_ui_variant "$WANT_UI"
fi

# ---- 编译：dirclean + rebuild（一条龙） ----
if [ "$EDIT_ONLY" = "1" ]; then
    echo "--edit-only：配置已改，未编译"
    exit 0
fi

# SELF_DIR = <root>/source/vendor/hcn/application -> 上四级到工程根
cd "$SELF_DIR/../../.."
cd ..
MAKE_ARGS=${HCN_MAKE_ARGS:-"-a nor-64-ddr-256 -d userdebug -m false"}

echo
echo "== dirclean（清 CMake 缓存）=="
./allmake.sh $MAKE_ARGS hcn_application-dirclean

echo
echo "== rebuild（重新 configure + 编译）=="
./allmake.sh $MAKE_ARGS hcn_application-rebuild

echo
echo "完成。生效判据：build_ac83xx.log 中 hcn-application 段有 Configuring 步骤，"
echo "且 full 模式出现 -DHCN_APP_FULL / 'vehicle services init success!'。当前配置："
grep -E '^set\(HCN_APP_CFG' "$CFG_FILE"
