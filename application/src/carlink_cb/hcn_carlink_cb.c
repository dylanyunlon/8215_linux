/**
*
* @file hcn_carlink_cb.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/09/10 10:45
* @author och
*
*/

#include "hcn_carlink_cb.h"
bt_data_t bt_data ;
bt_call_t bt_call ;
bt_music_info_t music ;
void carlink_cb_init(void) {

}

void hcn_initialize(HcnLibConfig* HcnCfg, IhcnCallBack* HcnCallback) {

}
int32_t hcn_ec_loadNightModeStatus(uint32_t isNightModeOn){return 0;}  ///< 切换亿联的白天黑夜模式
int32_t hcn_ec_startMirror(){return 0;}                        ///< 开始镜像
void hcn_ec_stopMirror(){return ;}                           ///< 停止镜像
const char* hcn_ec_get_version(){return "carbit 2026.1.0.1";}                   ///< 获取亿联SDK版本
const char* hcn_ec_get_qr_code_url(){return "http:\\4399.com";}               ///< 获取亿联连接的二维码
const char* hcn_ec_get_uuid(){return "HCNDC0015486155MAC";}	                    ///< 获取UUID

///< 蓝牙api
void hcn_bt_switch_state(bool on){return  ;}      				///< 打开或关闭手机蓝牙  
void hcn_bt_download_book(){return  ;}         				///< 手机蓝牙模块下载电话本
void hcn_bt_pick_up(){return  ;}                   			///< 接听
void hcn_bt_hung_up(){return  ;}                   			///< 挂断
bool hcn_bt_is_Call(){return false ;}                   		    ///< 当前是否在通话

const bt_call_t* hcn_bt_get_call(){return &bt_call;}              ///< 获取通话数据
const bt_data_t* hcn_bt_get_data(){return &bt_data;}          ///< 获取数据
const char *hcn_get_bt_version(){return "MCA BT 12.182.0.1";}         ///< 获取蓝牙库版本
const char* hcn_bt_get_name(){return "i . iphone";}                 ///< 获取蓝牙名称
const char* hcn_bt_get_mac_addr(){return "";}
const char* hcn_bt_get_ble_name(){return "";}
const char* hcn_bt_get_ble_mac_addr(){return "";}

///< 蓝牙音乐
const bt_music_info_t* hcn_bt_get_music_data(){return &music;}   ///< 获取蓝牙音乐相关信息
void hcn_send_music_cmd(bt_music_cmd_e cmd){return ;} 

const char* hcn_get_ota_ssid()
{
    return "ap630";
} 
const char* hcn_get_ota_ap_pwd()
{
    return "888888";
} 