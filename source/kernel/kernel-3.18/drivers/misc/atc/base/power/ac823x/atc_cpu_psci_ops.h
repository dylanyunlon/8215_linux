

extern const struct cpu_operations cpu_psci_ops;

//#ifdef CONFIG_ATC_PSCI
extern const struct cpu_operations atc_cpu_psci_ops;

int spm_table_cpu_boot(unsigned int cpu);

//#endif

