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

/**
 * draw_dialog.cpp
 */

#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>
#include "printer_operation.h"

#include "../../../sd/cardreader.h"
#include "../../../gcode/queue.h"
#include "../../../module/temperature.h"
#include "../../../module/planner.h"
#include "../../../gcode/gcode.h"
#include "../../../inc/MarlinConfig.h"

#if ENABLED(EEPROM_SETTINGS)
#include "../../../module/settings.h"
#endif

#if ENABLED(POWER_LOSS_RECOVERY)
#include "../../../feature/powerloss.h"
#endif

#if ENABLED(PARK_HEAD_ON_PAUSE)
#include "../../../feature/pause.h"
#endif

#if ENABLED(TOUCH_SCREEN_CALIBRATION)
#include "../../tft_io/touch_calibration.h"
#include "draw_touch_calibration.h"
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
// #include "../../../../feature/bedlevel/bedlevel.h"
#include "../../../../src/feature/bedlevel/bedlevel.h"

#endif

// #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
//   extern bed_mesh_t z_values;
// #endif

#define REPRINT_FLAG 0
extern uint8_t Auto_leveling_succse;
extern lv_group_t *g;
static lv_obj_t *scr, *tempText1, *filament_bar;
uint8_t wait_for_g28_flag;
extern uint8_t sel_id;
extern bool once_flag, gcode_preview_over;
extern int upload_result;
extern uint32_t upload_time_sec;
extern uint32_t upload_size;
extern bool temps_update_flag;
float z_offset_add = 0;
extern uint8_t layer_stop_flag;
extern int32_t save_layer_stop_num;
extern int32_t save_disp_layer_stop_num;
extern int32_t print_times, disp_print_times;

// #define CANCEL_ON_RIGHT   // Put 'Cancel' on the right (as it was before)

#define BTN_OK_X TERN(CANCEL_ON_RIGHT, 100, 280)
#define BTN_CANCEL_X TERN(CANCEL_ON_RIGHT, 280, 100)
#define BTN_OK_Y 180
#define BTN_CANCEL_Y 180

enum
{
    ID_LOG_OK = 1,
    ID_LOG_CANCEL,
};
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;
    switch (obj->mks_obj_id)
    {
    case ID_LOG_OK:
        wait_for_heatup = false;
        stop_print_time();
        lv_clear_dialog();
        lv_draw_ready_print();

#if ENABLED(SDSUPPORT)
        uiCfg.print_state = IDLE;
        card.abortFilePrintSoon();
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
        if (uiCfg.adjustZoffset)
        {
#if DISABLED(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN)
            for (uint8_t x = 0; x < GRID_MAX_POINTS_X; x++)
                for (uint8_t y = 0; y < GRID_MAX_POINTS_Y; y++)
                    bedlevel.z_values[x][y] = bedlevel.z_values[x][y] + uiCfg.babyStepZoffsetDiff;
#endif
            TERN_(EEPROM_SETTINGS, (void)settings.save());
            uiCfg.babyStepZoffsetDiff = 0;
            uiCfg.adjustZoffset = 0;
        }
#endif
        break;
    case ID_LOG_CANCEL:
        if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
        {
            TERN_(ADVANCED_PAUSE_FEATURE, pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT);
        }
        else if (DIALOG_IS(TYPE_FILAMENT_LOAD_HEAT, TYPE_FILAMENT_UNLOAD_HEAT, TYPE_FILAMENT_HEAT_LOAD_COMPLETED, TYPE_FILAMENT_HEAT_UNLOAD_COMPLETED))
        {
            thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);
            clear_cur_ui();
            draw_return_ui();
        }
        else if (DIALOG_IS(TYPE_FILAMENT_LOADING, TYPE_FILAMENT_UNLOADING))
        {
            queue.enqueue_one(PSTR("M410"));
            uiCfg.filament_rate = 0;
            uiCfg.filament_loading_completed = false;
            uiCfg.filament_unloading_completed = false;
            uiCfg.filament_loading_time_flg = false;
            uiCfg.filament_loading_time_cnt = 0;
            uiCfg.filament_unloading_time_flg = false;
            uiCfg.filament_unloading_time_cnt = 0;
            thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);
            clear_cur_ui();
            draw_return_ui();
        }
        else
        {
            clear_cur_ui();
            draw_return_ui();
        }
        break;
    }
}

