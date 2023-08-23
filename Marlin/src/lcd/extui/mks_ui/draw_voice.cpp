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

// #include <TMCStepper.h>
#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>
#include "draw_endstop.h"

#include "../../../gcode/queue.h"
#include "../../../inc/MarlinConfig.h"

extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *labelButtonSound, *labelEndstopSound, *labelPrint, *labelNoFilament;
// static lv_obj_t *imgtop, *labelname;
static lv_obj_t *btnButton, *btnEndstop, *btnPrint, *btnNoFilament;//*buttonReturn, 
// static bool ButtonState = true, EndstopState = true, PrintState = true, NoFilamentState = true;
// static bool endstopfinish = false;
static bool x_axis_triggered = false;
static bool y_axis_triggered = false;
static bool z_axis_min_triggered = false;
static bool z_axis_max_triggered = false;

enum {
  ID_VOICE_BUTTON = 1,
  ID_VOICE_ENDSTOP,
  ID_VOICE_PRINT,
  ID_VOICE_NOFILAMENT,
  ID_VOICE_RETURN,
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) 
    return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW); 

  switch (obj->mks_obj_id) {
    case ID_VOICE_RETURN:
      lv_clear_voice();
      lv_draw_tool();
      break;
    case ID_VOICE_BUTTON:
    if(voiceStatic.ButtonState){
      voiceStatic.ButtonState = false;
    }
    else{
      voiceStatic.ButtonState = true;
    }
    // update_spi_flash();
    lv_voice_temp_refr();
    break;

    case ID_VOICE_ENDSTOP:
    if(voiceStatic.EndstopState){
      voiceStatic.EndstopState = false;
    }
    else{
      voiceStatic.EndstopState = true;
    }
    // update_spi_flash();
    lv_voice_temp_refr();
    break;

    case ID_VOICE_PRINT:
    if(voiceStatic.PrintState){
      voiceStatic.PrintState = false;
      lv_imgbtn_set_src_both(btnPrint, "F:/bmp_voice_off.bin");
      gCfgItems.print_completion_tone = voiceStatic.PrintState;
    }
    else{
      voiceStatic.PrintState = true;
      lv_imgbtn_set_src_both(btnPrint, "F:/bmp_voice_on.bin");
      gCfgItems.print_completion_tone = voiceStatic.PrintState;
    }
    lv_obj_refresh_ext_draw_pad(btnPrint);
    update_spi_flash();
    // lv_voice_temp_refr();
    break;

    case ID_VOICE_NOFILAMENT:
    if(voiceStatic.NoFilamentState){
      voiceStatic.NoFilamentState = false;
      gCfgItems.no_filament_tone = false;
    }
    else{
      voiceStatic.NoFilamentState = true;
      gCfgItems.no_filament_tone = true;
    }
    // update_spi_flash();
    lv_voice_temp_refr();
    break;

    default:
    break;
  }
}

