/**
 * @file armv6_mmu.c
 * @brief ARMv6 MMU Configuration Driver Implementation
 * @author AI Assistant
 * @date 2025-12-10
 */

#include "armv6_mmu.h"
#include <string.h>

/* MMU Context */
static mmu_context_t mmu_ctx;

/* ==================== Helper Macros ==================== */

#define ALIGN_DOWN(addr, align)     ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align)       (((addr) + (align) - 1) & ~((align) - 1))
#define IS_ALIGNED(addr, align)     (((addr) & ((align) - 1)) == 0)

#define L1_INDEX(va)                (((va) >> 20) & 0xFFF)
#define L2_INDEX(va)                (((va) >> 12) & 0xFF)

/* ==================== Private Function Prototypes ==================== */

static uint32_t build_section_descriptor(uint32_t phys_addr, const mmu_attributes_t *attr);

static uint32_t build_page_table_descriptor(uint32_t l2_table_phys, uint8_t domain);

static uint32_t build_large_page_descriptor(uint32_t phys_addr, const mmu_attributes_t *attr);

static uint32_t build_small_page_descriptor(uint32_t phys_addr, const mmu_attributes_t *attr);

static void get_memory_attributes_bits(const mmu_attributes_t *attr, uint32_t *tex, uint32_t *c, uint32_t *b);

static void get_access_permission_bits(mmu_permission_t perm, uint32_t *ap, uint32_t *apx);

static mmu_l2_table_t* allocate_l2_table(mmu_context_t *ctx, uint32_t virt_addr, uint8_t domain);

static int map_with_sections(mmu_context_t *ctx, const mmu_region_t *region,
                             uint32_t *virt_addr, uint32_t *phys_addr,
                             uint32_t *remaining);

static int map_with_large_pages(mmu_context_t *ctx, const mmu_region_t *region,
                               uint32_t *virt_addr, uint32_t *phys_addr,
                               uint32_t *remaining);

static int map_with_small_pages(mmu_context_t *ctx, const mmu_region_t *region,
                               uint32_t *virt_addr, uint32_t *phys_addr,
                               uint32_t *remaining);

/* ==================== CP15 Register Access Functions ==================== */

/**
 * @brief Read CP15 Control Register (c1)
 */
static inline uint32_t cp15_read_control(void)
{
    uint32_t val;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (val));
    return val;
}

/**
 * @brief Write CP15 Control Register (c1)
 */
static inline void cp15_write_control(uint32_t val)
{
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (val));
#if MMU_FEATURE_ISB
    /* Instruction Synchronization Barrier - CP15 register 7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
#else
    /* ARM926EJ-S: Drain write buffer instead of ISB */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
#endif
}

/**
 * @brief Write Translation Table Base Register 0 (TTBR0)
 */
static inline void cp15_write_ttbr0(uint32_t val)
{
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0" : : "r" (val));
#if MMU_FEATURE_ISB
    /* Instruction Synchronization Barrier - CP15 register 7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
#else
    /* ARM926EJ-S: Drain write buffer instead of ISB */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
#endif
}

/**
 * @brief Write Domain Access Control Register
 */
static inline void cp15_write_dacr(uint32_t val)
{
    __asm__ volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (val));
#if MMU_FEATURE_ISB
    /* Instruction Synchronization Barrier - CP15 register 7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
#else
    /* ARM926EJ-S: Drain write buffer instead of ISB */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
#endif
}

/**
 * @brief Invalidate entire TLB
 */
