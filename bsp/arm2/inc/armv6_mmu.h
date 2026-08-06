/**
 * @file armv6_mmu.h
 * @brief ARMv6 MMU Configuration Driver Framework
 * @author AI Assistant
 * @date 2025-12-10
 *
 * This header defines the ARMv6 MMU driver framework with support for:
 * - Dynamic MMU configuration
 * - Minimal page table generation based on region ranges
 * - Flexible permission and cache attribute configuration
 * - Section (1MB), Large Page (64KB), and Small Page (4KB) mappings
 */

#ifndef ARMV6_MMU_H
#define ARMV6_MMU_H

#include <stdint.h>
#include <stdbool.h>
#include "armv6_mmu_config.h"  /* Processor-specific configuration */

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Constants and Definitions ==================== */

/* Page table sizes */
#define MMU_L1_TABLE_SIZE           4096    /* 4096 entries * 4 bytes = 16KB */
#define MMU_L2_TABLE_SIZE           256     /* 256 entries * 4 bytes = 1KB */
#define MMU_SECTION_SIZE            (1024 * 1024)       /* 1MB */
#define MMU_LARGE_PAGE_SIZE         (64 * 1024)         /* 64KB */
#define MMU_SMALL_PAGE_SIZE         (4 * 1024)          /* 4KB */

/* Alignment requirements */
#define MMU_L1_TABLE_ALIGN          (16 * 1024)         /* 16KB aligned */
#define MMU_L2_TABLE_ALIGN          (1 * 1024)          /* 1KB aligned */

/* Descriptor type bits [1:0] */
#define MMU_DESC_TYPE_FAULT         0x0
#define MMU_DESC_TYPE_PAGE_TABLE    0x1     /* Coarse page table */
#define MMU_DESC_TYPE_SECTION       0x2
#define MMU_DESC_TYPE_SUPERSECTION  0x40002 /* Section with bit 18 set */

/* L2 descriptor types */
#define MMU_L2_DESC_TYPE_FAULT      0x0
#define MMU_L2_DESC_TYPE_LARGE_PAGE 0x1
#define MMU_L2_DESC_TYPE_SMALL_PAGE 0x2
#define MMU_L2_DESC_TYPE_SMALL_XN   0x3     /* Extended small page (NOT supported on ARM926EJ-S) */

/* Access Permission encoding (AP bits) */
#define MMU_AP_NO_ACCESS            0x0     /* No access */
#define MMU_AP_PRIV_RW              0x1     /* Privileged RW, User no access */
#define MMU_AP_PRIV_RW_USER_RO      0x2     /* Privileged RW, User RO */
#define MMU_AP_FULL_ACCESS          0x3     /* Privileged RW, User RW */

/* Extended Access Permission (APX bit) - when set with AP */
#define MMU_APX_ENABLE              0x1     /* Enables read-only modes */

/* Domain field values (0-15) */
#define MMU_DOMAIN_0                0x0
#define MMU_DOMAIN_1                0x1
#define MMU_DOMAIN_15               0xF

/* Domain access control types */
#define MMU_DOMAIN_NO_ACCESS        0x0     /* Any access generates fault */
#define MMU_DOMAIN_CLIENT           0x1     /* Check access permissions */
#define MMU_DOMAIN_MANAGER          0x3     /* No permission checks */

/* Cache and buffer attributes */
#define MMU_CACHE_DISABLED          0x0
#define MMU_CACHE_ENABLED           0x1

#define MMU_BUFFER_DISABLED         0x0
#define MMU_BUFFER_ENABLED          0x1

/* TEX (Type Extension) field for memory attributes */
#define MMU_TEX_STRONGLY_ORDERED    0x0
#define MMU_TEX_SHARED_DEVICE       0x0
#define MMU_TEX_NORMAL              0x1
#define MMU_TEX_DEVICE              0x2

/* Shared bit */
#define MMU_SHARED                  0x1
#define MMU_NON_SHARED              0x0

/* Execute Never (XN) bit - NOT SUPPORTED on ARM926EJ-S */
/* ARM926EJ-S does not have XN feature, use domain protection instead */

/* Control register bits */
#define MMU_CR_M                    (1 << 0)    /* MMU enable */
#define MMU_CR_A                    (1 << 1)    /* Alignment fault enable */
#define MMU_CR_C                    (1 << 2)    /* Cache enable */
#define MMU_CR_W                    (1 << 3)    /* Write buffer enable */
#define MMU_CR_S                    (1 << 8)    /* System protection */
#define MMU_CR_R                    (1 << 9)    /* ROM protection */
#define MMU_CR_Z                    (1 << 11)   /* Branch prediction enable */
#define MMU_CR_I                    (1 << 12)   /* I-cache enable */
#define MMU_CR_V                    (1 << 13)   /* High exception vectors */
#define MMU_CR_RR                   (1 << 14)   /* Round-robin cache replacement */
#define MMU_CR_HA                   (1 << 17)   /* Hardware Access Flag enable */
#define MMU_CR_TR                   (1 << 28)   /* TEX remap enable */
#define MMU_CR_AFE                  (1 << 29)   /* Access Flag Enable */
#define MMU_CR_TRE                  (1 << 28)   /* TEX Remap Enable */

