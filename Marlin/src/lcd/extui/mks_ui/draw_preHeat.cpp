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
#include "../../../inc/MarlinConfig.h"

#ifndef USE_NEW_LVGL_CONF
  static lv_obj_t *scr;
#endif

extern lv_group_t*  g;
static lv_obj_t *buttonType, *buttonStep, *buttonAdd, *buttonDec, *buttonBedAdd, *buttonBedDec;
static lv_obj_t *labelType;
static lv_obj_t *labelStep;
static lv_obj_t *tempText1;
static lv_obj_t *btn_pla;
static lv_obj_t *btn_abs;
static lv_obj_t *label_abs;
static lv_obj_t *label_pla;
static lv_obj_t* labelBed;
static lv_obj_t* labelExt1;
static lv_obj_t *imgExt1;
static lv_obj_t *imgBed;
static lv_obj_t *imgabs;
static lv_obj_t *imgpla;
static lv_obj_t *labelExt1Heat;
static lv_obj_t *labelBedHeat;

static lv_style_t btn_style_pre;
static lv_style_t btn_style_rel;
static lv_obj_t *imgtop, *buttonReturn;

enum {
  ID_P_ADD = 1,
  ID_P_DEC,
  ID_P_TYPE,
  ID_P_STEP,
  ID_P_OFF,
  ID_P_RETURN,
  ID_P_ABS,
  ID_P_PLA,
  ID_P_ADD_BED,
  ID_P_DEC_BED
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  // lv_clear_preHeat();
  switch (obj->mks_obj_id) {
    case ID_P_ADD: {
      uiCfg.curTempType = 0;
      if (uiCfg.curTempType == 0) {
        int16_t max_target;
        thermalManager.temp_hotend[uiCfg.extruderIndex].target += uiCfg.stepHeat;
// #ifdef WATCH_TEMP_INCREASE
        if (uiCfg.extruderIndex == 0){
            max_target = 400/*HEATER_0_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1)*/;
          
        }
        else {
          #if HAS_MULTI_HOTEND
            max_target = 400/*HEATER_1_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1)*/;
          #endif
        }
        max_target = 400;
        if (thermalManager.degTargetHotend(uiCfg.extruderIndex) > max_target)
          thermalManager.setTargetHotend(max_target, uiCfg.extruderIndex);
        thermalManager.start_watching_hotend(uiCfg.extruderIndex);
// #endif
      }
      else {
        #if HAS_HEATED_BED
          thermalManager.temp_bed.target += uiCfg.stepHeat;
          // #ifdef WATCH_BED_TEMP_INCREASE
          constexpr int16_t max_target = 120;
          // constexpr int16_t max_target = BED_MAXTEMP - (WATCH_BED_TEMP_INCREASE + TEMP_BED_HYSTERESIS + 1);
          if (thermalManager.degTargetBed() > max_target)
            thermalManager.setTargetBed(max_target);
          thermalManager.start_watching_bed();
          // #endif
        #endif
      }
      disp_desire_temp();
    } break;
    case ID_P_ADD_BED:{
      uiCfg.curTempType = 1;
      if (uiCfg.curTempType == 0) {
        int16_t max_target = 120;
        thermalManager.temp_hotend[uiCfg.extruderIndex].target += uiCfg.stepHeat;
// #ifdef WATCH_TEMP_INCREASE
        // if (uiCfg.extruderIndex == 0){
        //     max_target = HEATER_0_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1);
          
        // }
        // else {
        //   #if HAS_MULTI_HOTEND
        //     max_target = HEATER_1_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1);
        //   #endif
        // }
        if (thermalManager.degTargetHotend(uiCfg.extruderIndex) > max_target)
          thermalManager.setTargetHotend(max_target, uiCfg.extruderIndex);
        thermalManager.start_watching_hotend(uiCfg.extruderIndex);
// #endif
      }
      else {
        #if HAS_HEATED_BED
          thermalManager.temp_bed.target += uiCfg.stepHeat;
          // #ifdef WATCH_BED_TEMP_INCREASE
          constexpr int16_t max_target = 120/*BED_MAXTEMP - (WATCH_BED_TEMP_INCREASE + TEMP_BED_HYSTERESIS + 1)*/;
          if (thermalManager.degTargetBed() > max_target)
            thermalManager.setTargetBed(max_target);
          thermalManager.start_watching_bed();
          // #endif
        #endif
      }
      disp_desire_temp();
    }break;

    case ID_P_DEC:
      uiCfg.curTempType = 0;
      if (uiCfg.curTempType == 0) {
        if (thermalManager.degTargetHotend(uiCfg.extruderIndex) > uiCfg.stepHeat)
          thermalManager.temp_hotend[uiCfg.extruderIndex].target -= uiCfg.stepHeat;
        else
          thermalManager.setTargetHotend(0, uiCfg.extruderIndex);
        thermalManager.start_watching_hotend(uiCfg.extruderIndex);
      }
      else {
        #if HAS_HEATED_BED
          if (thermalManager.degTargetBed() > uiCfg.stepHeat)//温度是否大于增加或减少值
            thermalManager.temp_bed.target -= uiCfg.stepHeat;//温度数值
          else
            thermalManager.setTargetBed(0);//将温度设置为0

          thermalManager.start_watching_bed();//检测温度是否发生变化
        #endif
      }
      disp_desire_temp();
      break;
    case ID_P_DEC_BED:
      uiCfg.curTempType = 1;
      if (uiCfg.curTempType == 0) {
        if (thermalManager.degTargetHotend(uiCfg.extruderIndex) > uiCfg.stepHeat)
          thermalManager.temp_hotend[uiCfg.extruderIndex].target -= uiCfg.stepHeat;
        else
          thermalManager.setTargetHotend(0, uiCfg.extruderIndex);
        thermalManager.start_watching_hotend(uiCfg.extruderIndex);
      }
      else {
        #if HAS_HEATED_BED
          if (thermalManager.degTargetBed() > uiCfg.stepHeat)
            thermalManager.temp_bed.target -= uiCfg.stepHeat;
          else
            thermalManager.setTargetBed(0);

          thermalManager.start_watching_bed();
        #endif
      }
      disp_desire_temp();
      break;

    case ID_P_TYPE:
      if (uiCfg.curTempType == 0) {
        if (ENABLED(HAS_MULTI_EXTRUDER)) {
          if (uiCfg.extruderIndex == 0) {
            uiCfg.extruderIndex = 1;
          }
          else if (uiCfg.extruderIndex == 1) {
            if (ENABLED(HAS_HEATED_BED)) {
              uiCfg.curTempType = 1;
            }
            else {
              uiCfg.curTempType = 0;
              uiCfg.extruderIndex = 0;
            }
          }
        }
        else if (uiCfg.extruderIndex == 0) {
          uiCfg.curTempType = TERN(HAS_HEATED_BED, 1, 0);
        }
      }
      else if (uiCfg.curTempType == 1) {
        uiCfg.extruderIndex = 0;
        uiCfg.curTempType = 0;
      }
      disp_temp_type();
      break;
    case ID_P_STEP:
      switch (uiCfg.stepHeat) {
        case  1: uiCfg.stepHeat =  5; break;
        case  5: uiCfg.stepHeat = 10; break;
        case 10: uiCfg.stepHeat =  1; break;
        default: break;
      }
      // disp_step_heat();
      break;
    case ID_P_OFF:
      // if (uiCfg.curTempType == 0) {
      //   thermalManager.setTargetHotend(0, uiCfg.extruderIndex);
      //   thermalManager.start_watching_hotend(uiCfg.extruderIndex);
      // }
      // else {
      //   #if HAS_HEATED_BED
      //     thermalManager.temp_bed.target = 0;
      //     thermalManager.start_watching_bed();
      //   #endif
      // }
      // disp_desire_temp();

      uiCfg.curTempType = 0;
      thermalManager.setTargetHotend(0, uiCfg.extruderIndex);//設置溫度
      thermalManager.start_watching_hotend(uiCfg.extruderIndex);//檢測是否有降溫
      disp_desire_temp();//刷新界面
      uiCfg.curTempType = 1;
      #if HAS_HEATED_BED
          //thermalManager.temp_bed.target = 0;
          thermalManager.setTargetBed(0);
          thermalManager.start_watching_bed();
      #endif
      disp_desire_temp();
      break;
    case ID_P_RETURN:
      clear_cur_ui();
      draw_return_ui();
      // lv_clear_preHeat();
      // lv_draw_ready_print();
      break;
    case ID_P_ABS:
      // if (uiCfg.curTempType == 0)
      //   thermalManager.setTargetHotend(PREHEAT_2_TEMP_HOTEND, 0);
      // else if (uiCfg.curTempType == 1)
      //   #if HAS_HEATED_BED
      //   thermalManager.setTargetBed(PREHEAT_2_TEMP_BED);
      //   #endif
      uiCfg.curTempType = 0;
      // thermalManager.setTargetHotend(PREHEAT_2_TEMP_HOTEND, 0);
      thermalManager.setTargetHotend(gCfgItems.ABS_EXT1, uiCfg.extruderIndex);
      thermalManager.start_watching_hotend(uiCfg.extruderIndex);//檢測是否有降溫
      disp_desire_temp();
      uiCfg.curTempType = 1;
      #if HAS_HEATED_BED
        // thermalManager.setTargetBed(PREHEAT_2_TEMP_BED);
        thermalManager.setTargetBed(gCfgItems.ABS_BED);
          thermalManager.start_watching_bed();
      #endif
      disp_desire_temp();
        
      break;
    case ID_P_PLA:
      // if (uiCfg.curTempType == 0)
      //   thermalManager.setTargetHotend(PREHEAT_1_TEMP_HOTEND, 0);
      // else if (uiCfg.curTempType == 1)
      //   #if HAS_HEATED_BED
      //   thermalManager.setTargetBed(PREHEAT_1_TEMP_BED);
      //   #endif
      uiCfg.curTempType = 0;
      // thermalManager.setTargetHotend(PREHEAT_1_TEMP_HOTEND, 0);
      thermalManager.setTargetHotend(gCfgItems.PLA_EXT1, uiCfg.extruderIndex);
      thermalManager.start_watching_hotend(uiCfg.extruderIndex);//檢測是否有降溫
      disp_desire_temp();
      uiCfg.curTempType = 1;
      #if HAS_HEATED_BED
        // thermalManager.setTargetBed(PREHEAT_1_TEMP_BED);
        thermalManager.setTargetBed(gCfgItems.PLA_BED);
        thermalManager.start_watching_bed();
      #endif
      disp_desire_temp();
      break;
    default:
      break;
  }
}

