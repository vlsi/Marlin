#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>

#include "../../../gcode/queue.h"
#include "../../../inc/MarlinConfig.h"
#include "../../../module/temperature.h"
#include "../../../feature/bedlevel/bedlevel.h"
extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *button_start, *button_zoffset, *buttonReturn;
static lv_obj_t *label_start;
static lv_obj_t *imgExt1;
static lv_obj_t *imgBed;
static lv_obj_t* labelBed;
static lv_obj_t* labelExt1;
static lv_obj_t* label_val[GRID_MAX_POINTS_X][GRID_MAX_POINTS_Y];
extern bool baby_step_set_data;

enum
{
    ID_START = 1,
    ID_ZOFFSET,
    ID_RETURN,
};

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    switch (obj->mks_obj_id)
    {
    case ID_START:
        lv_clear_auto_leveling();
        lv_draw_dialog(DIALOG_TYPE_IF_AUTO_LEVELING);
        //uiCfg.autoLeveling = 1;
        break;

    case ID_ZOFFSET:
        lv_clear_auto_leveling();
        lv_draw_baby_stepping();
        break;

    case ID_RETURN:
        lv_clear_auto_leveling();
        lv_draw_tool();
        break;
    }
}

void lv_draw_auto_leveling()
{
    scr = lv_screen_create(AUTO_LEVELING_UI, "");

    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);

    //标题文本
    lv_obj_t *labelAuto = lv_label_create_empty(imgtop);
    lv_label_set_text(labelAuto, leveling_menu.auto_level);
    lv_obj_set_style(labelAuto, &label_dialog_white);
    lv_obj_set_pos(labelAuto, 69, 13);

    
    //zoffset按钮
    button_zoffset = lv_imgbtn_create(scr, "F:/bmp_zoffset_2.bin", 16, 210, event_handler, ID_ZOFFSET);

    //开始调平按键
    button_start = lv_btn_create(scr, event_handler, ID_START, nullptr);
    lv_btn_set_style(button_start, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
    lv_btn_set_style(button_start, LV_BTN_STYLE_PR,  &tft_style_button_GREEN);
    lv_obj_set_size(button_start, 137, 78);
    lv_obj_set_pos(button_start, 20, 76);

    //开始调平文本
    label_start = lv_label_create_empty(scr);
    if(uiCfg.autoLeveling)
        lv_label_set_text(label_start, leveling_menu.waiting);
    else
        lv_label_set_text(label_start, leveling_menu.start);
    lv_obj_set_style(label_start, &label_dialog_white);
    lv_obj_align(label_start, button_start, LV_ALIGN_CENTER, 0, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    //4个点偏差
    uint8_t i = 0, j = 0;
    for(i=0; i<GRID_MAX_POINTS_X; i++)
    {
        for(j=0; j<GRID_MAX_POINTS_Y; j++)
        {
            label_val[i][j] = lv_label_create_empty(scr);
            lv_obj_set_style(label_val[i][j], &label_dialog_black);
            lv_obj_set_pos(label_val[i][j], 180+i*50, 90+j*30);
        }
    }
    lv_auto_level_val_refr();


    //返回按钮
    buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_RETURN);
    lv_obj_set_pos(buttonReturn, 6, 3);

    //右上角实时温度显示
    imgExt1 = lv_img_create(imgtop, NULL);
    lv_img_set_src(imgExt1, "F:/bmp_preHeat_Ext1.bin");
    lv_img_set_style(imgExt1 , LV_IMG_STYLE_MAIN , &tft_style_preHeat_scr);
    lv_obj_set_pos(imgExt1, 270, 8);

    imgBed = lv_img_create(imgtop, NULL);
    lv_img_set_src(imgBed, "F:/bmp_preHeat_Bed.bin");
    lv_obj_set_pos(imgBed, 380, 8);

    labelExt1 = lv_label_create_empty(imgtop);
    lv_label_set_style(labelExt1, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_obj_align(labelExt1, imgExt1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegHotend(0));
    lv_label_set_text(labelExt1, public_buf_l);

    labelBed = lv_label_create_empty(imgtop);
    lv_label_set_style(labelBed, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_obj_align(labelBed, imgBed, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegBed());
    lv_label_set_text(labelBed, public_buf_l);    
}


void lv_auto_level_temp_refr() 
{
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegHotend(0));
    lv_label_set_text(labelExt1, public_buf_l);
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegBed());
    lv_label_set_text(labelBed, public_buf_l);
    // lv_refr_now(lv_refr_get_disp_refreshing());
}




void lv_auto_level_val_refr()
{
    uint8_t i = 0, j = 0;

    for(i=0; i<GRID_MAX_POINTS_X; i++)
    {
        for(j=0; j<GRID_MAX_POINTS_Y; j++)
        {
            if(isnan(bedlevel.z_values[i][j]))
                lv_label_set_text_fmt(label_val[i][j],"%0.2f", "0.00");
            else
                lv_label_set_text_fmt(label_val[i][j],"%0.2f", bedlevel.z_values[i][j]);

            lv_refr_now(lv_refr_get_disp_refreshing());
        }
    }
    
}

void lv_clear_auto_leveling()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