static inline void cp15_invalidate_tlb(void)
{
    uint32_t val = 0;
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 0" : : "r" (val));
    /* Data Synchronization Barrier - CP15 c7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
#if MMU_FEATURE_ISB
    /* Instruction Synchronization Barrier - CP15 c7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
#endif
}

/**
 * @brief Invalidate TLB entry by MVA
 */
static inline void cp15_invalidate_tlb_mva(uint32_t mva)
{
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 1" : : "r" (mva));
    /* Data Synchronization Barrier - CP15 c7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
#if MMU_FEATURE_ISB
    /* Instruction Synchronization Barrier - CP15 c7 */
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
#endif
}

/**
 * @brief Data synchronization barrier (ARMv6 compatible)
 */
static inline void dsb(void)
{
    /* ARMv6: Use CP15 c7, c10, 4 for Data Synchronization Barrier */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
}

/**
 * @brief Instruction synchronization barrier (ARMv6 compatible)
 */
static inline void isb(void)
{
#if MMU_FEATURE_ISB
    /* ARMv6: Use CP15 c7, c5, 4 for Instruction Synchronization Barrier */
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
#else
    /* ARM926EJ-S: No ISB instruction, use drain write buffer as best effort */
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
#endif
}

/* ==================== Public API Implementation ==================== */

/**
 * @brief Initialize MMU context
 */
int mmu_init(mmu_context_t *ctx, uint32_t *l1_table,
             mmu_l2_table_t *l2_tables, mmu_l2_table_info_t *l2_table_info,
             uint32_t max_l2_tables)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!l1_table || !l2_tables || !l2_table_info) {
        return MMU_ERR_INVALID_PARAM;
    }

    /* Check L1 table alignment (must be 16KB aligned) */
    if (!IS_ALIGNED((uint32_t)l1_table, MMU_L1_TABLE_ALIGN)) {
        return MMU_ERR_ALIGNMENT;
    }

    /* Check L2 tables alignment (each must be 1KB aligned) */
    if (!IS_ALIGNED((uint32_t)l2_tables, MMU_L2_TABLE_ALIGN)) {
        return MMU_ERR_ALIGNMENT;
    }

    /* Initialize context */
    memset(ctx, 0, sizeof(mmu_context_t));
    ctx->l1_table = l1_table;
    ctx->l2_tables = l2_tables;
    ctx->l2_table_info = l2_table_info;
    ctx->l2_table_count = 0;
    ctx->l2_table_max = max_l2_tables;
    ctx->domain_access = 0;
    ctx->initialized = true;

    /* Clear L1 table - all entries set to fault */
    memset(l1_table, 0, MMU_L1_TABLE_SIZE * sizeof(uint32_t));

    /* Clear L2 tables */
    memset(l2_tables, 0, max_l2_tables * sizeof(mmu_l2_table_t));

    /* Clear L2 table metadata */
    memset(l2_table_info, 0, max_l2_tables * sizeof(mmu_l2_table_info_t));

    /* Set default domain access - domain 0 as client */
    mmu_set_domain_access(ctx, 0, MMU_DOMAIN_CLIENT);

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Configure a memory region
 */
int mmu_configure_region(mmu_context_t *ctx, const mmu_region_t *region)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!region) {
        return MMU_ERR_INVALID_PARAM;
    }

    if (!ctx->initialized) {
        return MMU_ERR_NOT_INITIALIZED;
    }

    if (region->size == 0) {
        return MMU_ERR_INVALID_PARAM;
    }

    /* Ensure the domain is configured in DACR */
    uint8_t domain = region->attributes.domain;
    if (domain > 15) {
        return MMU_ERR_INVALID_PARAM;
    }

    /* Check if domain is already configured (not NO_ACCESS) */
    uint32_t domain_bits = (ctx->domain_access >> (domain * 2)) & 0x3;
    if (domain_bits == MMU_DOMAIN_NO_ACCESS) {
        /* Auto-configure as CLIENT if not set */
        mmu_set_domain_access(ctx, domain, MMU_DOMAIN_CLIENT);
    }

    uint32_t va = region->virt_addr;
    uint32_t pa = region->phys_addr;
    uint32_t remaining = region->size;
    uint32_t mapped;
    int ret;

    while (remaining > 0) {
        /* Try to map with 1MB section */
        if (remaining >= MMU_SECTION_SIZE &&
            IS_ALIGNED(va, MMU_SECTION_SIZE) &&
            IS_ALIGNED(pa, MMU_SECTION_SIZE)) {

            mapped = remaining;
            ret = map_with_sections(ctx, region, &va, &pa, &mapped);
            if (ret < 0) return ret;
            remaining = mapped;
            continue;
        }

        /* Try to map with 64KB large page */
        if (remaining >= MMU_LARGE_PAGE_SIZE &&
            IS_ALIGNED(va, MMU_LARGE_PAGE_SIZE) &&
            IS_ALIGNED(pa, MMU_LARGE_PAGE_SIZE)) {

            mapped = remaining;
            ret = map_with_large_pages(ctx, region, &va, &pa, &mapped);
            if (ret < 0) return ret;
            remaining = mapped;
            continue;
        }

        /* Map with 4KB small page */
        mapped = remaining;
        ret = map_with_small_pages(ctx, region, &va, &pa, &mapped);
        if (ret < 0) return ret;
        remaining = mapped;
    }

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Configure multiple memory regions
 */
int mmu_configure_regions(mmu_context_t *ctx, const mmu_region_t *regions,
                         uint32_t count)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!regions) {
        return MMU_ERR_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < count; i++) {
        int ret = mmu_configure_region(ctx, &regions[i]);
        if (ret < 0) {
            return ret;
        }
    }

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Set domain access control
 */
