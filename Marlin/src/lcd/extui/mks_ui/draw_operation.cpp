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
#include "../../../module/motion.h"
#include "../../../sd/cardreader.h"
#include "../../../inc/MarlinConfig.h"
#include "../../../gcode/queue.h"
extern lv_group_t *g;

extern int32_t disp_layer_stop_num;
extern int32_t save_disp_layer_stop_num;

static lv_obj_t *scr;
static lv_obj_t *btn_zoffset, *btn_fan;
static lv_obj_t *label_zoffset, *label_fan;
static lv_obj_t *btn_levelstop, *btn_op_filament, *btn_light;
static lv_obj_t *label_levelstop, *label_op_filament, *label_light;
enum {
  ID_O_PRE_HEAT = 1,
  ID_O_EXTRUCT,
  ID_O_MOV,
  ID_O_LAYER_STOP,
  ID_O_FILAMENT,
  ID_O_SPEED,
  ID_O_RETURN,
  ID_O_FAN,
  ID_O_POWER_OFF,
  ID_O_BABY_STEP,
  ID_O_ZOFFSET,
  ID_O_LIGHT,
};
bool more_filament = false;

static lv_obj_t *label_PowerOff;
static lv_obj_t *buttonPowerOff;
uint8_t op_count = 0;

lv_obj_t *buttonExtrusion = nullptr, *buttonSpeed = nullptr,
          *buttonBack = nullptr,
          *buttonMove = nullptr, 
          *buttonBabyStep = nullptr,
          *labelPreHeat = nullptr, *labelExtrusion = nullptr,
          *label_Back = nullptr, *label_Speed = nullptr, *label_Fan = nullptr,
          *label_Move = nullptr,
          *label_BabyStep = nullptr,
          *label_Filament = nullptr;

lv_obj_t *buttonPreHeat;
lv_obj_t *buttonFilament;
lv_obj_t *buttonFan;

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW); 

  switch (obj->mks_obj_id) {
    case ID_O_PRE_HEAT:
      lv_clear_operation();
      lv_draw_preHeat();
      break;
    case ID_O_LAYER_STOP:
      disp_layer_stop_num = save_disp_layer_stop_num;

      lv_clear_operation();
      lv_draw_layer_stop();
      break;
    case ID_O_EXTRUCT:
      lv_clear_operation();
      lv_draw_extrusion();
      break;
    case ID_O_MOV:
      lv_clear_operation();
      lv_draw_move_motor();
      break;
    case ID_O_FILAMENT:
      #if HAS_MULTI_EXTRUDER
        uiCfg.extruderIndexBak = active_extruder;
      #endif
      if (uiCfg.print_state == WORKING) {
        #if ENABLED(SDSUPPORT)
          card.pauseSDPrint();
          stop_print_time();
          uiCfg.print_state = PAUSING;
        #endif
      }
      uiCfg.moveSpeed_bak = (uint16_t)feedrate_mm_s;
      uiCfg.hotendTargetTempBak = thermalManager.degTargetHotend(active_extruder);
      more_filament = true;
      lv_clear_operation();
      lv_draw_filament_change();
      break;
    case ID_O_FAN:
      // lv_clear_operation();
      // lv_draw_fan();
      temp_value = fan_speed;
      keyboard_value = GTempsetting;
      lv_clear_operation();
      lv_draw_numkeyboard();
      break;
    case ID_O_SPEED:
      lv_clear_operation();
      lv_draw_change_speed();
      break;
    case ID_O_RETURN:
      if (gCfgItems.from_flash_pic)
      flash_preview_begin = true;
      else
      default_preview_flg = true;
      clear_cur_ui();
      lv_draw_printing();
      break;
    case ID_O_POWER_OFF:
      if (gCfgItems.finish_power_off) {
        gCfgItems.finish_power_off = false;
        lv_imgbtn_set_src_both(buttonPowerOff, "F:/bmp_manual_off.bin");
        lv_label_set_text(label_PowerOff, printing_more_menu.manual);
      }
      else {
        gCfgItems.finish_power_off = true;
        lv_imgbtn_set_src_both(buttonPowerOff, "F:/bmp_auto_off.bin");
        lv_label_set_text(label_PowerOff, printing_more_menu.auto_close);
      }
      lv_obj_align(label_PowerOff, buttonPowerOff, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
      lv_obj_refresh_ext_draw_pad(label_PowerOff);
      update_spi_flash();
      break;
    case ID_O_BABY_STEP:
      lv_clear_operation();
      lv_draw_baby_stepping();
      break;

    case ID_O_LIGHT:
        gCfgItems.floodlight_state = !gCfgItems.floodlight_state;
        lv_light_ref();
        if(!gCfgItems.floodlight_state)
        {
            digitalWrite(HEATER_1_PIN, LOW);
        }
        else
        {
            digitalWrite(HEATER_1_PIN, HIGH);
        }
        break;
  }
}

