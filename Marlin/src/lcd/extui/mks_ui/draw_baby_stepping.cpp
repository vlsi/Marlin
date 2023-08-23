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
#include "../../../inc/MarlinConfig.h"

#if ENABLED(EEPROM_SETTINGS)
#include "../../../module/settings.h"
#endif

#if HAS_BED_PROBE
#include "../../../module/probe.h"
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
#include "../../../feature/bedlevel/bedlevel.h"
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
extern bed_mesh_t z_values;
#endif

extern lv_group_t *g;
static lv_obj_t *scr;

static lv_obj_t *labelV, *buttonV, *zOffsetText;
static lv_obj_t *z_step1, *z_step01, *z_step001;
static lv_obj_t *z_offset_scr, *z_offset_label;
bool baby_step_set_data = false;
extern lv_font_t lv_font_roboto_28;

enum
{
    ID_BABY_STEP_X_P = 1,
    ID_BABY_STEP_X_N,
    ID_BABY_STEP_Y_P,
    ID_BABY_STEP_Y_N,
    ID_BABY_STEP_Z_P,
    ID_BABY_STEP_Z_N,
    ID_BABY_STEP_DIST,
    ID_BABY_STEP_RETURN,
    ID_BABY_STEP_1,
    ID_BABY_STEP_01,
    ID_BABY_STEP_001,
};

