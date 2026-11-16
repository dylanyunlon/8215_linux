#include "animation_ctrl.h"
#include "lvgl.h"
#include "ui_objects.h"
#include <stdio.h>
#include <stdlib.h>
#include "definition.h"
#include "aic_ui.h"
#include <string.h>
#include <dirent.h>
#include "ui_util.h"
#include "selfcheck.h"

#define ANI_IMAGE_COUNT 20
#define ANI_IMAGE_PATH  256
#define ANI_MAX_FRAMES  160
/* Frames per second. HCN_APPLICATION_ANI_FPS overrides it at startup (1..60) so the
 * animation CPU cost can be measured on the board without touching lvgl. */
#define ANI_DEFAULT_FPS 20
#define ANI_MIN_FPS     1
#define ANI_MAX_FPS     60

lv_timer_t *anim_timer = NULL ;

cartoon_state_e current_theme = CARTOON_STYLE_1 ;
cartoon_theme_e current_state = CARTOON_STATE_SAYHELLO   ;
static int current_image_index = 0;
static int max_image_count = 0 ;
static int len = 0 ;
static const char* state_path[] = {"cartoon_sayHello/" ,"cartoon_ready/" , "cartoon_running/"} ;
static const char* theme_path[] = {"style_1/" , "style_2/" , "style_3/" } ;
static char  path[ANI_IMAGE_PATH] = {0};

/* ---------------------------------------------------------------------------
 * One lv_image per cartoon frame, instead of swapping the src of a single
 * object: the GL draw unit keys its texture cache on the image draw descriptor,
 * which differs only by the src pointer, so a reused object can collapse to one
 * cache key across all frames and reuse a stale texture.
 * Each frame's src string is allocated once and never freed, making every
 * frame a stable, distinct cache key. Only the current frame is visible; the
 * others carry LV_OBJ_FLAG_HIDDEN and are skipped by the renderer.
 * --------------------------------------------------------------------------- */
static lv_obj_t * frame_objs[ANI_MAX_FRAMES];
static lv_obj_t * cur_frame = NULL ;
/* The home page owns the initial visibility. A leave callback hides the frames;
 * the next enter callback restores them. */
static bool ani_visible = true ;

static int ani_trace_on(void)
{
    static int on = -1 ;

    if(on < 0) {
        const char * env = getenv("HCN_APPLICATION_TRACE") ;
        on = (env && env[0] && env[0] != '0') ? 1 : 0 ;
    }

    return on ;
}

static int ani_fps(void)
{
    static int fps = 0 ;
    const char * env = NULL ;
    int v = 0 ;

    if(fps != 0) return fps ;

    fps = ANI_DEFAULT_FPS ;
    env = getenv("HCN_APPLICATION_ANI_FPS") ;
    if((env != NULL) && (env[0] != 0)) {
        v = atoi(env) ;
        if(v < ANI_MIN_FPS) v = ANI_MIN_FPS ;
        if(v > ANI_MAX_FPS) v = ANI_MAX_FPS ;
        fps = v ;
    }

    return fps ;
}

static void ani_frame_path(int index)
{
    lv_snprintf(path + len, ANI_IMAGE_PATH - len - 1, "%03d.png" , index);
}

static lv_obj_t * ani_frame_obj(lv_obj_t * anchor , int index)
{
    lv_obj_t * parent = NULL ;
    lv_obj_t * obj = NULL ;
    const void * src = NULL ;

    if(index < 0 || index >= max_image_count) return NULL ;

    if((frame_objs[index] != NULL) && !lv_obj_is_valid(frame_objs[index])) {
        frame_objs[index] = NULL ;      /* page was rebuilt: drop the stale pointer */
    }

    if(frame_objs[index] != NULL) {
        /* theme/state switch reuses the object, only the file name changed.
         * lv_image_set_src frees the old string and stores a fresh one, so the
         * per-frame key stays unique. */
        src = lv_image_get_src(frame_objs[index]) ;
        ani_frame_path(index);
        if((src == NULL) || (strcmp((const char *)src , path) != 0)) {
            lv_image_set_src(frame_objs[index] , path) ;
        }
        return frame_objs[index] ;
    }

    parent = lv_obj_get_parent(anchor) ;
    if(parent == NULL) return NULL ;

    obj = lv_image_create(parent) ;
    if(obj == NULL) return NULL ;

    /* same geometry as the anchor created in home_page.c */
    lv_image_set_pivot(obj , 50 , 50);
    lv_image_set_offset_x(obj , 0);
    lv_image_set_offset_y(obj , 0);
    lv_image_set_rotation(obj , 0);
    lv_obj_set_pos(obj , lv_obj_get_x(anchor) , lv_obj_get_y(anchor));

    ani_frame_path(index);
    lv_image_set_src(obj , path) ;
    lv_obj_add_flag(obj , LV_OBJ_FLAG_HIDDEN);

    frame_objs[index] = obj ;

    return frame_objs[index] ;
}

