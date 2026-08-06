#ifndef _3363_IRQS_VECTOR_H_
#define _3363_IRQS_VECTOR_H_


#define   VECTOR_GRPC                 0  
#define   VECTOR_GRPB                 1  
//#define   VECTOR_(Reserved)           2  
#define   VECTOR_EXT                  3  
#define   VECTOR_SPI_MOTO            4     //3360  
#define   VECTOR_EXT2                 5  
#define   VECTOR_VDOIN                6   //VECTOR_DEMUX in 8550  
#define   VECTOR_GRAPH                7  
#define   VECTOR_JPGDEC               8  
#define   VECTOR_FONT                 9  
#define   VECTOR_IOMMU                10  
#define   VECTOR_NDMA                 11  // 3360 New DMA  
#define   VECTOR_JAVA                 12  
#define   VECTOR_RLE                  13  
#define   VECTOR_RS232_1              14   //3360  
#define   VECTOR_DSP                  15   //3360 DSPA2RC 
#define   VECTOR_SPD                  16   //3360 SPDF_RC  
#define   VECTOR_VDOUTREAR            17   //3360  
#define   VECTOR_SVO_IFINT            18  
#define   VECTOR_SVOIF                18  
#define   VECTOR_SFDMAI               19  
#define   VECTOR_DDMAI                20  
#define   VECTOR_PL310                21  
#define   VECTOR_USB                  22  
#define   VECTOR_CORISC               23  
#define   VECTOR_AXI64_WR             24  
#define   VECTOR_T2                   25  
#define   VECTOR_T1                   26  
#define   VECTOR_T0                   27  
#define   VECTOR_DSPC                 28    //3360 DSPC2RC  
#define   VECTOR_SVO_FE1INT           29  
#define   VECTOR_SVO_FE0INT           30  
#define   VECTOR_VSYNC                31

#define IRQ_N  128

#endif
