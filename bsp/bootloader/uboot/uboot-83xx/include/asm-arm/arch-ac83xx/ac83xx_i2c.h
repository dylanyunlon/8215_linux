#ifndef _AC83XX_I2C_
#define _AC83XX_I2C_
/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Memory (register) address within the chip default set 8bit
 *              so alen=1
 *              
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
extern int i2c_write (u8 chip, u32 addr, int alen, u8 * buffer, int len);
extern int i2c_read (u8 chip, u32 addr, int alen, u8 * buffer, int len);
/*
 *  speed : not used ,default set speed 400KHz
 *
*/
extern void i2c_init(int speed);
/*
  * bus:set which i2c bus num to used ,we have two i2c bus num 
  *        0 for master0, 1 for master1
*/
extern int i2c_set_bus_num(unsigned int bus);
/*
  * return which bus num now used
*/
extern unsigned int i2c_get_bus_num(void);
#endif