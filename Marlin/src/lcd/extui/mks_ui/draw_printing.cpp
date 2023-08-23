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
#include "../../../module/planner.h"

#include "../../../MarlinCore.h" // for marlin_state
#include "../../../module/temperature.h"
#include "../../../module/motion.h"
#include "../../../sd/cardreader.h"
#include "../../../gcode/queue.h"
#include "../../../gcode/gcode.h"
#include "../../../inc/MarlinConfig.h"

#if ENABLED(POWER_LOSS_RECOVERY)
#include "../../../feature/powerloss.h"
#endif

#if BOTH(LCD_SET_PROGRESS_MANUALLY, USE_M73_REMAINING_TIME)
#include "../../marlinui.h"
#endif

extern lv_group_t *g;
extern uint8_t sel_id;
static lv_obj_t *scr;
static lv_obj_t *labelExt1, *labelFan, *labelZpos, *labelTime, *labelSpeed, *labelUp, *labelFlowSpeed, *labelHeat;
// static lv_obj_t *labelPause, *labelStop, *labelOperat;
static lv_obj_t *bar1, *bar1ValueText, *gcodeName;
static lv_obj_t *buttonPause, *buttonOperat, *buttonStop, *buttonExt1, *buttonFanstate, *buttonZpos, *buttonSpeed, *buttonUp, *buttonMore;

#if HAS_MULTI_EXTRUDER
static lv_obj_t *labelExt2;
static lv_obj_t *buttonExt2;
#endif

#if HAS_HEATED_BED
static lv_obj_t *labelBed;
static lv_obj_t *buttonBedstate;
#endif

enum
{
    ID_PAUSE = 1,
    ID_STOP,
    ID_OPTION,
    ID_TEMP_EXT,
    ID_TEMP_BED,
    ID_BABYSTEP,
    ID_FAN,
    ID_MOVSPEED,
    ID_PRINT_SPEED_CHANGE,
    ID_FLOW_SPEED_CHANGE,
    ID_MORE,
};

bool once_flag; // = false
extern bool flash_preview_begin, default_preview_flg, gcode_preview_over;
extern uint32_t To_pre_view;
extern int32_t print_times;
uint8_t fanValue = 0;
uint8_t layer_stop_flag = LAYER_STOP_NO_TRIGGERED;