int mmu_set_domain_access(mmu_context_t *ctx, uint8_t domain, uint8_t access)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (domain > 15 || access > 3) {
        return MMU_ERR_INVALID_PARAM;
    }

    /* Clear and set the 2-bit domain access field */
    ctx->domain_access &= ~(0x3 << (domain * 2));
    ctx->domain_access |= (access & 0x3) << (domain * 2);

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Enable MMU
 */
int mmu_enable(mmu_context_t *ctx)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!ctx->initialized) {
        return MMU_ERR_NOT_INITIALIZED;
    }

    /* Ensure all memory operations complete */
    dsb();

    /* Set Translation Table Base Register 0 */
    cp15_write_ttbr0((uint32_t)ctx->l1_table);

    /* Set Domain Access Control Register */
    cp15_write_dacr(ctx->domain_access);

    /* Invalidate entire TLB */
    cp15_invalidate_tlb();

    /* Read current control register */
    uint32_t cr = cp15_read_control();

    /* Enable MMU, caches, and write buffer */
    cr |= MMU_CR_M;     /* Enable MMU */
    cr |= MMU_CR_C;     /* Enable D-cache */
        /* Branch prediction (Z) not present on ARM926 */
    cr |= MMU_CR_I;     /* Enable I-cache */
    cr |= MMU_CR_A;     /* Enable Alignment fault  */

    /* Apply user-requested S/R settings */
    if (MMU_SC_S_VALUE) cr |= MMU_CR_S; else cr &= ~MMU_CR_S;  /* System protection */
    if (MMU_SC_R_VALUE) cr |= MMU_CR_R; else cr &= ~MMU_CR_R;  /* ROM protection */

    /* Write back control register */
    cp15_write_control(cr);

    /* Ensure changes take effect */
    isb();

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Disable MMU
 */
int mmu_disable(void)
{
    uint32_t cr = cp15_read_control();
    cr &= ~MMU_CR_M;    /* Disable MMU */
    cp15_write_control(cr);
    isb();
    return MMU_ERR_SUCCESS;
}

/**
 * @brief Invalidate entire TLB
 */
void mmu_invalidate_tlb(void)
{
    cp15_invalidate_tlb();
}

/**
 * @brief Invalidate TLB entry by virtual address
 */
void mmu_invalidate_tlb_entry(uint32_t virt_addr)
{
    cp15_invalidate_tlb_mva(virt_addr);
}

/**
 * @brief Unmap a memory region
 */