static float babystep_dist = 0.01;
static uint8_t has_adjust_z = 0;
float z_offset_step = 0.01;

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;
    char baby_buf[30] = {0};
    char buf[20];
    char str_1[16];
    voice_button_on();
    hal.delay_ms(100);
    WRITE(BEEPER_PIN, LOW);
    // if((obj->mks_obj_id == ID_BABY_STEP_X_P) || )

    switch (obj->mks_obj_id)
    {
    case ID_BABY_STEP_X_P:
        sprintf_P(baby_buf, PSTR("M290 X%s"), dtostrf(babystep_dist, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(baby_buf));
        has_adjust_z = 1;
        break;
    case ID_BABY_STEP_X_N:
        sprintf_P(baby_buf, PSTR("M290 X%s"), dtostrf(-babystep_dist, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(baby_buf));
        has_adjust_z = 1;
        break;
    case ID_BABY_STEP_Y_P:
        sprintf_P(baby_buf, PSTR("M290 Y%s"), dtostrf(babystep_dist, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(baby_buf));
        has_adjust_z = 1;
        break;
    case ID_BABY_STEP_Y_N:
        sprintf_P(baby_buf, PSTR("M290 Y%s"), dtostrf(-babystep_dist, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(baby_buf));
        has_adjust_z = 1;
        break;
    case ID_BABY_STEP_Z_P:
        if (gCfgItems.add_full)
            break;
        if (z_offset_step < 0)
            z_offset_step = z_offset_step * -1;
        if ((z_offset_add + z_offset_step) > 10)
            break;
        sprintf_P(baby_buf, PSTR("M290 Z%s"), dtostrf(babystep_dist, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(baby_buf));
        has_adjust_z = 1;

        z_offset_add = z_offset_add + z_offset_step;
        if (z_offset_add >= 10)
        {
            gCfgItems.add_full = true;
        }
        else
        {
            gCfgItems.add_full = false;
        }
        gCfgItems.del_full = false;
        sprintf_P(buf, PSTR("%s"), dtostrf(z_offset_add, 1, 2, str_1));
        lv_label_set_text(z_offset_label, buf);
        gCfgItems.babystep_data = z_offset_add;
        update_spi_flash();
        break;

    case ID_BABY_STEP_Z_N:
        if (gCfgItems.del_full)
            break;
        if (z_offset_step > 0)
            z_offset_step = z_offset_step * -1;
        if ((z_offset_add + z_offset_step) < -10)
            break;
        sprintf_P(baby_buf, PSTR("M290 Z%s"), dtostrf(-babystep_dist, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(baby_buf));
        has_adjust_z = 1;
        z_offset_add = z_offset_add + z_offset_step;

        if (z_offset_add <= -10)
        {
            gCfgItems.del_full = true;
        }
        else
        {
            gCfgItems.del_full = false;
        }
        gCfgItems.add_full = false;
        sprintf_P(buf, PSTR("%s"), dtostrf(z_offset_add, 1, 2, str_1));
        lv_label_set_text(z_offset_label, buf);
        gCfgItems.babystep_data = z_offset_add;
        update_spi_flash();
        break;
    case ID_BABY_STEP_DIST:
        if (ABS((int)(100 * babystep_dist)) == 1)
            babystep_dist = 0.05;
        else if (ABS((int)(100 * babystep_dist)) == 5)
            babystep_dist = 0.1;
        else
            babystep_dist = 0.01;
        disp_baby_step_dist();
        break;
    case ID_BABY_STEP_RETURN:
        if (has_adjust_z == 1)
        {
            TERN_(EEPROM_SETTINGS, (void)settings.save());
            has_adjust_z = 0;
        }
        clear_cur_ui();
        draw_return_ui();
        break;
    case ID_BABY_STEP_1:
        babystep_dist = 1;
        z_offset_step = 1;
        lv_btn_set_style_both(z_step1, &z_step_re_style);
        lv_btn_set_style_both(z_step01, &z_step_style);
        lv_btn_set_style_both(z_step001, &z_step_style);
        break;
    case ID_BABY_STEP_01:
        babystep_dist = 0.1;
        z_offset_step = 0.1;
        lv_btn_set_style_both(z_step1, &z_step_style);
        lv_btn_set_style_both(z_step01, &z_step_re_style);
        lv_btn_set_style_both(z_step001, &z_step_style);
        break;
    case ID_BABY_STEP_001:
        babystep_dist = 0.01;
        z_offset_step = 0.01;
        lv_btn_set_style_both(z_step1, &z_step_style);
        lv_btn_set_style_both(z_step01, &z_step_style);
        lv_btn_set_style_both(z_step001, &z_step_re_style);
        break;
    }
}

void lv_draw_baby_stepping()
{
    scr = lv_screen_create(BABY_STEP_UI);
#if 0
    lv_big_button_create(scr, "F:/bmp_xAdd.bin", move_menu.x_add, INTERVAL_V, titleHeight, event_handler, ID_BABY_STEP_X_P);
    lv_big_button_create(scr, "F:/bmp_xDec.bin", move_menu.x_dec, INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BABY_STEP_X_N);
    lv_big_button_create(scr, "F:/bmp_yAdd.bin", move_menu.y_add, BTN_X_PIXEL + INTERVAL_V * 2, titleHeight, event_handler, ID_BABY_STEP_Y_P);
    lv_big_button_create(scr, "F:/bmp_yDec.bin", move_menu.y_dec, BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BABY_STEP_Y_N);
    lv_big_button_create(scr, "F:/bmp_zAdd.bin", move_menu.z_add, BTN_X_PIXEL * 2 + INTERVAL_V * 3, titleHeight, event_handler, ID_BABY_STEP_Z_P);
    lv_big_button_create(scr, "F:/bmp_zDec.bin", move_menu.z_dec, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BABY_STEP_Z_N);
    buttonV = lv_imgbtn_create(scr, nullptr, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_BABY_STEP_DIST);
    labelV = lv_label_create_empty(buttonV);
#if HAS_ROTARY_ENCODER
      if (gCfgItems.encoder_enable)
        lv_group_add_obj(g, buttonV);
#endif

    lv_big_button_create(scr, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BABY_STEP_RETURN);

    disp_baby_step_dist();

    zOffsetText = lv_label_create(scr, 290, TITLE_YPOS, nullptr);
    lv_label_set_style(zOffsetText, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
    disp_z_offset_value();
#else
    static lv_style_t step_label_style_back;

    lv_style_copy(&step_label_style_back, &tft_style_preHeat_label_BLACK);
    step_label_style_back.text.font = &lv_font_roboto_28;

    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_BABY_STEP_RETURN);
    lv_obj_set_pos(buttonReturn, 6, 3);

    lv_obj_t *labelname = lv_label_create_empty(imgtop);
    lv_label_set_text(labelname, leveling_menu.zoffset);
    lv_obj_set_style(labelname, &label_dialog_white);
    lv_obj_set_pos(labelname, 69, 13);

    lv_obj_t *buttonZadd = lv_imgbtn_create(scr, "F:/bmp_babystep_add.bin", event_handler, ID_BABY_STEP_Z_P);
    lv_obj_set_pos(buttonZadd, 23, 189);

    lv_obj_t *buttonZdel = lv_imgbtn_create(scr, "F:/bmp_babystep_del.bin", event_handler, ID_BABY_STEP_Z_N);
    lv_obj_set_pos(buttonZdel, 383, 189);

    z_step1 = lv_button_btn_create(scr, 114, 94, 82, 60, event_handler, ID_BABY_STEP_1);
    z_step01 = lv_button_btn_create(scr, 198, 94, 82, 60, event_handler, ID_BABY_STEP_01);
    z_step001 = lv_button_btn_create(scr, 284, 94, 82, 60, event_handler, ID_BABY_STEP_001);

    if (babystep_dist * 100 == 100)
        lv_btn_set_style_both(z_step1, &z_step_re_style);
    else
        lv_btn_set_style_both(z_step1, &z_step_style);

    if (babystep_dist * 100 == 10)
        lv_btn_set_style_both(z_step01, &z_step_re_style);
    else
        lv_btn_set_style_both(z_step01, &z_step_style);

    if (babystep_dist * 100 == 1)
        lv_btn_set_style_both(z_step001, &z_step_re_style);
    else
        lv_btn_set_style_both(z_step001, &z_step_style);

    lv_obj_t *labelz1 = lv_label_create_empty(z_step1);
    lv_label_set_style(labelz1, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_label_set_text(labelz1, "1");
    lv_obj_align(labelz1, nullptr, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *labelz01 = lv_label_create_empty(z_step01);
    lv_label_set_style(labelz01, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_label_set_text(labelz01, "0.1");
    lv_obj_align(labelz01, nullptr, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *labelz001 = lv_label_create_empty(z_step001);
    lv_label_set_style(labelz001, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_label_set_text(labelz001, "0.01");
    lv_obj_align(labelz001, nullptr, LV_ALIGN_CENTER, 0, 0);

    // lv_refr_now(lv_refr_get_disp_refreshing());

    z_offset_scr = lv_obj_create(scr, NULL);
    lv_obj_set_pos(z_offset_scr, 109, 167);
    lv_obj_set_size(z_offset_scr, 261, 115);
    lv_obj_set_style(z_offset_scr, &z_label_bg_style);

    // sprintf_P(buf, PSTR("%s"), dtostrf(probe.offset.z, 1, 3, str_1));
    z_offset_label = lv_label_create_empty(z_offset_scr);
    lv_label_set_style(z_offset_label, LV_LABEL_STYLE_MAIN, &step_label_style_back);
    disp_z_offset_value();
    lv_obj_align(z_offset_label, nullptr, LV_ALIGN_CENTER, 0, 0);

#endif
}

void disp_baby_step_dist()
{
    if ((int)(100 * babystep_dist) == 1)
        lv_imgbtn_set_src_both(buttonV, "F:/bmp_baby_move0_01.bin");
    else if ((int)(100 * babystep_dist) == 5)
        lv_imgbtn_set_src_both(buttonV, "F:/bmp_baby_move0_05.bin");
    else if ((int)(100 * babystep_dist) == 10)
        lv_imgbtn_set_src_both(buttonV, "F:/bmp_baby_move0_1.bin");

    if (gCfgItems.multiple_language)
    {
        if ((int)(100 * babystep_dist) == 1)
        {
            lv_label_set_text(labelV, move_menu.step_001mm);
            lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
        else if ((int)(100 * babystep_dist) == 5)
        {
            lv_label_set_text(labelV, move_menu.step_005mm);
            lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
        else if ((int)(100 * babystep_dist) == 10)
        {
            lv_label_set_text(labelV, move_menu.step_01mm);
            lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
    }
}

void disp_z_offset_value()
{
    char buf[20];
    char str_1[16];
#if HAS_BED_PROBE
    sprintf_P(buf, PSTR("%s"), dtostrf(z_offset_add, 1, 2, str_1));
#else
    // strcpy_P(buf, PSTR("Offset Z: 0 mm"));
    sprintf_P(buf, PSTR("%s"), dtostrf(z_offset_add, 1, 2, str_1));
#endif
    lv_label_set_text(z_offset_label, buf);
    lv_obj_align(z_offset_label, nullptr, LV_ALIGN_CENTER, 0, 0);
}

void lv_clear_baby_stepping()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

void send_m290()
{
    char buf[20];
    char str_1[16];
    ZERO(buf);
    ZERO(str_1);
    if (baby_step_set_data)
    {
        sprintf_P(buf, PSTR("M290 Z%s"), dtostrf(z_offset_add, 1, 3, str_1));
        gcode.process_subcommands_now(PSTR(buf));
        baby_step_set_data = false;
    }
}

#endif // HAS_TFT_LVGL_UI