/* HCN_APPLICATION_TRACE=1: every 5 s report how many frame objects exist and how many
 * distinct src pointers they hold. All frames unique and stable == the fix is
 * in effect; a count of 1..2 means the draw descriptor key aliases again. */
static void ani_trace_tick(void)
{
    static uint32_t last = 0 ;
    static uint32_t ticks = 0 ;
    static uint32_t ticks_last = 0 ;
    uint32_t now = 0 ;
    uint32_t achieved = 0 ;
    uint32_t elapsed = 0 ;
    const void * seen[ANI_MAX_FRAMES] ;
    int created = 0 ;
    int distinct = 0 ;
    int i = 0 ;
    int j = 0 ;

    ticks++ ;

    if(ani_trace_on() == 0) return ;
    now = lv_tick_get() ;
    if(now - last < 5000u) return ;
    elapsed = now - last ;
    achieved = (elapsed > 0) ? ((ticks - ticks_last) * 1000u / elapsed) : 0u ;
    ticks_last = ticks ;
    last = now ;

    for(i = 0 ; i < ANI_MAX_FRAMES ; i++) {
        if(frame_objs[i] == NULL) continue ;
        created++ ;
        seen[distinct] = lv_image_get_src(frame_objs[i]) ;
        for(j = 0 ; j < distinct ; j++) {
            if(seen[j] == seen[distinct]) break ;
        }
        if(j == distinct) distinct++ ;
    }

    fprintf(stderr , "[hcn_application] ani: fps=%d/%u ticks=%u objs=%d/%d distinct_src=%d idx=%d\n" ,
            ani_fps() , (unsigned)achieved , (unsigned)ticks , created , max_image_count , distinct , current_image_index);
}

static void on_animation_timer(struct _lv_timer_t * timer)
{
    // if (checkself_get_state() != CHECK_STATE_FINISHED /*|| get_demonstration_state()*/ ) 
    //     return  ;

    home_page_t *page = home_page_get(&ui_manager) ;
    lv_obj_t * next = NULL ;
    VIEW_MEMBER_VALID(page, cartoon_image)

    next = ani_frame_obj(cartoon_image , current_image_index) ;
    if(next == NULL) {
        ani_trace_tick();
        return ;
    }

    if((cur_frame != NULL) && (cur_frame != next)) {
        lv_obj_add_flag(cur_frame , LV_OBJ_FLAG_HIDDEN);
    }
    cur_frame = next ;
    if(ani_visible) {
        lv_obj_remove_flag(cur_frame , LV_OBJ_FLAG_HIDDEN);
    }

    ani_trace_tick();

    current_image_index = (current_image_index + 1) % max_image_count ;
}

void animation_init() 
{
    len = lv_snprintf(path , ANI_IMAGE_PATH - 1 , LVGL_IMAGE_PATH(%s%s),\
    state_path[current_state], theme_path[current_theme] ) ;

    max_image_count = count_png_files(path) ;
    current_image_index = 0 ;
    // len = strlen(path) ;

    if ((max_image_count == 0 )|| (max_image_count == -1))
    {
        LV_LOG_ERROR("not found *.png picture\n max_image_count = %d" ,max_image_count);
        return ;
    }

    if(max_image_count > ANI_MAX_FRAMES) {
        LV_LOG_WARN("cartoon frames %d exceeds %d, clamped" , max_image_count , ANI_MAX_FRAMES);
        max_image_count = ANI_MAX_FRAMES ;
    }
    
    
    if (anim_timer == NULL)
        anim_timer = lv_timer_create(on_animation_timer , (uint32_t)(1000 / ani_fps()) , NULL) ;

    return ;
}


void animation_resume()
{
    if (anim_timer == NULL)
        return ;
    
    lv_timer_resume(anim_timer) ;
}


void animation_stop() 
{
    if (anim_timer == NULL)
        return ;
    
    lv_timer_pause(anim_timer) ;
    
    current_image_index = 0 ;
    return ;
}

void home_refresh_animation_state(bool flag) 
{
    home_page_t *page = home_page_get(&ui_manager) ;
    VIEW_MEMBER_VALID(page, cartoon_image);

    ani_visible = flag ;

    if(flag){
        lv_obj_remove_flag(cartoon_image , LV_OBJ_FLAG_HIDDEN);
        animation_resume();
    }
    else{
        lv_obj_add_flag(cartoon_image , LV_OBJ_FLAG_HIDDEN);
        animation_stop();
    }

    if(cur_frame != NULL) {
        if(ani_visible)
            lv_obj_remove_flag(cur_frame , LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(cur_frame , LV_OBJ_FLAG_HIDDEN);
    }

    return ;
}
 
void animation_set_theme(cartoon_theme_e theme) 
{
    if (theme > CARTOON_STYLE_3)
        return ;
    
    current_theme = theme ;
    animation_init() ;
}
 
void animation_set_state(cartoon_state_e state) 
{
    if (state > CARTOON_STATE_RUNNING)
        return ;
    
    current_state = state ;
    animation_init() ;
}