int mmu_unmap_region(mmu_context_t *ctx, uint32_t virt_addr, uint32_t size)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!ctx->initialized) {
        return MMU_ERR_NOT_INITIALIZED;
    }

    if (size == 0) {
        return MMU_ERR_INVALID_PARAM;
    }

    uint32_t va = virt_addr;
    uint32_t remaining = size;

    /* Process the region */
    while (remaining > 0) {
        uint32_t l1_idx = L1_INDEX(va);
        uint32_t l1_entry = ctx->l1_table[l1_idx];
        uint32_t desc_type = l1_entry & 0x3;

        /* Check if it's a section mapping */
        if (desc_type == MMU_DESC_TYPE_SECTION) {
            /* Check if we need to unmap entire section or convert to page table */
            uint32_t section_start = va & 0xFFF00000;
            uint32_t section_end = section_start + MMU_SECTION_SIZE;
            uint32_t unmap_start = va;
            uint32_t unmap_end = va + remaining;

            /* If unmapping entire section, just clear L1 entry */
            if (unmap_start <= section_start && unmap_end >= section_end) {
                ctx->l1_table[l1_idx] = MMU_DESC_TYPE_FAULT;
                va = section_end;
                remaining = (unmap_end > section_end) ? (unmap_end - section_end) : 0;
            } else {
                /* Partial section unmap - need to convert to page table first */
                /* This is complex, for now just clear the whole section */
                ctx->l1_table[l1_idx] = MMU_DESC_TYPE_FAULT;
                uint32_t advance = MMU_SECTION_SIZE - (va & 0xFFFFF);
                if (advance > remaining) advance = remaining;
                va += advance;
                remaining -= advance;
            }
        } else if (desc_type == MMU_DESC_TYPE_PAGE_TABLE) {
            /* Find corresponding L2 table */
            mmu_l2_table_t *l2_table = NULL;
            uint32_t l2_idx_found = 0;
            for (uint32_t i = 0; i < ctx->l2_table_count; i++) {
                if (!ctx->l2_table_info[i].in_use) continue;
                if (L1_INDEX(ctx->l2_table_info[i].base_virt_addr) == l1_idx) {
                    l2_table = &ctx->l2_tables[i];
                    l2_idx_found = i;
                    break;
                }
            }

            if (l2_table) {
                /* Unmap pages within this L2 table */
                while (remaining > 0 && L1_INDEX(va) == l1_idx) {
                    uint32_t l2_idx = L2_INDEX(va);
                    uint32_t l2_entry = l2_table->entries[l2_idx];
                    uint32_t l2_type = l2_entry & 0x3;

                    if (l2_type == MMU_L2_DESC_TYPE_LARGE_PAGE) {
                        /* Clear 16 consecutive entries for large page */
                        for (uint32_t i = 0; i < 16 && (l2_idx + i) < MMU_L2_TABLE_SIZE; i++) {
                            l2_table->entries[l2_idx + i] = MMU_L2_DESC_TYPE_FAULT;
                        }
                        uint32_t advance = MMU_LARGE_PAGE_SIZE;
                        if (advance > remaining) advance = remaining;
                        va += advance;
                        remaining -= advance;
                    } else if (l2_type == MMU_L2_DESC_TYPE_SMALL_PAGE ||
                             l2_type == MMU_L2_DESC_TYPE_SMALL_XN) {
                        /* Clear small page entry */
                        l2_table->entries[l2_idx] = MMU_L2_DESC_TYPE_FAULT;
                        uint32_t advance = MMU_SMALL_PAGE_SIZE;
                        if (advance > remaining) advance = remaining;
                        va += advance;
                        remaining -= advance;
                    } else {
                        /* Already unmapped or fault, skip this page */
                        uint32_t advance = MMU_SMALL_PAGE_SIZE;
                        if (advance > remaining) advance = remaining;
                        va += advance;
                        remaining -= advance;
                    }
                }

                /* Check if entire L2 table is now empty */
                bool table_empty = true;
                for (uint32_t i = 0; i < MMU_L2_TABLE_SIZE; i++) {
                    if ((l2_table->entries[i] & 0x3) != MMU_L2_DESC_TYPE_FAULT) {
                        table_empty = false;
                        break;
                    }
                }

                /* If empty, clear L1 entry and mark L2 table as unused */
                if (table_empty) {
                    ctx->l1_table[l1_idx] = MMU_DESC_TYPE_FAULT;
                    ctx->l2_table_info[l2_idx_found].in_use = false;
                }
            } else {
                /* L2 table not found, skip this section */
                uint32_t advance = MMU_SECTION_SIZE - (va & 0xFFFFF);
                if (advance > remaining) advance = remaining;
                va += advance;
                remaining -= advance;
            }
        } else {
            /* Already unmapped or fault, skip to next section */
            uint32_t advance = MMU_SECTION_SIZE - (va & 0xFFFFF);
            if (advance > remaining) advance = remaining;
            va += advance;
            remaining -= advance;
        }
    }

    /* Invalidate TLB for unmapped region */
    mmu_invalidate_tlb();

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Get MMU statistics
 */
int mmu_get_statistics(const mmu_context_t *ctx, mmu_stats_t *stats)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!stats) {
        return MMU_ERR_INVALID_PARAM;
    }

    memset(stats, 0, sizeof(mmu_stats_t));

    /* Scan L1 table */
    for (uint32_t i = 0; i < MMU_L1_TABLE_SIZE; i++) {
        uint32_t l1_entry = ctx->l1_table[i];
        uint32_t desc_type = l1_entry & 0x3;

        if (desc_type == MMU_DESC_TYPE_SECTION) {
            stats->section_count++;
            stats->total_mapped_size += MMU_SECTION_SIZE;
        } else if (desc_type == MMU_DESC_TYPE_PAGE_TABLE) {
            /* Find corresponding L2 table */
            uint32_t l2_table_phys = l1_entry & 0xFFFFFC00;

            /* Scan L2 tables */
            for (uint32_t j = 0; j < ctx->l2_table_count; j++) {
                if (!ctx->l2_table_info[j].in_use) continue;

                uint32_t expected_va = ctx->l2_table_info[j].base_virt_addr;
                if (L1_INDEX(expected_va) != i) continue;

                stats->l2_table_count++;

                /* Count pages in this L2 table */
                for (uint32_t k = 0; k < MMU_L2_TABLE_SIZE; k++) {
                    uint32_t l2_entry = ctx->l2_tables[j].entries[k];
                    uint32_t l2_type = l2_entry & 0x3;

                    if (l2_type == MMU_L2_DESC_TYPE_LARGE_PAGE) {
                        stats->large_page_count++;
                        stats->total_mapped_size += MMU_LARGE_PAGE_SIZE;
                    } else if (l2_type == MMU_L2_DESC_TYPE_SMALL_PAGE ||
                             l2_type == MMU_L2_DESC_TYPE_SMALL_XN) {
                        stats->small_page_count++;
                        stats->total_mapped_size += MMU_SMALL_PAGE_SIZE;
                    }
                }
                break;
            }
        }
    }

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Create default attributes
 */
