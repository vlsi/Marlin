/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>
#include "../../../gcode/queue.h"
#include "../../../gcode/gcode.h"
#include "../../../module/planner.h"
#include "../../../module/temperature.h"
#include "../../../module/motion.h"
#include "../../../sd/cardreader.h"
#include "../../../inc/MarlinConfig.h"
#if ENABLED(POWER_LOSS_RECOVERY)
  #include "../../../feature/powerloss.h"
#endif
extern lv_group_t *g;
extern uint8_t layer_stop_flag;
extern uint8_t layer_stop_flag;
extern int32_t save_layer_stop_num ;
extern int32_t save_disp_layer_stop_num;
extern uint8_t sel_id;
extern bool once_flag, gcode_preview_over;

static lv_obj_t *scr;
static lv_obj_t *btn_zoffset, *btn_fan;
static lv_obj_t *label_zoffset, *label_fan;
static lv_obj_t *btn_levelstop, *btn_op_filament;
static lv_obj_t *label_levelstop, *label_op_filament;
static lv_obj_t *imgbtn_break;
static lv_obj_t *btn_save, *btn_conveyor_num;
static lv_obj_t *label_save, *label_number, *label_conveyor, *label_mm;

extern lv_style_t save_btn_style;
extern lv_style_t num_btn_style;
extern lv_style_t label_btn_style;
extern lv_style_t label_big_black_style;

int32_t disp_print_times = 0;
int32_t print_times = 1;

enum
{
    ID_C_SAVE = 1,
    ID_C_NUM,
    ID_C_RETURN,
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
    case ID_C_NUM:
        temp_value = set_print_times;
        keyboard_value = GTempsetting;
        lv_clear_print_times();
        lv_draw_numkeyboard();
        break;
        
    case ID_C_SAVE:
        print_times = disp_print_times;
        start_printing();
        break;

    case ID_C_RETURN:
        lv_clear_print_times();
        lv_draw_print_file();
        break;
    }
}

void lv_draw_print_times()
{
    scr = lv_screen_create(PRINT_TIMES);
    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 42);
    lv_obj_set_pos(imgtop, 0, 0);

    //标题文本
    lv_obj_t *labelname = lv_label_create_empty(scr);
    lv_label_set_text(labelname, printing_MENU.print_times);
    lv_obj_set_style(labelname, &label_dialog_white);
    lv_obj_set_pos(labelname, 18, 10);

    //保存按钮样式
    lv_style_copy(&save_btn_style, &lv_style_scr);
    save_btn_style.body.main_color = LV_COLOR_MAKE(0x41, 0x85, 0xf7); // LV_COLOR_MAKE(0x41,0x85,0xf7);
    save_btn_style.body.grad_color = LV_COLOR_MAKE(0x41, 0x85, 0xf7); // LV_COLOR_MAKE(0x41,0x85,0xf7);
    save_btn_style.body.radius = 10;

    lv_style_copy(&num_btn_style, &lv_style_scr);
    num_btn_style.body.main_color = LV_COLOR_MAKE(0xF9, 0xF9, 0xF9); // LV_COLOR_MAKE(0x41,0x85,0xf7);
    num_btn_style.body.grad_color = LV_COLOR_MAKE(0xF9, 0xF9, 0xF9); // LV_COLOR_MAKE(0x41,0x85,0xf7);
    num_btn_style.body.border.width = 1;
    num_btn_style.body.border.part = LV_BORDER_FULL;
    num_btn_style.body.border.color = LV_COLOR_MAKE(0xC9, 0xC9, 0xC9); // LV_COLOR_MAKE(0x41,0x85,0xf7);
    num_btn_style.body.border.opa = 170;

    lv_style_copy(&label_btn_style, &tft_style_preHeat_label_BLACK);
    label_btn_style.text.color = LV_COLOR_BLACK;
    label_btn_style.text.font = &lv_font_roboto_28;

    lv_style_copy(&label_big_black_style, &tft_style_preHeat_label);
    label_big_black_style.text.font = &gb2312_puhui32;

    //time_txt
    label_conveyor = lv_label_create_empty(scr);
    lv_label_set_style(label_conveyor, LV_LABEL_STYLE_MAIN, &label_big_black_style);
    lv_label_set_text(label_conveyor, printing_MENU.times);
    lv_obj_set_pos(label_conveyor, 91, 108);


    //save_btn
    btn_save = lv_button_btn_create(scr, 66, 191, 135, 97, event_handler, ID_C_SAVE);
    lv_btn_set_style_both(btn_save, &save_btn_style);

    //save_txt
    label_save = lv_label_create_empty(btn_save);
    lv_label_set_style(label_save, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_label_set_text(label_save, printing_MENU.save);
    lv_obj_align(label_save, nullptr, LV_ALIGN_CENTER, 0, 0);

    //number_btn
    btn_conveyor_num = lv_button_btn_create(scr, 158, 86, 229, 80, event_handler, ID_C_NUM);
    lv_btn_set_style_both(btn_conveyor_num, &num_btn_style);

    imgbtn_break = lv_imgbtn_create(scr, "F:/print_times_return.bin", event_handler, ID_C_RETURN);
    lv_obj_set_pos(imgbtn_break, 279, 191);

    //移动数值
    char buf[30];
    ZERO(buf);
    sprintf(buf, "%ld", disp_print_times);
    label_number = lv_label_create_empty(btn_conveyor_num);
    lv_label_set_style(label_number, LV_LABEL_STYLE_MAIN, &label_btn_style);
    lv_label_set_text(label_number, buf);
    lv_obj_align(label_number, nullptr, LV_ALIGN_CENTER, 0, 0);

    lv_refr_now(lv_refr_get_disp_refreshing());
}












void lv_clear_print_times()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