void lv_draw_voice() {
  scr = lv_screen_create(VOICE_UI , "");

  // lv_top_return_name(scr, "Voice" , event_handler, ID_VOICE_RETURN);
  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);
  // lv_refr_now(lv_refr_get_disp_refreshing());

  lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_VOICE_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);
  
  // lv_refr_now(lv_refr_get_disp_refreshing());

  //create X-Endstop label
  labelButtonSound = lv_label_create_empty(scr);
  lv_label_set_text(labelButtonSound,voice_menu.button_sound);
  lv_obj_set_style(labelButtonSound, &tft_style_preHeat_label);
  lv_obj_set_pos(labelButtonSound, 55, 76);

  //create Y-Endstop label
  labelEndstopSound = lv_label_create_empty(scr);
  lv_label_set_text(labelEndstopSound,voice_menu.endstop_sound);
  lv_obj_set_style(labelEndstopSound, &tft_style_preHeat_label);
  lv_obj_set_pos(labelEndstopSound , 55, 139);

  //create Z-Endstop label
  labelPrint = lv_label_create_empty(scr);
  lv_label_set_text(labelPrint,voice_menu.print_complete_tone);
  lv_obj_set_style(labelPrint, &tft_style_preHeat_label);
  lv_obj_set_pos(labelPrint, 55, 204);
  
  //create material detection label
  labelNoFilament = lv_label_create_empty(scr);
  lv_label_set_text(labelNoFilament,voice_menu.no_filament_tone);
  lv_obj_set_style(labelNoFilament, &tft_style_preHeat_label);
  lv_obj_set_pos(labelNoFilament, 55, 271);
  
  // lv_refr_now(lv_refr_get_disp_refreshing());

  //create on/off button
  voiceStatic.ButtonState = gCfgItems.button_sound;
  voiceStatic.EndstopState = gCfgItems.endstop_sound;
  voiceStatic.PrintState = gCfgItems.print_completion_tone;
  voiceStatic.NoFilamentState = gCfgItems.no_filament_tone;

  if(voiceStatic.ButtonState)
    btnButton = lv_imgbtn_create(scr, "F:/bmp_voice_on.bin", event_handler, ID_VOICE_BUTTON);
  else
    btnButton = lv_imgbtn_create(scr, "F:/bmp_voice_off.bin", event_handler, ID_VOICE_BUTTON);
  lv_obj_set_pos(btnButton, 365, 70);

  if(voiceStatic.EndstopState)
    btnEndstop = lv_imgbtn_create(scr, "F:/bmp_voice_on.bin", event_handler, ID_VOICE_ENDSTOP);
  else
    btnEndstop = lv_imgbtn_create(scr, "F:/bmp_voice_off.bin", event_handler, ID_VOICE_ENDSTOP);
  lv_obj_set_pos(btnEndstop, 365, 135);

  if(voiceStatic.PrintState)
    btnPrint = lv_imgbtn_create(scr, "F:/bmp_voice_on.bin", event_handler, ID_VOICE_PRINT);
  else
    btnPrint = lv_imgbtn_create(scr, "F:/bmp_voice_off.bin", event_handler, ID_VOICE_PRINT);
  lv_obj_set_pos(btnPrint, 365, 200);

  if(voiceStatic.NoFilamentState)
    btnNoFilament = lv_imgbtn_create(scr, "F:/bmp_voice_on.bin", event_handler, ID_VOICE_NOFILAMENT);
  else
    btnNoFilament = lv_imgbtn_create(scr, "F:/bmp_voice_off.bin", event_handler, ID_VOICE_NOFILAMENT);
  lv_obj_set_pos(btnNoFilament, 365, 265);

  // lv_voice_temp_refr();
  // lv_img_set_src(imgtop, "F:/bmp_preHeat_top.bin");
  // lv_imgbtn_set_src_both(buttonReturn,"F:/bmp_preHeat_return.bin");

  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, tool_menu.voice);
  lv_obj_set_style(labelname, &label_dialog_white);
  lv_obj_set_pos(labelname, 69, 13);
}

void lv_clear_voice() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

void lv_voice_temp_refr()
{
  if(voiceStatic.ButtonState){
    lv_imgbtn_set_src_both(btnButton, "F:/bmp_voice_on.bin");
  }
  else{
    lv_imgbtn_set_src_both(btnButton, "F:/bmp_voice_off.bin");
  }
  gCfgItems.button_sound = voiceStatic.ButtonState;
  lv_obj_refresh_ext_draw_pad(btnButton);
  // update_spi_flash();

  if(voiceStatic.EndstopState){
    lv_imgbtn_set_src_both(btnEndstop, "F:/bmp_voice_on.bin");
  }
  else{
    lv_imgbtn_set_src_both(btnEndstop, "F:/bmp_voice_off.bin");
  }
  gCfgItems.endstop_sound = voiceStatic.EndstopState;
  lv_obj_refresh_ext_draw_pad(btnEndstop);
  // update_spi_flash();

  if(voiceStatic.PrintState){
    lv_imgbtn_set_src_both(btnPrint, "F:/bmp_voice_on.bin");
    gCfgItems.print_completion_tone = voiceStatic.PrintState;
  }
  else{
    lv_imgbtn_set_src_both(btnPrint, "F:/bmp_voice_off.bin");
    gCfgItems.print_completion_tone = voiceStatic.PrintState;
  }
  lv_obj_refresh_ext_draw_pad(btnPrint);
  update_spi_flash();

  if(voiceStatic.NoFilamentState){
    lv_imgbtn_set_src_both(btnNoFilament, "F:/bmp_voice_on.bin");
  }
  else{
    lv_imgbtn_set_src_both(btnNoFilament, "F:/bmp_voice_off.bin");
  }
  gCfgItems.no_filament_tone = voiceStatic.NoFilamentState;
  lv_obj_refresh_ext_draw_pad(btnNoFilament);
  update_spi_flash();
    
  lv_refr_now(lv_refr_get_disp_refreshing());
}

