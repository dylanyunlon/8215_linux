/**
*
* @file hcn_read_eeprom.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/09/02 09:42
* @author och
*
*/
#include <stdint.h>
#include <stdlib.h>
#include "hcn_read_eeprom.h"
#define DEBUG_E2PROM_ENABLE

static void e2prom_write_protect(int value) {
    return  ;
}

int e2prom_write_data(uint16_t addr, uint8_t *buf, int size) {
    return 0 ;
}

int e2prom_read_data(uint16_t addr, uint8_t *buf, int size) {
    return 0 ;
}

int e2prom_byte_write (uint16_t addr,uint8_t *buf, uint8_t length) {
   return 0 ;
}

int e2prom_byte_read (uint16_t addr, uint8_t *buf, uint8_t length) {
  return 0 ;
}

int e2prom_test(uint16_t addr,int size,uint16_t start_data) {
   return 0 ;
}

