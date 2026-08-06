/**
 * @file armv6_mmu_config.h
 * @brief ARMv6 MMU Configuration - Processor-specific features
 * @author AI Assistant
 * @date 2025-12-10
 */

#ifndef ARMV6_MMU_CONFIG_H
#define ARMV6_MMU_CONFIG_H

/* ==================== Processor Configuration ==================== */

/**
 * Define your target processor here
 * Uncomment ONE of the following:
 */
#define MMU_CPU_ARM926EJS           /* ARM926EJ-S processor */
/* #define MMU_CPU_ARM1136JFS */    /* ARM1136JF-S processor */
/* #define MMU_CPU_ARM1176JZFS */   /* ARM1176JZF-S processor */
/* #define MMU_CPU_ARM11MPCORE */   /* ARM11 MPCore processor */

/* ==================== Feature Detection ==================== */

#if defined(MMU_CPU_ARM926EJS)
    /* ARM926EJ-S - ARMv5TEJ architecture */
    #define MMU_FEATURE_XN              0       /* No Execute Never support */
    #define MMU_FEATURE_SUPERSECTION    0       /* No Supersection support */
    #define MMU_FEATURE_TEX_REMAP       0       /* No TEX remap */
    #define MMU_FEATURE_ACCESS_FLAG     0       /* No Access Flag */
    #define MMU_FEATURE_ISB             0       /* No ISB instruction (use drain write buffer instead) */
    #define MMU_CPU_NAME                "ARM926EJ-S"

    /* Desired System Control S/R settings (user-configurable) */
    /* S: System protection bit (0=disabled), R: ROM protection bit (1=enabled) */
    #define MMU_SC_S_VALUE              0
    #define MMU_SC_R_VALUE              1

#elif defined(MMU_CPU_ARM1136JFS)
    /* ARM1136JF-S - ARMv6 architecture */
    #define MMU_FEATURE_XN              0       /* No Execute Never in base ARMv6 */
    #define MMU_FEATURE_SUPERSECTION    1       /* Supersection supported */
    #define MMU_FEATURE_TEX_REMAP       1       /* TEX remap supported */
    #define MMU_FEATURE_ACCESS_FLAG     0       /* No Access Flag */
    #define MMU_FEATURE_ISB             1       /* ISB instruction supported */
    #define MMU_CPU_NAME                "ARM1136JF-S"
    /* Default S/R for ARM1136 (can be adjusted as needed) */
    #define MMU_SC_S_VALUE              0
    #define MMU_SC_R_VALUE              0

#elif defined(MMU_CPU_ARM1176JZFS)
    /* ARM1176JZF-S - ARMv6KZ architecture */
    #define MMU_FEATURE_XN              1       /* Execute Never supported */
    #define MMU_FEATURE_SUPERSECTION    1       /* Supersection supported */
    #define MMU_FEATURE_TEX_REMAP       1       /* TEX remap supported */
    #define MMU_FEATURE_ACCESS_FLAG     1       /* Access Flag supported */
    #define MMU_FEATURE_ISB             1       /* ISB instruction supported */
    #define MMU_CPU_NAME                "ARM1176JZF-S"
    #define MMU_SC_S_VALUE              0
    #define MMU_SC_R_VALUE              0

#elif defined(MMU_CPU_ARM11MPCORE)
    /* ARM11 MPCore - ARMv6K architecture */
    #define MMU_FEATURE_XN              1       /* Execute Never supported */
    #define MMU_FEATURE_SUPERSECTION    1       /* Supersection supported */
    #define MMU_FEATURE_TEX_REMAP       1       /* TEX remap supported */
    #define MMU_FEATURE_ACCESS_FLAG     1       /* Access Flag supported */
    #define MMU_FEATURE_ISB             1       /* ISB instruction supported */
    #define MMU_CPU_NAME                "ARM11 MPCore"
    #define MMU_SC_S_VALUE              0
    #define MMU_SC_R_VALUE              0

#else
    #error "No processor defined! Please define MMU_CPU_xxx in armv6_mmu_config.h"
#endif

/* ==================== Feature Validation Helpers ==================== */

#if MMU_FEATURE_XN
    #define MMU_CHECK_XN_SUPPORT()      (1)
    #define MMU_APPLY_XN(desc, xn)      ((xn) ? ((desc) | (1 << 4)) : (desc))
#else
    #define MMU_CHECK_XN_SUPPORT()      (0)
    #define MMU_APPLY_XN(desc, xn)      (desc)  /* XN not supported, ignore */
#endif

/* ==================== Processor Information ==================== */

/**
 * @brief Get processor name string
 */
static inline const char* mmu_get_cpu_name(void)
{
    return MMU_CPU_NAME;
}

/**
 * @brief Check if Execute Never is supported
 */
static inline int mmu_has_xn_support(void)
{
    return MMU_FEATURE_XN;
}

#endif /* ARMV6_MMU_CONFIG_H */
