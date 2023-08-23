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
#include "../../../inc/MarlinConfig.h"

extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *buttonPoint1, *buttonPoint2, *buttonPoint3, *buttonPoint4;
static lv_obj_t *imgtop, *labelname, *imgbackground;
extern bool baby_step_set_data;

enum
{
    ID_M_POINT1 = 1,
    ID_M_POINT2,
    ID_M_POINT3,
    ID_M_POINT4,
    ID_M_POINT5,
    ID_M_Z,
    ID_MANUAL_RETURN,
    ID_M_ZOFFSET,
};

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;

    bool is_rb_full = true;

    if (!queue.ring_buffer.full(1))
    {
        is_rb_full = true;
    }
    else
    {
        is_rb_full = false;
    }

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    switch (obj->mks_obj_id)
    {
    case ID_M_POINT1 ... ID_M_POINT5:
        if (queue.ring_buffer.empty())
        {
            if (uiCfg.leveling_first_time)
            {
                uiCfg.leveling_first_time = false;
                baby_step_set_data = true;
                queue.inject_P(G28_STR);
            }
            const int ind = obj->mks_obj_id - ID_M_POINT1;
            sprintf_P(public_buf_l, PSTR("G28\nG1 Z5\nG1 X%d Y%d F4000\nG1 Z0"), gCfgItems.trammingPos[ind].x, gCfgItems.trammingPos[ind].y);
            queue.inject(public_buf_l);
            // send_m290();
        }
        break;
    case ID_M_Z:
        if (is_rb_full)
        {
            baby_step_set_data = true;
            queue.inject_P(PSTR("G28Z"));
        }
        break;

    case ID_MANUAL_RETURN:
        lv_clear_manualLevel();
        lv_draw_level_choose();
        break;

    case ID_M_ZOFFSET:
        lv_clear_manualLevel();
        lv_draw_baby_stepping();
        break;
    }
}

void lv_draw_manualLevel()
{
    scr = lv_screen_create(LEVELING_UI, "");

    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);

    lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_MANUAL_RETURN);
    lv_obj_set_pos(buttonReturn, 6, 3);

    lv_obj_t *labelname = lv_label_create_empty(imgtop);
    lv_label_set_text(labelname, leveling_menu.manual);
    lv_obj_set_style(labelname, &label_dialog_white);
    lv_obj_set_pos(labelname, 69, 13);

    imgbackground = lv_obj_create(scr, nullptr); 

    lv_obj_set_style(imgbackground, &tft_style_level_gray);
    lv_obj_set_size(imgbackground, 353, 233);
    lv_obj_set_pos(imgbackground, INTERVAL_V * 4 + 1, titleHeight * 2 - 8);
    lv_refr_now(lv_refr_get_disp_refreshing());

    // Create an Image button
    buttonPoint1 = lv_imgbtn_create(imgbackground, "F:/bmp_leveling1.bin", event_handler, ID_M_POINT1);
    lv_obj_align(buttonPoint1, imgbackground, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    buttonPoint2 = lv_imgbtn_create(imgbackground, "F:/bmp_leveling2.bin", event_handler, ID_M_POINT2);
    lv_obj_align(buttonPoint2, imgbackground, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    buttonPoint3 = lv_imgbtn_create(imgbackground, "F:/bmp_leveling3.bin", event_handler, ID_M_POINT3);
    lv_obj_align(buttonPoint3, imgbackground, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    buttonPoint4 = lv_imgbtn_create(imgbackground, "F:/bmp_leveling4.bin", event_handler, ID_M_POINT4);
    lv_obj_align(buttonPoint4, imgbackground, LV_ALIGN_IN_TOP_LEFT, 0, 0); // 110


    lv_obj_t *buttonZeroZ = lv_imgbtn_create(scr, "F:/bmp_zeroZ.bin", event_handler, ID_M_Z);
    lv_obj_set_pos(buttonZeroZ, 380, 62);


    lv_obj_t *buttonZoffset = lv_imgbtn_create(scr, "F:/bmp_level_zoffset.bin", event_handler, ID_M_ZOFFSET);
    lv_obj_set_pos(buttonZoffset, 380, 209);
}

void lv_clear_manualLevel()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
