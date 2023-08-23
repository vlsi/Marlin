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
#include "../../../inc/MarlinConfig.h"

extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *labelPLA, *labelABS, *labelExtrusion1, *labelHotbed;
// static lv_obj_t *imgtop, *labelname;
static lv_obj_t *buttonReturn, *btnPLA_Ex1, *btnPLA_Bed, *btnABS_Ex1, *btnABS_Bed;
static lv_obj_t *labelPLA_Ex1, *labelPLA_Bed, *labelABS_Ex1, *labelABS_Bed;
static char num[10] = {};

enum {
  ID_VOICE_BUTTON = 1,
  ID_TEMPSETTING_RETURN,
  ID_TEMPSETTING_PLA_EXT1,
  ID_TEMPSETTING_PLA_BED,
  ID_TEMPSETTING_ABS_EXT1,
  ID_TEMPSETTING_ABS_BED,
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) 
    return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  switch (obj->mks_obj_id) {
    case ID_TEMPSETTING_PLA_EXT1:
      temp_value = pla_ext1;
      keyboard_value = GTempsetting;
      lv_clear_tempsetting();
      lv_draw_numkeyboard();
      break;
    case ID_TEMPSETTING_PLA_BED:
      temp_value = pla_bed;
      keyboard_value = GTempsetting;
      lv_clear_tempsetting();
      lv_draw_numkeyboard();
      break;
    case ID_TEMPSETTING_ABS_EXT1:
      temp_value = abs_ext1;
      keyboard_value = GTempsetting;
      lv_clear_tempsetting();
      lv_draw_numkeyboard();
      break;
    case ID_TEMPSETTING_ABS_BED:
      temp_value = abs_bed;
      keyboard_value = GTempsetting;
      lv_clear_tempsetting();
      lv_draw_numkeyboard();
      break;
    case ID_TEMPSETTING_RETURN:
        lv_clear_tempsetting();
        lv_draw_tool();
        break;
    default:
        break;
  }
}

void lv_draw_tempsetting() {
  scr = lv_screen_create(TEMPSETTING_UI , "");

  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);
  // lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_TEMPSETTING_RETURN);
  // lv_obj_set_pos(buttonReturn, 6, 3);

  //create X-Endstop label
  labelPLA = lv_label_create_empty(scr);
  lv_label_set_text(labelPLA,"PLA");
  lv_obj_set_style(labelPLA, &tft_style_label_big_black);
  lv_obj_set_pos(labelPLA, 50, 140);

  //create Y-Endstop label
  labelABS = lv_label_create_empty(scr);
  lv_label_set_text(labelABS,"ABS");
  lv_obj_set_style(labelABS, &tft_style_label_big_black);
  lv_obj_set_pos(labelABS , 50, 250);

//   //create label in the button
  btnPLA_Ex1 = lv_imgbtn_create(scr, "F:/bmp_tempsetting_bg.bin", event_handler, ID_TEMPSETTING_PLA_EXT1);
  lv_obj_set_style(btnPLA_Ex1, &tft_style_preHeat_scr);
  lv_obj_set_pos(btnPLA_Ex1, 145, 115);
  labelPLA_Ex1 = lv_label_create_empty(btnPLA_Ex1);
  sprintf(num , "%d℃" , gCfgItems.PLA_EXT1);
  lv_label_set_text(labelPLA_Ex1,num);
  lv_label_set_style(labelPLA_Ex1, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelPLA_Ex1, btnPLA_Ex1, LV_ALIGN_CENTER, 0, 0);

  btnPLA_Bed = lv_imgbtn_create(scr, "F:/bmp_tempsetting_bg.bin", event_handler, ID_TEMPSETTING_PLA_BED);
  lv_obj_set_pos(btnPLA_Bed, 320, 115);
  labelPLA_Bed = lv_label_create_empty(btnPLA_Bed);
  sprintf(num , "%d℃" , gCfgItems.PLA_BED);
  lv_label_set_text(labelPLA_Bed,num);
  lv_label_set_style(labelPLA_Bed, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelPLA_Bed, btnPLA_Bed, LV_ALIGN_CENTER, 0, 0);

  btnABS_Ex1 = lv_imgbtn_create(scr, "F:/bmp_tempsetting_bg.bin", event_handler, ID_TEMPSETTING_ABS_EXT1);
  lv_obj_set_pos(btnABS_Ex1, 145, 225);  
  labelABS_Ex1 = lv_label_create_empty(btnABS_Ex1);
  sprintf(num , "%d℃" , gCfgItems.ABS_EXT1);
  lv_label_set_text(labelABS_Ex1,num);
  lv_label_set_style(labelABS_Ex1, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelABS_Ex1, btnABS_Ex1, LV_ALIGN_CENTER, 0, 0);

  btnABS_Bed = lv_imgbtn_create(scr, "F:/bmp_tempsetting_bg.bin", event_handler, ID_TEMPSETTING_ABS_BED);
  lv_obj_set_pos(btnABS_Bed, 320, 225);
  labelABS_Bed = lv_label_create_empty(btnABS_Bed);
  sprintf(num , "%d℃" , gCfgItems.ABS_BED);
  lv_label_set_text(labelABS_Bed,num);
  lv_label_set_style(labelABS_Bed, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelABS_Bed, btnABS_Bed, LV_ALIGN_CENTER, 0, 0);

  //create Z-Endstop label
  labelExtrusion1 = lv_label_create_empty(scr);
  lv_label_set_text(labelExtrusion1,main_menu.Ext);
  lv_obj_set_style(labelExtrusion1, &tft_style_preHeat_label_BLACK);
  // lv_obj_set_pos(labelExtrusion1, 145, 75);
  lv_obj_align(labelExtrusion1 , btnPLA_Ex1 , LV_ALIGN_CENTER, 0, -60);
  
  //create material detection label
  labelHotbed = lv_label_create_empty(scr);
  lv_label_set_text(labelHotbed,main_menu.hotbed);
  lv_obj_set_style(labelHotbed, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelHotbed , btnPLA_Bed , LV_ALIGN_CENTER, 0, -60);
  // lv_obj_set_pos(labelHotbed, 335, 75);

  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, tool_menu.tempsetting);
  lv_obj_set_style(labelname, &label_dialog_white);
  lv_obj_set_pos(labelname, 69, 13);

  lv_refr_now(lv_refr_get_disp_refreshing());
  // lv_imgbtn_set_src_both(buttonReturn, "F:/bmp_preHeat_return.bin");
  lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_TEMPSETTING_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);
  
}

void lv_clear_tempsetting() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

void lv_tempsetting_temp_refr()
{

}


#endif // HAS_TFT_LVGL_UI