void disp_add_dec() {
  char buf[20] = {0};
  // Create image buttons
#ifndef USE_NEW_LVGL_CONF
  // buttonAdd = lv_imgbtn_create(scr, "F:/bmp_preHeat_Add1.bin", event_handler, ID_P_ADD);
  // lv_obj_set_pos(buttonAdd, 18, 55);
  buttonDec = lv_imgbtn_create(scr, "F:/bmp_preHeat_Dec.bin", event_handler, ID_P_DEC);
  // lv_obj_set_style(buttonDec, &tft_style_preHeat_WHITE_CIRCLE);
  lv_obj_set_pos(buttonDec, 272, 65);
  buttonBedAdd = lv_imgbtn_create(scr, "F:/bmp_preHeat_Add1.bin", event_handler, ID_P_ADD_BED);
  // lv_obj_set_style(buttonBedAdd, &tft_style_preHeat_WHITE_CIRCLE);
  lv_obj_set_pos(buttonBedAdd, 8, 207);
  buttonBedDec = lv_imgbtn_create(scr, "F:/bmp_preHeat_Dec.bin", event_handler, ID_P_DEC_BED);
  // lv_obj_set_style(buttonBedDec, &tft_style_preHeat_WHITE_CIRCLE);
  lv_obj_set_pos(buttonBedDec, 272, 207);

  // lv_refr_now(lv_refr_get_disp_refreshing());
#else
  buttonAdd = lv_big_button_create(mks_ui.src_main, "F:/bmp_Add.bin", preheat_menu.add, INTERVAL_V, titleHeight, event_handler, ID_P_ADD);
  buttonDec = lv_big_button_create(mks_ui.src_main, "F:/bmp_Dec.bin", preheat_menu.dec, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_P_DEC);
#endif

  btn_abs = lv_imgbtn_create(scr, "F:/bmp_preHeat_red.bin", event_handler, ID_P_ABS);
  lv_obj_set_pos(btn_abs, 387, 143);
  btn_pla = lv_imgbtn_create(scr, "F:/bmp_preHeat_red.bin", event_handler, ID_P_PLA);
  lv_obj_set_pos(btn_pla, 387, 53);

  // if (TERN0(HAS_ROTARY_ENCODER, gCfgItems.encoder_enable)){
  //   lv_group_add_obj(g, buttonAdd);
  //   lv_group_add_obj(g, buttonDec);
  //   lv_group_add_obj(g, buttonBedAdd);
  //   lv_group_add_obj(g, buttonBedDec);
  //   lv_group_add_obj(g, btn_abs);
  //   lv_group_add_obj(g, btn_pla);
  // }

  // imgabs = lv_img_create(scr, NULL);
  // lv_img_set_src(imgabs, "F:/bmp_preHeat_abs.bin");
  imgabs = lv_label_create(scr, PREHEAT_2_LABEL);
  lv_label_set_style(imgabs, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_scr);
  lv_obj_align(imgabs, btn_abs, 0, 0, 0);//LV_ALIGN_IN_BOTTOM_MID

  imgpla = lv_label_create(scr, PREHEAT_1_LABEL);
  lv_label_set_style(imgpla, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_scr);
  lv_obj_align(imgpla, btn_pla, 0, 0, 0);

  labelExt1Heat = lv_label_create(scr, "210℃");
  uiCfg.curTempType = 0;
  disp_desire_temp();

  labelBedHeat = lv_label_create(scr, "110℃");
  uiCfg.curTempType = 1;
  disp_desire_temp();
}