void paused_print()
{
#if ENABLED(SDSUPPORT)
    card.pauseSDPrint();
    stop_print_time();
    uiCfg.print_state = PAUSING;
    if (disp_state == PRINTING_UI)
    {
        lv_imgbtn_set_src_both(buttonPause, "F:/bmp_continue.bin");
    }
#endif
}

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    if (gcode_preview_over)
        return;
    switch (obj->mks_obj_id)
    {
    case ID_PAUSE:
        if (uiCfg.print_state == WORKING)
        {
// lv_clear_printing();
// lv_draw_dialog(DIALOG_TYPE_PAUSE);
#if ENABLED(SDSUPPORT)
            card.pauseSDPrint();
            stop_print_time();
            uiCfg.print_state = PAUSING;
#endif
            lv_imgbtn_set_src_both(buttonPause, "F:/bmp_continue.bin");
        }
        else if (uiCfg.print_state == PAUSED)
        {
            if (layer_stop_flag == LAYER_STOP_TRIGGERED)
            {
                layer_stop_flag = LAYER_STOP_CANNOT_TRIGGER;
            }
            uiCfg.print_state = RESUMING;
            start_print_time();
            lv_imgbtn_set_src_both(buttonPause, "F:/bmp_pause.bin");
        }
#if ENABLED(POWER_LOSS_RECOVERY)
        else if (uiCfg.print_state == REPRINTING)
        {
            uiCfg.print_state = REPRINTED;
            print_time.minutes = recovery.info.print_job_elapsed / 60;
            print_time.seconds = recovery.info.print_job_elapsed % 60;
            print_time.hours = print_time.minutes / 60;
            lv_imgbtn_set_src_both(buttonPause, "F:/bmp_pause.bin");
        }
#endif
        break;
    case ID_STOP:
        lv_clear_printing();
        lv_draw_dialog(DIALOG_TYPE_STOP);
        break;
    case ID_OPTION:
        lv_clear_printing();
        lv_draw_operation();
        break;
    case ID_TEMP_EXT:
        // uiCfg.curTempType = 0;
        // lv_clear_printing();
        // lv_draw_preHeat();
        temp_value = ext_heat;
        keyboard_value = GTempsetting;
        lv_clear_printing();
        lv_draw_numkeyboard();
        break;
    case ID_TEMP_BED:
        // uiCfg.curTempType = 1;
        // lv_clear_printing();
        // lv_draw_preHeat();
        temp_value = bed_heat;
        keyboard_value = GTempsetting;
        lv_clear_printing();
        lv_draw_numkeyboard();
        break;
    case ID_BABYSTEP:
        lv_clear_printing();
        lv_draw_baby_stepping();
        break;
    case ID_FAN:
        // switch(fanValue)
        // {
        //   case 0: fanValue = 50; break;
        //   case 50: fanValue = 100; break;
        //   case 100: fanValue = 0; break;
        // }
        // thermalManager.set_fan_speed(0, map(fanValue, 0, 100, 0, 255));
        temp_value = fan_speed;
        keyboard_value = GTempsetting;
        lv_clear_printing();
        lv_draw_numkeyboard();
        break;
    case ID_MOVSPEED:
        switch (uiCfg.extruSpeed)
        {
        case uiCfg.eSpeedL:
            uiCfg.extruSpeed = uiCfg.eSpeedN;
            break;
        case uiCfg.eSpeedN:
            uiCfg.extruSpeed = uiCfg.eSpeedH;
            break;
        case uiCfg.eSpeedH:
            uiCfg.extruSpeed = uiCfg.eSpeedL;
            break;
        }
        break;
    case ID_PRINT_SPEED_CHANGE:
        temp_value = print_speed;
        keyboard_value = GTempsetting;
        lv_clear_printing();
        lv_draw_numkeyboard();

        break;
    case ID_FLOW_SPEED_CHANGE:
        temp_value = ext_speed;
        keyboard_value = GTempsetting;
        lv_clear_printing();
        lv_draw_numkeyboard();
        break;
    case ID_MORE:
        lv_clear_printing();
        lv_draw_operation();
        break;
    default:
        break;
    }
}

