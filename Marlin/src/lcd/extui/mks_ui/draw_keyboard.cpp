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

#include "../../../inc/MarlinConfig.h"
#include "../../../gcode/queue.h"
#include "../../../module/temperature.h"
#include "../../../module/motion.h"
#include "../../../module/planner.h"
#include "../Marlin/src/gcode/gcode.h"

extern lv_group_t *g;
extern int32_t disp_layer_stop_num, disp_print_times;

static lv_obj_t *scr;
static lv_obj_t *numta, *ta;
static lv_obj_t *buttonReturn, *btngcodesend;

enum
{
    ID_GCADE_RETURN = 1,
    ID_GCADE_BREAK,
    ID_GCADE_TEMPSETTING,
    ID_GCADE_SEND,
};

#define LV_KB_CTRL_BTN_FLAGS (LV_BTNM_CTRL_NO_REPEAT | LV_BTNM_CTRL_CLICK_TRIG)

static const char *kb_map_lc[] = {"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                  "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                  "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                  LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const lv_btnm_ctrl_t kb_ctrl_lc_map[] = {
    LV_KB_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    LV_KB_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    LV_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, LV_KB_CTRL_BTN_FLAGS | 2};

static const char *kb_map_uc[] = {"1#", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", LV_SYMBOL_BACKSPACE, "\n",
                                  "abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", LV_SYMBOL_NEW_LINE, "\n",
                                  "_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":", "\n",
                                  LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const lv_btnm_ctrl_t kb_ctrl_uc_map[] = {
    LV_KB_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    LV_KB_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    LV_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, LV_KB_CTRL_BTN_FLAGS | 2};

static const char *kb_map_spec[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", LV_SYMBOL_BACKSPACE, "\n",
                                    "abc", "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
                                    "\\", "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'", "\n",
                                    LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const char *gcade_kb_map_spec[] = {"M", "G", "S", "P", "E", " ", "\n",
                                          "1", "2", "3", ".", "X", " ", "\n",
                                          "4", "5", "6", "-", "Y", "____", "\n",
                                          "7", "8", "9", "0", "Z", LV_SYMBOL_BACKSPACE, ""};
static const lv_btnm_ctrl_t gcade_kb_ctrl_lc_map[] = {
    5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5};

static const char *gtempsetting_kb_map_spec[] = {"1", "2", "3", "0", "\n",
                                                 "4", "5", "6", ".", "\n",
                                                 "7", "8", "9", LV_SYMBOL_BACKSPACE, ""};
static const lv_btnm_ctrl_t gtempsetting_kb_ctrl_lc_map[] = {
    5,
    5,
    5,
    5,
    5,
    5,
    5,
    5,
    5,
    5,
    5,
    5,
};

static const lv_btnm_ctrl_t kb_ctrl_spec_map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, LV_KB_CTRL_BTN_FLAGS | 2,
    LV_KB_CTRL_BTN_FLAGS | 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    LV_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, LV_KB_CTRL_BTN_FLAGS | 2};

static const lv_btnm_ctrl_t kb_ctrl_num_map[] = {
    1, 1, 1, LV_KB_CTRL_BTN_FLAGS | 2,
    1, 1, 1, LV_KB_CTRL_BTN_FLAGS | 2,
    1, 1, 1, 2,
    1, 1, 1, 1, 1};

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;

    uint32_t speed = 0;
    char gcodebuf[50];

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    switch (obj->mks_obj_id)
    {
    case ID_GCADE_RETURN:
        lv_clear_keyboard();
        switch (temp_value)
        {
        case layer_stop:
            lv_draw_layer_stop();
            break;

        case set_print_times:
            lv_draw_print_times();
            break;

        case pla_ext1:
        case pla_bed:
        case abs_ext1:
        case abs_bed:
            lv_draw_tempsetting();
            break;
        case fan_speed:
            draw_return_ui();
            break;
        case print_speed:
        case ext_speed:
            draw_return_ui();
            break;
        case ext_heat:
        case bed_heat:
            if (gCfgItems.from_flash_pic)
                flash_preview_begin = true;
            else
                default_preview_flg = true;
            lv_draw_printing();
            break;
        default:
            break;
        }
        break;
    case ID_GCADE_SEND:
        strcpy(gcodebuf, lv_ta_get_text(ta));
        gcode.process_subcommands_now(gcodebuf);
        // lv_ta_add_text(ta, gcodebuf);
        // if(!queue.ring_buffer.full(1))
        // {
        //   gcode.process_subcommands_now(gcodebuf);
        // }

        // if (!queue.ring_buffer.full(3)) {
        //       strcpy(gcodebuf, lv_ta_get_text(numta));
        //       // Hook anything that goes to the serial port
        //       MYSERIAL1.setHook(lv_serial_capt_hook, lv_eom_hook, 0);
        //       queue.enqueue_one_now(gcodebuf);
        // }
        break;
    case ID_GCADE_BREAK:
        lv_clear_keyboard();
        lv_draw_tool();
        break;
    case ID_GCADE_TEMPSETTING:
        switch (temp_value)
        {
        case layer_stop:
            disp_layer_stop_num = atoi(lv_ta_get_text(numta));
            break;
        case set_print_times:
            disp_print_times = atoi(lv_ta_get_text(numta));
            if(disp_print_times > 100)
                disp_print_times = 100;
            break;
        case pla_ext1:
            gCfgItems.PLA_EXT1 = atoi(lv_ta_get_text(numta));
            update_spi_flash();
            break;
        case pla_bed:
            gCfgItems.PLA_BED = atoi(lv_ta_get_text(numta));
            update_spi_flash();
            break;
        case abs_ext1:
            gCfgItems.ABS_EXT1 = atoi(lv_ta_get_text(numta));
            update_spi_flash();
            break;
        case abs_bed:
            gCfgItems.ABS_BED = atoi(lv_ta_get_text(numta));
            update_spi_flash();
            break;
        case fan_speed:
            speed = atoi(lv_ta_get_text(numta));
            if (speed > 100)
            {
                thermalManager.set_fan_speed(0, map(100, 0, 100, 0, 255));
                lv_ta_set_text(numta, "100");
            }
            else if (speed < 0)
            {
                thermalManager.set_fan_speed(0, map(0, 0, 100, 0, 255));
                lv_ta_set_text(numta, "0");
            }
            else
            {
                thermalManager.set_fan_speed(0, map(speed, 0, 100, 0, 255));
            }
            break;
        case print_speed:
            speed = atoi(lv_ta_get_text(numta));
            if (speed > MAX_PRINT_SPEED)
            {
                feedrate_percentage = MAX_PRINT_SPEED;
                lv_ta_set_text(numta, "200");
            }
            else if (speed < MIN_PRINT_SPEED)
            {
                feedrate_percentage = MIN_PRINT_SPEED;
                lv_ta_set_text(numta, "20");
            }
            else
            {
                feedrate_percentage = speed;
            }
            break;
        case ext_speed:
            speed = atoi(lv_ta_get_text(numta));
            if (speed > MAX_EXT_SPEED)
            {
                planner.flow_percentage[0] = MAX_EXT_SPEED;
                lv_ta_set_text(numta, "900");
            }
            else if (speed < MIN_EXT_SPEED)
            {
                planner.flow_percentage[0] = MIN_EXT_SPEED;
                lv_ta_set_text(numta, "0");
            }
            else
            {
                planner.flow_percentage[0] = speed;
            }
            planner.refresh_e_factor(0);
            break;
        case ext_heat:
            speed = atoi(lv_ta_get_text(numta));
            if (speed > MAX_EXT_HEAT)
            {
                thermalManager.temp_hotend[0].target = MAX_EXT_HEAT;
                thermalManager.start_watching_hotend(0);
                lv_ta_set_text(numta, "400");
            }
            else if (speed < MIN_EXT_HEAT)
            {
                thermalManager.temp_hotend[0].target = MIN_EXT_HEAT;
                thermalManager.start_watching_hotend(0);
                lv_ta_set_text(numta, "0");
            }
            else
            {
                thermalManager.temp_hotend[0].target = speed;
                thermalManager.start_watching_hotend(0);
            }
            break;
        case bed_heat:
            speed = atoi(lv_ta_get_text(numta));
            if (speed > MAX_BED_HEAT)
            {
                // thermalManager.degTargetBed() = MAX_BED_HEAT;
                thermalManager.temp_bed.target = MAX_BED_HEAT;
                thermalManager.start_watching_bed();

                lv_ta_set_text(numta, "120");
            }
            else if (speed < MIN_BED_HEAT)
            {
                // thermalManager.degTargetBed() = MIN_BED_HEAT;
                thermalManager.temp_bed.target = MIN_BED_HEAT;
                thermalManager.start_watching_bed();
                lv_ta_set_text(numta, "0");
            }
            else
            {
                // thermalManager.degTargetBed() = speed;
                thermalManager.temp_bed.target = speed;
                thermalManager.start_watching_bed();
            }
            break;
        default:
            break;
        }
        lv_clear_keyboard();
        draw_return_ui();
        break;
    default:
        break;
    }
}