void disp_preHeat_img()
{
    lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);
    lv_refr_now(lv_refr_get_disp_refreshing());

    imgExt1 = lv_img_create(scr, NULL);
    lv_img_set_src(imgExt1, "F:/bmp_preHeat_Ext1.bin");
    // lv_obj_set_style(imgExt1, &tft_style_preHeat_scr);
    lv_img_set_style(imgExt1 , LV_IMG_STYLE_MAIN , &tft_style_preHeat_scr);
    lv_obj_set_pos(imgExt1, 270, 8);

    lv_obj_t *imgExt1_2 = lv_img_create(scr, NULL);
    lv_img_set_src(imgExt1_2, "F:/bmp_preHeat_Ext1_W.bin");
    lv_obj_set_pos(imgExt1_2, 115, 114);

    imgBed = lv_img_create(scr, NULL);
    lv_img_set_src(imgBed, "F:/bmp_preHeat_Bed.bin");
    lv_obj_set_pos(imgBed, 380, 8);

    lv_obj_t *imgBed_2 = lv_img_create(scr, NULL);
    lv_img_set_src(imgBed_2, "F:/bmp_preHeat_Bed_W.bin");
    lv_obj_set_pos(imgBed_2, 114, 256);

    lv_obj_t *labelExt1_2 = lv_label_create(scr, "Extrusion1");
    lv_label_set_style(labelExt1_2, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label);
    lv_obj_align(labelExt1_2 , imgExt1_2, LV_ALIGN_OUT_RIGHT_MID , 0, 0);
    // lv_obj_set_pos(labelExt1_2 , 164 , 119);

    lv_obj_t *labelBed_2 = lv_label_create(scr, "HeatBed");
    lv_label_set_style(labelBed_2, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label);
    lv_obj_align(labelBed_2 , imgBed_2, LV_ALIGN_OUT_RIGHT_MID , 0, 0);
    // lv_obj_set_pos(labelBed_2 , 190 , 271);

}

