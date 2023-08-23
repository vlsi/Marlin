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

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *fw_type, *board;

enum { ID_A_RETURN = 1 };

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  hal.delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  switch (obj->mks_obj_id) {
    case ID_A_RETURN:
      clear_cur_ui();
      draw_return_ui();
      break;
  }
}

void lv_draw_about() {
  scr = lv_screen_create(ABOUT_UI);

  lv_obj_t *labelModel = lv_label_create_empty(scr);
  lv_obj_t *labelModel_1 = lv_label_create_empty(scr);
  lv_obj_set_style(labelModel, &tft_style_preHeat_label_BLACK);
  lv_obj_set_style(labelModel_1, &tft_style_preHeat_label_GRAY);
  lv_label_set_text(labelModel, about_MENU.printerModel);
  lv_label_set_text(labelModel_1, ": FlyingBear Reborn2");
  lv_obj_set_pos(labelModel, 12, 66);
  lv_obj_align(labelModel_1, labelModel , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  
  lv_obj_t *labelVolume = lv_label_create_empty(scr);
  lv_obj_t *labelVolume_1 = lv_label_create_empty(scr);
  lv_label_set_text(labelVolume, about_MENU.printVolume);
  lv_label_set_text(labelVolume_1, ": 325mm*325mm*350mm");
  lv_obj_set_style(labelVolume, &tft_style_preHeat_label_BLACK);
  lv_obj_set_style(labelVolume_1, &tft_style_preHeat_label_GRAY);
  lv_obj_set_pos(labelVolume, 12, 109);
  lv_obj_align(labelVolume_1, labelVolume , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  
  lv_obj_t *labelDiameter = lv_label_create_empty(scr);
  lv_obj_t *labelDiameter_1 = lv_label_create_empty(scr);
  lv_label_set_text(labelDiameter, about_MENU.filamentDiameter);
  lv_label_set_text(labelDiameter_1, ": 1.75mm");
  lv_obj_set_style(labelDiameter, &tft_style_preHeat_label_BLACK);
  lv_obj_set_style(labelDiameter_1, &tft_style_preHeat_label_GRAY);
  lv_obj_set_pos(labelDiameter, 12, 153);
  lv_obj_align(labelDiameter_1, labelDiameter , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  
  // lv_obj_t *labelVoltage = lv_label_create_empty(scr);
  // lv_obj_t *labelVoltage_1 = lv_label_create_empty(scr);
  // lv_label_set_text(labelVoltage, about_MENU.inputVoltage);
  // lv_label_set_text(labelVoltage_1, ": AC220V/AC110V");
  // lv_obj_set_style(labelVoltage, &tft_style_preHeat_label);
  // lv_obj_set_style(labelVoltage_1, &tft_style_preHeat_label_BLACK);
  // lv_obj_set_pos(labelVoltage, 12, 168);
  // lv_obj_align(labelVoltage_1, labelVoltage , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  
  lv_obj_t *labelVersion = lv_label_create_empty(scr);
  lv_obj_t *labelVersion_1 = lv_label_create_empty(scr);
  lv_label_set_text(labelVersion, about_MENU.version);
  lv_label_set_text(labelVersion_1, ": Reborn2_Auto_V2");//": Reborn2_V2.0_14x02_020"
  lv_obj_set_style(labelVersion, &tft_style_preHeat_label_BLACK);
  lv_obj_set_style(labelVersion_1, &tft_style_preHeat_label_GRAY);
  lv_obj_set_pos(labelVersion, 12, 196);
  lv_obj_align(labelVersion_1, labelVersion , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  
  lv_obj_t *labelSupport = lv_label_create_empty(scr);
  lv_obj_t *labelSupport_1 = lv_label_create_empty(scr);
  lv_label_set_text(labelSupport, about_MENU.support);
  lv_label_set_text(labelSupport_1, ": support@3dflyingbear.com");
  lv_obj_set_style(labelSupport, &tft_style_preHeat_label_BLACK);
  lv_obj_set_style(labelSupport_1, &tft_style_preHeat_label_GRAY);
  lv_obj_set_pos(labelSupport, 12, 237);
  lv_obj_align(labelSupport_1, labelSupport , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  
  lv_obj_t *labelContact = lv_label_create_empty(scr);
  lv_obj_t *labelContact_1 = lv_label_create_empty(scr);
  lv_label_set_text(labelContact, about_MENU.contact);
  lv_label_set_text(labelContact_1, ": adam@3dflyingbear.com");
  lv_obj_set_style(labelContact, &tft_style_preHeat_label_BLACK);
  lv_obj_set_style(labelContact_1, &tft_style_preHeat_label_GRAY);
  lv_obj_set_pos(labelContact, 12, 283);
  lv_obj_align(labelContact_1, labelContact , LV_ALIGN_OUT_RIGHT_MID, 10, 0);

  // lv_top_return_name(scr, set_menu.about, event_handler, ID_A_RETURN);
  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);
  lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_A_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);
  // lv_refr_now(lv_refr_get_disp_refreshing());
  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, set_menu.about);
  lv_obj_align(labelname, buttonReturn , LV_ALIGN_OUT_RIGHT_MID, 10, 0);
}

void lv_clear_about() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