/* ==================== Type Definitions ==================== */

/**
 * @brief Memory region types
 */
typedef enum {
    MMU_REGION_STRONGLY_ORDERED,    /* Strongly ordered memory */
    MMU_REGION_DEVICE,               /* Device memory */
    MMU_REGION_NORMAL_NC,            /* Normal memory, non-cacheable */
    MMU_REGION_NORMAL_WT,            /* Normal memory, write-through */
    MMU_REGION_NORMAL_WB,            /* Normal memory, write-back */
    MMU_REGION_NORMAL_WB_WA          /* Normal memory, write-back with write allocate */
} mmu_region_type_t;

/**
 * @brief Access permission settings
 *
 * ARM926EJ-S Limitations:
 * - Only supports AP[1:0] bits (no APX extension)
 * - Cannot enforce true read-only modes
 * - MMU_PERM_PRIV_RO maps to Privileged RW
 * - MMU_PERM_PRIV_RO_USER_RO maps to Privileged RW + User RO
 */
typedef enum {
    MMU_PERM_NO_ACCESS,              /* No access for all (AP=00) */
    MMU_PERM_PRIV_RW,                /* Privileged RW, User no access (AP=01) */
    MMU_PERM_PRIV_RW_USER_RO,        /* Privileged RW, User read-only (AP=10) */
    MMU_PERM_PRIV_RW_USER_RW,        /* Full read-write access (AP=11) */
    MMU_PERM_PRIV_RO,                /* Privileged read-only (NOT SUPPORTED - maps to PRIV_RW) */
    MMU_PERM_PRIV_RO_USER_RO         /* Read-only for all (NOT SUPPORTED - maps to PRIV_RW_USER_RO) */
} mmu_permission_t;

/**
 * @brief Memory attributes structure
 *
 * Note: The domain field must be properly configured in DACR before use.
 * - Domain 0 is pre-configured as CLIENT by mmu_init()
 * - Other domains will be auto-configured as CLIENT when first used
 * - Use mmu_set_domain_access() to explicitly configure domain behavior
 */
typedef struct {
    mmu_region_type_t type;          /* Memory region type */
    mmu_permission_t permission;      /* Access permissions */
    uint8_t domain;                   /* Domain (0-15), defaults to 0 */
    bool execute_never;               /* Execute never flag (not supported on ARM926EJ-S) */
} mmu_attributes_t;

/**
 * @brief Memory region descriptor
 */
typedef struct {
    uint32_t virt_addr;              /* Virtual address (start) */
    uint32_t phys_addr;              /* Physical address (start) */
    uint32_t size;                   /* Region size in bytes */
    mmu_attributes_t attributes;     /* Memory attributes */
} mmu_region_t;

/**
 * @brief Page table entry types
 */
typedef enum {
    MMU_ENTRY_SECTION,               /* 1MB section */
    MMU_ENTRY_LARGE_PAGE,            /* 64KB large page */
    MMU_ENTRY_SMALL_PAGE             /* 4KB small page */
} mmu_entry_type_t;

/**
 * @brief Level-2 page table (exactly 1KB)
 * This is the actual page table that hardware uses.
 * Must be 1KB aligned.
 */
typedef struct {
    uint32_t entries[MMU_L2_TABLE_SIZE];    /* 256 entries = 1024 bytes */
} mmu_l2_table_t;

/**
 * @brief Level-2 page table metadata (separate from actual table)
 */
typedef struct {
    uint32_t base_virt_addr;                 /* Base virtual address */
    bool in_use;                             /* Table allocation status */
} mmu_l2_table_info_t;

/**
 * @brief MMU configuration context
 */
typedef struct {
    uint32_t *l1_table;                      /* Level-1 page table (16KB) */
    mmu_l2_table_t *l2_tables;               /* Array of L2 tables (actual page tables) */
    mmu_l2_table_info_t *l2_table_info;      /* Array of L2 table metadata */
    uint32_t l2_table_count;                 /* Number of L2 tables allocated */
    uint32_t l2_table_max;                   /* Maximum L2 tables available */
    uint32_t domain_access;                  /* Domain access control register value */
    bool initialized;                        /* Initialization status */
} mmu_context_t;

