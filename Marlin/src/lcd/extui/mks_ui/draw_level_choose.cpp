#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>

#include "../../../gcode/queue.h"
#include "../../../inc/MarlinConfig.h"

extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *button_manual, *button_auto;
static lv_obj_t *label_manual, *label_auto;


enum
{
    ID_MANUAL = 1,
    ID_AUTO,
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
    case ID_MANUAL:
        uiCfg.leveling_first_time = true;
        lv_clear_level_choose();
        lv_draw_manualLevel();
        break;

    case ID_AUTO:
        lv_clear_level_choose();
        lv_draw_dialog(DIALOG_TYPE_IF_ZERO);
        // uiCfg.autoLeveling = 1;
        break;

    case ID_RETURN:
        lv_clear_level_choose();
        lv_draw_tool();
        break;
    }
}

void lv_draw_level_choose()
{
    scr = lv_screen_create(LEVELING_CHOOSE_UI, "");

    //标题栏
    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);

    //标题文本
    lv_obj_t *labelAuto = lv_label_create_empty(imgtop);
    lv_label_set_text(labelAuto, tool_menu.leveling);
    lv_obj_set_style(labelAuto, &label_dialog_white);
    lv_obj_set_pos(labelAuto, 69, 13);


    //手动调平按钮
    button_manual = lv_imgbtn_create(scr, "F:/bmp_manual.bin", 28, 127, event_handler, ID_MANUAL);
    //手动调平文本
    label_manual = lv_label_create_empty(button_manual);
    lv_obj_set_style(label_manual, &label_dialog_white);
    lv_label_set_text(label_manual, leveling_menu.manual);
    lv_obj_align(label_manual, button_manual, LV_ALIGN_CENTER, 20, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());


    //自动调平按钮
    button_auto = lv_imgbtn_create(scr, "F:/bmp_auto.bin", 252, 127, event_handler, ID_AUTO);
    //自动调平文本
    label_auto = lv_label_create_empty(button_auto);
    lv_obj_set_style(label_auto, &label_dialog_white);
    lv_label_set_text(label_auto, leveling_menu.auto_level);
    lv_obj_align(label_auto, button_auto, LV_ALIGN_CENTER, 20, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    //返回按钮
    lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", 6, 3, event_handler, ID_RETURN);
    lv_refr_now(lv_refr_get_disp_refreshing());

}

void lv_clear_level_choose()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

#endif
