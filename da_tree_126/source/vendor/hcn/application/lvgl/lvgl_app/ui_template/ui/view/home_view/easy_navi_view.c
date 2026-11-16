#include "easy_navi_view.h"
#include "aic_ui.h"
#include "animation_ctrl.h"
#include "mw_easy_navi.h"
#include "vehicle_data.h"

static easy_navi_data_t g_navigation_info = { 0 };
static bool g_mirror_navigation = false ;


void home_refresh_navi_cont(bool state)
{
    home_page_t *page = home_page_get(&ui_manager) ;
    VIEW_MEMBER_VALID(page, navi_cont) 

    if (state)
        lv_obj_remove_flag(navi_cont , LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(navi_cont , LV_OBJ_FLAG_HIDDEN);
}


void home_refresh_nav_road(char *value)
{
    home_page_t *page = home_page_get(&ui_manager) ;
    VIEW_MEMBER_VALID(page, road_label) 

    lv_label_set_text(road_label, value);

    return ;
}

void home_refresh_nav_image(char *value)
{
    home_page_t *page = home_page_get(&ui_manager) ;
    VIEW_MEMBER_VALID(page, navi_image) 

    lv_image_set_src(navi_image, value);

    return  ;
}


bool file_exists(const char *path) 
{
    lv_fs_file_t file;
    lv_fs_res_t res = lv_fs_open(&file, path, LV_FS_MODE_RD);
    if (res == LV_FS_RES_OK) {
        lv_fs_close(&file);
        return true;
    }
    return false;
}


void home_refresh_nav_distance(char *value)
{

    home_page_t *page = home_page_get(&ui_manager) ;
    VIEW_MEMBER_VALID(page, distance_label) 

    lv_label_set_text(distance_label, value);
    
    return  ;
}


bool parse_navigation_data(const easy_navi_data_t *_navigation_info)
{   
    if (_navigation_info == NULL) return false ;
    
    char format_buff[256] = {0};

    if (g_navigation_info.naviIcon != _navigation_info->naviIcon)
    {
        lv_snprintf(format_buff , sizeof(format_buff) , LVGL_IMAGE_PATH(navi/icon_nav_%02d.png), _navigation_info->naviIcon);

        if(file_exists(format_buff)){
            home_refresh_nav_image(format_buff);
            lv_log("naviIcon  changed = %d\n" , _navigation_info->naviIcon);
        }else{
            home_refresh_nav_image(LVGL_IMAGE_PATH(navi/icon_nav_01.png));
            lv_log("easy_navi image = navi/icon_nav_%02d.png is not exit" ,_navigation_info->naviIcon) ;
        }

    }
    
    if (g_navigation_info.roadRemainingDistance != _navigation_info->roadRemainingDistance)
    {
        memset(format_buff , 0x0 , sizeof(format_buff));

        if(_navigation_info->roadRemainingDistance != 0xFFFF)
        {
            if (_navigation_info->roadRemainingDistance > 1000 ) {
                int km_int = _navigation_info->roadRemainingDistance / 1000;
                int km_dec = (_navigation_info->roadRemainingDistance % 1000) / 100;
                lv_snprintf(format_buff , sizeof(format_buff) , "%d.%d km", km_int, km_dec);
            } else
                lv_snprintf(format_buff , sizeof(format_buff) , "%d m", _navigation_info->roadRemainingDistance);

        }

        home_refresh_nav_distance(format_buff);

        // printf("roadRemainingDistance  changed = %d\n" , _navigation_info->roadRemainingDistance);
    }
    
    // if (strcmp(g_navigation_info.nextRoad, _navigation_info->nextRoad))
    // {
    //     memset(format_buff , 0x0 , sizeof(format_buff));
    //     lv_snprintf(format_buff , sizeof(format_buff) , "%s", _navigation_info->nextRoad);
    //     home_refresh_nav_road(format_buff);

    //     // printf("nextRoad  changed = %s\n" ,_navigation_info->nextRoad);
    // }

    return true ;
}


void navigation_view_update()
{

    bool  _mirror_navigation = vehicle_get_mirror_navigation();
    if (g_mirror_navigation != _mirror_navigation)
    {
        home_refresh_animation_state(!_mirror_navigation);
        home_refresh_navi_cont(_mirror_navigation) ;

        g_mirror_navigation = _mirror_navigation ;

        // printf("mirror_navigation  changed = %s \n" , _mirror_state ? "open" : "close");
    }


    if (g_mirror_navigation)
    {
        const easy_navi_data_t *_navigation_info = get_easy_navi_info() ;

        if ( memcmp(_navigation_info , &g_navigation_info , sizeof(easy_navi_data_t)) )
        {
            if(parse_navigation_data(_navigation_info))
                memcpy(&g_navigation_info , _navigation_info , sizeof(easy_navi_data_t)) ;
        }
        
    }
        

    return ;

}
