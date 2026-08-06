#ifndef _HMI_TASK_TOUCHPANEL_DRIVER_H
#define _HMI_TASK_TOUCHPANEL_DRIVER_H

#define EEP_ADDR_SYS_CAL_X_OFFSET		0

void hmi_eepwrite(UINT16 address,UINT8 length, UINT8 *pData);
void hmi_eepread(UINT16 address,UINT8 length, UINT8 *pData);

#endif

