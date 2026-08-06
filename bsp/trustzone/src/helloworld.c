int simple_secure_calc(int a, int b)
{
#if 0
  unsigned int i;
  unsigned int j=0;

  for (i = 0; i < 10; i++)
  {
    j++;
    asm volatile(
		".arch_extension sec\n"
		"smc	#0" : : :	/* switch to secure world */
		);
  } 

  return 0;
 #else
    asm volatile(
		".arch_extension sec\n"
		"smc	#0" : : :	/* switch to secure world */
		);
 #endif
}