static void btn_Reload_event_cb(lv_obj_t *btn, lv_event_t event)
{
    char buf[100] = {0};
    if (event != LV_EVENT_RELEASED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    //移动传送带
    sprintf(buf, PSTR("G92 I0\nG1 I%d F1000"), I_MOVE_DISTANCE);
    gcode.process_subcommands_now(PSTR(buf));
    planner.synchronize();

    start_printing();
}

static void btn_ok_event_cb(lv_obj_t *btn, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    if (DIALOG_IS(TYPE_PRINT_FILE))
    {

        if (convetor_t.open_flag)
        {
            print_times = 1;
            disp_print_times = print_times;
            lv_clear_dialog();
            lv_draw_print_times();
        }
        else
        {
            start_printing();
        }
    }
    else if (DIALOG_IS(TYPE_STOP))
    {
        if (Layout_stop_num.already_obtained_flag)
            Layout_stop_num.already_obtained_flag = false;
        if (Layout_stop_num.bottom_already_obtained_flag)
            Layout_stop_num.bottom_already_obtained_flag = false;
        wait_for_heatup = false;
        stop_print_time();
        baby_step_set_data = false;

#if ENABLED(SDSUPPORT)
        uiCfg.print_state = IDLE;
        card.abortFilePrintSoon();
        recovery.purge();
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
        if (uiCfg.adjustZoffset)
        {
#if DISABLED(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN)
            for (uint8_t x = 0; x < GRID_MAX_POINTS_X; x++)
                for (uint8_t y = 0; y < GRID_MAX_POINTS_Y; y++)
                    bedlevel.z_values[x][y] = bedlevel.z_values[x][y] + uiCfg.babyStepZoffsetDiff;
#endif
            TERN_(EEPROM_SETTINGS, (void)settings.save());
            uiCfg.babyStepZoffsetDiff = 0;
            uiCfg.adjustZoffset = 0;
        }
#endif

        feedrate_percentage = 100;
        planner.flow_percentage[0] = 100;
        planner.e_factor[0] = planner.flow_percentage[0] * 0.01f;

        lv_clear_dialog();
        lv_draw_ready_print();
    }
    else if (DIALOG_IS(TYPE_PAUSE))
    {
#if ENABLED(SDSUPPORT)
        card.pauseSDPrint();
        stop_print_time();
        uiCfg.print_state = PAUSING;
        clear_cur_ui();
        draw_return_ui();
#endif
    }
    else if (DIALOG_IS(TYPE_FINISH_PRINT))
    {
        if (Layout_stop_num.already_obtained_flag)
            Layout_stop_num.already_obtained_flag = false;
        if (Layout_stop_num.bottom_already_obtained_flag)
            Layout_stop_num.bottom_already_obtained_flag = false;

        feedrate_percentage = 100;
        planner.flow_percentage[0] = 100;
        planner.e_factor[0] = planner.flow_percentage[0] * 0.01f;
        baby_step_set_data = false;

        clear_cur_ui();
        lv_draw_ready_print();
#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
        if (uiCfg.adjustZoffset)
        {
#if DISABLED(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN)
            for (uint8_t x = 0; x < GRID_MAX_POINTS_X; x++)
                for (uint8_t y = 0; y < GRID_MAX_POINTS_Y; y++)
                    bedlevel.z_values[x][y] = bedlevel.z_values[x][y] + uiCfg.babyStepZoffsetDiff;
#endif
            TERN_(EEPROM_SETTINGS, (void)settings.save());
            uiCfg.babyStepZoffsetDiff = 0;
            uiCfg.adjustZoffset = 0;
        }
#endif
    }
#if ENABLED(ADVANCED_PAUSE_FEATURE)
    else if (DIALOG_IS(PAUSE_MESSAGE_WAITING, PAUSE_MESSAGE_INSERT, PAUSE_MESSAGE_HEAT))
        wait_for_user = false;
    else if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
        // pause_menu_response = PAUSE_RESPONSE_EXTRUDE_MORE;
        else if (DIALOG_IS(PAUSE_MESSAGE_RESUME))
        {
            clear_cur_ui();
            draw_return_ui();
        }
#endif
    else if (DIALOG_IS(STORE_EEPROM_TIPS))
    {
        TERN_(EEPROM_SETTINGS, (void)settings.save());
        clear_cur_ui();
        draw_return_ui();
    }
    else if (DIALOG_IS(READ_EEPROM_TIPS))
    {
        TERN_(EEPROM_SETTINGS, (void)settings.load());
        clear_cur_ui();
        draw_return_ui();
    }
    else if (DIALOG_IS(REVERT_EEPROM_TIPS))
    {
        TERN_(EEPROM_SETTINGS, (void)settings.reset());
        clear_cur_ui();
#if ENABLED(TOUCH_SCREEN_CALIBRATION)
        const bool do_draw_cal = touch_calibration.need_calibration();
        if (do_draw_cal)
        {
            disp_state_stack._disp_index--; // We are asynchronous from the dialog, so let's remove the dialog from the stack
            lv_draw_touch_calibration_screen();
        }
#else
        constexpr bool do_draw_cal = false;
#endif
        if (!do_draw_cal)
            draw_return_ui();
    }
    else if (DIALOG_IS(WIFI_CONFIG_TIPS))
    {
        uiCfg.configWifi = true;
        clear_cur_ui();
        draw_return_ui();
    }
    else if (DIALOG_IS(TYPE_FILAMENT_HEAT_LOAD_COMPLETED))
        uiCfg.filament_heat_completed_load = true;
    else if (DIALOG_IS(TYPE_FILAMENT_HEAT_UNLOAD_COMPLETED))
        uiCfg.filament_heat_completed_unload = true;
    else if (DIALOG_IS(TYPE_FILAMENT_LOAD_COMPLETED, TYPE_FILAMENT_UNLOAD_COMPLETED))
    {
        clear_cur_ui();
        draw_return_ui();
    }
#if ENABLED(MKS_WIFI_MODULE)
    else if (DIALOG_IS(TYPE_UNBIND))
    {
        cloud_unbind();
        clear_cur_ui();
        draw_return_ui();
    }
#endif
    else if (DIALOG_IS(TYPE_FILAMENT_RUNOUT))
    {
        uiCfg.print_speed = feedrate_percentage;
        uiCfg.ext_speed = planner.flow_percentage[0];

        feedrate_percentage = 100;
        planner.flow_percentage[0] = 100;
        planner.e_factor[0] = planner.flow_percentage[0] * 0.01f;

        uiCfg.moveSpeed_bak = (uint16_t)feedrate_mm_s;
        uiCfg.hotendTargetTempBak = thermalManager.degTargetHotend(active_extruder);
        clear_cur_ui();
        lv_draw_filament_change();
    }
    else if (DIALOG_IS(TYPE_REPRINT))
    {
        baby_step_set_data = true;
        if (gCfgItems.from_flash_pic)
            flash_preview_begin = true;
        else
            default_preview_flg = true;

        uiCfg.print_state = REPRINTING; // REPRINTED
        print_time.minutes = recovery.info.print_job_elapsed / 60;
        print_time.seconds = recovery.info.print_job_elapsed % 60;
        print_time.hours = print_time.minutes / 60;
        // start_print_time();

        clear_cur_ui();
        lv_draw_printing();
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_IF_ZERO)
    {
        queue.inject_P(PSTR("G28"));
        clear_cur_ui();
        lv_draw_dialog(DIALOG_TYPE_HOMING_WAIT);
        wait_for_g28_flag = 1;
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_IF_AUTO_LEVELING)
    {
        uiCfg.autoLeveling = 1;
        Auto_leveling_succse = 1;
        clear_cur_ui();
        lv_draw_auto_leveling();
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_AUTO_LEVEL_FINISH)
    {
        clear_cur_ui();
        lv_draw_auto_leveling();
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_PROBING_FAILED)
    {
        clear_cur_ui();
        lv_draw_auto_leveling();
    }
    else
    {
        clear_cur_ui();
        draw_return_ui();
    }
}

static void btn_cancel_event_cb(lv_obj_t *btn, lv_event_t event)
{
    if (event != LV_EVENT_RELEASED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
    {
        TERN_(ADVANCED_PAUSE_FEATURE, pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT);
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_IF_ZERO)
    {
        clear_cur_ui();
        lv_draw_tool();
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_IF_AUTO_LEVELING)
    {
        clear_cur_ui();
        lv_draw_auto_leveling();
    }
    else if (DIALOG_IS(TYPE_FILAMENT_LOAD_HEAT, TYPE_FILAMENT_UNLOAD_HEAT, TYPE_FILAMENT_HEAT_LOAD_COMPLETED, TYPE_FILAMENT_HEAT_UNLOAD_COMPLETED))
    {
        thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);
        clear_cur_ui();
        draw_return_ui();
    }
    else if (DIALOG_IS(TYPE_FILAMENT_LOADING, TYPE_FILAMENT_UNLOADING))
    { //, TYPE_FILAMENT_RUNOUT
        queue.enqueue_one(PSTR("M410"));
        uiCfg.filament_rate = 0;
        uiCfg.filament_loading_completed = false;
        uiCfg.filament_unloading_completed = false;
        uiCfg.filament_loading_time_flg = false;
        uiCfg.filament_loading_time_cnt = 0;
        uiCfg.filament_unloading_time_flg = false;
        uiCfg.filament_unloading_time_cnt = 0;
        thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);
        clear_cur_ui();
        draw_return_ui();
    }
    else if (DIALOG_IS(TYPE_FILAMENT_RUNOUT))
    {
        wait_for_heatup = false;
        stop_print_time();

#if ENABLED(SDSUPPORT)
        uiCfg.print_state = IDLE;
        card.abortFilePrintSoon();
        recovery.purge();
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
        if (uiCfg.adjustZoffset)
        {
#if DISABLED(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN)
            for (uint8_t x = 0; x < GRID_MAX_POINTS_X; x++)
                for (uint8_t y = 0; y < GRID_MAX_POINTS_Y; y++)
                    bedlevel.z_values[x][y] = bedlevel.z_values[x][y] + uiCfg.babyStepZoffsetDiff;
#endif
            TERN_(EEPROM_SETTINGS, (void)settings.save());
            uiCfg.babyStepZoffsetDiff = 0;
            uiCfg.adjustZoffset = 0;
        }
#endif
        lv_clear_dialog();
        lv_draw_ready_print();
    }
    else if (DIALOG_IS(TYPE_REPRINT))
    {
        Layout_stop_num.data = 0;
        Layout_stop_num.bottom_data = 0;
        save_layer_stop_num = 0;
        save_disp_layer_stop_num = 0;
#if REPRINT_FLAG
        lv_clear_dialog();
#if ENABLED(SDSUPPORT)
        uiCfg.print_state = IDLE;
        card.abortFilePrintSoon();
        recovery.purge();
#endif
        lv_draw_ready_print();
#else
        wait_for_heatup = false;
        stop_print_time();
        lv_clear_dialog();
        lv_draw_ready_print();
// recoveryed_flag = false;
#if ENABLED(SDSUPPORT)
        uiCfg.print_state = IDLE;
        card.abortFilePrintSoon();
#endif
#endif
    }
    else
    {
        clear_cur_ui();
        draw_return_ui();
    }
}

