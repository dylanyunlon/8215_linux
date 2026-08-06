#!/bin/sh

DESC="atc_combo"
TAG="atc_combo"

pr_info() {
    echo "[$TAG][I] $@"
    logger -p info -t $TAG "$@"
}

pr_err() {
    echo "[$TAG][E] $@"
    logger -p info -t $TAG "$@"
}

check_error_exit() {
    local ret=${PIPESTATUS[0]}

    if [ $ret -ne 0 ]; then
        pr_err "[ERROR] ${FUNCNAME[1]},${BASH_LINENO[0]} ret=$ret"
        exit $ret
    fi
}

init() {
    source /etc/default/wifi_env.conf
    atc_combo_dir="${WIFI_CONFIG_FILE_PATH}/atc_combo"
    #atc_combo_drv_dir="/sys/devices/platform/atc_combo" # for 8015
    atc_combo_drv_dir="/sys/devices/soc/soc:atc_combo"
    drv_detect_file="${atc_combo_drv_dir}/detect"
    drv_chip_type_file="${atc_combo_drv_dir}/chip_type"
    drv_power_init_file="${atc_combo_drv_dir}/power_init"

    #insmod /usr/drivers/atc_combo.ko
    modprobe atc_combo.ko
    if [ ! -d $atc_combo_drv_dir ]; then
        pr_err "$atc_combo_drv_dir not exist"
        return 1
    fi

    for i in $(seq 100); do # 10s
        if [[ -d "/data" && -n "$(ls -A "/data" 2>/dev/null)" ]]; then
            break
        fi
        usleep 100000
    done
    if [[ ! (-d "/data" && -n "$(ls -A "/data" 2>/dev/null)") ]]; then
        pr_err "wait for data mount timeout"
        return 1
    fi

    if [ ! -d ${atc_combo_dir} ]; then
        mkdir -p ${atc_combo_dir}
    fi

    chip_type="$(cat $drv_chip_type_file)"
    if [[ -n "$chip_type" && "$chip_type" != "unknown" ]]; then
        pr_info "get chip_type: $chip_type"
        if [ ! -e ${atc_combo_dir}/atc_wifi_chip_type ]; then
            set_atc_wifi_chip_type $chip_type
        fi
    fi
}

set_atc_wifi_chip_type() {
    local chip_type="$1"

    if command -v wpa_supplicant-fixup_chip_type.sh >/dev/null 2>&1; then
        wpa_supplicant-fixup_chip_type.sh $chip_type
    fi
    if command -v wifiserver-fixup_chip_type.sh >/dev/null 2>&1; then
        wifiserver-fixup_chip_type.sh $chip_type
    fi
    echo $chip_type > ${atc_combo_dir}/atc_wifi_chip_type
    touch ${atc_combo_dir}/atc_wifi_chip_type_${chip_type}
    sync
}

start_pcie() {
    insmod /usr/drivers/msi-atc.ko
    insmod /usr/drivers/pcie-atc.ko
}

start_mt6630() {
    pr_info "${FUNCNAME[0]} start"
    systemctl start wmt-loader
    systemctl start 6630-launcher
    systemctl start stp-dump
}

start_cypress() {
    pr_info "${FUNCNAME[0]} start"
    if [ "$chip_type" = "cypress_pcie" ]; then
        start_pcie
    fi
    cypress_wlan_driver.sh -a
}

start_aic8800() {
    pr_info "${FUNCNAME[0]} start"
    aic8800_wlan_driver.sh start
}

self_adaptive() {
    pr_info "${FUNCNAME[0]} start"

    if [ ! -e ${atc_combo_dir}/atc_wifi_chip_type ]; then
        chip_type="$(cat $drv_chip_type_file)"
        if [ "$chip_type" = "unknown" ]; then
            pr_info "start chip_type detect..."
            echo 1 > $drv_detect_file
            chip_type="$(cat $drv_chip_type_file)"
        fi
        if [[ -n "$chip_type" && "$chip_type" != "unknown" ]]; then
            pr_info "detect chip_type: $chip_type"
            set_atc_wifi_chip_type $chip_type
        else
            pr_err "detect chip_type failed"
            return 1
        fi
    else
        chip_type="$(cat ${atc_combo_dir}/atc_wifi_chip_type)"
        echo -n "$chip_type" > $drv_chip_type_file
        chip_type="$(cat $drv_chip_type_file)"
        pr_info "chip_type: $chip_type"
    fi

    if [ "$chip_type" != "unknown" ]; then
        pr_info "power_init...start"
        echo 1 > $drv_power_init_file
        pr_info "power_init...done"
    fi

    if [ "$chip_type" = "mt6630" ]; then
        start_mt6630
    elif [ "$chip_type" = "cypress_pcie" ]; then
        start_cypress
    elif [ "$chip_type" = "cypress_sdio" ]; then
        start_cypress
    elif [ "$chip_type" = "aic8800_sdio" ]; then
        start_aic8800
    else
        pr_err "chip_type: $chip_type unsupported"
        return 1
    fi

    return $?
}

main() {
    source /etc/default/atc_combo.conf
    pr_info "${FUNCNAME[0]} start"
    pr_info "ATC_WIFI_CHIP: $ATC_WIFI_CHIP"
    pr_info "ATC_WLAN_TRANSMISSION_MODE: $ATC_WLAN_TRANSMISSION_MODE"

    init
    check_error_exit

    case "$ATC_WIFI_CHIP" in
        ATC_CHIP_MT6630)
            start_mt6630
            ;;
        CYPRESS)
            start_cypress
            ;;
        AIC8800)
            start_aic8800
            ;;
        SELF_ADAPTIVE)
            self_adaptive
            ;;
        *)
            pr_err "ATC_WIFI_CHIP $ATC_WIFI_CHIP not supported"
            return 1
            ;;
    esac

    return $?
}

#main "$@" 2>&1
main "$@" 2>&1 | tee /dev/console; exit ${PIPESTATUS[0]}