void lv_draw_preHeat() {
#ifndef USE_NEW_LVGL_CONF
  scr = lv_screen_create(PRE_HEAT_UI, "");
  // scr = lv_screen_create(PRINT_READY_UI, "");
#else 
  mks_ui.src_main = lv_set_scr_id_title(mks_ui.src_main, PRE_HEAT_UI, "");
#endif
  // Create image
  imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);
  
  lv_obj_t *imgExt1_2 = lv_img_create(scr, NULL);
  lv_img_set_src(imgExt1_2, "F:/bmp_preHeat_Ext1_W.bin");
  lv_obj_set_pos(imgExt1_2, 125, 124);

  lv_obj_t *imgBed_2 = lv_img_create(scr, NULL);
  lv_img_set_src(imgBed_2, "F:/bmp_preHeat_Bed_W.bin");
  lv_obj_set_pos(imgBed_2, 125, 266);

  lv_obj_t *labelExt1_2 = lv_label_create(scr, main_menu.Ext);
  lv_label_set_style(labelExt1_2, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);//tft_style_preHeat_label
  lv_obj_align(labelExt1_2 , imgExt1_2, LV_ALIGN_OUT_RIGHT_MID , 10, 0);
  // lv_obj_set_pos(labelExt1_2 , 164 , 119);

  lv_obj_t *labelBed_2 = lv_label_create(scr, main_menu.hotbed);
  lv_label_set_style(labelBed_2, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);//tft_style_preHeat_label
  lv_obj_align(labelBed_2 , imgBed_2, LV_ALIGN_OUT_RIGHT_MID , 10, 0);
  // Create image buttons
  disp_add_dec();
  // disp_ext_heart();