mmu_attributes_t mmu_create_attributes(mmu_region_type_t type,
                                       mmu_permission_t permission)
{
    mmu_attributes_t attr = {0};

    attr.type = type;
    attr.permission = permission;
    attr.domain = 0;  /* Default to domain 0 (pre-configured as CLIENT in mmu_init) */
    attr.execute_never = false;

    return attr;
}

/**
 * @brief Translate virtual address to physical address
 */
int mmu_translate_address(const mmu_context_t *ctx, uint32_t virt_addr,
                          uint32_t *phys_addr)
{
    if (!ctx) {
        ctx = &mmu_ctx;
    }

    if (!phys_addr) {
        return MMU_ERR_INVALID_PARAM;
    }

    uint32_t l1_idx = L1_INDEX(virt_addr);
    uint32_t l1_entry = ctx->l1_table[l1_idx];
    uint32_t desc_type = l1_entry & 0x3;

    if (desc_type == MMU_DESC_TYPE_FAULT) {
        return MMU_ERR_OUT_OF_RANGE;
    }

    if (desc_type == MMU_DESC_TYPE_SECTION) {
        /* Section mapping */
        uint32_t section_base = l1_entry & 0xFFF00000;
        uint32_t offset = virt_addr & 0x000FFFFF;
        *phys_addr = section_base | offset;
        return MMU_ERR_SUCCESS;
    }

    if (desc_type == MMU_DESC_TYPE_PAGE_TABLE) {
        /* Find L2 table */
        for (uint32_t i = 0; i < ctx->l2_table_count; i++) {
            if (!ctx->l2_table_info[i].in_use) continue;
            if (L1_INDEX(ctx->l2_table_info[i].base_virt_addr) != l1_idx) continue;

            uint32_t l2_idx = L2_INDEX(virt_addr);
            uint32_t l2_entry = ctx->l2_tables[i].entries[l2_idx];
            uint32_t l2_type = l2_entry & 0x3;

            if (l2_type == MMU_L2_DESC_TYPE_LARGE_PAGE) {
                uint32_t page_base = l2_entry & 0xFFFF0000;
                uint32_t offset = virt_addr & 0x0000FFFF;
                *phys_addr = page_base | offset;
                return MMU_ERR_SUCCESS;
            }

            if (l2_type == MMU_L2_DESC_TYPE_SMALL_PAGE ||
                l2_type == MMU_L2_DESC_TYPE_SMALL_XN) {
                uint32_t page_base = l2_entry & 0xFFFFF000;
                uint32_t offset = virt_addr & 0x00000FFF;
                *phys_addr = page_base | offset;
                return MMU_ERR_SUCCESS;
            }

            return MMU_ERR_OUT_OF_RANGE;
        }
    }

    return MMU_ERR_OUT_OF_RANGE;
}

/**
 * @brief Dump page table contents (for debugging)
 */
void mmu_dump_page_table(void)
{
    printf("Max L2 Tables:%d ---> Used:%d.\n", mmu_ctx.l2_table_max, mmu_ctx.l2_table_count);
}

/* ==================== Private Helper Functions ==================== */

/**
 * @brief Build section descriptor (1MB)
 */
static uint32_t build_section_descriptor(uint32_t phys_addr,
                                        const mmu_attributes_t *attr)
{
    uint32_t desc = phys_addr & 0xFFF00000;  /* Physical address [31:20] */
    desc |= MMU_DESC_TYPE_SECTION;           /* Descriptor type */

    /* Get memory type attributes */
    uint32_t tex, c, b;
    get_memory_attributes_bits(attr, &tex, &c, &b);
    /* ARM926EJ-S (ARMv5) does not support TEX; only set C/B */
#if MMU_FEATURE_TEX_REMAP
    desc |= (tex & 0x7) << 12;   /* TEX[2:0] */
#else
    (void)tex;                   /* Unused when TEX not supported */
#endif
    desc |= (c & 0x1) << 3;      /* C bit */
    desc |= (b & 0x1) << 2;      /* B bit */

    /* Access permissions */
    uint32_t ap, apx;
    get_access_permission_bits(attr->permission, &ap, &apx);
    desc |= (ap & 0x3) << 10;    /* AP[1:0] */
    /* Note: APX not supported on ARM926EJ-S, apx will always be 0 */

    /* Domain - ensure it's configured in DACR */
    desc |= (attr->domain & 0xF) << 5;

    /* Execute Never - only on ARMv6K and later (ARM1176, ARM11 MPCore) */
#if MMU_FEATURE_XN
    if (attr->execute_never) {
        desc |= (1 << 4);  /* XN bit for sections */
    }
#else
    (void)attr->execute_never;  /* Not supported on ARM926EJ-S */
#endif

    return desc;
}