void lv_draw_dialog(uint8_t type)
{
    lv_obj_t *btnOk = nullptr, *btnCancel = nullptr;
    uiCfg.dialogType = type;
    scr = lv_screen_create(DIALOG_UI);

    lv_obj_t *labelDialog = lv_label_create(scr, "");
    lv_obj_set_style(labelDialog, &tft_style_preHeat_label);

    // if(DIALOG_IS(TYPE_STOP))
    if (DIALOG_IS(TYPE_FINISH_PRINT, PAUSE_MESSAGE_RESUME))
    {
        lv_obj_t *btnComplete = lv_button_btn_create(scr, 28, 228, 200, 80, btn_ok_event_cb);
        lv_btn_set_style(btnComplete, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
        lv_btn_set_style(btnComplete, LV_BTN_STYLE_PR, &tft_style_button_GREEN);
        lv_obj_t *labelOk = lv_label_create_empty(btnComplete); // Add a label to the button
        lv_obj_set_style(labelOk, &label_dialog_white);
        lv_label_set_text(labelOk, printing_MENU.print_complete); // Set the labels text

        lv_obj_t *btnAgain = lv_button_btn_create(scr, 253, 228, 200, 80, btn_Reload_event_cb);
        lv_btn_set_style(btnAgain, LV_BTN_STYLE_REL, &tft_style_button_YELLOW);
        lv_btn_set_style(btnAgain, LV_BTN_STYLE_PR, &tft_style_button_YELLOW);

        lv_obj_t *labelCancel = lv_label_create_empty(btnAgain); // Add a label to the button
        lv_obj_set_style(labelCancel, &label_dialog_white);
        lv_label_set_text(labelCancel, printing_MENU.print_it_again); // Set the labels text
        if (DIALOG_IS(TYPE_FINISH_PRINT))
            if (gCfgItems.print_completion_tone)
                voice_button_on();
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_WAITING, PAUSE_MESSAGE_INSERT, PAUSE_MESSAGE_HEAT))
    {
        btnOk = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_ok_event_cb);
        lv_obj_t *labelOk = lv_label_create_empty(btnOk);           // Add a label to the button
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm); // Set the labels text
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_PARKING, PAUSE_MESSAGE_CHANGING, PAUSE_MESSAGE_UNLOAD, PAUSE_MESSAGE_LOAD, PAUSE_MESSAGE_PURGE, PAUSE_MESSAGE_RESUME, PAUSE_MESSAGE_HEATING))
    {
        // nothing to do
    }
    else if (DIALOG_IS(WIFI_ENABLE_TIPS))
    {
        // btnCancel
        lv_obj_t *btnWifiEnableCancle = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_cancel_event_cb);
        lv_btn_set_style(btnWifiEnableCancle, LV_BTN_STYLE_REL, &tft_style_preHeat_GREEN);
        lv_btn_set_style(btnWifiEnableCancle, LV_BTN_STYLE_PR, &tft_style_preHeat_GREEN);
        lv_obj_t *labelCancel = lv_label_create_empty(btnWifiEnableCancle);
        // lv_obj_set_style(labelCancel, &tft_style_preHeat_label);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
        lv_obj_set_style(labelCancel, &label_dialog_white);
        lv_obj_align(btnWifiEnableCancle, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    }
    else if (DIALOG_IS(TRANSFER_NO_DEVICE))
    {
        btnCancel = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_cancel_event_cb);
        lv_obj_t *labelCancel = lv_label_create_empty(btnCancel);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
    }
