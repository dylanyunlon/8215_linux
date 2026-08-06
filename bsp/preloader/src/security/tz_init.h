#ifndef TZ_INIT_V7_H
#define TZ_INIT_V7_H


/* TEE magic */
#define TEE_ARGUMENT_MAGIC  (0x4B54444DU)

typedef struct {
    unsigned int magic;           // Magic number
    //unsigned int version;         // version
    unsigned int NWEntry;         // NW Entry point after t-base
    unsigned int NWBootArgs;      // NW boot args (propagated by t-base in r4 before jump)
    unsigned int NWBootArgsSize;  // NW boot args size (propagated by t-base in r5 before jump)
    unsigned int dRamBase;        // NonSecure DRAM start address
    unsigned int dRamSize;        // NonSecure DRAM size
    unsigned int secDRamBase;     // Secure DRAM start address
    unsigned int secDRamSize;     // Secure DRAM size
    unsigned int sRamBase;        // NonSecure Scratch RAM start address
    unsigned int sRamSize;        // NonSecure Scratch RAM size
    unsigned int secSRamBase;     // Secure Scratch RAM start address
    unsigned int secSRamSize;     // Secure Scratch RAM size
    unsigned int log_port;        // uart base address for logging
    unsigned int log_baudrate;    // uart baud rate
    //unsigned int hwuid[4];        // HW Unique id for t-base used
    unsigned int gicd_base;       //GICD register address base
    unsigned int gicc_base;       //GICC register address base
} tee_v7_arg_t, *tee_v7_arg_t_ptr;

typedef struct {
    unsigned int args1;           
	unsigned int args2;
	unsigned int args3;
} nw_entry_arg_t, *nw_entry_arg_t_ptr;

typedef struct {
    unsigned int magic;           // Magic number
    unsigned int version;         // version
    unsigned int svp_mem_start;   // MM sec mem pool start addr.
    unsigned int svp_mem_end;     // MM sec mem pool end addr.
    //unsigned int tplay_table_start; //tplay handle-to-physical table start
    //unsigned int tplay_table_size;  //tplay handle-to-physical table size
    //unsigned int tplay_mem_start;   //tplay physcial memory start address for crypto operation
    //unsigned int tplay_mem_size;    //tplay phsycial memory size for crypto operation
    unsigned int secmem_obfuscation;//MM sec mem obfuscation or not
  	//unsigned int msg_auth_key[8]; /* size of message auth key is 32bytes(256 bits) */
    //unsigned int rpmb_size;       /* size of rpmb partition */
    //unsigned int emmc_rel_wr_sec_c;  //emmc ext_csd[222]
} sec_mem_arg_t;


/**************************************************************************
 * EXPORTED FUNCTIONS
 **************************************************************************/
//void trustzone_pre_init_v7(void);
//void trustzone_post_init_v7(void);
void trustzone_jump_v7(unsigned int addr, unsigned int arg1, unsigned int arg2);

#endif /* TZ_INIT_V7_H */