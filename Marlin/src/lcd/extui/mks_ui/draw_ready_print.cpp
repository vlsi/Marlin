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

#include "draw_ready_print.h"
#include "draw_tool.h"
#include <lv_conf.h>
#include "tft_lvgl_configuration.h"
#include "draw_ui.h"
#include "../../../gcode/gcode.h"
#include "../../../gcode/queue.h"
#include "../Marlin/src/module/planner.h"
#include "../Marlin/src/module/stepper.h"

#include <lvgl.h>

#include "../../../module/temperature.h"
#include "../../../inc/MarlinConfig.h"

#if ENABLED(TOUCH_SCREEN_CALIBRATION)
  #include "../../tft_io/touch_calibration.h"
  #include "draw_touch_calibration.h"
#endif

#include "mks_hardware.h"
#include <stdio.h>

#define ICON_POS_Y          260
#define TARGET_LABEL_MOD_Y -36
#define LABEL_MOD_Y         30

extern lv_group_t*  g;
#ifndef USE_NEW_LVGL_CONF
  static lv_obj_t *scr;
#endif
static lv_obj_t *buttonExt1, *labelExt1, *buttonFanstate, *labelFan;
static lv_obj_t *img;
static lv_obj_t *btnPreheat;
static lv_obj_t *imgbtnPreheat, *buttonMove, *buttonFilament, *buttonTool, *buttonPrinting, *buttonEmergency;
static lv_obj_t *labelPreheat, *labelMove, *labelFilament, *labelTool, *labelPrinting, *labelEmergency;
static lv_obj_t *scr1;

static void Ready_Ui_Click(bool state);

#if HAS_MULTI_HOTEND
  static lv_obj_t *labelExt2;
  static lv_obj_t *buttonExt2;
#endif

#if HAS_HEATED_BED
  static lv_obj_t* labelBed;
  static lv_obj_t* buttonBedstate;
#endif

#if ENABLED(MKS_TEST)
  uint8_t current_disp_ui = 0;
#endif

enum { ID_PRE_HEAT = 1, ID_MOV, ID_FILAMENT_CHANGE, ID_TOOL, ID_PRINT, ID_EMERGENCY, ID_SET, ID_INFO_EXT, ID_INFO_BED, ID_INFO_FAN, ID_EMERGENCY_STOPED};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  // if(event == LV_EVENT_PRESSED)
  // {
  //   //voice_button_on();
  //   WRITE(BEEPER_PIN, HIGH);
  // }
  
  // if(event == LV_EVENT_PRESS_LOST)
  // {
  //   WRITE(BEEPER_PIN, LOW);
  // }

  // if(event == LV_EVENT_RELEASED)
  // {
  //   WRITE(BEEPER_PIN, LOW);
    lv_clear_ready_print();
    switch (obj->mks_obj_id) {
      case ID_PRE_HEAT:  
        // lv_clear_ready_print();
        lv_draw_preHeat(); 
        break;
      case ID_MOV: 
        // lv_clear_ready_print();   
        lv_draw_move_motor();
      break;
      case ID_FILAMENT_CHANGE:
        // lv_clear_ready_print(); 
        uiCfg.hotendTargetTempBak = thermalManager.degTargetHotend(active_extruder);  
        lv_draw_filament_change();
        break;
      case ID_TOOL: 
        // lv_clear_ready_print();  
        lv_draw_tool();
        break;
      case ID_PRINT:
        // lv_clear_ready_print(); 
        TERN(MULTI_VOLUME, lv_draw_media_select(), lv_draw_print_file()); 
        break;
      case ID_EMERGENCY: 
        Emergemcy_flog = true;
        queue.clear();
        memset(planner.block_buffer, 0, sizeof(planner.block_buffer));
        memset(queue.injected_commands, 0, sizeof(queue.injected_commands));
        while(planner.has_blocks_queued())
        {
          stepper.discard_current_block();
        }
        planner.quick_stop();
        // gcode.process_subcommands_now(PSTR("M410"));
        queue.inject_P(PSTR("M84"));
        queue.clear();

        uiCfg.curTempType = 0;
        thermalManager.setTargetHotend(0, uiCfg.extruderIndex);//設置溫度
        thermalManager.start_watching_hotend(uiCfg.extruderIndex);//檢測是否有降溫
        
        #if HAS_HEATED_BED
        uiCfg.curTempType = 1;
            thermalManager.temp_bed.target = 0;
            thermalManager.start_watching_bed();
        #endif
        
        lv_draw_emergemcy();
        break;
      case ID_EMERGENCY_STOPED:
        Emergemcy_flog = false;
        lv_draw_ready_print();
      default:
        break;
    }
  // }
}