#ifndef USE_NEW_LVGL_CONF
  // buttonType = lv_imgbtn_create(scr, nullptr, INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_TYPE);
  // buttonStep = lv_imgbtn_create(scr, nullptr, BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_STEP);
#else
  buttonType = lv_imgbtn_create(mks_ui.src_main, nullptr, INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_TYPE);
  buttonStep = lv_imgbtn_create(mks_ui.src_main, nullptr, BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_STEP);
#endif

#ifndef USE_NEW_LVGL_CONF
  lv_obj_t *buttonspeed0 = lv_imgbtn_create(scr, "F:/bmp_preHeat_speed0.bin", event_handler, ID_P_OFF);
  lv_obj_set_pos(buttonspeed0, 387, 233);

#else
  lv_big_button_create(mks_ui.src_main, "F:/bmp_speed0.bin", preheat_menu.off, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_OFF);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_RETURN);
#endif
  // Create labels on the image buttons
  // labelType = lv_label_create_empty(buttonType);
  // labelStep = lv_label_create_empty(buttonStep);

  // disp_temp_type();
  // disp_step_heat();

#ifndef USE_NEW_LVGL_CONF
  // tempText1 = lv_label_create_empty(scr);//温度显示label
  // lv_obj_set_style(tempText1, &tft_style_label_rel);
#else
  tempText1 = lv_label_create_empty(mks_ui.src_main);
#endif
  // disp_desire_temp();
  buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_P_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);
  buttonAdd = lv_imgbtn_create(scr, "F:/bmp_preHeat_Add1.bin", event_handler, ID_P_ADD);
  lv_obj_set_pos(buttonAdd, 8, 65);

  lv_refr_now(lv_refr_get_disp_refreshing());
  imgExt1 = lv_img_create(scr, NULL);
  lv_img_set_src(imgExt1, "F:/bmp_preHeat_Ext1.bin");
  lv_img_set_style(imgExt1 , LV_IMG_STYLE_MAIN , &tft_style_preHeat_scr);
  lv_obj_set_pos(imgExt1, 270, 8);

  imgBed = lv_img_create(scr, NULL);
  lv_img_set_src(imgBed, "F:/bmp_preHeat_Bed.bin");
  lv_obj_set_pos(imgBed, 380, 8);

#ifdef USE_NEW_LVGL_CONF
    TERN_(HAS_HOTEND, labelExt1 = lv_label_create_empty(mks_ui.src_main));
    TERN_(HAS_MULTI_HOTEND, labelExt2 = lv_label_create_empty(mks_ui.src_main));
    TERN_(HAS_HEATED_BED, labelBed = lv_label_create_empty(mks_ui.src_main));
    TERN_(HAS_FAN, labelFan = lv_label_create_empty(mks_ui.src_main));
