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

#include "../../../module/temperature.h"
#include "../../../gcode/gcode.h"
#include "../../../module/motion.h"
#include "../../../module/planner.h"
#include "../../../inc/MarlinConfig.h"

extern lv_group_t *g;

#ifndef USE_NEW_LVGL_CONF
static lv_obj_t *scr;
#endif

static lv_obj_t *buttonType;
static lv_obj_t *buttonIn, *buttonOut;

static lv_obj_t *labelType;
static lv_obj_t *labelTemp;
static lv_obj_t *tempText1;
static lv_obj_t *labelSpeed;
static lv_obj_t *btnone;
static lv_obj_t *btnfive;
static lv_obj_t *btnten;
static lv_obj_t *btnauto;
extern bool more_filament;

typedef enum
{
    LOW_SPEED,
    HIGH_SPEED,
} FIlAMENT_SPEED;

typedef struct
{

    FIlAMENT_SPEED cur_speed_step = LOW_SPEED;
    uint16_t cur_speed = gCfgItems.filamentchange_load_speed;
    bool contimued = false;
    bool contimued_dir = false;
    bool contimued_open = false;
} FILAMNT_DATA;
FILAMNT_DATA fila_data;

enum
{
    ID_FILAMNT_IN = 1,
    ID_FILAMNT_OUT,
    ID_FILAMNT_TYPE,
    ID_FILAMNT_RETURN,
    ID_FILAMNT_HEAT,
    ID_FILAMNT_STOP,
    ID_FILAMNT_SPEED,
    ID_FILAMNT_ONE,
    ID_FILAMNT_FIVE,
    ID_FILAMNT_TEN,
    ID_FILAMNT_AUTO,
};

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    bool is_rb_full = true;
    if (event != LV_EVENT_RELEASED)
        return;

    voice_button_on();
    _delay_ms(100);
    WRITE(BEEPER_PIN, LOW);

    // if(!queue.ring_buffer.full(1)) {
    //   is_rb_full = true;
    // }else{
    //   is_rb_full = false;
    // }

    switch (obj->mks_obj_id)
    {
    case ID_FILAMNT_IN:
        // uiCfg.filament_load_heat_flg = true;
        // if (ABS(thermalManager.degTargetHotend(uiCfg.extruderIndex) - thermalManager.wholeDegHotend(uiCfg.extruderIndex)) <= 1
        //   || gCfgItems.filament_limit_temp <= thermalManager.wholeDegHotend(uiCfg.extruderIndex)
        // ) {
        //   lv_clear_filament_change();
        //   lv_draw_dialog(DIALOG_TYPE_FILAMENT_HEAT_LOAD_COMPLETED);
        // }
        // else {
        //   lv_clear_filament_change();
        //   lv_draw_dialog(DIALOG_TYPE_FILAMENT_LOAD_HEAT);
        //   if (thermalManager.degTargetHotend(uiCfg.extruderIndex) < gCfgItems.filament_limit_temp) {
        //     thermalManager.setTargetHotend(gCfgItems.filament_limit_temp, uiCfg.extruderIndex);
        //     thermalManager.start_watching_hotend(uiCfg.extruderIndex);
        //   }
        // }
        if (thermalManager.degTargetHotend(uiCfg.extruderIndex) && (ABS(thermalManager.degTargetHotend(uiCfg.extruderIndex) - thermalManager.wholeDegHotend(uiCfg.extruderIndex)) <= 1 || thermalManager.wholeDegHotend(uiCfg.extruderIndex) >= gCfgItems.filament_limit_temp))
        {

            if (fila_data.contimued)
            {
                if ((fila_data.contimued_open) && (!fila_data.contimued_dir))
                {
                    queue.clear();
                    fila_data.contimued_dir = true;
                    fila_data.contimued_open = true;
                    gcode.process_subcommands_now(PSTR("M410"));
                }
                // sprintf_P((char *)public_buf_l, PSTR("G91\nG1 E%d F%d\nG90"), 10, 60 * uiCfg.extruSpeed);
                // if(fila_data.cur_speed_step == LOW_SPEED)
                sprintf_P((char *)public_buf_l, PSTR("G91\nG1 E%d F%d\nG90"), 10, (gCfgItems.filamentchange_load_speed - 850));
                // else
                // sprintf_P((char *)public_buf_l, PSTR("G91\nG1 E%d F%d\nG90"), 10, (gCfgItems.filamentchange_load_speed -700));
                queue.inject(public_buf_l);
                fila_data.contimued_dir = true;
                fila_data.contimued_open = true;
            }
            else
            {
                sprintf_P((char *)public_buf_l, PSTR("G91\nG1 E%d F%d\nG90"), uiCfg.extruStep, 60 * uiCfg.extruSpeed);
                queue.inject(public_buf_l);
                // extrudeAmount += uiCfg.extruStep;
            }
            // disp_extru_amount();
        }
        break;
    case ID_FILAMNT_OUT:
        // uiCfg.filament_unload_heat_flg = true;
        // if (thermalManager.degTargetHotend(uiCfg.extruderIndex)
        //     && (ABS(thermalManager.degTargetHotend(uiCfg.extruderIndex) - thermalManager.wholeDegHotend(uiCfg.extruderIndex)) <= 1
        //         || thermalManager.wholeDegHotend(uiCfg.extruderIndex) >= gCfgItems.filament_limit_temp)
        // ) {
        //   lv_clear_filament_change();
        //   lv_draw_dialog(DIALOG_TYPE_FILAMENT_HEAT_UNLOAD_COMPLETED);
        // }
        // else {
        //   lv_clear_filament_change();
        //   lv_draw_dialog(DIALOG_TYPE_FILAMENT_UNLOAD_HEAT);
        //   if (thermalManager.degTargetHotend(uiCfg.extruderIndex) < gCfgItems.filament_limit_temp) {
        //     thermalManager.setTargetHotend(gCfgItems.filament_limit_temp, uiCfg.extruderIndex);
        //     thermalManager.start_watching_hotend(uiCfg.extruderIndex);
        //   }
        //   filament_sprayer_temp();
        // }
        if (thermalManager.degTargetHotend(uiCfg.extruderIndex) && (ABS(thermalManager.degTargetHotend(uiCfg.extruderIndex) - thermalManager.wholeDegHotend(uiCfg.extruderIndex)) <= 1 || thermalManager.wholeDegHotend(uiCfg.extruderIndex) >= gCfgItems.filament_limit_temp))
        {

            // disp_extru_amount();
            if (fila_data.contimued)
            {
                if ((fila_data.contimued_open) && (fila_data.contimued_dir))
                {
                    queue.clear();
                    fila_data.contimued_dir = false;
                    fila_data.contimued_open = true;
                    gcode.process_subcommands_now(PSTR("M410"));
                }

                sprintf_P((char *)public_buf_l, PSTR("G91\nG1 E%d F%d\nG90"), 3, (gCfgItems.filamentchange_load_speed - 850));
                gcode.process_subcommands_now(public_buf_l);
                fila_data.contimued_dir = false;
                fila_data.contimued_open = true;
            }
            else
            {
                sprintf_P((char *)public_buf_l, PSTR("G91\nG1 E%d F%d\nG90"), 0 - uiCfg.extruStep, 60 * uiCfg.extruSpeed);
                // queue.enqueue_one_now(public_buf_l);
                queue.inject(public_buf_l);
                // extrudeAmount -= uiCfg.extruStep;
            }
        }
        break;
    case ID_FILAMNT_TYPE:
#if HAS_MULTI_EXTRUDER
        uiCfg.extruderIndex = !uiCfg.extruderIndex;
#endif
        // disp_filament_type();
        break;
    // case ID_FILAMNT_SPEED:
    //   if((gCfgItems.filamentchange_load_speed % 1000) == 0)
    //   {
    //     // lv_label_set_text(labelSpeed, "low");
    //     gCfgItems.filamentchange_load_speed = 1500;
    //     gCfgItems.filamentchange_unload_speed = 1500;
    //     uiCfg.filament_loading_time = (uint32_t)((gCfgItems.filamentchange_load_length*60.0/gCfgItems.filamentchange_load_speed)+0.5);
    //     uiCfg.filament_unloading_time = (uint32_t)((gCfgItems.filamentchange_unload_length*60.0/gCfgItems.filamentchange_unload_speed)+0.5);
    //   }
    //   else if((gCfgItems.filamentchange_load_speed % 1000) == 500)
    //   {
    //     // lv_label_set_text(labelSpeed, "high");
    //     gCfgItems.filamentchange_load_speed = 1000;
    //     gCfgItems.filamentchange_unload_speed = 1000;
    //     uiCfg.filament_loading_time = (uint32_t)((gCfgItems.filamentchange_load_length*60.0/gCfgItems.filamentchange_load_speed)+0.5);
    //     uiCfg.filament_unloading_time = (uint32_t)((gCfgItems.filamentchange_unload_length*60.0/gCfgItems.filamentchange_unload_speed)+0.5);
    //   }
    //   refresh_filament_change();
    //   update_spi_flash();
    //   break;
    case ID_FILAMNT_ONE:
        if (!fila_data.contimued)
        {
            uiCfg.extruStep = uiCfg.eStepMin;
            // lv_group_remove_all_objs(g);
            // lv_group_add_obj(g, btnone);
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
        }
        break;
    case ID_FILAMNT_FIVE:
        if (!fila_data.contimued)
        {
            uiCfg.extruStep = uiCfg.eStepMed;
            // lv_group_remove_all_objs(g);
            // lv_group_add_obj(g, btnfive);
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
        }
        break;
    case ID_FILAMNT_TEN:
        if (!fila_data.contimued)
        {
            uiCfg.extruStep = uiCfg.eStepMax;
            // lv_group_remove_all_objs(g);
            // lv_group_add_obj(g, btnten);
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

            lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
        }
        break;
    case ID_FILAMNT_AUTO:
        if (!fila_data.contimued)
        {
            fila_data.contimued = true;
            // lv_group_remove_all_objs(g);
            // lv_group_add_obj(g, btnauto);
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_imgbtn_set_src_both(btnauto, "F:/png_loop_on.bin");
        }
        else
        {
            switch (uiCfg.extruStep)
            {
            case uiCfg.eStepMin:
                // lv_group_remove_all_objs(g);
                // lv_group_add_obj(g, btnone);
                lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
                lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

                lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
                lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

                lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
                lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

                lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
                break;
            case uiCfg.eStepMed:
                // lv_group_remove_all_objs(g);
                // lv_group_add_obj(g, btnfive);
                lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
                lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

                lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
                lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

                lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
                lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

                lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
                break;
            case uiCfg.eStepMax:
                // lv_group_remove_all_objs(g);
                // lv_group_add_obj(g, btnten);
                lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
                lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

                lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
                lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

                lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
                lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

                lv_imgbtn_set_src_both(btnauto, "F:/png_loop_off.bin");
                break;
            }
            lv_refr_now(lv_refr_get_disp_refreshing());

            if (fila_data.contimued_open)
            {
                fila_data.contimued_open = false;
                queue.clear();
                gcode.process_subcommands_now(PSTR("M410"));
            }
            fila_data.contimued = false;
        }
        break;
    case ID_FILAMNT_STOP:
        // sprintf(public_buf_l, "%d℃/0℃", thermalManager.wholeDegHotend(0));
        // lv_label_set_text(labelTemp, public_buf_l);

        // lv_refr_now(lv_refr_get_disp_refreshing());
        fila_data.contimued_open = false;
        queue.clear();
        gcode.process_subcommands_now(PSTR("M410"));
        thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);
        break;
    case ID_FILAMNT_RETURN:
#if 0
#if HAS_MULTI_EXTRUDER
        if (uiCfg.print_state != IDLE && uiCfg.print_state != REPRINTED)
          gcode.process_subcommands_now(uiCfg.extruderIndexBak == 1 ? PSTR("T1") : PSTR("T0"));
#endif
      feedrate_mm_s = (float)uiCfg.moveSpeed_bak;
      if (uiCfg.print_state == PAUSED)
        planner.set_e_position_mm((destination.e = current_position.e = uiCfg.current_e_position_bak));

      //thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);

      if(fila_data.contimued_open)
      {
        fila_data.contimued_open = false;
        gcode.process_subcommands_now(PSTR("M410"));
      }
      // lv_clear_filament_change();
      // lv_draw_tool();
      clear_cur_ui();
      draw_return_ui();
#else
        thermalManager.setTargetHotend(uiCfg.hotendTargetTempBak, uiCfg.extruderIndex);
        feedrate_mm_s = (float)uiCfg.moveSpeed_bak;
        if (fila_data.contimued_open)
        {
            fila_data.contimued_open = false;
            // planner.quick_stop();
            queue.clear();
            gcode.process_subcommands_now(PSTR("M410"));
            queue.clear();
        }
        clear_cur_ui();
        if (uiCfg.print_state == PAUSED)
            planner.set_e_position_mm((destination.e = current_position.e = uiCfg.current_e_position_bak));

        if (more_filament)
        {
            more_filament = false;
            feedrate_percentage = uiCfg.print_speed;
            planner.flow_percentage[0] = uiCfg.ext_speed;
            planner.e_factor[0] = planner.flow_percentage[0] * 0.01f;
            uiCfg.print_speed = 100;
            uiCfg.ext_speed = 100;
            clear_cur_ui();
            lv_draw_operation();
        }
        else
        {
            if ((uiCfg.print_state == PAUSING) || (uiCfg.print_state == PAUSED))
            {
                if (gCfgItems.from_flash_pic)
                    flash_preview_begin = true;
                else
                    default_preview_flg = true;
                feedrate_percentage = uiCfg.print_speed;
                planner.flow_percentage[0] = uiCfg.ext_speed;
                planner.e_factor[0] = planner.flow_percentage[0] * 0.01f;

                uiCfg.print_speed = 100;
                uiCfg.ext_speed = 100;
                lv_draw_printing();
            }
            else
            {
                feedrate_mm_s = (float)uiCfg.moveSpeed_bak;
                draw_return_ui();
            }
        }

