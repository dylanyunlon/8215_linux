	.section .trackdata,#alloc
	.globl	track_data
track_data:

#if USE_16BITS_TRACK_IMAGE
	.incbin	"track.16.mrf"
#else
	.incbin	"track.32.mrf"
#endif

	.globl  track_data_end	
track_data_end:
