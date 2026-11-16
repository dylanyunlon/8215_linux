/**
 * @file ui_variant.h
 * @brief Boot selection for the template and fallback UI variants.
 */

#ifndef UI_VARIANT_H
#define UI_VARIANT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HCN_UI_VARIANT_NONE = 0,
    HCN_UI_VARIANT_TEMPLATE,
    HCN_UI_VARIANT_FALLBACK
} hcn_ui_variant_t;

int hcn_ui_boot(void);
void hcn_ui_shutdown(void);
hcn_ui_variant_t hcn_ui_active_variant(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_VARIANT_H */
