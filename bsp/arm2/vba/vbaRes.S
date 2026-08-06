    .section vbadata,#alloc
    .globl  vba_data
vba_data:
    .incbin "vba.h264"
    .globl  vba_data_end 
vba_data_end:
