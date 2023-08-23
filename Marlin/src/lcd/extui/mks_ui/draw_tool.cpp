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
#include "draw_endstop.h"

#include "../../../gcode/queue.h"
#include "../../../module/temperature.h"
#include "../../../inc/MarlinConfig.h"

extern lv_group_t *g;

#ifndef USE_NEW_LVGL_CONF
static lv_obj_t *scr;
#endif

enum {
  ID_T_PRE_HEAT = 1,
  ID_T_EXTRUCT,
  ID_T_MOV,
  ID_T_HOME,
  ID_T_LEVELING,
  ID_T_FILAMENT,
  ID_T_RETURN,
  ID_T_GCODE,
  ID_T_ABOUT,
  ID_T_LANGUAGE,
  ID_T_ENDSTOP,
  ID_T_Conveyor,
  ID_T_WIFI,
  ID_T_TEMPSETTING,
  ID_T_VOICE,
};

#if ENABLED(MKS_TEST)
  extern uint8_t current_disp_ui;
#endif

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  // if (TERN1(AUTO_BED_LEVELING_BILINEAR, obj->mks_obj_id != ID_T_LEVELING))

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  // lv_clear_tool();
  switch (obj->mks_obj_id) {
    case ID_T_PRE_HEAT: lv_clear_tool();lv_draw_preHeat(); break;
    case ID_T_EXTRUCT:  lv_clear_tool();lv_draw_extrusion(); break;
    case ID_T_MOV:      lv_clear_tool();lv_draw_move_motor(); break;
    case ID_T_HOME:     lv_clear_tool();lv_draw_home(); break;
    case ID_T_LEVELING:   
        lv_clear_tool();
        lv_draw_dialog(DIALOG_TYPE_IF_ZERO);
      break;

    case ID_T_FILAMENT:
      lv_clear_tool();
      uiCfg.hotendTargetTempBak = thermalManager.degTargetHotend(uiCfg.extruderIndex);
      lv_draw_filament_change();
      break;

    case ID_T_Conveyor:
      lv_clear_tool();
      lv_draw_Conveyor(); 
      break;

    case ID_T_TEMPSETTING:
      lv_clear_tool();
      lv_draw_tempsetting();
      break;

    case ID_T_WIFI:
      lv_clear_tool();
      if (gCfgItems.wifi_mode_sel == STA_MODEL) {
          if (wifi_link_state == WIFI_CONNECTED) {
            last_disp_state = SET_UI;
            lv_draw_wifi();
          }
          else {
            if (uiCfg.command_send) {
              uint8_t cmd_wifi_list[] = { 0xA5, 0x07, 0x00, 0x00, 0xFC };
              raw_send_to_wifi(cmd_wifi_list, COUNT(cmd_wifi_list));
              last_disp_state = SET_UI;
              lv_draw_wifi_list();
            }
            else {
              last_disp_state = SET_UI;
              lv_draw_dialog(DIALOG_WIFI_ENABLE_TIPS);
            }
          }
        }
      else {
        last_disp_state = SET_UI;

        lv_draw_wifi();
      }
      break;

    case ID_T_ENDSTOP:
      lv_clear_tool();
      lv_draw_endstop();
      break;

    case ID_T_VOICE:
      lv_clear_tool();
      lv_draw_voice();
      break;

    case ID_T_RETURN:
      lv_clear_tool();
      TERN_(MKS_TEST, current_disp_ui = 1);
      lv_draw_ready_print();
      break;

    case ID_T_GCODE: 
      lv_clear_tool();
      keyboard_value = GCade;
      lv_draw_keyboard();
      break;

    case ID_T_ABOUT:
      lv_clear_tool();
      lv_draw_about();
      break;

    case ID_T_LANGUAGE:
      lv_clear_tool();
      lv_draw_language();
      break;

    default: 
      break;
  }
}

void lv_draw_tool()
{
  scr = lv_screen_create(TOOL_UI);

  //传动带
  lv_obj_t *btnConveyor = lv_imgbtn_create(scr, "F:/bmp_Conveyor.bin", event_handler, ID_T_Conveyor);
  lv_obj_set_pos(btnConveyor, 241, 9);
  lv_obj_t *labelConveyor = lv_label_create_empty(btnConveyor);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelConveyor, tool_menu.Conveyor);
    lv_label_set_style(labelConveyor,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelConveyor, btnConveyor, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }
  
  //WIFI
  lv_obj_t *btnWifi = lv_imgbtn_create(scr, "F:/bmp_wifi.bin", event_handler, ID_T_WIFI);
  lv_obj_set_pos(btnWifi, 357, 9);
  lv_obj_t *labelWifi = lv_label_create_empty(btnWifi);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelWifi, "WiFi");
    lv_label_set_style(labelWifi,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelWifi, btnWifi, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //温度设置
  lv_obj_t *btnTempsetting = lv_imgbtn_create(scr, "F:/bmp_tool_tempset.bin", event_handler, ID_T_TEMPSETTING);
  lv_obj_set_pos(btnTempsetting, 9, 127);
  lv_obj_t *labelTempsetting = lv_label_create_empty(btnTempsetting);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelTempsetting, tool_menu.tempsetting);
    lv_label_set_style(labelTempsetting,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelTempsetting, btnTempsetting, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //voice
  lv_obj_t *btnVoice = lv_imgbtn_create(scr, "F:/bmp_tool_voice.bin", event_handler, ID_T_VOICE);
  lv_obj_set_pos(btnVoice, 125, 127);
  lv_obj_t *labelVoice = lv_label_create_empty(btnVoice);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelVoice, tool_menu.voice);
    lv_label_set_style(labelVoice,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelVoice, btnVoice, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //语言设置
  lv_obj_t *btnLanguage = lv_imgbtn_create(scr, "F:/bmp_language.bin", event_handler, ID_T_LANGUAGE);
  lv_obj_set_pos(btnLanguage, 241, 127);
  lv_obj_t *labelLanguage = lv_label_create_empty(btnLanguage);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelLanguage, set_menu.language);
    lv_label_set_style(labelLanguage,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelLanguage, btnLanguage, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //返回
  lv_obj_t *btnBack = lv_imgbtn_create(scr, "F:/bmp_tool_back.bin", event_handler, ID_T_RETURN);
  lv_obj_set_pos(btnBack, 357, 127);
  lv_obj_t *labelBack = lv_label_create_empty(btnBack);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelBack, common_menu.text_back);
    lv_label_set_style(labelBack,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelBack, btnBack, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }
  
  //Gcode
  lv_obj_t *btnGcode = lv_imgbtn_create(scr, "F:/bmp_tool_gcode.bin", event_handler, ID_T_GCODE);
  lv_obj_set_pos(btnGcode, 9, 245);
  lv_obj_t *labelGcode = lv_label_create_empty(btnGcode);
  lv_label_set_text(labelGcode, more_menu.gcode);
    lv_label_set_style(labelGcode,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelGcode, btnGcode, 0, 35, 0);

  //about
  lv_obj_t *btnAbout = lv_imgbtn_create(scr, "F:/bmp_tool_about.bin", event_handler, ID_T_ABOUT);
  lv_obj_set_pos(btnAbout, 241, 245);
  lv_obj_t *labelAbout = lv_label_create_empty(btnAbout);
  lv_label_set_text(labelAbout, set_menu.about);
    lv_label_set_style(labelAbout,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelAbout, btnAbout, 0, 20, 0);
  lv_refr_now(lv_refr_get_disp_refreshing());
  
  //调平
  lv_obj_t *btnLeveling = lv_imgbtn_create(scr, "F:/bmp_auto_level.bin", event_handler, ID_T_LEVELING);
  lv_obj_set_pos(btnLeveling, 9, 9);
  lv_obj_t *labelLeveling = lv_label_create_empty(btnLeveling);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelLeveling, tool_menu.leveling);
    lv_label_set_style(labelLeveling,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelLeveling, btnLeveling, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //限位
  lv_obj_t *btnEndStop = lv_imgbtn_create(scr, "F:/bmp_tool_endstop.bin", event_handler, ID_T_ENDSTOP);
  lv_obj_set_pos(btnEndStop, 125, 9);
  lv_obj_t *labelEndStop = lv_label_create_empty(btnEndStop);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelEndStop, tool_menu.endstop);
    lv_label_set_style(labelEndStop,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelEndStop, btnEndStop, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }
}

void lv_clear_tool() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
#ifdef USE_NEW_LVGL_CONF
  lv_obj_clean(mks_ui.src_main);
#else
  lv_obj_del(scr);
#endif
}

#endif // HAS_TFT_LVGL_UI