lv_obj_t *limit_info, *det_info;
lv_style_t limit_style, det_style;
void disp_Limit_ok() {
  limit_style.text.color.full = 0xFFFF;
  lv_obj_set_style(limit_info, &limit_style);
  lv_label_set_text(limit_info, "Limit:ok");
}

void disp_Limit_error() {
  limit_style.text.color.full = 0xF800;
  lv_obj_set_style(limit_info, &limit_style);
  lv_label_set_text(limit_info, "Limit:error");
}

void disp_det_ok() {
  det_style.text.color.full = 0xFFFF;
  lv_obj_set_style(det_info, &det_style);
  lv_label_set_text(det_info, "det:ok");
}

void disp_det_error() {
  det_style.text.color.full = 0xF800;
  lv_obj_set_style(det_info, &det_style);
  lv_label_set_text(det_info, "det:error");
}

lv_obj_t *e1, *e2, *e3, *bed;

void mks_disp_test() {
  char buf[30] = {0};
  #if HAS_HOTEND
    sprintf_P(buf, PSTR("e1:%d"), thermalManager.wholeDegHotend(0));
    lv_label_set_text(e1, buf);
  #endif
  #if HAS_MULTI_HOTEND
    sprintf_P(buf, PSTR("e2:%d"), thermalManager.wholeDegHotend(1));
    lv_label_set_text(e2, buf);
  #endif
  #if HAS_HEATED_BED
    sprintf_P(buf, PSTR("bed:%d"), thermalManager.wholeDegBed());
    lv_label_set_text(bed, buf);
  #endif
}

void set_main_screen(void) {
#ifdef USE_NEW_LVGL_CONF
  mks_ui.src_main = lv_obj_create(nullptr, nullptr);
  lv_obj_set_style(mks_ui.src_main, &tft_style_scr);
  lv_scr_load(mks_ui.src_main);
#endif
}

void lv_draw_ready_print() {
  
  char buf[30] = {0};
  lv_obj_t *buttonPreheat;

  disp_state_stack._disp_index = 0;
  ZERO(disp_state_stack._disp_state);
  
#ifdef USE_NEW_LVGL_CONF
  mks_ui.src_main = lv_set_scr_id_title(mks_ui.src_main, PRINT_READY_UI, "");
#else 
  scr = lv_screen_create(PRINT_READY_UI, "");
  
  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_GREEN);
  lv_obj_set_size(imgtop, 150, 100);
  lv_obj_set_pos(imgtop, 8, 8);
  // lv_refr_now(lv_refr_get_disp_refreshing());