static void lv_kb_event_cb(lv_obj_t *kb, lv_event_t event)
{
    if (event != LV_EVENT_VALUE_CHANGED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    lv_kb_ext_t *ext = (lv_kb_ext_t *)lv_obj_get_ext_attr(kb);
    const uint16_t btn_id = lv_btnm_get_active_btn(kb);
    if (btn_id == LV_BTNM_BTN_NONE)
        return;
    if (lv_btnm_get_btn_ctrl(kb, btn_id, LV_BTNM_CTRL_HIDDEN | LV_BTNM_CTRL_INACTIVE))
        return;
    if (lv_btnm_get_btn_ctrl(kb, btn_id, LV_BTNM_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT)
        return;

    const char *txt = lv_btnm_get_active_btn_text(kb);
    if (!txt)
        return;

    // Do the corresponding action according to the text of the button
    if (strcmp(txt, "abc") == 0)
    {
        lv_btnm_set_map(kb, kb_map_lc);
        lv_btnm_set_ctrl_map(kb, kb_ctrl_lc_map);
        return;
    }
    else if (strcmp(txt, "ABC") == 0)
    {
        lv_btnm_set_map(kb, kb_map_uc);
        lv_btnm_set_ctrl_map(kb, kb_ctrl_uc_map);
        return;
    }
    else if (strcmp(txt, "1#") == 0)
    {
        lv_btnm_set_map(kb, kb_map_spec);
        lv_btnm_set_ctrl_map(kb, kb_ctrl_spec_map);
        return;
    }
    else if (strcmp(txt, LV_SYMBOL_CLOSE) == 0)
    {
        if (kb->event_cb != lv_kb_def_event_cb)
        {
            lv_clear_keyboard();
            draw_return_ui();
        }
        else
        {
            lv_kb_set_ta(kb, nullptr); // De-assign the text area to hide its cursor if needed
            lv_obj_del(kb);
            return;
        }
        return;
    }
    else if (strcmp(txt, LV_SYMBOL_OK) == 0)
    {
        if (kb->event_cb != lv_kb_def_event_cb)
        {
            const char *ret_ta_txt = lv_ta_get_text(ext->ta);
            switch (keyboard_value)
            {
            case GCade:
                if (!queue.ring_buffer.full(3))
                {
                    // Hook anything that goes to the serial port
                    MYSERIAL1.setHook(lv_serial_capt_hook, lv_eom_hook, 0);
                    queue.enqueue_one_now(ret_ta_txt);
                }
#if ENABLED(MKS_WIFI_MODULE)
            case wifiName:
                memcpy(uiCfg.wifi_name, ret_ta_txt, sizeof(uiCfg.wifi_name));
                lv_clear_keyboard();
                draw_return_ui();
                break;
            case wifiPassWord:
                memcpy(uiCfg.wifi_key, ret_ta_txt, sizeof(uiCfg.wifi_name));
                lv_clear_keyboard();
                draw_return_ui();
                break;
            case wifiConfig:
                ZERO(uiCfg.wifi_name);
                memcpy((void *)uiCfg.wifi_name, wifi_list.wifiName[wifi_list.nameIndex], 32);

                ZERO(uiCfg.wifi_key);
                memcpy((void *)uiCfg.wifi_key, ret_ta_txt, sizeof(uiCfg.wifi_key));

                gCfgItems.wifi_mode_sel = STA_MODEL;

                package_to_wifi(WIFI_PARA_SET, nullptr, 0);

                public_buf_l[0] = 0xA5;
                public_buf_l[1] = 0x09;
                public_buf_l[2] = 0x01;
                public_buf_l[3] = 0x00;
                public_buf_l[4] = 0x01;
                public_buf_l[5] = 0xFC;
                public_buf_l[6] = 0x00;
                raw_send_to_wifi((uint8_t *)public_buf_l, 6);

                last_disp_state = KEYBOARD_UI;
                lv_clear_keyboard();
                wifi_tips_type = TIPS_TYPE_JOINING;
                lv_draw_wifi_tips();
                break;
#endif // MKS_WIFI_MODULE
            case autoLevelGcodeCommand:
                uint8_t buf[100];
                strncpy((char *)buf, ret_ta_txt, sizeof(buf));
                update_gcode_command(AUTO_LEVELING_COMMAND_ADDR, buf);
                lv_clear_keyboard();
                draw_return_ui();
                break;
            case GCodeCommand:
                if (!queue.ring_buffer.full(3))
                {
                    // Hook anything that goes to the serial port
                    MYSERIAL1.setHook(lv_serial_capt_hook, lv_eom_hook, 0);
                    queue.enqueue_one_now(ret_ta_txt);
                }
                lv_clear_keyboard();
                // draw_return_ui is called in the end of message hook
                break;
            default:
                break;
            }
        }
        else
            lv_kb_set_ta(kb, nullptr); // De-assign the text area to hide it cursor if needed
        return;
    }

    // Add the characters to the text area if set
    if (!ext->ta)
        return;

    if (strcmp(txt, "Enter") == 0 || strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
        lv_ta_add_char(ext->ta, '\n');
    else if (strcmp(txt, LV_SYMBOL_LEFT) == 0)
        lv_ta_cursor_left(ext->ta);
    else if (strcmp(txt, LV_SYMBOL_RIGHT) == 0)
        lv_ta_cursor_right(ext->ta);
    // else if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
    else if (strcmp(txt, "<--") == 0 || strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
        lv_ta_del_char(ext->ta);
    else if (strcmp(txt, " ") == 0)
    {
        if (keyboard_value != GCade)
            lv_ta_add_text(ext->ta, " ");
    }
    else if (strcmp(txt, "____") == 0)
    {
        if (keyboard_value == GCade)
            lv_ta_add_text(ext->ta, " ");
    }
    else if (strcmp(txt, "+/-") == 0)
    {
        uint16_t cur = lv_ta_get_cursor_pos(ext->ta);
        const char *ta_txt = lv_ta_get_text(ext->ta);
        if (ta_txt[0] == '-')
        {
            lv_ta_set_cursor_pos(ext->ta, 1);
            lv_ta_del_char(ext->ta);
            lv_ta_add_char(ext->ta, '+');
            lv_ta_set_cursor_pos(ext->ta, cur);
        }
        else if (ta_txt[0] == '+')
        {
            lv_ta_set_cursor_pos(ext->ta, 1);
            lv_ta_del_char(ext->ta);
            lv_ta_add_char(ext->ta, '-');
            lv_ta_set_cursor_pos(ext->ta, cur);
        }
        else
        {
            lv_ta_set_cursor_pos(ext->ta, 0);
            lv_ta_add_char(ext->ta, '-');
            lv_ta_set_cursor_pos(ext->ta, cur + 1);
        }
    }
    else
    {
        lv_ta_add_text(ext->ta, txt);
    }
}

void lv_draw_keyboard()
{
    scr = lv_screen_create(KEYBOARD_UI, "");

    // Create styles for the keyboard
    static lv_style_t rel_style, pr_style, label_style, ta_style;

    lv_style_copy(&rel_style, &lv_style_btn_rel);
    rel_style.body.radius = 0;
    rel_style.body.border.width = 1;
    rel_style.body.main_color = lv_color_make(0xA9, 0x62, 0x1D);
    rel_style.body.grad_color = lv_color_make(0xA7, 0x59, 0x0E);

    lv_style_copy(&pr_style, &lv_style_btn_pr);
    pr_style.body.radius = 0;
    pr_style.body.border.width = 1;
    pr_style.body.main_color = lv_color_make(0x72, 0x42, 0x15);
    pr_style.body.grad_color = lv_color_make(0x6A, 0x3A, 0x0C);

    lv_style_copy(&label_style, &tft_style_label_big_black); // tft_style_preHeat_label
    label_style.body.radius = 0;
    label_style.body.border.width = 1;

    lv_style_copy(&ta_style, &tft_style_label_big_black);
    ta_style.body.main_color = LV_COLOR_BLACK;
    ta_style.body.grad_color = LV_COLOR_BLACK;
    ta_style.text.color = LV_COLOR_WHITE;

    // Create a keyboard and apply the styles
    lv_obj_t *kb = lv_kb_create(scr, nullptr);
    lv_obj_set_event_cb(kb, lv_kb_event_cb);
    lv_kb_set_cursor_manage(kb, true);
    lv_kb_set_style(kb, LV_KB_STYLE_BG, &lv_style_transp_tight);

    if (keyboard_value == GCade)
    {
        lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &label_style);
        lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &label_style);
    }
    else
    {
        lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &rel_style);
        lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &pr_style);
    }

#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
    {
    }
#endif

    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    // create top img
    //  if(keyboard_value == GCade)
    //  {
    //    // lv_obj_set_size(kb, 480, 255);
    //    // lv_obj_set_pos(kb , 0, 65);

    //   // buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_GCADE_BREAK);
    //   // lv_obj_set_pos(buttonReturn, 6, 3);
    //   // lv_obj_t *labelname = lv_label_create_empty(scr);
    //   // lv_label_set_text(labelname, "Gcode");
    //   // lv_obj_set_pos(labelname, 63, 13);
    //   // btngcodesend = lv_imgbtn_create(scr, "F:/bmp_gcode_send.bin", event_handler, ID_GCADE_SEND);
    //   // lv_obj_align(btngcodesend,imgtop, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    //   // lv_refr_now(lv_refr_get_disp_refreshing());
    // }
    // else
    // {
    //   // lv_obj_align(kb, scr, LV_ALIGN_CENTER, 1, 85);
    // }

    // Create a text area. The keyboard will write here
    ta = lv_ta_create(scr, nullptr);
    lv_obj_set_size(ta, 238, 31);
    if (keyboard_value == GCade)
        lv_obj_set_style(ta, &ta_style);
    else
        // lv_ta_set_style(ta, LV_TA_STYLE_BG, &lv_style_pretty);
        lv_obj_set_style(ta, &lv_style_pretty);
    lv_obj_set_pos(ta, 130, 6);
    switch (keyboard_value)
    {
    // case wifiConfig:
    case autoLevelGcodeCommand:
        get_gcode_command(AUTO_LEVELING_COMMAND_ADDR, (uint8_t *)public_buf_m);
        public_buf_m[sizeof(public_buf_m) - 1] = '\0';
        lv_ta_set_text(ta, public_buf_m);
        break;
    case GCodeCommand:
        // Start with uppercase by default
        lv_btnm_set_map(kb, kb_map_uc);
        lv_btnm_set_ctrl_map(kb, kb_ctrl_uc_map);
        // Fallthrough
    case GCade:
        lv_btnm_set_map(kb, gcade_kb_map_spec);
        lv_btnm_set_ctrl_map(kb, gcade_kb_ctrl_lc_map);
    case GTempsetting:
    default:
        lv_ta_set_text(ta, "");
    }

    // Assign the text area to the keyboard
    lv_kb_set_ta(kb, ta);
    if (keyboard_value == GCade)
    {
        lv_obj_set_size(kb, 480, 255);
        lv_obj_set_pos(kb, 0, 65);

        lv_refr_now(lv_refr_get_disp_refreshing());
        buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_GCADE_BREAK);
        lv_obj_set_pos(buttonReturn, 6, 3);
        lv_obj_t *labelname = lv_label_create_empty(scr);
        lv_label_set_text(labelname, "Gcode");
        lv_obj_set_pos(labelname, 63, 13);
        btngcodesend = lv_imgbtn_create(scr, "F:/bmp_gcode_send.bin", event_handler, ID_GCADE_SEND);
        lv_obj_align(btngcodesend, imgtop, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
        // lv_refr_now(lv_refr_get_disp_refreshing());
        // lv_imgbtn_set_src_both(buttonReturn, "F:/bmp_preHeat_return.bin");
        // lv_refr_now(lv_refr_get_disp_refreshing());
        // lv_imgbtn_set_src_both(btngcodesend, "F:/bmp_gcode_send.bin");
    }
    else
    {
        lv_obj_align(kb, scr, LV_ALIGN_CENTER, 1, 85);
    }
}

