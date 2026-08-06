
.macro FUNC name colon
.global \name
.type \name , %function
\name \colon
.endm

.macro END_FUNC name
.size \name , .-\name
.endm


FUNC smc_entry_func , :
    /*push    {r0-r7}*/
    ldr r0, [r0]
    bl  smc_handle_func
    /*
     * Normally thread_handle_std_smc() should return via
     * thread_exit(), thread_rpc(), but if thread_handle_std_smc()
     * hasn't switched stack (error detected) it will do a normal "C"
     * return.
     */
    pop {r1-r8}
    ldr r0, =0
.arch_extension sec
    smc #0
    b   .   /* SMC should not return */
END_FUNC smc_entry_func