#else
    TERN_(HAS_HOTEND, labelExt1 = lv_label_create_empty(scr));
    // lv_label_set_text(labelExt1, "0℃");
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegHotend(0));
    lv_label_set_text(labelExt1, public_buf_l);
    lv_label_set_style(labelExt1,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelExt1, imgExt1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    TERN_(HAS_HEATED_BED, labelBed = lv_label_create_empty(scr));
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegBed());
    lv_label_set_text(labelBed, public_buf_l);
    lv_label_set_style(labelBed,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelBed, imgBed, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
#endif
  // lv_refr_now(lv_refr_get_disp_refreshing());
  // lv_refr_now(lv_refr_get_disp_refreshing());
  // lv_imgbtn_set_src_both(buttonReturn, "F:/bmp_preHeat_return.bin");
  
  // lv_preHeat_temp_refr();
  // lv_imgbtn_set_src_both(buttonAdd, "F:/bmp_preHeat_Add1.bin");
}

void disp_ext_heart() {
#ifndef USE_NEW_LVGL_CONF
  // btn_abs = lv_btn_create(scr, 160, 40, 80, 40, event_handler, ID_P_ABS);
  // btn_pla = lv_btn_create(scr, 260, 40, 80, 40, event_handler, ID_P_PLA);

  // imgabs = lv_label_create_empty(btn_abs);
  // imgpla = lv_label_create_empty(btn_pla);
#else
  btn_abs = lv_btn_create(mks_ui.src_main, 160, 40, 80, 40, event_handler, ID_P_ABS);
  btn_pla = lv_btn_create(mks_ui.src_main, 260, 40, 80, 40, event_handler, ID_P_PLA);
#endif

  // lv_btn_set_style(btn_abs, LV_BTN_STYLE_PR, &btn_style_pre);
  // lv_btn_set_style(btn_abs, LV_BTN_STYLE_REL, &btn_style_rel);
  // lv_btn_set_style(btn_pla, LV_BTN_STYLE_PR, &btn_style_pre);
  // lv_btn_set_style(btn_pla, LV_BTN_STYLE_REL, &btn_style_rel);

  // label_abs = lv_label_create(btn_abs, PREHEAT_2_LABEL);
  // label_pla = lv_label_create(btn_pla, PREHEAT_1_LABEL);
}

void disp_temp_type() {
  if (uiCfg.curTempType == 0) {
    if (uiCfg.extruderIndex == 1) {
      lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru2.bin");
      if (gCfgItems.multiple_language) {
        lv_label_set_text(labelType, preheat_menu.ext2);
        lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
      }
    }
    else {
      lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru1.bin");
      if (gCfgItems.multiple_language) {
        lv_label_set_text(labelType, preheat_menu.ext1);
        lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
      }
    }
  }
  else {
    lv_imgbtn_set_src_both(buttonType, "F:/bmp_bed.bin");
    if (gCfgItems.multiple_language) {
      lv_label_set_text(labelType, preheat_menu.hotbed);
      lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
  }
}

void  disp_desire_temp() {
  char buf[20] = { 0 };
  public_buf_l[0] = '\0';

  if (uiCfg.curTempType == 0) {
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.degTargetHotend(uiCfg.extruderIndex));
  }
  else {
    #if HAS_HEATED_BED
      sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.degTargetBed());
    #endif
  }

  if (uiCfg.curTempType == 0) {
    lv_label_set_text(labelExt1Heat, public_buf_l);
    lv_label_set_style(labelExt1Heat, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
    lv_obj_set_pos(labelExt1Heat , 148, 66);
  }
  else{
    lv_label_set_text(labelBedHeat, public_buf_l);
    lv_label_set_style(labelBedHeat, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
    lv_obj_set_pos(labelBedHeat , 150, 210);
  }
}

void disp_step_heat() {

}

void lv_preHeat_temp_refr() {
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegHotend(0));
    lv_label_set_text(labelExt1, public_buf_l);
    sprintf(public_buf_l, TEMP_UNIT_SYBOL, thermalManager.wholeDegBed());
    lv_label_set_text(labelBed, public_buf_l);
}

void lv_clear_preHeat() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
#ifndef USE_NEW_LVGL_CONF
  lv_obj_del(scr);
#else
  lv_obj_clean(mks_ui.src_main);
#endif
}

#endif // HAS_TFT_LVGL_UI