/**
 * @brief Build page table descriptor
 */
static uint32_t build_page_table_descriptor(uint32_t l2_table_phys, uint8_t domain)
{
    uint32_t desc = l2_table_phys & 0xFFFFFC00;  /* L2 table base [31:10] */
    desc |= MMU_DESC_TYPE_PAGE_TABLE;             /* Descriptor type */
    desc |= (domain & 0xF) << 5;                  /* Domain [8:5] */
    return desc;
}

/**
 * @brief Build large page descriptor (64KB)
 */
static uint32_t build_large_page_descriptor(uint32_t phys_addr,
                                           const mmu_attributes_t *attr)
{
    uint32_t desc = phys_addr & 0xFFFF0000;  /* Physical address [31:16] */
    desc |= MMU_L2_DESC_TYPE_LARGE_PAGE;     /* Descriptor type */

    /* Get memory type attributes */
    uint32_t tex, c, b;
    get_memory_attributes_bits(attr, &tex, &c, &b);
    /* ARM926EJ-S (ARMv5) does not support TEX; only set C/B */
#if MMU_FEATURE_TEX_REMAP
    desc |= (tex & 0x7) << 12;   /* TEX[2:0] */
#else
    (void)tex;                   /* Unused when TEX not supported */
#endif
    desc |= (c & 0x1) << 3;      /* C bit */
    desc |= (b & 0x1) << 2;      /* B bit */

    /* Access permissions - repeated 4 times for large page */
    uint32_t ap, apx;
    get_access_permission_bits(attr->permission, &ap, &apx);
    desc |= (ap & 0x3) << 4;     /* AP[1:0] at [5:4] */
    desc |= (ap & 0x3) << 6;     /* AP[1:0] at [7:6] */
    desc |= (ap & 0x3) << 8;     /* AP[1:0] at [9:8] */
    desc |= (ap & 0x3) << 10;    /* AP[1:0] at [11:10] */
    /* Note: APX not supported on ARM926EJ-S */

    /* Execute Never - NOT supported for large pages even on ARMv6K */
    (void)attr->execute_never;

    return desc;
}

/**
 * @brief Build small page descriptor (4KB)
 */
static uint32_t build_small_page_descriptor(uint32_t phys_addr,
                                           const mmu_attributes_t *attr)
{
    uint32_t desc = phys_addr & 0xFFFFF000;  /* Physical address [31:12] */

    /* Extended Small Pages (type 0x3 with XN) only on ARMv6K and later */
#if MMU_FEATURE_XN
    if (attr->execute_never) {
        desc |= MMU_L2_DESC_TYPE_SMALL_XN;  /* Type 0x3 - Extended small page */
    } else {
        desc |= MMU_L2_DESC_TYPE_SMALL_PAGE;  /* Type 0x2 - Standard small page */
    }
#else
    /* ARM926EJ-S: Always use standard small page (type 0x2) */
    desc |= MMU_L2_DESC_TYPE_SMALL_PAGE;
    (void)attr->execute_never;  /* Not supported */
#endif

    /* Get memory type attributes */
    uint32_t tex, c, b;
    get_memory_attributes_bits(attr, &tex, &c, &b);
    /* ARM926EJ-S (ARMv5) does not support TEX; only set C/B */
#if MMU_FEATURE_TEX_REMAP
    desc |= (tex & 0x7) << 6;    /* TEX[2:0] */
#else
    (void)tex;                   /* Unused when TEX not supported */
#endif
    desc |= (c & 0x1) << 3;      /* C bit */
    desc |= (b & 0x1) << 2;      /* B bit */

    /* Access permissions */
    uint32_t ap, apx;
    get_access_permission_bits(attr->permission, &ap, &apx);
    desc |= (ap & 0x3) << 4;     /* AP[1:0] at [5:4] */
    desc |= (ap & 0x3) << 6;     /* AP[1:0] at [7:6] */
    desc |= (ap & 0x3) << 8;     /* AP[1:0] at [9:8] */
    desc |= (ap & 0x3) << 10;    /* AP[1:0] at [11:10] */
    /* Note: APX not supported on ARM926EJ-S */

    return desc;
}

