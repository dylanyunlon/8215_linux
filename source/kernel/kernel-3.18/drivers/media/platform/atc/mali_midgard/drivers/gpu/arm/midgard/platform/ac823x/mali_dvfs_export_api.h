#ifndef MALI_DVFS_EXPORT_API
#define MALI_DVFS_EXPORT_API

void get_avaliable_clk_levels(int *s, int *e);
/*avaliable clk level 0-5*/
int set_current_max_clk_level(int clk);
int get_current_max_clk_level(void);
int get_current_clk_level(void);

#endif