#if ENABLED(MKS_WIFI_MODULE)
    else if (DIALOG_IS(TYPE_UPLOAD_FILE))
    {
        if (upload_result == 2)
        {
            btnCancel = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_cancel_event_cb);
            lv_obj_t *labelCancel = lv_label_create_empty(btnCancel);
            lv_label_set_text(labelCancel, print_file_dialog_menu.cancel); // bmp_wifi_reconnect
        }
        else if (upload_result == 3)
        {
            btnOk = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_ok_event_cb);
            lv_obj_t *labelOk = lv_label_create_empty(btnOk);
            lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
        }
    }
    else if (DIALOG_IS(TYPE_UPDATE_ESP_FIRMWARE))
    {
        // nothing to do
    }
#endif
    else if (DIALOG_IS(TYPE_FILAMENT_LOAD_HEAT, TYPE_FILAMENT_UNLOAD_HEAT))
    {
        btnCancel = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_cancel_event_cb);
        lv_obj_t *labelCancel = lv_label_create_empty(btnCancel);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);

        tempText1 = lv_label_create_empty(scr);
        filament_sprayer_temp();
    }
    else if (DIALOG_IS(TYPE_FILAMENT_LOAD_COMPLETED, TYPE_FILAMENT_UNLOAD_COMPLETED))
    {
        btnOk = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_ok_event_cb);
        lv_obj_t *labelOk = lv_label_create_empty(btnOk);
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_LOADING, TYPE_FILAMENT_UNLOADING))
    {
        btnCancel = lv_button_btn_create(scr, BTN_OK_X + 90, BTN_OK_Y, 100, 50, btn_cancel_event_cb);
        lv_obj_t *labelCancel = lv_label_create_empty(btnCancel);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);

        filament_bar = lv_bar_create(scr, nullptr);
        lv_obj_set_pos(filament_bar, (TFT_WIDTH - 400) / 2, ((TFT_HEIGHT - titleHeight) - 40) / 2);
        lv_obj_set_size(filament_bar, 400, 25);
        // lv_bar_set_style(filament_bar, LV_BAR_STYLE_INDIC, &lv_bar_style_indic);
        lv_bar_set_anim_time(filament_bar, 1000);
        lv_bar_set_value(filament_bar, 0, LV_ANIM_ON);
    }
#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
    else if (DIALOG_IS(TYPE_AUTO_LEVELING_TIPS))
    {
        // nothing to do
    }
#endif
    else if (DIALOG_IS(TYPE_STOP))
    {
        lv_obj_t *btnConfirm = lv_button_btn_create(scr, 168, 192, 130, 72, btn_ok_event_cb);
        lv_btn_set_style(btnConfirm, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(btnConfirm, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
        lv_obj_t *labelOk = lv_label_create_empty(btnConfirm); // Add a label to the button
        lv_obj_set_style(labelOk, &label_dialog_white);

        lv_obj_t *buttonCancel = lv_button_btn_create(scr, 15, 192, 130, 72, btn_cancel_event_cb);
        lv_btn_set_style(buttonCancel, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(buttonCancel, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
        lv_obj_t *labelCancel = lv_label_create_empty(buttonCancel); // Add a label to the button
        lv_obj_set_style(labelCancel, &label_dialog_white);

        lv_obj_t *btnReloading = lv_button_btn_create(scr, 320, 192, 130, 72, btn_Reload_event_cb);
        lv_btn_set_style(btnReloading, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(btnReloading, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
        lv_obj_t *labelReloading = lv_label_create_empty(btnReloading); // Add a label to the button
        lv_obj_set_style(labelReloading, &label_dialog_white);
        lv_refr_now(lv_refr_get_disp_refreshing());

        if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
        {
            lv_label_set_text(labelOk, pause_msg_menu.purgeMore); // Set the labels text
            lv_label_set_text(labelCancel, pause_msg_menu.continuePrint);
        }
        else
        {
            lv_label_set_text(labelOk, print_file_dialog_menu.confirm); // Set the labels text
            lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
            lv_label_set_text(labelReloading, print_file_dialog_menu.reloading);
        }
    }
    else if (DIALOG_IS(TYPE_PAUSE))
    {
        lv_obj_t *btnConfirm = lv_button_btn_create(scr, 168, 192, 130, 72, btn_ok_event_cb);
        lv_btn_set_style(btnConfirm, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(btnConfirm, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
        lv_obj_t *labelOk = lv_label_create_empty(btnConfirm); // Add a label to the button
        lv_obj_set_style(labelOk, &label_dialog_white);

        lv_obj_t *buttonCancel = lv_button_btn_create(scr, 15, 192, 130, 72, btn_cancel_event_cb);
        lv_btn_set_style(buttonCancel, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(buttonCancel, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
        lv_obj_t *labelCancel = lv_label_create_empty(buttonCancel); // Add a label to the button
        lv_obj_set_style(labelCancel, &label_dialog_white);

        lv_obj_t *btnReloading = lv_button_btn_create(scr, 320, 192, 130, 72, btn_Reload_event_cb);
        lv_btn_set_style(btnReloading, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
        lv_btn_set_style(btnReloading, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
        lv_obj_t *labelReloading = lv_label_create_empty(btnReloading); // Add a label to the button
        lv_obj_set_style(labelReloading, &label_dialog_white);
        lv_refr_now(lv_refr_get_disp_refreshing());

        if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
        {
            lv_label_set_text(labelOk, pause_msg_menu.purgeMore); // Set the labels text
            lv_label_set_text(labelCancel, pause_msg_menu.continuePrint);
        }
        else
        {
            lv_label_set_text(labelOk, print_file_dialog_menu.confirm); // Set the labels text
            lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
            lv_label_set_text(labelReloading, print_file_dialog_menu.reloading);
        }
    }
    else if (DIALOG_IS(TYPE_FILAMENT_RUNOUT))
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_runout_err.bin");
        lv_obj_set_pos(imgerr, 201, 32);

        lv_obj_t *labelerr = lv_label_create_empty(scr);
        lv_label_set_text(labelerr, printing_MENU.FILAMENT_OUT);
        lv_obj_set_style(labelerr, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelerr, imgerr, LV_ALIGN_CENTER, 0, 60);

        lv_obj_t *labelerr_1 = lv_label_create_empty(scr);
        lv_label_set_text(labelerr_1, printing_MENU.FILAMENT_OUT_1);
        lv_obj_set_style(labelerr_1, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelerr_1, labelerr, LV_ALIGN_CENTER, 0, 30);

        lv_obj_t *labelerr_2 = lv_label_create_empty(scr);
        lv_label_set_text(labelerr_2, printing_MENU.FILAMENT_OUT_2);
        lv_obj_set_style(labelerr_2, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelerr_2, labelerr_1, LV_ALIGN_CENTER, 0, 30);
        // lv_obj_set_pos(labelerr, 82,118);

        lv_obj_t *btnContinueP = lv_button_btn_create(scr, 28, 228, 200, 80, btn_ok_event_cb);
        lv_btn_set_style(btnContinueP, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
        lv_btn_set_style(btnContinueP, LV_BTN_STYLE_PR, &tft_style_button_GREEN);
        lv_obj_t *labelContinueP = lv_label_create_empty(btnContinueP); // Add a label to the button
        lv_obj_set_style(labelContinueP, &label_dialog_white);
        lv_label_set_text(labelContinueP, printing_MENU.add_filament); // Set the labels text

        lv_obj_t *btnReturnP = lv_button_btn_create(scr, 253, 228, 200, 80, btn_cancel_event_cb);
        lv_btn_set_style(btnReturnP, LV_BTN_STYLE_REL, &tft_style_button_YELLOW);
        lv_btn_set_style(btnReturnP, LV_BTN_STYLE_PR, &tft_style_button_YELLOW);
        lv_obj_t *labelReturnP = lv_label_create_empty(btnReturnP); // Add a label to the button
        lv_obj_set_style(labelReturnP, &label_dialog_white);
        lv_label_set_text(labelReturnP, printing_MENU.print_return); // Set the labels text
    }
    else if (DIALOG_IS(TYPE_REPRINT))
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_reprint_warning.bin");
        lv_obj_set_pos(imgerr, 201, 32);

        lv_obj_t *labelerr = lv_label_create_empty(scr);
        lv_obj_set_style(labelerr, &tft_style_preHeat_label_BLACK);
        lv_label_set_text(labelerr, printing_MENU.found_reprint);
        lv_obj_align(labelerr, imgerr, LV_ALIGN_CENTER, 0, 60);
        lv_obj_t *labelreprintfilename = lv_label_create_empty(scr);
        lv_label_set_text(labelreprintfilename, list_file.long_name[sel_id]);
        lv_obj_set_style(labelreprintfilename, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelreprintfilename, labelerr, LV_ALIGN_CENTER, 0, 40);

        lv_obj_t *btnContinueP = lv_button_btn_create(scr, 28, 228, 200, 80, btn_ok_event_cb);
        lv_btn_set_style(btnContinueP, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
        lv_btn_set_style(btnContinueP, LV_BTN_STYLE_PR, &tft_style_button_GREEN);
        lv_obj_t *labelContinueP = lv_label_create_empty(btnContinueP); // Add a label to the button
        lv_obj_set_style(labelContinueP, &label_dialog_white);
        lv_label_set_text(labelContinueP, printing_MENU.continueprint); // Set the labels text

        lv_obj_t *btnReturnP = lv_button_btn_create(scr, 253, 228, 200, 80, btn_cancel_event_cb);
        lv_btn_set_style(btnReturnP, LV_BTN_STYLE_REL, &tft_style_button_YELLOW);
        lv_btn_set_style(btnReturnP, LV_BTN_STYLE_PR, &tft_style_button_YELLOW);
        lv_obj_t *labelReturnP = lv_label_create_empty(btnReturnP); // Add a label to the button
        lv_obj_set_style(labelReturnP, &label_dialog_white);
        lv_label_set_text(labelReturnP, printing_MENU.back); // Set the labels text
    }
    else if (DIALOG_IS(TYPE_PRINT_FILE))
    {
        lv_obj_t *btnToprint = lv_button_btn_create(scr, 37, 203, 173, 78, btn_ok_event_cb);
        lv_btn_set_style(btnToprint, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
        lv_btn_set_style(btnToprint, LV_BTN_STYLE_PR, &tft_style_button_GREEN);
        lv_obj_t *labelOk = lv_label_create_empty(btnToprint); // Add a label to the button
        lv_label_set_style(labelOk, LV_LABEL_STYLE_MAIN, &label_dialog_white);
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
        lv_refr_now(lv_refr_get_disp_refreshing());

        lv_obj_t *btnCancelprint = lv_button_btn_create(scr, 270, 203, 173, 78, btn_cancel_event_cb);
        lv_btn_set_style(btnCancelprint, LV_BTN_STYLE_REL, &tft_style_button_YELLOW);
        lv_btn_set_style(btnCancelprint, LV_BTN_STYLE_PR, &tft_style_button_YELLOW);
        lv_obj_t *labelCancel = lv_label_create_empty(btnCancelprint); // Add a label to the button
        lv_label_set_style(labelCancel, LV_LABEL_STYLE_MAIN, &label_dialog_white);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
        lv_refr_now(lv_refr_get_disp_refreshing());
    }
    else if (uiCfg.dialogType == DIALOG_TYPE_IF_ZERO)
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_warning.bin");
        lv_obj_set_pos(imgerr, 210, 58);

        lv_label_set_text(labelDialog, leveling_menu.if_zero);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, 0);

        btnOk = lv_btn_create(scr, btn_ok_event_cb, NULL, &tft_style_button_GREEN);
        lv_obj_set_size(btnOk, 194, 78);
        lv_obj_set_pos(btnOk, 31, 229);

        lv_obj_t *labelOk = lv_label_create_empty(btnOk);
        lv_obj_set_style(labelOk, &label_dialog_white);
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
        lv_obj_align(labelOk, btnOk, LV_ALIGN_CENTER, 0, 0);

        btnCancel = lv_btn_create(scr, btn_cancel_event_cb, NULL, &tft_style_button_GREEN);
        lv_obj_set_size(btnCancel, 194, 78);
        lv_obj_set_pos(btnCancel, 255, 229);

        lv_obj_t *labelCancel = lv_label_create_empty(btnCancel);
        lv_obj_set_style(labelCancel, &label_dialog_white);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
        lv_obj_align(labelCancel, btnCancel, LV_ALIGN_CENTER, 0, 0);
    }
    else if (uiCfg.dialogType == DIALOG_TYPE_IF_AUTO_LEVELING)
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_warning.bin");
        lv_obj_set_pos(imgerr, 210, 58);

        lv_label_set_text(labelDialog, leveling_menu.if_calibrate);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, 0);

        btnOk = lv_btn_create(scr, btn_ok_event_cb, NULL, &tft_style_button_GREEN);
        lv_obj_set_size(btnOk, 194, 78);
        lv_obj_set_pos(btnOk, 31, 229);

        lv_obj_t *labelOk = lv_label_create_empty(btnOk);
        lv_obj_set_style(labelOk, &label_dialog_white);
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
        lv_obj_align(labelOk, btnOk, LV_ALIGN_CENTER, 0, 0);

        btnCancel = lv_btn_create(scr, btn_cancel_event_cb, NULL, &tft_style_button_GREEN);
        lv_obj_set_size(btnCancel, 194, 78);
        lv_obj_set_pos(btnCancel, 255, 229);

        lv_obj_t *labelCancel = lv_label_create_empty(btnCancel);
        lv_obj_set_style(labelCancel, &label_dialog_white);
        lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
        lv_obj_align(labelCancel, btnCancel, LV_ALIGN_CENTER, 0, 0);
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_HOMING_WAIT)
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_homing_wait.bin");
        lv_obj_set_pos(imgerr, 198, 96);

        lv_label_set_text(labelDialog, leveling_menu.homing);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, 47);
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_PROBING_FAILED)
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_warning.bin");
        lv_obj_set_pos(imgerr, 198, 96);

        lv_label_set_text(labelDialog, leveling_menu.level_failed);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, 47);

        btnOk = lv_btn_create(scr, btn_ok_event_cb, NULL, &tft_style_button_GREEN);
        lv_obj_set_size(btnOk, 194, 64);
        lv_obj_set_pos(btnOk, 143, 240);

        lv_obj_t *labelOk = lv_label_create_empty(btnOk);
        lv_obj_set_style(labelOk, &label_dialog_white);
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
        lv_obj_align(labelOk, btnOk, LV_ALIGN_CENTER, 0, 0);
    }
    else if(uiCfg.dialogType == DIALOG_TYPE_AUTO_LEVEL_FINISH)
    {
        lv_obj_t *imgerr = lv_img_create(scr, NULL);
        lv_img_set_src(imgerr, "F:/bmp_warning.bin");
        lv_obj_set_pos(imgerr, 210, 20);

        lv_label_set_text(labelDialog, leveling_menu.leve_tip);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -30);

        // lv_obj_t *label_txt = lv_label_create(scr, "");
        // lv_obj_set_style(label_txt, &tft_style_preHeat_label);
        // lv_label_set_text(label_txt, "It can be adjusted up or down using   \nthe Z-Offset");
        // lv_obj_align(label_txt, nullptr, LV_ALIGN_CENTER, 0, 40);

        btnOk = lv_btn_create(scr, btn_ok_event_cb, NULL, &tft_style_button_GREEN);
        lv_obj_set_size(btnOk, 194, 64);
        lv_obj_set_pos(btnOk, 143, 240);

        lv_obj_t *labelOk = lv_label_create_empty(btnOk);
        lv_obj_set_style(labelOk, &label_dialog_white);
        lv_label_set_text(labelOk, print_file_dialog_menu.confirm);
        lv_obj_align(labelOk, btnOk, LV_ALIGN_CENTER, 0, 0);
    }
    else
    {
        btnOk = lv_button_btn_create(scr, 31, 229, 194, 78, btn_ok_event_cb);
        lv_obj_t *labelOk = lv_label_create_empty(btnOk); // Add a label to the button
        lv_refr_now(lv_refr_get_disp_refreshing());

        btnCancel = lv_button_btn_create(scr, 225, 229, 194, 78, btn_cancel_event_cb);
        lv_obj_t *labelCancel = lv_label_create_empty(btnCancel); // Add a label to the button
        lv_refr_now(lv_refr_get_disp_refreshing());

        if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
        {
            lv_label_set_text(labelOk, pause_msg_menu.purgeMore); // Set the labels text
            lv_label_set_text(labelCancel, pause_msg_menu.continuePrint);
        }
        else
        {
            lv_label_set_text(labelOk, print_file_dialog_menu.confirm); // Set the labels text
            lv_label_set_text(labelCancel, print_file_dialog_menu.cancel);
        }
    }

    // create label
    if (DIALOG_IS(TYPE_PRINT_FILE))
    {
        lv_label_set_text(labelDialog, print_file_dialog_menu.print_file);
        lv_obj_set_style(labelDialog, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -5);
        // lv_obj_set_pos(labelDialog, 142, 65);

        lv_obj_t *labelFile = lv_label_create(scr, list_file.long_name[sel_id]);
        lv_obj_set_style(labelFile, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelFile, nullptr, LV_ALIGN_CENTER, 0, -60);
    }
    else if (DIALOG_IS(TYPE_STOP))
    {
        lv_label_set_text(labelDialog, printing_MENU.stop_print); // print_file_dialog_menu.cancel_print
        lv_obj_set_style(labelDialog, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_PAUSE))
    {
        lv_label_set_text(labelDialog, printing_MENU.pause_print);
        lv_obj_set_style(labelDialog, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FINISH_PRINT))
    {
        lv_label_set_text(labelDialog, list_file.long_name[sel_id]);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -100);

        lv_obj_t *labelFPD = lv_label_create_empty(scr);
        lv_label_set_text(labelFPD, printing_MENU.file_printed);
        lv_obj_set_style(labelFPD, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelFPD, nullptr, LV_ALIGN_CENTER, 0, -40);

        lv_obj_t *labelFT = lv_label_create_empty(scr);
        // char FTIME[100];
        // sprintf(FTIME, "When %s", print_finish_time);
        lv_label_set_text(labelFT, print_finish_time);
        lv_obj_set_style(labelFT, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelFT, nullptr, LV_ALIGN_CENTER, 0, 0);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_PARKING))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.pausing);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_CHANGING))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.changing);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_UNLOAD))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.unload);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_WAITING))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.waiting);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_INSERT))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.insert);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_LOAD))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.load);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_PURGE))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.purge);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_RESUME))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.resume);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_HEAT))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.heat);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_HEATING))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.heating);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(PAUSE_MESSAGE_OPTION))
    {
        lv_label_set_text(labelDialog, pause_msg_menu.option);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(STORE_EEPROM_TIPS))
    {
        lv_label_set_text(labelDialog, eeprom_menu.storeTips);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(READ_EEPROM_TIPS))
    {
        lv_label_set_text(labelDialog, eeprom_menu.readTips);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(REVERT_EEPROM_TIPS))
    {
        lv_label_set_text(labelDialog, eeprom_menu.revertTips);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(WIFI_CONFIG_TIPS))
    {
        lv_label_set_text(labelDialog, machine_menu.wifiConfigTips);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(WIFI_ENABLE_TIPS))
    {
        lv_label_set_text(labelDialog, print_file_dialog_menu.wifi_enable_tips);
        lv_obj_set_style(labelDialog, &tft_style_preHeat_label_BLACK);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -60);

        lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
        lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
        lv_obj_set_size(imgtop, 480, 50);
        lv_obj_set_pos(imgtop, 0, 0);
        lv_obj_t *labelname = lv_label_create_empty(imgtop);
        lv_label_set_text(labelname, "WIFI");
        lv_obj_align(labelname, imgtop, LV_ALIGN_IN_LEFT_MID, 0, 0);
    }
    else if (DIALOG_IS(TRANSFER_NO_DEVICE))
    {
        lv_label_set_text(labelDialog, DIALOG_UPDATE_NO_DEVICE_EN);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
#if ENABLED(MKS_WIFI_MODULE)
    else if (DIALOG_IS(TYPE_UPLOAD_FILE))
    {
        if (upload_result == 1)
        {
            lv_label_set_text(labelDialog, DIALOG_UPLOAD_ING_EN);
            lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
        }
        else if (upload_result == 2)
        {
            lv_label_set_text(labelDialog, DIALOG_UPLOAD_ERROR_EN);
            lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
        }
        else if (upload_result == 3)
        {
            char buf[200];
            int _index = 0;

            strcpy_P(buf, PSTR(DIALOG_UPLOAD_FINISH_EN));
            _index = strlen(buf);
            buf[_index++] = '\n';
            strcat_P(buf, PSTR(DIALOG_UPLOAD_SIZE_EN));

            _index = strlen(buf);
            buf[_index++] = ':';
            sprintf_P(&buf[_index], PSTR(" %d KBytes\n"), (int)(upload_size / 1024));

            strcat_P(buf, PSTR(DIALOG_UPLOAD_TIME_EN));
            _index = strlen(buf);
            buf[_index++] = ':';
            sprintf_P(&buf[_index], PSTR(" %d s\n"), (int)upload_time_sec);

            strcat_P(buf, PSTR(DIALOG_UPLOAD_SPEED_EN));
            _index = strlen(buf);
            buf[_index++] = ':';
            sprintf_P(&buf[_index], PSTR(" %d KBytes/s\n"), (int)(upload_size / upload_time_sec / 1024));

            lv_label_set_text(labelDialog, buf);
            lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
        }
    }
    else if (DIALOG_IS(TYPE_UPDATE_ESP_FIRMWARE))
    {
        lv_label_set_text(labelDialog, DIALOG_UPDATE_WIFI_FIRMWARE_EN);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
#endif // MKS_WIFI_MODULE
    else if (DIALOG_IS(TYPE_FILAMENT_LOAD_HEAT))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_load_heat);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_HEAT_LOAD_COMPLETED))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_load_heat_confirm);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_UNLOAD_HEAT))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_unload_heat);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_HEAT_UNLOAD_COMPLETED))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_unload_heat_confirm);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_LOAD_COMPLETED))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_load_completed);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_UNLOAD_COMPLETED))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_unload_completed);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -20);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_LOADING))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_loading);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -70);
    }
    else if (DIALOG_IS(TYPE_FILAMENT_UNLOADING))
    {
        lv_label_set_text(labelDialog, filament_menu.filament_dialog_unloading);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -70);
    }
