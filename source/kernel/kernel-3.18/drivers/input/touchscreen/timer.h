#define BIM_BASE                                        (IO_BASE + 0x08000)

#define REG_RW_T64b_LO_0   0x0728
#define REG_RW_T64b_HI_0   0x072C
#define REG_RW_T64b_EN_0   0x0730
#define T64B_INIT() \
  BIM_WRITE32(REG_RW_T64b_LO_0, 0); \
    BIM_WRITE32(REG_RW_T64b_EN_0, 0); \
    BIM_WRITE32(REG_RW_T64b_LO_0, 0); \
    BIM_WRITE32(REG_RW_T64b_HI_0, 0); \
    BIM_WRITE32(REG_RW_T64b_EN_0, 1)

#define T64B_GET_LOW()  BIM_READ32(REG_RW_T64b_LO_0)
#define T64B_GET_HIGH() BIM_READ32(REG_RW_T64b_HI_0) 









