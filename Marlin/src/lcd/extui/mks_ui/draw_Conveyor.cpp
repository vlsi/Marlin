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

// #include <TMCStepper.h>
#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>
#include "draw_endstop.h"

#include "../../../gcode/queue.h"
#include "../../../inc/MarlinConfig.h"
#include "../Marlin/src/module/planner.h"

extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *label_stop, *label_switch;
static lv_obj_t *buttonReturn, *btnfive, *btnauto, *btnConveyor_switch, *btnConveyor_left, *btnConveyor_right;
static lv_obj_t *img_Conveyor, *btn_stop, *buttonBack, *labelfive;

Conveyor_T convetor_t;

enum
{
    ID_Conveyor_ON_OFF = 1,
    ID_Conveyor_LEFT,
    ID_Conveyor_RIGHT,
    ID_Conveyor_STEP,
    ID_Conveyor_AUTO,
    ID_Conveyor_STOP,
    ID_Conveyor_RETURN,
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
    case ID_Conveyor_ON_OFF:
        convetor_t.open_flag = !convetor_t.open_flag;
        lv_switch_refr();
        break;

    case ID_Conveyor_LEFT:
        if(!convetor_t.open_flag) return;

        if(convetor_t.loop)
        {
            queue.inject_P(PSTR("G92 I0\nG1 I2000 F1000"));
        }
        else
        {
            queue.inject_P(PSTR("G92 I0\nG1 I50 F1000"));
        }
        break;

    case ID_Conveyor_RIGHT: 
        if(!convetor_t.open_flag) return;

        if(convetor_t.loop)
        {
            queue.inject_P(PSTR("G92 I0\nG1 I-2000 F1000"));
        }
        else
        {
            queue.inject_P(PSTR("G92 I0\nG1 I-50 F1000"));
        }
        break;
    
    case ID_Conveyor_AUTO:
        convetor_t.loop = true;
        lv_loop_refr();
        break;

    case ID_Conveyor_STOP:
        if(!convetor_t.open_flag) return;

        planner.quick_stop();
        queue.inject_P(PSTR("M84"));
        break;

    case ID_Conveyor_STEP:
        convetor_t.loop = false;
        lv_loop_refr();
        break;

    case ID_Conveyor_RETURN:
        lv_clear_Conveyor();
        lv_draw_tool();
        break;

    default:
        break;
    }

}

void lv_draw_Conveyor()
{
    scr = lv_screen_create(CONVEYOR_UI, "");

    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);

    //标题文本
    lv_obj_t *labelname = lv_label_create_empty(scr);
    lv_label_set_text(labelname, tool_menu.Conveyor);
    lv_obj_set_style(labelname, &label_dialog_white);
    lv_obj_set_pos(labelname, 19, 13);

    //开关按钮
    if(convetor_t.open_flag)
    {
        btnConveyor_switch = lv_imgbtn_create(scr, "F:/bmp_conveyor_on.bin", 50, 61, event_handler, ID_Conveyor_ON_OFF);
        label_switch = lv_label_create_empty(btnConveyor_switch);
        lv_label_set_text(label_switch, "ON");
        lv_label_set_style(label_switch, LV_LABEL_STYLE_MAIN, &label_dialog_white);
        lv_obj_align(label_switch, btnConveyor_switch, LV_ALIGN_CENTER, 0, 0);
    }
    else
    {
        btnConveyor_switch = lv_imgbtn_create(scr, "F:/bmp_conveyor_off.bin", 50, 61, event_handler, ID_Conveyor_ON_OFF);
        label_switch = lv_label_create_empty(btnConveyor_switch);
        lv_label_set_text(label_switch, "OFF");
        lv_label_set_style(label_switch, LV_LABEL_STYLE_MAIN, &label_dialog_white);
        lv_obj_align(label_switch, btnConveyor_switch, LV_ALIGN_CENTER, 0, 0);
    }

    //向前移动按钮
    btnConveyor_left = lv_imgbtn_create(scr, "F:/bmp_conveyor_left.bin", 25, 154, event_handler, ID_Conveyor_LEFT);

    //向后移动按钮
    btnConveyor_right = lv_imgbtn_create(scr, "F:/bmp_conveyor_right.bin", 375, 154, event_handler, ID_Conveyor_RIGHT);

    //停止按钮
    btn_stop = lv_imgbtn_create(scr, "F:/bmp_conveyor_stop.bin", 81, 239, event_handler, ID_Conveyor_STOP);
    label_stop = lv_label_create_empty(btn_stop);
    lv_label_set_text(label_stop, "Stop");
    lv_label_set_style(label_stop, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_obj_align(label_stop, btn_stop, LV_ALIGN_CENTER, 0, 0);

    //步进值按钮
    btnfive = lv_btn_create(scr, event_handler, ID_Conveyor_STEP, nullptr);
    lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
    lv_btn_set_style(btnfive, LV_BTN_STYLE_PR,  &tft_style_button_BLUE);
    lv_obj_set_size(btnfive, 96, 61);
    lv_obj_set_pos(btnfive, 226, 62);
    labelfive = lv_label_create_empty(btnfive);
    lv_label_set_text(labelfive, "50mm");
    lv_obj_set_style(labelfive, &label_dialog_white);
    lv_obj_align(labelfive, nullptr, LV_ALIGN_CENTER, 0, 0);

    //传动带图片
    img_Conveyor = lv_img_create(scr, nullptr);
    lv_img_set_src(img_Conveyor, "F:/img_conveyor.bin");
    lv_obj_set_pos(img_Conveyor, 128, 155);

    //循环按钮
    btnauto = lv_imgbtn_create(scr, "F:/png_loop_off.bin", event_handler, ID_Conveyor_AUTO);
    lv_obj_set_pos(btnauto, 337, 61);

    //返回按钮
    buttonBack = lv_imgbtn_create(scr, "F:/bmp_conveyor_return.bin", 261, 237, event_handler, ID_Conveyor_RETURN);
}

void lv_switch_refr()
{
    if(convetor_t.open_flag)
    {
        lv_label_set_text(label_switch, "ON");
        lv_imgbtn_set_src_both(btnConveyor_switch, "F:/bmp_conveyor_on.bin");
    }
    else
    {
        lv_label_set_text(label_switch, "OFF");
        lv_imgbtn_set_src_both(btnConveyor_switch, "F:/bmp_conveyor_off.bin");
    }
    lv_obj_refresh_ext_draw_pad(btnConveyor_switch);
    lv_refr_now(lv_refr_get_disp_refreshing());
}


void lv_loop_refr()
{
    if(!convetor_t.loop)
    {
        lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
        lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(btnfive, LV_BTN_STYLE_PR,  &tft_style_button_BLUE);
    }
    else
    {
        lv_imgbtn_set_src_both(btnauto, "F:/png_loop_on.bin");
        lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
        lv_btn_set_style(btnfive, LV_BTN_STYLE_PR,  &tft_style_button_BLUE_WHITE);
    }
    
 
    // lv_obj_refresh_ext_draw_pad(btnConveyor_switch);
    lv_refr_now(lv_refr_get_disp_refreshing());
}


void lv_clear_Conveyor()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}


#endif