#endif
        break;
    case ID_FILAMNT_HEAT:
        // lv_clear_filament_change();
        // lv_draw_preHeat();

        sprintf(public_buf_l, "%d℃/%d℃", thermalManager.wholeDegHotend(0), gCfgItems.filament_limit_temp);
        lv_label_set_text(labelTemp, public_buf_l);
        uiCfg.curTempType = 0;
        thermalManager.setTargetHotend(gCfgItems.filament_limit_temp, uiCfg.extruderIndex);
        thermalManager.start_watching_hotend(uiCfg.extruderIndex); // 檢測是否有降溫
        break;
    }
}

void lv_draw_filament_change()
{
    scr = lv_screen_create(FILAMENTCHANGE_UI);

    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    lv_obj_t *labelname = lv_label_create_empty(scr);
    lv_label_set_text(labelname, tool_menu.filament);
    lv_obj_set_style(labelname, &label_dialog_white);
    lv_obj_set_pos(labelname, 69, 13);

    //停止坐标
    lv_obj_t *btnStop = lv_btn_create(scr, event_handler, ID_FILAMNT_STOP, nullptr);
    lv_btn_set_style(btnStop, LV_BTN_STYLE_REL, &tft_style_button_RED);
    lv_btn_set_style(btnStop, LV_BTN_STYLE_PR, &tft_style_button_RED);
    lv_obj_set_size(btnStop, 120, 70);
    lv_obj_set_pos(btnStop, 26, 230);
    lv_obj_t *labelStop = lv_label_create_empty(btnStop);
    lv_label_set_text(labelStop, main_menu.stop);
    lv_label_set_style(labelStop, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_obj_align(labelStop, btnStop, LV_ALIGN_CENTER, 0, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    //图标
    lv_obj_t *imgExt = lv_img_create(scr, NULL);
    lv_img_set_src(imgExt, "F:/bmp_filamentchange_ext.bin");
    lv_obj_align(imgExt, scr, LV_ALIGN_CENTER, -20, 10);

    //温度
    labelTemp = lv_label_create_empty(scr);
    sprintf(public_buf_l, "%d℃/%d℃", thermalManager.wholeDegHotend(0), thermalManager.degTargetHotend(uiCfg.extruderIndex));
    lv_label_set_text(labelTemp, public_buf_l);
    lv_label_set_style(labelTemp, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
    lv_obj_align(labelTemp, imgExt, LV_ALIGN_CENTER, 0, -100);

    //进料
    buttonIn = lv_imgbtn_create(scr, "F:/bmp_in.bin", event_handler, ID_FILAMNT_IN);
    lv_obj_align(buttonIn, imgExt, LV_ALIGN_OUT_LEFT_TOP, -10, -10);

    //退料
    buttonOut = lv_imgbtn_create(scr, "F:/bmp_out.bin", event_handler, ID_FILAMNT_OUT);
    lv_obj_align(buttonOut, imgExt, LV_ALIGN_OUT_RIGHT_TOP, 10, -10);

    //加热图标
    lv_obj_t *btnHeating = lv_btn_create(scr, event_handler, ID_FILAMNT_HEAT, nullptr);
    lv_btn_set_style(btnHeating, LV_BTN_STYLE_REL, &tft_style_filament_YELLOW);
    lv_btn_set_style(btnHeating, LV_BTN_STYLE_PR, &tft_style_filament_YELLOW);
    lv_obj_set_size(btnHeating, 120, 70);
    lv_obj_set_pos(btnHeating, 270, 230);

    //加热文本
    lv_obj_t *labelHeating = lv_label_create_empty(btnHeating);
    lv_label_set_text(labelHeating, main_menu.Heating_nozzle);
    lv_label_set_style(labelHeating, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_obj_align(labelHeating, btnHeating, LV_ALIGN_CENTER, 0, 0);

    btnone = lv_btn_create(scr, event_handler, ID_FILAMNT_ONE, nullptr);
    lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
    lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
    lv_obj_set_size(btnone, 70, 59);
    lv_obj_set_pos(btnone, 400, 63);
    lv_obj_t *labelone = lv_label_create_empty(btnone);
    lv_label_set_text(labelone, "1mm");
    lv_obj_set_style(labelone, &label_dialog_white);
    lv_obj_align(labelone, nullptr, LV_ALIGN_CENTER, 0, 0);

    btnfive = lv_btn_create(scr, event_handler, ID_FILAMNT_FIVE, nullptr);
    lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
    lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
    lv_obj_set_size(btnfive, 70, 59);
    lv_obj_align(btnfive, btnone, LV_ALIGN_CENTER, 0, 62);
    lv_obj_t *labelfive = lv_label_create_empty(btnfive);
    lv_label_set_text(labelfive, "5mm");
    lv_obj_set_style(labelfive, &label_dialog_white);
    lv_obj_align(labelfive, nullptr, LV_ALIGN_CENTER, 0, 0);

    btnten = lv_btn_create(scr, event_handler, ID_FILAMNT_TEN, nullptr);
    lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
    lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
    lv_obj_set_size(btnten, 70, 59);
    lv_obj_align(btnten, btnfive, LV_ALIGN_CENTER, 0, 62);
    lv_obj_t *labelten = lv_label_create_empty(btnten);
    lv_label_set_text(labelten, "10mm");
    lv_obj_set_style(labelten, &label_dialog_white);
    lv_obj_align(labelten, nullptr, LV_ALIGN_CENTER, 0, 0);

    //   #if HAS_ROTARY_ENCODER
    // if (gCfgItems.encoder_enable)
    // {
    if (fila_data.contimued == false)
    {
        switch (uiCfg.extruStep)
        {
        case uiCfg.eStepMin:
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);
            break;
        case uiCfg.eStepMed:
            // lv_group_remove_all_objs(g);
            // lv_group_add_obj(g, btnfive);
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);
            break;
        case uiCfg.eStepMax:
            lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
            lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

            lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
            lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE);
            break;
        }
        btnauto = lv_imgbtn_create(scr, "F:/png_loop_off.bin", event_handler, ID_FILAMNT_AUTO);
        lv_obj_align(btnauto, btnten, LV_ALIGN_CENTER, 0, 62);
    }
    else
    {
        lv_btn_set_style(btnone, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
        lv_btn_set_style(btnone, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

        lv_btn_set_style(btnfive, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
        lv_btn_set_style(btnfive, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

        lv_btn_set_style(btnten, LV_BTN_STYLE_REL, &tft_style_button_BLUE_WHITE);
        lv_btn_set_style(btnten, LV_BTN_STYLE_PR, &tft_style_button_BLUE_WHITE);

        btnauto = lv_imgbtn_create(scr, "F:/png_loop_on.bin", event_handler, ID_FILAMNT_AUTO);
        lv_obj_align(btnauto, btnten, LV_ALIGN_CENTER, 0, 62);
    }

    lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_FILAMNT_RETURN);
    lv_obj_set_pos(buttonReturn, 6, 3);
}

void disp_filament_type()
{
    if (uiCfg.extruderIndex == 1)
    {
        lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru2.bin");
        if (gCfgItems.multiple_language)
        {
            lv_label_set_text(labelType, preheat_menu.ext2);
            lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
    }
    else
    {
        lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru1.bin");
        if (gCfgItems.multiple_language)
        {
            lv_label_set_text(labelType, preheat_menu.ext1);
            lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
    }
}

void disp_filament_temp()
{
    char buf[20] = {0};
    char buff[50];
    public_buf_l[0] = '\0';

    if ((fila_data.contimued_open) && (fila_data.contimued))
    {
        memset(buff, 0, sizeof(buff));
        if (fila_data.contimued_dir)
        {
            sprintf_P((char *)buff, PSTR("G91\nG1 E%d F%d\nG90"), gCfgItems.filamentchange_load_length, (gCfgItems.filamentchange_load_speed - 850));
            queue.inject(buff);
        }
        else
        {
            sprintf_P((char *)buff, PSTR("G91\nG1 E-%d F%d\nG90"), gCfgItems.filamentchange_load_length, (gCfgItems.filamentchange_load_speed - 850));
            queue.inject(buff);
        }
    }

    sprintf(public_buf_l, "%d℃/%d℃", thermalManager.wholeDegHotend(0), thermalManager.degTargetHotend(uiCfg.extruderIndex));
    lv_label_set_text(labelTemp, public_buf_l);
}

void lv_clear_filament_change()
{
#if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable)
        lv_group_remove_all_objs(g);
#endif

#ifdef USE_NEW_LVGL_CONF
    lv_obj_clean(mks_ui.src_main);
#else
    lv_obj_del(scr);
#endif
}

void lv_filament_temp_refr()
{
    sprintf(public_buf_l, "%d℃/%d℃", thermalManager.wholeDegHotend(0), thermalManager.degTargetHotend(uiCfg.extruderIndex)); // thermalManager.wholeDegBed()
    lv_label_set_text(labelTemp, public_buf_l);
}

void refresh_filament_change()
{
    if ((gCfgItems.filamentchange_load_speed % 1000) == 0)
    {
        lv_label_set_text(labelSpeed, main_menu.low);
    }
    else if ((gCfgItems.filamentchange_load_speed % 1000) == 500)
    {
        lv_label_set_text(labelSpeed, main_menu.high);
    }
}

#endif // HAS_TFT_LVGL_UI