#if ENABLED(MKS_WIFI_MODULE)
    else if (DIALOG_IS(TYPE_UNBIND))
    {
        lv_label_set_text(labelDialog, common_menu.unbind_printer_tips);
        lv_obj_align(labelDialog, nullptr, LV_ALIGN_CENTER, 0, -70);
    }
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)
    else if (DIALOG_IS(TYPE_AUTO_LEVELING_TIPS))
    {
        lv_label_set_text(labelDialog, print_file_dialog_menu.autolevelingTips);
        lv_obj_align(labelDialog, NULL, LV_ALIGN_CENTER, 0, 0);
    }
#endif

#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
    {
        if (btnOk)
            lv_group_add_obj(g, btnOk);
        if (btnCancel)
            lv_group_add_obj(g, btnCancel);
    }
#endif
}

void filament_sprayer_temp()
{
    char buf[20] = {0};
    sprintf(buf, preheat_menu.value_state, thermalManager.wholeDegHotend(uiCfg.extruderIndex), thermalManager.degTargetHotend(uiCfg.extruderIndex));

    strcpy(public_buf_l, uiCfg.extruderIndex < 1 ? extrude_menu.ext1 : extrude_menu.ext2);
    strcat_P(public_buf_l, PSTR(": "));
    strcat(public_buf_l, buf);
    lv_label_set_text(tempText1, public_buf_l);
    lv_label_set_style(tempText1, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label);
    lv_obj_align(tempText1, nullptr, LV_ALIGN_CENTER, 0, -50);
}