/**
 * @brief Get memory attributes bits (TEX, C, B)
 */
static void get_memory_attributes_bits(const mmu_attributes_t *attr,
                                       uint32_t *tex, uint32_t *c, uint32_t *b)
{
    switch (attr->type) {
        case MMU_REGION_STRONGLY_ORDERED:
            *tex = 0; *c = 0; *b = 0;
            break;
        case MMU_REGION_DEVICE:
            *tex = 0; *c = 0; *b = 1;   /* Device: bufferable, not cacheable */
            break;
        case MMU_REGION_NORMAL_NC:
            /* No TEX on ARM926; non-cacheable normal: C=0,B=0 */
            *tex = 0; *c = 0; *b = 0;
            break;
        case MMU_REGION_NORMAL_WT:
            *tex = 0; *c = 1; *b = 0;   /* Write-through */
            break;
        case MMU_REGION_NORMAL_WB:
            *tex = 0; *c = 1; *b = 1;   /* Write-back */
            break;
        case MMU_REGION_NORMAL_WB_WA:
        #if MMU_FEATURE_TEX_REMAP
            *tex = 1; *c = 1; *b = 1;   /* WBWA requires TEX */
        #else
            *tex = 0; *c = 1; *b = 1;   /* Downgrade to WB on ARM926 */
        #endif
            break;
        default:
            *tex = 0; *c = 0; *b = 0;
            break;
    }
}

/**
 * @brief Get access permission bits (AP, APX)
 * Note: ARM926EJ-S does NOT support APX bit, only AP[1:0]
 */
static void get_access_permission_bits(mmu_permission_t perm,
                                       uint32_t *ap, uint32_t *apx)
{
    /* ARM926EJ-S only has AP[1:0], no APX support */
    *apx = 0;  /* Always 0 for ARM926EJ-S */

    /* Consider S/R control settings when selecting AP */
    const int S = MMU_SC_S_VALUE;
    const int R = MMU_SC_R_VALUE;

    switch (perm) {
        case MMU_PERM_NO_ACCESS:
            /* AP=00 yields NoAccess only if S=0,R=0; with S=0,R=1 it becomes RO/RO */
            if (S == 0 && R == 0) {
                *ap = 0;  /* 00: No access */
            } else {
                /* No strict no-access possible via AP with current S/R */
                *ap = 0;  /* Best effort; caller should set domain to NO_ACCESS */
            }
            break;
        case MMU_PERM_PRIV_RW:
            /* Priv RW, User no access: AP=01 irrespective of S/R */
            *ap = 1;
            break;
        case MMU_PERM_PRIV_RW_USER_RO:
            /* Priv RW, User RO: AP=10 irrespective of S/R */
            *ap = 2;
            break;
        case MMU_PERM_PRIV_RW_USER_RW:
            /* Full RW: AP=11 irrespective of S/R */
            *ap = 3;
            break;
        case MMU_PERM_PRIV_RO:
            /* Priv RO, User no access not achievable on ARM926 without APX; use closest */
            *ap = (S == 0 && R == 1) ? 0 : 1;
            /* If S=0,R=1 then AP=00 yields RO/RO, otherwise AP=01 gives RW/No access */
            break;
        case MMU_PERM_PRIV_RO_USER_RO:
            /* RO for all: AP=00 when S=0,R=1; else AP=2 gives RW/RO */
            *ap = (S == 0 && R == 1) ? 0 : 2;
            break;
        default:
            *ap = 0;
            break;
    }
}

/**
 * @brief Allocate or find L2 table for virtual address
 */
static mmu_l2_table_t* allocate_l2_table(mmu_context_t *ctx, uint32_t virt_addr, uint8_t domain)
{
    /* Check if L2 table already exists for this address */
    for (uint32_t i = 0; i < ctx->l2_table_count; i++) {
        if (ctx->l2_table_info[i].in_use &&
            ctx->l2_table_info[i].base_virt_addr == (virt_addr & 0xFFF00000)) {
            return &ctx->l2_tables[i];
        }
    }

    /* Allocate new L2 table */
    if (ctx->l2_table_count >= ctx->l2_table_max) {
        return NULL;
    }

    uint32_t idx = ctx->l2_table_count;
    mmu_l2_table_t *l2_table = &ctx->l2_tables[idx];
    ctx->l2_table_count++;

    /* Clear the L2 table entries */
    memset(l2_table, 0, sizeof(mmu_l2_table_t));

    /* Set metadata */
    ctx->l2_table_info[idx].base_virt_addr = virt_addr & 0xFFF00000;
    ctx->l2_table_info[idx].in_use = true;

    /* Update L1 table entry with domain */
    uint32_t l1_idx = L1_INDEX(virt_addr);
    uint32_t l2_phys = (uint32_t)l2_table;
    ctx->l1_table[l1_idx] = build_page_table_descriptor(l2_phys, domain);

    return l2_table;
}