#endif

  if (mks_test_flag == 0x1E) {
    // Create image buttons
#ifdef USE_NEW_LVGL_CONF
    buttonPreheat = lv_imgbtn_create(mks_ui.src_main, "F:/bmp_tool.bin", event_handler, ID_TOOL);
#else
    // buttonPreheat = lv_imgbtn_create(scr, "F:/bmp_preHeat.bin", event_handler, ID_PRE_HEAT);//buttonTool = lv_imgbtn_create(scr, "F:/bmp_tool.bin", event_handler, ID_TOOL);
    // lv_obj_set_pos(buttonPreheat, 8, 8);
    // lv_big_button_create(scr, "F:/bmp_preHeat.bin", tool_menu.preheat, INTERVAL_V, titleHeight, event_handler, ID_T_PRE_HEAT);
#endif

    // lv_obj_set_pos(buttonPreheat, 8, 8);//lv_obj_set_pos(buttonTool, 360, 180);

    #if HAS_HOTEND
#ifdef USE_NEW_LVGL_CONF
      e1 = lv_label_create_empty(mks_ui.src_main);
#else
      e1 = lv_label_create_empty(scr);
#endif
      lv_obj_set_pos(e1, 20, 20);
      sprintf_P(buf, PSTR("e1:  %d"), thermalManager.wholeDegHotend(0));
      lv_label_set_text(e1, buf);
    #endif
    #if HAS_MULTI_HOTEND
      e2 = lv_label_create_empty(scr);
      lv_obj_set_pos(e2, 20, 45);
      sprintf_P(buf, PSTR("e2:  %d"), thermalManager.wholeDegHotend(1));
      lv_label_set_text(e2, buf);
    #endif
    #if HAS_HEATED_BED
#ifdef USE_NEW_LVGL_CONF
      bed = lv_label_create_empty(mks_ui.src_main);
#else
      bed = lv_label_create_empty(scr);
#endif
      lv_obj_set_pos(bed, 20, 95);
      sprintf_P(buf, PSTR("bed:  %d"), thermalManager.wholeDegBed());
      lv_label_set_text(bed, buf);
    #endif
#ifdef USE_NEW_LVGL_CONF
    limit_info = lv_label_create_empty(mks_ui.src_main);
#else
    limit_info = lv_label_create_empty(scr);
#endif
    lv_style_copy(&limit_style, &lv_style_scr);
    limit_style.body.main_color.full = 0x0000;
    limit_style.body.grad_color.full = 0x0000;
    limit_style.text.color.full      = 0xFFFF;
    lv_obj_set_style(limit_info, &limit_style);

    lv_obj_set_pos(limit_info, 20, 120);
    lv_label_set_text(limit_info, " ");

#ifdef USE_NEW_LVGL_CONF
    det_info = lv_label_create_empty(mks_ui.src_main);
#else 
    det_info = lv_label_create_empty(scr);
#endif
    lv_style_copy(&det_style, &lv_style_scr);
    det_style.body.main_color.full = 0x0000;
    det_style.body.grad_color.full = 0x0000;
    det_style.text.color.full      = 0xFFFF;
    lv_obj_set_style(det_info, &det_style);

    lv_obj_set_pos(det_info, 20, 145);
    lv_label_set_text(det_info, " ");
  }
  else {
#ifdef USE_NEW_LVGL_CONF
    lv_big_button_create(mks_ui.src_main, "F:/bmp_tool.bin", main_menu.tool, 20, 90, event_handler, ID_TOOL);
    lv_big_button_create(mks_ui.src_main, "F:/bmp_set.bin", main_menu.set, 180, 90, event_handler, ID_SET);
    lv_big_button_create(mks_ui.src_main, "F:/bmp_printing.bin", main_menu.print, 340, 90, event_handler, ID_PRINT);
#else
    buttonMove = lv_imgbtn_create(scr, "F:/bmp_mov.bin", event_handler, ID_MOV);
    lv_obj_set_pos(buttonMove, 165, 8);
    buttonFilament = lv_imgbtn_create(scr, "F:/bmp_filamentchange.bin", event_handler, ID_FILAMENT_CHANGE);
    lv_obj_set_pos(buttonFilament, 322, 8);   

    buttonTool = lv_imgbtn_create(scr, "F:/bmp_printing.bin", event_handler, ID_TOOL);
    lv_obj_set_pos(buttonTool, 165, 164);
    buttonPrinting = lv_imgbtn_create(scr, "F:/bmp_tool.bin", event_handler, ID_PRINT);
    lv_obj_set_pos(buttonPrinting, 8, 164);

    buttonEmergency = lv_imgbtn_create(scr, "F:/bmp_Emergency.bin", event_handler, ID_EMERGENCY);
    lv_obj_set_pos(buttonEmergency, 322, 164);
    labelMove = lv_label_create(scr , 171, 132, main_menu.move);
    lv_label_set_style(labelMove, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    labelFilament = lv_label_create(scr , 328, 132, tool_menu.filament);
    lv_label_set_style(labelFilament, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    labelTool = lv_label_create(scr , 171, 288, main_menu.tool);
    lv_label_set_style(labelTool, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    labelPrinting = lv_label_create(scr , 12, 288, main_menu.print);
    lv_label_set_style(labelPrinting, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    labelEmergency = lv_label_create(scr , 328, 288, main_menu.Emergency);
    lv_label_set_style(labelEmergency, LV_LABEL_STYLE_MAIN, &label_dialog_white);

    // if (TERN0(HAS_ROTARY_ENCODER, gCfgItems.encoder_enable)){
    //   lv_group_add_obj(g, buttonPreheat);
    //   lv_group_add_obj(g, buttonMove);
    //   lv_group_add_obj(g, buttonFilament);
    //   lv_group_add_obj(g, buttonTool);
    //   lv_group_add_obj(g, buttonPrinting);
    //   lv_group_add_obj(g, buttonEmergency);
    // }
    
#endif
    // Monitoring
    #if HAS_HOTEND
#ifdef USE_NEW_LVGL_CONF
      buttonExt1 = lv_big_button_create(mks_ui.src_main, "F:/bmp_ext1_state.bin", " ", 20, ICON_POS_Y, event_handler, ID_INFO_EXT);
#else
      // buttonExt1 = lv_big_button_create(scr, "F:/bmp_ext1_state.bin", " ", 20, ICON_POS_Y, event_handler, ID_INFO_EXT);
#endif
    #endif
    #if HAS_MULTI_HOTEND
      buttonExt2 = lv_big_button_create(scr, "F:/bmp_ext2_state.bin", " ", 180, ICON_POS_Y, event_handler, ID_INFO_EXT);
    #endif
    #if HAS_HEATED_BED
#ifdef USE_NEW_LVGL_CONF
      buttonBedstate = lv_big_button_create(mks_ui.src_main, "F:/bmp_bed_state.bin", " ", TERN(HAS_MULTI_HOTEND, 271, 210), ICON_POS_Y, event_handler, ID_INFO_BED);
#else
      // buttonBedstate = lv_big_button_create(scr, "F:/bmp_bed_state.bin", " ", TERN(HAS_MULTI_HOTEND, 340, 210), ICON_POS_Y, event_handler, ID_INFO_BED);
#endif
    #endif

  btnPreheat = lv_button_btn_create(scr, 8, 8, 150, 50, event_handler, ID_PRE_HEAT);
  lv_btn_set_style(btnPreheat, LV_BTN_STYLE_REL, &tft_style_preHeat_GREEN);
  lv_btn_set_style(btnPreheat, LV_BTN_STYLE_PR,  &tft_style_preHeat_GREEN);
  // lv_refr_now(lv_refr_get_disp_refreshing());

  // lv_refr_now(lv_refr_get_disp_refreshing());
  img = lv_img_create(scr, NULL);
  lv_img_set_src(img,"F:/bmp_ready_exit1.bin");
  lv_obj_set_pos(img, 11, 12);
  // lv_obj_align(img, btnPreheat, LV_ALIGN_IN_TOP_LEFT, 0, 0);

#ifdef USE_NEW_LVGL_CONF
    TERN_(HAS_HOTEND, labelExt1 = lv_label_create_empty(mks_ui.src_main));
    TERN_(HAS_MULTI_HOTEND, labelExt2 = lv_label_create_empty(mks_ui.src_main));
    TERN_(HAS_HEATED_BED, labelBed = lv_label_create_empty(mks_ui.src_main));
    TERN_(HAS_FAN, labelFan = lv_label_create_empty(mks_ui.src_main));
#else
    TERN_(HAS_HOTEND, labelExt1 = lv_label_create_empty(scr));
    lv_label_set_text(labelExt1, "0℃");
    lv_label_set_style(labelExt1,LV_LABEL_STYLE_MAIN,&tft_style_label_preheat);
    lv_obj_align(labelExt1, img, LV_ALIGN_OUT_RIGHT_TOP, 5, -5);
    // TERN_(HAS_MULTI_HOTEND, labelExt2 = lv_label_create_empty(scr));
    TERN_(HAS_HEATED_BED, labelBed = lv_label_create_empty(scr));
    lv_label_set_text(labelBed, "0℃");
    lv_label_set_style(labelBed,LV_LABEL_STYLE_MAIN,&tft_style_label_preheat);
    lv_obj_align(labelBed, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 5);
    // TERN_(HAS_FAN, labelFan = lv_label_create_empty(scr));
#endif

  imgbtnPreheat = lv_imgbtn_create(scr, "F:/bmp_preHeat1.bin", event_handler, ID_PRE_HEAT);
  lv_obj_align(imgbtnPreheat, btnPreheat, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  labelPreheat = lv_label_create(scr , 14, 132, main_menu.preheat);
  lv_label_set_style(labelPreheat, LV_LABEL_STYLE_MAIN, &label_dialog_white);



    lv_temp_refr();
  }

  #if ENABLED(TOUCH_SCREEN_CALIBRATION)
    // If calibration is required, let's trigger it now, handles the case when there is default value in configuration files
    if (!touch_calibration.calibration_loaded()) {
      lv_clear_ready_print();
      lv_draw_touch_calibration_screen();
    }
  #endif
}

void lv_temp_refr() {
  #if HAS_HOTEND
    // sprintf(public_buf_l, printing_menu.temp1, thermalManager.wholeDegHotend(0), thermalManager.degTargetHotend(0));
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegHotend(0));
    lv_label_set_text(labelExt1, public_buf_l);
    lv_label_set_style(labelExt1,LV_LABEL_STYLE_MAIN,&tft_style_label_preheat);
    lv_obj_align(labelExt1, img, LV_ALIGN_OUT_RIGHT_TOP, 5, -5);
  #endif
  #if HAS_MULTI_HOTEND
    sprintf(public_buf_l, printing_menu.temp1, thermalManager.wholeDegHotend(1), thermalManager.degTargetHotend(1));
    lv_label_set_text(labelExt2, public_buf_l);
    lv_obj_align(labelExt2, buttonExt2, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  #endif
  #if HAS_HEATED_BED
    // sprintf(public_buf_l, printing_menu.bed_temp, thermalManager.wholeDegBed(), thermalManager.degTargetBed());
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegBed());
    lv_label_set_text(labelBed, public_buf_l);
    lv_label_set_style(labelBed,LV_LABEL_STYLE_MAIN,&tft_style_label_preheat);
    lv_obj_align(labelBed, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 5);
    // lv_obj_align(labelBed, buttonBedstate, LV_ALIGN_OUT_RIGHT_MID, 0, -20);
  #endif
  #if HAS_FAN
    // sprintf_P(public_buf_l, PSTR("%d%%"), (int)thermalManager.fanSpeedPercent(0));
    // lv_label_set_text(labelFan, public_buf_l);
    // lv_obj_align(labelFan, img, LV_ALIGN_OUT_RIGHT_MID, 0, 30);
    // lv_obj_align(labelFan, buttonFanstate, LV_ALIGN_OUT_RIGHT_MID, 0, -30);
  #endif
}

void lv_clear_ready_print() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
#ifdef USE_NEW_LVGL_CONF
  lv_obj_clean(mks_ui.src_main);
#else
  lv_obj_del(scr);
#endif
}

void lv_draw_emergemcy()
{
  scr = lv_screen_create(PRINT_READY_UI, "");

  lv_obj_t *test = lv_label_create_empty(scr);
  lv_label_set_text(test, main_menu.Emergency);
  lv_obj_set_style(test, &tft_style_preHeat_label_BLACK);
  lv_obj_align(test, nullptr, LV_ALIGN_CENTER, 0, -50);

  lv_obj_t *btnStoped = lv_btn_create(scr, event_handler, ID_EMERGENCY_STOPED, nullptr);
  lv_btn_set_style(btnStoped, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
  lv_btn_set_style(btnStoped, LV_BTN_STYLE_PR,  &tft_style_button_GREEN);
  lv_obj_set_size(btnStoped, 140, 50);
  lv_obj_align(btnStoped, nullptr, LV_ALIGN_CENTER, 0, 50);
  lv_obj_t *labelStoped = lv_label_create_empty(btnStoped);
  lv_label_set_text(labelStoped, main_menu.ok);
  lv_obj_set_style(labelStoped, &label_dialog_white);
  lv_obj_align(labelStoped, nullptr, LV_ALIGN_CENTER, 0, 0);
}

void disp_emergency()
{
  lv_obj_t *btnStoped = lv_btn_create(scr, event_handler, ID_EMERGENCY_STOPED, nullptr);
  lv_btn_set_style(btnStoped, LV_BTN_STYLE_REL, &tft_style_button_GREEN);
  lv_btn_set_style(btnStoped, LV_BTN_STYLE_PR,  &tft_style_button_GREEN);
  lv_obj_set_size(btnStoped, 140, 50);
  lv_obj_align(btnStoped, nullptr, LV_ALIGN_CENTER, 0, 50);
  lv_obj_t *labelStoped = lv_label_create_empty(btnStoped);
  lv_label_set_text(labelStoped, main_menu.ok);
  lv_obj_set_style(labelStoped, &label_dialog_white);
  lv_obj_align(labelStoped, nullptr, LV_ALIGN_CENTER, 0, 0);
}

static void Ready_Ui_Click(bool state)
{
  lv_obj_set_click(btnPreheat, state);
  lv_obj_set_click(imgbtnPreheat, state);
  lv_obj_set_click(buttonMove, state);
  lv_obj_set_click(buttonFilament, state);
  lv_obj_set_click(buttonTool, state);
  lv_obj_set_click(buttonPrinting, state);
  lv_obj_set_click(buttonEmergency, state);
}
#endif // HAS_TFT_LVGL_UI