void filament_dialog_handle()
{
    if (temps_update_flag && (DIALOG_IS(TYPE_FILAMENT_LOAD_HEAT, TYPE_FILAMENT_UNLOAD_HEAT)))
    {
        filament_sprayer_temp();
        temps_update_flag = false;
    }
    if (uiCfg.filament_heat_completed_load)
    {
        uiCfg.filament_heat_completed_load = false;
        lv_clear_dialog();
        lv_draw_dialog(DIALOG_TYPE_FILAMENT_LOADING);
        planner.synchronize();
        uiCfg.filament_loading_time_flg = true;
        uiCfg.filament_loading_time_cnt = 0;
        sprintf_P(public_buf_m, PSTR("T%d\nG91\nG1 E%d F%d\nG90"), uiCfg.extruderIndex, gCfgItems.filamentchange_load_length, gCfgItems.filamentchange_load_speed);
        queue.inject(public_buf_m);
    }
    if (uiCfg.filament_heat_completed_unload)
    {
        uiCfg.filament_heat_completed_unload = false;
        lv_clear_dialog();
        lv_draw_dialog(DIALOG_TYPE_FILAMENT_UNLOADING);
        planner.synchronize();
        uiCfg.filament_unloading_time_flg = true;
        uiCfg.filament_unloading_time_cnt = 0;
        sprintf_P(public_buf_m, PSTR("T%d\nG91\nG1 E-%d F%d\nG90"), uiCfg.extruderIndex, gCfgItems.filamentchange_unload_length, gCfgItems.filamentchange_unload_speed);
        queue.inject(public_buf_m);
    }

    if (uiCfg.filament_load_heat_flg)
    {
        const celsius_t diff = thermalManager.wholeDegHotend(uiCfg.extruderIndex) - gCfgItems.filament_limit_temp;
        if (ABS(diff) < 2 || diff > 0)
        {
            uiCfg.filament_load_heat_flg = false;
            lv_clear_dialog();
            lv_draw_dialog(DIALOG_TYPE_FILAMENT_HEAT_LOAD_COMPLETED);
        }
    }

    if (uiCfg.filament_loading_completed)
    {
        uiCfg.filament_rate = 0;
        uiCfg.filament_loading_completed = false;
        lv_clear_dialog();
        lv_draw_dialog(DIALOG_TYPE_FILAMENT_LOAD_COMPLETED);
    }

    if (uiCfg.filament_unload_heat_flg)
    {
        const celsius_t diff = thermalManager.wholeDegHotend(uiCfg.extruderIndex) - gCfgItems.filament_limit_temp;
        if (ABS(diff) < 2 || diff > 0)
        {
            uiCfg.filament_unload_heat_flg = false;
            lv_clear_dialog();
            lv_draw_dialog(DIALOG_TYPE_FILAMENT_HEAT_UNLOAD_COMPLETED);
        }
    }

    if (uiCfg.filament_unloading_completed)
    {
        uiCfg.filament_rate = 0;
        uiCfg.filament_unloading_completed = false;
        lv_clear_dialog();
        lv_draw_dialog(DIALOG_TYPE_FILAMENT_UNLOAD_COMPLETED);
    }

    if (DIALOG_IS(TYPE_FILAMENT_LOADING, TYPE_FILAMENT_UNLOADING))
        lv_filament_setbar();
}