/**
 * @brief MMU statistics
 */
typedef struct {
    uint32_t section_count;                  /* Number of section mappings */
    uint32_t large_page_count;               /* Number of large page mappings */
    uint32_t small_page_count;               /* Number of small page mappings */
    uint32_t l2_table_count;                 /* Number of L2 tables used */
    uint32_t total_mapped_size;              /* Total mapped memory size */
} mmu_stats_t;

/* ==================== Function Prototypes ==================== */

/**
 * @brief Initialize MMU context with memory for page tables
 *
 * @param ctx MMU context structure
 * @param l1_table Pointer to 16KB-aligned L1 table memory
 * @param l2_tables Pointer to array of L2 tables (each 1KB aligned)
 * @param l2_table_info Pointer to array of L2 table metadata
 * @param max_l2_tables Maximum number of L2 tables available
 * @return 0 on success, negative error code on failure
 */
int mmu_init(mmu_context_t *ctx, uint32_t *l1_table,
             mmu_l2_table_t *l2_tables, mmu_l2_table_info_t *l2_table_info,
             uint32_t max_l2_tables);

/**
 * @brief Configure a memory region with specified attributes
 *
 * This function automatically selects the optimal page table granularity
 * (section, large page, or small page) to minimize page table entries.
 *
 * @param ctx MMU context
 * @param region Memory region descriptor
 * @return 0 on success, negative error code on failure
 */
int mmu_configure_region(mmu_context_t *ctx, const mmu_region_t *region);

/**
 * @brief Configure multiple memory regions
 *
 * @param ctx MMU context
 * @param regions Array of region descriptors
 * @param count Number of regions
 * @return 0 on success, negative error code on failure
 */
int mmu_configure_regions(mmu_context_t *ctx, const mmu_region_t *regions,
                         uint32_t count);

/**
 * @brief Unmap a memory region (remove mapping)
 *
 * This function removes the page table entries for the specified region,
 * effectively unmapping it. Access to unmapped regions will cause a fault.
 *
 * @param ctx MMU context
 * @param virt_addr Starting virtual address to unmap
 * @param size Size of region to unmap in bytes
 * @return 0 on success, negative error code on failure
 */
int mmu_unmap_region(mmu_context_t *ctx, uint32_t virt_addr, uint32_t size);

/**
 * @brief Set domain access control
 *
 * @param ctx MMU context
 * @param domain Domain number (0-15)
 * @param access Access control type (NO_ACCESS, CLIENT, MANAGER)
 * @return 0 on success, negative error code on failure
 */
int mmu_set_domain_access(mmu_context_t *ctx, uint8_t domain, uint8_t access);

/**
 * @brief Enable MMU with configured page tables
 *
 * @param ctx MMU context
 * @return 0 on success, negative error code on failure
 */
int mmu_enable(mmu_context_t *ctx);

/**
 * @brief Disable MMU
 *
 * @return 0 on success, negative error code on failure
 */
int mmu_disable(void);

/**
 * @brief Invalidate entire TLB
 */
void mmu_invalidate_tlb(void);

/**
 * @brief Invalidate TLB entry by virtual address
 *
 * @param virt_addr Virtual address
 */
void mmu_invalidate_tlb_entry(uint32_t virt_addr);

/**
 * @brief Get MMU statistics
 *
 * @param ctx MMU context
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int mmu_get_statistics(const mmu_context_t *ctx, mmu_stats_t *stats);

/**
 * @brief Create default attributes for common memory types
 *
 * @param type Memory region type
 * @param permission Access permission
 * @return Initialized attributes structure
 */
mmu_attributes_t mmu_create_attributes(mmu_region_type_t type,
                                       mmu_permission_t permission);

/**
 * @brief Dump page table used.
 */
void mmu_dump_page_table(void);

/**
 * @brief Translate virtual address to physical address
 *
 * @param ctx MMU context
 * @param virt_addr Virtual address
 * @param phys_addr Output physical address
 * @return 0 on success, negative error code on failure
 */
int mmu_translate_address(const mmu_context_t *ctx, uint32_t virt_addr,
                          uint32_t *phys_addr);

/* Error codes */
#define MMU_ERR_SUCCESS             0
#define MMU_ERR_INVALID_PARAM       -1
#define MMU_ERR_NOT_INITIALIZED     -2
#define MMU_ERR_NO_L2_TABLE         -3
#define MMU_ERR_ALIGNMENT           -4
#define MMU_ERR_OUT_OF_RANGE        -5
#define MMU_ERR_ALREADY_MAPPED      -6

#ifdef __cplusplus
}
#endif

#endif /* ARMV6_MMU_H */