void lv_draw_numkeyboard()
{
    scr = lv_screen_create(KEYBOARD_UI, "");

    // Create styles for the keyboard
    static lv_style_t rel_style, pr_style, label_style, label_blue_style;

    lv_style_copy(&rel_style, &lv_style_btn_rel);
    rel_style.body.radius = 0;
    rel_style.body.border.width = 1;
    rel_style.body.main_color = lv_color_make(0xA9, 0x62, 0x1D);
    rel_style.body.grad_color = lv_color_make(0xA7, 0x59, 0x0E);

    lv_style_copy(&pr_style, &lv_style_btn_pr);
    pr_style.body.radius = 0;
    pr_style.body.border.width = 1;
    pr_style.body.main_color = lv_color_make(0x72, 0x42, 0x15);
    pr_style.body.grad_color = lv_color_make(0x6A, 0x3A, 0x0C);

    lv_style_copy(&label_style, &tft_style_label_big_black);
    label_style.body.radius = 0;
    label_style.body.border.width = 1;

    lv_style_copy(&label_blue_style, &tft_style_preHeat_label);
    label_blue_style.body.main_color = LV_COLOR_BLUE;
    label_blue_style.body.grad_color = LV_COLOR_BLUE;
    label_blue_style.text.color = LV_COLOR_WHITE;

    // Create a keyboard and apply the styles
    lv_obj_t *kb = lv_kb_create(scr, nullptr);
    lv_obj_set_event_cb(kb, lv_kb_event_cb);
    lv_kb_set_cursor_manage(kb, true);
    lv_kb_set_style(kb, LV_KB_STYLE_BG, &lv_style_transp_tight);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &label_style);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &label_style);
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
    {
    }