void lv_filament_setbar()
{
    lv_bar_set_value(filament_bar, uiCfg.filament_rate, LV_ANIM_ON);
}

void start_printing()
{
#if HAS_GCODE_PREVIEW
    preview_gcode_prehandle(list_file.file_name[sel_id]);
#endif
    reset_print_time();
    start_print_time();
    baby_step_set_data = true;

    uiCfg.print_state = WORKING;
    layer_stop_flag = LAYER_STOP_NO_TRIGGERED;
    if (Layout_stop_num.already_obtained_flag)
        Layout_stop_num.already_obtained_flag = false;

    if (Layout_stop_num.bottom_already_obtained_flag)
        Layout_stop_num.bottom_already_obtained_flag = false;
    save_disp_layer_stop_num = 0;
    save_layer_stop_num = 0;
    clear_cur_ui();
    lv_draw_printing();

#if ENABLED(SDSUPPORT)
    if (!gcode_preview_over)
    {
        char *cur_name;
        cur_name = strrchr(list_file.file_name[sel_id], '/');
        cur_name++;
        SdFile file, *curDir;
        card.abortFilePrintNow();
        const char *const fname = card.diveToFile(false, curDir, cur_name);
        if (!fname)
            return;
        if (file.open(curDir, fname, O_READ))
        {
            gCfgItems.curFilesize = file.fileSize();
            file.close();
            update_spi_flash();
        }
        card.openFileRead(cur_name);
        if (card.isFileOpen())
        {
            feedrate_percentage = 100;
            planner.flow_percentage[0] = 100;
            planner.e_factor[0] = planner.flow_percentage[0] * 0.01f;
#if HAS_MULTI_EXTRUDER
            planner.flow_percentage[1] = 100;
            planner.e_factor[1] = planner.flow_percentage[1] * 0.01f;
#endif
            card.startOrResumeFilePrinting();
            TERN_(POWER_LOSS_RECOVERY, recovery.prepare());
            once_flag = false;
        }
    }
#endif
}

void lv_clear_dialog()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif
    lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
