

//*************************************************************************/
//*   INCLUDE FILE AND FUNCTION                                           */
//*************************************************************************/

//*************************************************************************/
//*   CODE START                                                          */
//*************************************************************************/

  .text
  .section .burst_ops, "ax"

	.global BurstR
	.global BurstW
//*************************************************************************/
//*     void BurstR(UINT32 u4Addr, UINT32 *pBuf, UINT32 u4Cnt)         */
//*************************************************************************/

BurstR:

  STMDB SP!, {R3-R12, LR}

R_START:
  CMP R2, #11
  BHS R_LOOP
  

  CMP R2, #0
  BEQ R_END
  CMP R2, #1
  BEQ R_CNT1
  CMP R2, #2
  BEQ R_CNT2
  CMP R2, #3
  BEQ R_CNT3
  CMP R2, #4
  BEQ R_CNT4
  CMP R2, #5
  BEQ R_CNT5
  CMP R2, #6
  BEQ R_CNT6
  CMP R2, #7
  BEQ R_CNT7
  CMP R2, #8
  BEQ R_CNT8
  CMP R2, #9
  BEQ R_CNT9
  CMP R2, #10
  BEQ R_CNT10
  ;B R_END
    
R_LOOP:
  LDMIA R0!, {R3-R12, LR}
  STMIA R1!, {R3-R12, LR}
  SUB R2, R2, #11
  BHS R_START
  B R_END

R_CNT1:
  LDM R0, {R3}
  STM R1, {R3}
  B R_END
R_CNT2:
  LDM R0!, {R3, R4}
  STM R1!, {R3, R4}
  B R_END
R_CNT3:
  LDMIA R0!, {R3-R5}
  STMIA R1!, {R3-R5}
  B R_END
R_CNT4:
  LDMIA R0!, {R3-R6}
  STMIA R1!, {R3-R6}
  B R_END
R_CNT5:
  LDMIA R0!, {R3-R7}
  STMIA R1!, {R3-R7}
  B R_END
R_CNT6:
  LDMIA R0!, {R3-R8}
  STMIA R1!, {R3-R8}
  B R_END
R_CNT7:
  LDMIA R0!, {R3-R9}
  STMIA R1!, {R3-R9}
  B R_END
R_CNT8:
  LDMIA R0!, {R3-R10}
  STMIA R1!, {R3-R10}
  B R_END
R_CNT9:
  LDMIA R0!, {R3-R11}
  STMIA R1!, {R3-R11}
  B R_END
R_CNT10:
  LDMIA R0!, {R3-R12}
  STMIA R1!, {R3-R12}
  B R_END
  
R_END:
  LDMIA SP!, {R3-R12, LR}

  BX LR
  
///*************************************************************************/
///*     void BurstW(UINT32 u4Addr, UINT32 *pBuf, UINT32 u4Cnt)         */
///*************************************************************************/
BurstW:

  STMDB SP!, {R3-R12, LR}

W_START:
  CMP R2, #11
  BHS W_LOOP
  

  CMP R2, #0
  BEQ W_END
  CMP R2, #1
  BEQ W_CNT1
  CMP R2, #2
  BEQ W_CNT2
  CMP R2, #3
  BEQ W_CNT3
  CMP R2, #4
  BEQ W_CNT4
  CMP R2, #5
  BEQ W_CNT5
  CMP R2, #6
  BEQ W_CNT6
  CMP R2, #7
  BEQ W_CNT7
  CMP R2, #8
  BEQ W_CNT8
  CMP R2, #9
  BEQ W_CNT9
  CMP R2, #10
  BEQ W_CNT10
  ;B W_END

W_LOOP:
  LDMIA R1!, {R3-R12, LR}
  STMIA R0!, {R3-R12, LR}
  SUB R2, R2, #11
  BHS W_START
  B W_END

W_CNT1:
  LDM R1, {R3}
  STM R0, {R3}
  B R_END
W_CNT2:
  LDMIA R1!, {R3, R4}
  STMIA R0!, {R3, R4}
  B W_END
W_CNT3:
  LDMIA R1!, {R3-R5}
  STMIA R0!, {R3-R5}
  B W_END
W_CNT4:
  LDMIA R1!, {R3-R6}
  STMIA R0!, {R3-R6}
  B W_END
W_CNT5:
  LDMIA R1!, {R3-R7}
  STMIA R0!, {R3-R7}
  B W_END
W_CNT6:
  LDMIA R1!, {R3-R8}
  STMIA R0!, {R3-R8}
  B W_END
W_CNT7:
  LDMIA R1!, {R3-R9}
  STMIA R0!, {R3-R9}
  B W_END
W_CNT8:
  LDMIA R1!, {R3-R10}
  STMIA R0!, {R3-R10}
  B W_END
W_CNT9:
  LDMIA R1!, {R3-R11}
  STMIA R0!, {R3-R11}
  B W_END
W_CNT10:
  LDMIA R1!, {R3-R12}
  STMIA R0!, {R3-R12}
  B W_END
  
W_END:
  LDMIA SP!, {R3-R12, LR}

  BX LR
  
  
///*************************************************************************/
///*     void MemBurstW(void *pDst,UINT32 pat,UINT32 length)               */
///*************************************************************************/

MemBurstW:
	stmfd r13!,{r4-r7}
	mov r4,r1
	mov r5,r1
	mov r6,r1
	mov r7,r1
setloop:
	stmia r0!,{r4-r7}
	subs r2,r2,#16
	bne setloop
	ldmfd r13!,{r4-r7}
	BX LR  

///*************************************************************************/
///*     void MemBurstR(void *pSrc,UINT32 length)                          */
///*************************************************************************/

MemBurstR:
	stmfd SP!,{R4-R7}
getloop:
	ldmia R0!,{R4-R7}
	subs R1,R1,#16
	bne getloop
	ldmfd SP!,{R4-R7}
	BX LR

///*************************************************************************/
///*     void MemBurstRW(void *pDst,void *pSrc,UINT32 length)         */
///*************************************************************************/

MemBurstRW:
	stmfd r13!,{r4-r7}
cpyloop:
	ldmia r1!,{r4-r7}
	stmia r0!,{r4-r7}
	subs r2,r2,#16
	bne cpyloop
	ldmfd r13!,{r4-r7}
	BX LR


///*************************************************************************/
///*   CODE END                                                            */
///*************************************************************************/ 

  .end  