void lv_draw_printing()
{
    static lv_style_t label_big_black;

    lv_style_copy(&label_big_black, &tft_style_preHeat_label);
    label_big_black.text.font = &gb2312_puhui32;

    disp_state_stack._disp_index = 0;
    ZERO(disp_state_stack._disp_state);
    scr = lv_screen_create(PRINTING_UI);

    labelHeat = lv_label_create_empty(scr);
    lv_label_set_text(labelHeat, printing_MENU.heating);
    lv_label_set_style(labelHeat, LV_LABEL_STYLE_MAIN, &label_big_black); // tft_style_preHeat_label_BLACK
    lv_obj_set_pos(labelHeat, 10, 10);

    lv_obj_t *imgflowspeed = lv_img_create(scr, nullptr);
    lv_img_set_src(imgflowspeed, "F:/bmp_prints_state.bin");
    lv_obj_set_pos(imgflowspeed, 110, 4);
    labelFlowSpeed = lv_label_create_empty(scr);
    // lv_label_set_text(labelFlowSpeed, "000.0mm/s");
    dtostrf(feedrate_mm_s, 1, 1, public_buf_l);
    strcat(public_buf_l, "mm/s");
    lv_label_set_text(labelFlowSpeed, public_buf_l);
    lv_label_set_style(labelFlowSpeed, LV_LABEL_STYLE_MAIN, &label_big_black);
    // lv_obj_set_pos(labelFlowSpeed , 160, 12);
    lv_obj_align(labelFlowSpeed, imgflowspeed, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    lv_obj_t *buttonTime = lv_img_create(scr, nullptr);
    lv_img_set_src(buttonTime, "F:/bmp_time_state.bin");
    lv_obj_set_pos(buttonTime, 260, 10);

    labelTime = lv_label_create_empty(scr);
    sprintf_P(public_buf_l, PSTR("%d%d:%d%d:%d%d"), print_time.hours / 10, print_time.hours % 10, print_time.minutes / 10, print_time.minutes % 10, print_time.seconds / 10, print_time.seconds % 10);
    strcpy(print_finish_time, public_buf_l);
    lv_label_set_text(labelTime, public_buf_l);
    lv_obj_set_style(labelTime, &label_big_black);
    lv_obj_align(labelTime, buttonTime, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    labelZpos = lv_label_create_empty(scr);
    char buf[30] = "Z:";
    strcat(buf, dtostrf(current_position.z, 1, 2, public_buf_l));
    lv_label_set_text(labelZpos, buf);
    lv_label_set_style(labelZpos, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_set_pos(labelZpos, 395, 4);

    // create progress bar

    static lv_style_t bar_yellow, bar_white;

    lv_style_copy(&bar_yellow, &label_big_black);
    bar_yellow.body.main_color = LV_COLOR_MAKE(0xff, 0xa2, 0x10);
    bar_yellow.body.grad_color = LV_COLOR_MAKE(0xff, 0xa2, 0x10);

    lv_style_copy(&bar_white, &bar_yellow);
    bar_white.body.main_color = LV_COLOR_WHITE;
    bar_white.body.grad_color = LV_COLOR_WHITE;

    bar1 = lv_bar_create(scr, nullptr);
    lv_obj_set_pos(bar1, 0, 36);
    lv_obj_set_size(bar1, 480, 45);
    lv_bar_set_style(bar1, LV_BAR_STYLE_INDIC, &bar_yellow);
    lv_bar_set_style(bar1, LV_BAR_STYLE_BG, &bar_white);
    lv_bar_set_anim_time(bar1, 1000);
    lv_bar_set_value(bar1, 0, LV_ANIM_ON);

    gcodeName = lv_label_create_empty(bar1);
    lv_label_set_text(gcodeName, list_file.long_name[sel_id]);
    lv_label_set_style(gcodeName, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_align(gcodeName, bar1, LV_ALIGN_IN_LEFT_MID, 10, 0);

    bar1ValueText = lv_label_create_empty(bar1);
    lv_label_set_text(bar1ValueText, "0%");
    // sprintf_P(public_buf_l, "%d%%", uiCfg.print_progress);
    // lv_label_set_text(bar1ValueText, public_buf_l);
    lv_label_set_style(bar1ValueText, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_align(bar1ValueText, bar1, LV_ALIGN_IN_RIGHT_MID, -30, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    // Create image buttons
    buttonExt1 = lv_imgbtn_create(scr, "F:/bmp_ext1_state.bin", 243, 82, event_handler, ID_TEMP_EXT);
    labelExt1 = lv_label_create_empty(scr);
    sprintf(public_buf_l, TEXT_VALUE_CN, thermalManager.wholeDegHotend(0), thermalManager.degTargetHotend(0)); // preheat_menu.value_state
    lv_label_set_text(labelExt1, public_buf_l);
    lv_label_set_style(labelExt1, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_align(labelExt1, buttonExt1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    // #if HAS_MULTI_EXTRUDER
    //   buttonExt2 = lv_imgbtn_create(scr, "F:/bmp_ext2_state.bin", 350, 136, event_handler, ID_TEMP_EXT);
    // #endif

#if HAS_HEATED_BED
    buttonBedstate = lv_imgbtn_create(scr, "F:/bmp_bed_state.bin", 243, 173, event_handler, ID_TEMP_BED);
    labelBed = lv_label_create_empty(scr);
    sprintf(public_buf_l, TEXT_VALUE_CN, thermalManager.wholeDegBed(), thermalManager.degTargetBed());
    lv_label_set_text(labelBed, public_buf_l);
    lv_label_set_style(labelBed, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_align(labelBed, buttonBedstate, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());
#endif

    // buttonFanstate = lv_imgbtn_create(scr, "F:/bmp_fan_state.bin", 230, 168, event_handler, ID_FAN);
    // labelFan  = lv_label_create_empty(scr);
    // sprintf_P(public_buf_l, PSTR("%d%%"), (int)thermalManager.fanSpeedPercent(0));
    // lv_label_set_text(labelFan, public_buf_l);
    // lv_label_set_style(labelFan, LV_LABEL_STYLE_MAIN, &label_big_black);
    // lv_obj_align(labelFan , buttonFanstate, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    buttonSpeed = lv_imgbtn_create(scr, "F:/bmp_movs_state.bin", 383, 82, event_handler, ID_PRINT_SPEED_CHANGE);
    labelSpeed = lv_label_create_empty(scr);
    sprintf_P(buf, PSTR("%d%%"), feedrate_percentage);
    lv_label_set_text(labelSpeed, buf);
    lv_label_set_style(labelSpeed, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_align(labelSpeed, buttonSpeed, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    buttonUp = lv_imgbtn_create(scr, "F:/bmp_flows_state.bin", 383, 173, event_handler, ID_FLOW_SPEED_CHANGE);
    labelUp = lv_label_create_empty(scr);
    sprintf_P(buf, PSTR("%d%%"), planner.flow_percentage[0]);
    lv_label_set_text(labelUp, buf);
    lv_label_set_style(labelUp, LV_LABEL_STYLE_MAIN, &label_big_black);
    lv_obj_align(labelUp, buttonUp, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    // lv_refr_now(lv_refr_get_disp_refreshing());

    if (uiCfg.print_state == WORKING || uiCfg.print_state == RESUMING)
        buttonPause = lv_imgbtn_create(scr, "F:/bmp_pause.bin", 310, 252, event_handler, ID_PAUSE);
    else
        buttonPause = lv_imgbtn_create(scr, "F:/bmp_continue.bin", 310, 252, event_handler, ID_PAUSE);

    buttonStop = lv_imgbtn_create(scr, "F:/bmp_stop.bin", 234, 252, event_handler, ID_STOP);
    buttonMore = lv_imgbtn_create(scr, "F:/bmp_print_more.bin", 410, 252, event_handler, ID_MORE);

    lv_obj_align(buttonPause, buttonStop, LV_ALIGN_OUT_RIGHT_TOP, 13, 2);
    lv_obj_align(buttonMore, buttonPause, LV_ALIGN_OUT_RIGHT_TOP, 13, 0);

    // #if HAS_ROTARY_ENCODER
    //   if (gCfgItems.encoder_enable) {
    //     lv_group_add_obj(g, buttonPause);
    //     lv_group_add_obj(g, buttonStop);
    //     lv_group_add_obj(g, buttonOperat);
    //     lv_group_add_obj(g, buttonPause);
    //     lv_group_add_obj(g, buttonPause);
    //     lv_group_add_obj(g, buttonPause);
    //   }
    // #endif

#if HAS_MULTI_EXTRUDER
    labelExt2 = lv_label_create(scr, 395, 146, nullptr);
#endif

#if HAS_HEATED_BED
#endif

    disp_print_time(); // 已花费时间刷新
    disp_fan_Zpos();   // Z点偏移刷新
    disp_ext_temp();   // 挤出头温度刷新
    disp_bed_temp();   // 热床温度刷新
    // disp_fan_speed();//风扇速度刷新
    disp_print_speed_change();
    disp_printing_ext_speed();
    setProBarRate();

    // lv_refr_now(lv_refr_get_disp_refreshing());
    // lv_img_set_src(imgflowspeed, "F:/bmp_prints_state.bin");
}

void disp_ext_temp()
{
    sprintf(public_buf_l, TEXT_VALUE_CN, thermalManager.wholeDegHotend(0), thermalManager.degTargetHotend(0)); // preheat_menu.value_state
    lv_label_set_text(labelExt1, public_buf_l);

#if HAS_MULTI_EXTRUDER
    sprintf(public_buf_l, printing_menu.temp1, thermalManager.wholeDegHotend(1), thermalManager.degTargetHotend(1));
    lv_label_set_text(labelExt2, public_buf_l);
#endif
}

void disp_bed_temp()
{
#if HAS_HEATED_BED
    sprintf(public_buf_l, TEXT_VALUE_CN, thermalManager.wholeDegBed(), thermalManager.degTargetBed());
    lv_label_set_text(labelBed, public_buf_l);
#endif
}

void disp_fan_speed()
{
    sprintf_P(public_buf_l, PSTR("%d%%"), (int)thermalManager.fanSpeedPercent(0));
    lv_label_set_text(labelFan, public_buf_l);
}

void disp_print_time()
{
#if BOTH(LCD_SET_PROGRESS_MANUALLY, USE_M73_REMAINING_TIME)
    const uint32_t r = ui.get_remaining_time();
    sprintf_P(public_buf_l, PSTR("%02d:%02d R"), r / 3600, (r % 3600) / 60);
#else
    sprintf_P(public_buf_l, PSTR("%d%d:%d%d:%d%d"), print_time.hours / 10, print_time.hours % 10, print_time.minutes / 10, print_time.minutes % 10, print_time.seconds / 10, print_time.seconds % 10);
#endif
    strcpy(print_finish_time, public_buf_l);
    lv_label_set_text(labelTime, public_buf_l);
}

void disp_fan_Zpos()
{
    // dtostrf(current_position.z, 1, 3, public_buf_l);
    char buf[30] = "Z:";
    strcat(buf, dtostrf(current_position.z, 1, 2, public_buf_l));
    lv_label_set_text(labelZpos, buf);
}

void reset_print_time()
{
    print_time.hours = 0;
    print_time.minutes = 0;
    print_time.seconds = 0;
    print_time.ms_10 = 0;
}

void start_print_time() { print_time.start = 1; }

void stop_print_time() { print_time.start = 0; }

void setProBarRate()
{
    int rate;
    volatile long long rate_tmp_r;

    if (!gCfgItems.from_flash_pic)
    {
#if ENABLED(SDSUPPORT)
        rate_tmp_r = (long long)card.getIndex() * 100;
#endif
        rate = rate_tmp_r / gCfgItems.curFilesize;
    }
    else
    {
#if ENABLED(SDSUPPORT)
        rate_tmp_r = (long long)card.getIndex();
#endif
        rate = (rate_tmp_r - (PREVIEW_SIZE + To_pre_view)) * 100 / (gCfgItems.curFilesize - (PREVIEW_SIZE + To_pre_view));
    }

    if (rate <= 0)
    {
        uiCfg.print_progress = 0;

        return;
    }

    uiCfg.print_progress = rate;


    if (disp_state == PRINTING_UI)
    {
        lv_bar_set_value(bar1, rate, LV_ANIM_ON);
        sprintf_P(public_buf_l, "%d%%", rate);
        lv_label_set_text(bar1ValueText, public_buf_l);
        lv_obj_align(bar1ValueText, bar1, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    }
}

void lv_clear_printing()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

void disp_print_speed_change()
{
    char buf[30] = {0};
    int16_t val;

    val = feedrate_percentage;
    sprintf_P(buf, PSTR("%d%%"), val);
    lv_label_set_text(labelSpeed, buf);

    val = planner.flow_percentage[0];
    sprintf_P(buf, PSTR("%d%%"), val);
    lv_label_set_text(labelUp, buf);
}

void disp_printing_ext_speed()
{
    dtostrf(feedrate_mm_s, 1, 1, public_buf_l);
    strcat(public_buf_l, "mm/s");
    lv_label_set_text(labelFlowSpeed, public_buf_l);
}

#endif // HAS_TFT_LVGL_UI