#endif

    // create top img
    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_GCADE_RETURN);
    lv_obj_set_pos(buttonReturn, 6, 3);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    lv_obj_t *btnOK = lv_label_btn_create(scr, event_handler, ID_GCADE_TEMPSETTING);
    lv_obj_set_size(btnOK, 44, 42);
    lv_btn_set_style(btnOK, LV_BTN_STYLE_REL, &label_blue_style);
    lv_btn_set_style(btnOK, LV_BTN_STYLE_PR, &label_blue_style);
    lv_obj_align(btnOK, imgtop, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    lv_obj_t *labelOK = lv_label_create_empty(btnOK);
    lv_label_set_text(labelOK, "OK");
    lv_obj_align(labelOK, btnOK, LV_ALIGN_CENTER, 0, 0);

    // Create a text area. The keyboard will write here
    numta = lv_ta_create(scr, nullptr);
    lv_obj_set_size(numta, 98, 32);
    lv_obj_set_style(numta, &tft_style_label_big_black);
    char value[10] = {};
    lv_obj_align(numta, imgtop, LV_ALIGN_OUT_BOTTOM_MID, 30, 10);
    // lv_obj_set_pos(ta, 126, 6);
    // lv_obj_align(ta, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

    switch (keyboard_value)
    {
    case autoLevelGcodeCommand:
        get_gcode_command(AUTO_LEVELING_COMMAND_ADDR, (uint8_t *)public_buf_m);
        public_buf_m[sizeof(public_buf_m) - 1] = '\0';
        lv_ta_set_text(numta, public_buf_m);
        break;
    case GCodeCommand:
        // Start with uppercase by default
        lv_btnm_set_map(kb, kb_map_uc);
        lv_btnm_set_ctrl_map(kb, kb_ctrl_uc_map);
        // Fallthrough
    case GCade:
        lv_btnm_set_map(kb, gcade_kb_map_spec);
        lv_btnm_set_ctrl_map(kb, gcade_kb_ctrl_lc_map);
    case GTempsetting:
        lv_btnm_set_map(kb, gtempsetting_kb_map_spec);
        lv_btnm_set_ctrl_map(kb, gtempsetting_kb_ctrl_lc_map);
    default:
        switch (temp_value)
        {
        case layer_stop:
            sprintf(value, "%d", disp_layer_stop_num);
            lv_ta_set_text(numta, value);
            break;

        case set_print_times:
            sprintf(value, "%d", disp_print_times);
            lv_ta_set_text(numta, value);
            break;

        case pla_ext1:
            sprintf(value, "%d", gCfgItems.PLA_EXT1);
            lv_ta_set_text(numta, value);
            break;

        case pla_bed:
            sprintf(value, "%d", gCfgItems.PLA_BED);
            lv_ta_set_text(numta, value);
            break;

        case abs_ext1:
            sprintf(value, "%d", gCfgItems.ABS_EXT1);
            lv_ta_set_text(numta, value);
            break;

        case abs_bed:
            sprintf(value, "%d", gCfgItems.ABS_BED);
            lv_ta_set_text(numta, value);
            break;
        case fan_speed:
            sprintf_P(value, PSTR("%d"), (int)thermalManager.fanSpeedPercent(0));
            lv_ta_set_text(numta, value);
            break;
        case print_speed:
            sprintf_P(value, PSTR("%d"), feedrate_percentage);
            lv_ta_set_text(numta, value);
            break;
        case ext_speed:
            sprintf_P(value, PSTR("%d"), planner.flow_percentage[0]);
            lv_ta_set_text(numta, value);
            break;
        case ext_heat:
            sprintf_P(value, PSTR("%d"), thermalManager.degTargetHotend(0));
            lv_ta_set_text(numta, value);
            break;

        case bed_heat:
            sprintf_P(value, PSTR("%d"), thermalManager.degTargetBed());
            lv_ta_set_text(numta, value);
            break;

        default:
            lv_ta_set_text(numta, "");
            break;
        }
    }

    // Assign the text area to the keyboard
    lv_kb_set_ta(kb, numta);
    lv_obj_set_size(kb, 480, 225);
    lv_obj_set_pos(kb, 0, 94);

    // lv_refr_now(lv_refr_get_disp_refreshing());
    // lv_imgbtn_set_src_both(buttonReturn , "F:/bmp_preHeat_return.bin");
}

void lv_clear_keyboard()
{
    lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