/**
 * @brief Map region using 1MB sections
 */
static int map_with_sections(mmu_context_t *ctx, const mmu_region_t *region,
                             uint32_t *virt_addr, uint32_t *phys_addr,
                             uint32_t *remaining)
{
    uint32_t va = *virt_addr;
    uint32_t pa = *phys_addr;
    uint32_t size = *remaining;

    /* Skip if not aligned or too small */
    if (!IS_ALIGNED(va, MMU_SECTION_SIZE) ||
        !IS_ALIGNED(pa, MMU_SECTION_SIZE) ||
        size < MMU_SECTION_SIZE) {
        return MMU_ERR_SUCCESS;
    }

    /* Map one section */
    uint32_t l1_idx = L1_INDEX(va);
    uint32_t desc = build_section_descriptor(pa, &region->attributes);
    ctx->l1_table[l1_idx] = desc;

    va += MMU_SECTION_SIZE;
    pa += MMU_SECTION_SIZE;
    size -= MMU_SECTION_SIZE;

    *virt_addr = va;
    *phys_addr = pa;
    *remaining = size;

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Map region using 64KB large pages
 */
static int map_with_large_pages(mmu_context_t *ctx, const mmu_region_t *region,
                               uint32_t *virt_addr, uint32_t *phys_addr,
                               uint32_t *remaining)
{
    uint32_t va = *virt_addr;
    uint32_t pa = *phys_addr;
    uint32_t size = *remaining;

    /* Skip if not aligned or too small */
    if (!IS_ALIGNED(va, MMU_LARGE_PAGE_SIZE) ||
        !IS_ALIGNED(pa, MMU_LARGE_PAGE_SIZE) ||
        size < MMU_LARGE_PAGE_SIZE) {
        return MMU_ERR_SUCCESS;
    }

    /* Map one large page */
    /* Get or allocate L2 table */
    mmu_l2_table_t *l2_table = allocate_l2_table(ctx, va, region->attributes.domain);
    if (!l2_table) {
        return MMU_ERR_NO_L2_TABLE;
    }

    /* Large page occupies 16 consecutive L2 entries */
    uint32_t l2_idx = L2_INDEX(va);

    /* Check if we have enough space in L2 table for 16 entries */
    if (l2_idx + 16 > MMU_L2_TABLE_SIZE) {
        /* Not enough space, return without mapping */
        return MMU_ERR_SUCCESS;
    }

    uint32_t desc = build_large_page_descriptor(pa, &region->attributes);

    for (uint32_t i = 0; i < 16; i++) {
        l2_table->entries[l2_idx + i] = desc;
    }

    va += MMU_LARGE_PAGE_SIZE;
    pa += MMU_LARGE_PAGE_SIZE;
    size -= MMU_LARGE_PAGE_SIZE;

    *virt_addr = va;
    *phys_addr = pa;
    *remaining = size;

    return MMU_ERR_SUCCESS;
}

/**
 * @brief Map region using 4KB small pages
 */
static int map_with_small_pages(mmu_context_t *ctx, const mmu_region_t *region,
                               uint32_t *virt_addr, uint32_t *phys_addr,
                               uint32_t *remaining)
{
    uint32_t va = *virt_addr;
    uint32_t pa = *phys_addr;
    uint32_t size = *remaining;

    /* Map one small page at a time */
    if (size > 0) {
        /* Get or allocate L2 table */
        mmu_l2_table_t *l2_table = allocate_l2_table(ctx, va, region->attributes.domain);
        if (!l2_table) {
            return MMU_ERR_NO_L2_TABLE;
        }

        uint32_t l2_idx = L2_INDEX(va);
        uint32_t desc = build_small_page_descriptor(pa, &region->attributes);
        l2_table->entries[l2_idx] = desc;

        /* Advance by 4KB */
        va += MMU_SMALL_PAGE_SIZE;
        pa += MMU_SMALL_PAGE_SIZE;
        size = (size >= MMU_SMALL_PAGE_SIZE) ? (size - MMU_SMALL_PAGE_SIZE) : 0;
    }

    *virt_addr = va;
    *phys_addr = pa;
    *remaining = size;

    return MMU_ERR_SUCCESS;
}
