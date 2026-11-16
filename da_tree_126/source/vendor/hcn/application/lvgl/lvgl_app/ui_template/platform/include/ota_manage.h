#ifndef __OTA_MANAGE_H__
#define __OTA_MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "dev_config.h"

#if MW_OTA_MANAGE_ENABLE

typedef enum {
    UPDATE_NONE = 0,
    UPDATE_USB_SOC,
    UPDATE_USB_MCU,
    UPDATE_SD_CARD,
    UPDATE_OTA
} update_type_e;

typedef enum {
    UPDATE_ERROR_NONE = 0,
    UPDATE_ERROR_CRC,
    UPDATE_ERROR_FLASH,
    UPDATE_ERROR_FILE_TYPE
} update_error_e;

typedef enum {
    UPDATE_STATUS_IDLE = 0,
    UPDATE_STATUS_IN_PROGRESS,
    UPDATE_STATUS_SUCCESS,
    UPDATE_STATUS_FAILED,
    UPDATE_STATUS_TIMEOUT
} update_state_e;

typedef struct {
    update_type_e type;
    update_state_e status;
    update_error_e error;
    uint8_t progress;
} update_info_t;

void set_update_state_reset(void);

/**
 * @brief  set start update type
 * @param  type 
 * @return none
 */
void set_update_state_start(update_type_e type);

/**
 * @brief  set update progress
 * @param  progress: 0-100 
 * @param  error_type: see update_error_e
 * @return none
 */
void set_update_state_progress(uint8_t progress, update_error_e error_type);

/**
 * @brief  set update finish status
 * @param  final_status: update status,see update_state_e
 * @param  error_type: see update_error_e
 * @return none
 */
void set_update_state_finish(update_state_e final_status, update_error_e error_type);

update_info_t* get_current_update_info(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