void voice_button_on()
{
  if(voiceStatic.ButtonState)
  {
    WRITE(BEEPER_PIN, HIGH);
    voiceStatic.VoiceStateTime = millis();
  }
}

void voice_button_off()
{
  voiceStatic.VoiceEndTime = millis();
  if((voiceStatic.VoiceEndTime - voiceStatic.VoiceEndTime) > 50)
    // WRITE(EXP1_10_PIN, LOW);
    digitalWrite(BEEPER_PIN, LOW);
    // digitalRead
}

void endstop_sound_on()
{
  // if(voiceStatic.ButtonState)
  // {
    if(voiceStatic.EndstopState)
    {
      if((READ(X_STOP_PIN) != X_MIN_ENDSTOP_INVERTING) && !x_axis_triggered)
      {
        x_axis_triggered = true;
        voiceStatic.VoiceStateTime = millis();
        WRITE(BEEPER_PIN, HIGH);
      }
      if(READ(X_STOP_PIN) == X_MIN_ENDSTOP_INVERTING)
      {
        x_axis_triggered = false;
      }

      if((READ(Y_STOP_PIN) != Y_MIN_ENDSTOP_INVERTING) && !y_axis_triggered)
      {
        y_axis_triggered = true;
        voiceStatic.VoiceStateTime = millis();
        WRITE(BEEPER_PIN, HIGH);
      }
      if(READ(Y_STOP_PIN) == Y_MIN_ENDSTOP_INVERTING)
      {
        y_axis_triggered = false;
      }

      if((READ(Z_MIN_PIN) != Z_MIN_ENDSTOP_INVERTING) && !z_axis_min_triggered)
      {
        z_axis_min_triggered = true;
        voiceStatic.VoiceStateTime = millis();
        WRITE(BEEPER_PIN, HIGH);
      }
      if(READ(Z_MIN_PIN) == Z_MIN_ENDSTOP_INVERTING)
      {
        z_axis_min_triggered = false;
      }

      if((READ(Z_MAX_PIN) != Z_MAX_ENDSTOP_INVERTING) && !z_axis_max_triggered)
      {
        z_axis_max_triggered = true;
        voiceStatic.VoiceStateTime = millis();
        WRITE(BEEPER_PIN, HIGH);
      }
      if(READ(Z_MAX_PIN) == Z_MAX_ENDSTOP_INVERTING)
      {
        z_axis_max_triggered = false;
      }
    }
  // }
}

void endstop_sound_off()
{
  voiceStatic.VoiceEndTime = millis();
  if((voiceStatic.VoiceEndTime - voiceStatic.VoiceStateTime) > 500)
  {
    // if(gCfgItems.endstop_sound && ((READ(X_STOP_PIN) == X_MIN_ENDSTOP_INVERTING) || (READ(Y_STOP_PIN) == Y_MIN_ENDSTOP_INVERTING) || (READ(Z_MIN_PIN) == Z_MIN_ENDSTOP_INVERTING)))
    // {
      WRITE(BEEPER_PIN, LOW);
      voiceStatic.VoiceEndTime = 0;
      voiceStatic.VoiceStateTime = 0;
    // }
  }
}


#endif // HAS_TFT_LVGL_UI