void lv_draw_operation() {
  scr = lv_screen_create(OPERATE_UI);
  
  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);

#if 1
  btn_zoffset = lv_imgbtn_create(scr, "F:/bmp_more_zoffset.bin", event_handler, ID_O_BABY_STEP);
  lv_obj_set_pos(btn_zoffset, 63, 66);
  label_zoffset = lv_label_create_empty(scr);
  lv_label_set_style(label_zoffset, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_label_set_text(label_zoffset, leveling_menu.zoffset);
  lv_obj_align(label_zoffset, btn_zoffset, LV_ALIGN_CENTER, 0, 50);

  btn_fan = lv_imgbtn_create(scr, "F:/bmp_fan_state.bin", event_handler, ID_O_FAN);
  lv_obj_set_pos(btn_fan, 357, 70);
  label_fan = lv_label_create_empty(scr);
  lv_label_set_style(label_fan, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_obj_align(label_fan, btn_fan, LV_ALIGN_CENTER, 0, 50);
  disp_more_fan_speed();
  
  btn_levelstop = lv_imgbtn_create(scr, "F:/more_layer_stop.bin", event_handler, ID_O_LAYER_STOP);
  lv_obj_set_pos(btn_levelstop, 225, 70);
  label_levelstop = lv_label_create_empty(scr);
  lv_label_set_style(label_levelstop, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_label_set_text(label_levelstop, printing_MENU.layer_stop/*leveling_menu.levelstop*/);
  lv_obj_align(label_levelstop, btn_levelstop, LV_ALIGN_CENTER, -10, 50);

  btn_op_filament = lv_imgbtn_create(scr, "F:/more_filament.bin", event_handler, ID_O_FILAMENT);
  lv_obj_set_pos(btn_op_filament, 74, 206);
  label_op_filament = lv_label_create_empty(scr);
  lv_label_set_style(label_op_filament, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_label_set_text(label_op_filament, printing_MENU.filament/*leveling_menu.op_filament*/);
  lv_obj_align(label_op_filament, btn_op_filament, LV_ALIGN_CENTER, 0, 50);

  lv_refr_now(lv_refr_get_disp_refreshing());
  
  //灯带开关
  btn_light = lv_imgbtn_create(scr, nullptr, event_handler, ID_O_LIGHT);
  lv_obj_set_pos(btn_light, 203, 193);
  label_light = lv_label_create_empty(scr);
  lv_label_set_style(label_light, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_label_set_text(label_light, "light strip");
  lv_obj_align(label_light, btn_light, LV_ALIGN_CENTER, 100, 50);
  lv_light_ref();
  
  lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_O_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);

  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, printing_more_menu.title);
  lv_obj_set_style(labelname, &label_dialog_white);
  lv_obj_set_pos(labelname, 69, 13);

#else
  // Create image buttons
  lv_obj_t *buttonPreHeat  = lv_imgbtn_create(scr, "F:/bmp_temp.bin", INTERVAL_V, titleHeight, event_handler, ID_O_PRE_HEAT);
  lv_obj_t *buttonFilament = lv_imgbtn_create(scr, "F:/bmp_filamentchange.bin", BTN_X_PIXEL + INTERVAL_V * 2, titleHeight, event_handler, ID_O_FILAMENT);
  lv_obj_t *buttonFan      = lv_imgbtn_create(scr, "F:/bmp_fan.bin", BTN_X_PIXEL * 2 + INTERVAL_V * 3, titleHeight, event_handler, ID_O_FAN);
  buttonPowerOff = lv_imgbtn_create(scr, gCfgItems.finish_power_off ? "F:/bmp_auto_off.bin" : "F:/bmp_manual_off.bin", BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_O_POWER_OFF);

  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) {
      lv_group_add_obj(g, buttonPreHeat);
      lv_group_add_obj(g, buttonFilament);
      lv_group_add_obj(g, buttonFan);
      lv_group_add_obj(g, buttonPowerOff);
    }
  #endif

  if (uiCfg.print_state != WORKING) {
    buttonExtrusion = lv_imgbtn_create(scr, "F:/bmp_extrude_opr.bin", INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_O_EXTRUCT);
    buttonMove      = lv_imgbtn_create(scr, "F:/bmp_move_opr.bin", BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_O_MOV);
    #if HAS_ROTARY_ENCODER
      if (gCfgItems.encoder_enable) {
        lv_group_add_obj(g, buttonExtrusion);
        lv_group_add_obj(g, buttonMove);
      }
    #endif
  }
  else {
    buttonSpeed    = lv_imgbtn_create(scr, "F:/bmp_speed.bin", INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_O_SPEED);
    buttonBabyStep = lv_imgbtn_create(scr, "F:/bmp_mov.bin", BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_O_BABY_STEP);
    #if HAS_ROTARY_ENCODER
      if (gCfgItems.encoder_enable) {
        lv_group_add_obj(g, buttonSpeed);
        lv_group_add_obj(g, buttonBabyStep);
      }
    #endif
  }

  buttonBack = lv_imgbtn_create(scr, "F:/bmp_return.bin", BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_O_RETURN);

  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_add_obj(g, buttonBack);
  #endif

  // Create labels on the image buttons
  labelPreHeat   = lv_label_create_empty(buttonPreHeat);
  label_Filament = lv_label_create_empty(buttonFilament);
  label_Fan      = lv_label_create_empty(buttonFan);
  label_PowerOff = lv_label_create_empty(buttonPowerOff);

  if (uiCfg.print_state != WORKING) {
    labelExtrusion = lv_label_create_empty(buttonExtrusion);
    label_Move = lv_label_create_empty(buttonMove);
  }
  else {
    label_Speed = lv_label_create_empty(buttonSpeed);
    label_BabyStep = lv_label_create_empty(buttonBabyStep);
  }
  label_Back = lv_label_create_empty(buttonBack);

  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelPreHeat, operation_menu.temp);
    lv_obj_align(labelPreHeat, buttonPreHeat, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

    lv_label_set_text(label_Filament, operation_menu.filament);
    lv_obj_align(label_Filament, buttonFilament, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

    lv_label_set_text(label_Fan, operation_menu.fan);
    lv_obj_align(label_Fan, buttonFan, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

    if (gCfgItems.finish_power_off)
      lv_label_set_text(label_PowerOff, printing_more_menu.auto_close);
    else
      lv_label_set_text(label_PowerOff, printing_more_menu.manual);
    lv_obj_align(label_PowerOff, buttonPowerOff, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

    if (uiCfg.print_state != WORKING) {
      lv_label_set_text(labelExtrusion, operation_menu.extr);
      lv_obj_align(labelExtrusion, buttonExtrusion, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

      lv_label_set_text(label_Move, operation_menu.move);
      lv_obj_align(label_Move, buttonMove, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else {
      lv_label_set_text(label_Speed, operation_menu.speed);
      lv_obj_align(label_Speed, buttonSpeed, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

      lv_label_set_text(label_BabyStep, operation_menu.babystep);
      lv_obj_align(label_BabyStep, buttonBabyStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }

    lv_label_set_text(label_Back, common_menu.text_back);
    lv_obj_align(label_Back, buttonBack, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
  }
  #endif
}

// void draw_operation_btn(void) {
//   buttonPreHeat  = lv_imgbtn_create(scr, "F:/bmp_temp.bin", INTERVAL_V, titleHeight, event_handler, ID_O_PRE_HEAT);
//   buttonFilament = lv_imgbtn_create(scr, "F:/bmp_filamentchange.bin", BTN_X_PIXEL + INTERVAL_V * 2, titleHeight, event_handler, ID_O_FILAMENT);
//   buttonFan      = lv_imgbtn_create(scr, "F:/bmp_fan.bin", BTN_X_PIXEL * 2 + INTERVAL_V * 3, titleHeight, event_handler, ID_O_FAN);
//   buttonPowerOff = lv_imgbtn_create(scr, gCfgItems.finish_power_off ? "F:/bmp_auto_off.bin" : "F:/bmp_manual_off.bin", BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_O_POWER_OFF);
// }

// void draw_opration_label(void) {
//   labelPreHeat   = lv_label_create_empty(buttonPreHeat);
//   label_Filament = lv_label_create_empty(buttonFilament);
//   label_Fan      = lv_label_create_empty(buttonFan);
//   label_PowerOff = lv_label_create_empty(buttonPowerOff);
// }



void lv_light_ref()
{
    if(gCfgItems.floodlight_state)
    {
        lv_imgbtn_set_src_both(btn_light, "F:/bmp_light_on.bin");
    }
    else
    {
        lv_imgbtn_set_src_both(btn_light, "F:/bmp_light_off.bin");
    }
    lv_obj_refresh_ext_draw_pad(btn_light);
    lv_refr_now(lv_refr_get_disp_refreshing());
}


void lv_clear_operation() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

void disp_more_fan_speed() {

  sprintf_P(public_buf_l, PSTR("%d%%"), (int)thermalManager.fanSpeedPercent(0));
  lv_label_set_text(label_fan, public_buf_l);
  lv_obj_align(label_fan, btn_fan, LV_ALIGN_CENTER, 0, 46);

}
#endif // HAS_TFT_LVGL_UI
