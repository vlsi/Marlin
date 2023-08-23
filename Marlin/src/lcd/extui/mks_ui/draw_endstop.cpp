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
static lv_obj_t *labelXEndstop, *labelYEndstop, *labelZEndstop, *labelMaterialDetection, *labelTrigger, *labelOpen;
// static lv_obj_t *labelname;
static lv_obj_t *btnXTrigger, *btnYTrigger, *btnZTrigger, *btnMDTrigger, *btnXOpen, *btnYOpen, *btnZOpen, *btnMDOpen;
extern bool XState = true, YState = true, ZState = true, MDState = true;
static lv_obj_t *buttonReturn;

enum {
  ID_ENDSTOP_X_TRIGGER = 1,
  ID_ENDSTOP_Y_TRIGGER,
  ID_ENDSTOP_Z_TRIGGER,
  ID_ENDSTOP_MD_TRIGGER,
  ID_ENDSTOP_X_OPEN,
  ID_ENDSTOP_Y_OPEN,
  ID_ENDSTOP_Z_OPEN,
  ID_ENDSTOP_MD_OPEN,
  ID_ENDSTOP_RETURN,
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  switch (obj->mks_obj_id) {
    case ID_ENDSTOP_RETURN:
      lv_clear_endstop();
      lv_draw_tool();
      break;
    case ID_ENDSTOP_X_TRIGGER:
    if(XState){
      XState = false;
      lv_imgbtn_set_src_both(btnXTrigger, "F:/bmp_endstop_on.bin");
      lv_imgbtn_set_src_both(btnXOpen, "F:/bmp_endstop_off.bin");
    }
      break;
    case ID_ENDSTOP_Y_TRIGGER:
    if(YState){
      YState = false;
      lv_imgbtn_set_src_both(btnYTrigger, "F:/bmp_endstop_on.bin");
      lv_imgbtn_set_src_both(btnYOpen, "F:/bmp_endstop_off.bin");
    }
      break;
    case ID_ENDSTOP_Z_TRIGGER:
    if(ZState){
      ZState = false;
      lv_imgbtn_set_src_both(btnZTrigger, "F:/bmp_endstop_on.bin");
      lv_imgbtn_set_src_both(btnZOpen, "F:/bmp_endstop_off.bin");
    }
      break;
    case ID_ENDSTOP_MD_TRIGGER:
    if(MDState){
      MDState = false;
      lv_imgbtn_set_src_both(btnMDTrigger, "F:/bmp_endstop_on.bin");
      lv_imgbtn_set_src_both(btnMDOpen, "F:/bmp_endstop_off.bin");
    }
      break;
    case ID_ENDSTOP_X_OPEN:
    if(!XState){
      XState = true;
      lv_imgbtn_set_src_both(btnXTrigger, "F:/bmp_endstop_off.bin");
      lv_imgbtn_set_src_both(btnXOpen, "F:/bmp_endstop_on.bin");
    }
      break;
    case ID_ENDSTOP_Y_OPEN:
    if(!YState){
      YState = true;
      lv_imgbtn_set_src_both(btnYTrigger, "F:/bmp_endstop_off.bin");
      lv_imgbtn_set_src_both(btnYOpen, "F:/bmp_endstop_on.bin");
    }
      break;
    case ID_ENDSTOP_Z_OPEN:
    if(!ZState){
      ZState = true;
      lv_imgbtn_set_src_both(btnZTrigger, "F:/bmp_endstop_off.bin");
      lv_imgbtn_set_src_both(btnZOpen, "F:/bmp_endstop_on.bin");
    }
      break;
    case ID_ENDSTOP_MD_OPEN:
    if(!MDState){
      MDState = true;
      lv_imgbtn_set_src_both(btnMDTrigger, "F:/bmp_endstop_off.bin");
      lv_imgbtn_set_src_both(btnMDOpen, "F:/bmp_endstop_on.bin");
    }
      break;
    default:
      break;
  }
}

void lv_draw_endstop() {
  scr = lv_screen_create(ENDSTOP_UI , "");

  //create top img
  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);

  buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_ENDSTOP_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);
  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, tool_menu.endstop);
  lv_obj_set_style(labelname, &label_dialog_white);
  lv_obj_set_pos(labelname, 69, 13);

  //create trigger label
  labelTrigger = lv_label_create_empty(scr);
  lv_label_set_text(labelTrigger,tool_menu.tringger);
  lv_obj_set_style(labelTrigger, &tft_style_preHeat_label_BLACK);
  lv_obj_set_pos(labelTrigger, 239, 79);

  //create open label
  labelOpen = lv_label_create_empty(scr);
  lv_label_set_text(labelOpen,tool_menu.open);
  lv_obj_set_style(labelOpen, &tft_style_preHeat_label_BLACK);
  lv_obj_set_pos(labelOpen, 377, 79);

  //create Trigger button
  // btnXTrigger = lv_imgbtn_create(scr, "F:/bmp_endstop_off.bin", event_handler, ID_ENDSTOP_X_TRIGGER);
  btnXTrigger = lv_img_create(scr,nullptr);
  lv_img_set_src(btnXTrigger, "F:/bmp_endstop_off.bin");
  lv_obj_align(btnXTrigger, labelTrigger, LV_ALIGN_CENTER, 0, 35);
  // btnYTrigger = lv_imgbtn_create(scr, "F:/bmp_endstop_off.bin", event_handler, ID_ENDSTOP_Y_TRIGGER);
  btnYTrigger = lv_img_create(scr,nullptr);
  lv_img_set_src(btnYTrigger, "F:/bmp_endstop_off.bin");
  lv_obj_align(btnYTrigger, btnXTrigger, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
  // btnZTrigger = lv_imgbtn_create(scr, "F:/bmp_endstop_off.bin", event_handler, ID_ENDSTOP_Z_TRIGGER);
  btnZTrigger = lv_img_create(scr,nullptr);
  lv_img_set_src(btnZTrigger, "F:/bmp_endstop_off.bin");
  lv_obj_align(btnZTrigger, btnYTrigger, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
  // btnMDTrigger = lv_imgbtn_create(scr, "F:/bmp_endstop_off.bin", event_handler, ID_ENDSTOP_MD_TRIGGER);
  btnMDTrigger = lv_img_create(scr,nullptr);
  lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_off.bin");
  lv_obj_align(btnMDTrigger, btnZTrigger, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);


  //create Open button
  // btnXOpen = lv_imgbtn_create(scr, "F:/bmp_endstop_on.bin", event_handler, ID_ENDSTOP_X_OPEN);
  btnXOpen = lv_img_create(scr,nullptr);
  lv_img_set_src(btnXOpen, "F:/bmp_endstop_on.bin");
  lv_obj_align(btnXOpen, labelOpen, LV_ALIGN_CENTER, 0, 35);
  // btnYOpen = lv_imgbtn_create(scr, "F:/bmp_endstop_on.bin", event_handler, ID_ENDSTOP_Y_OPEN);
  btnYOpen = lv_img_create(scr,nullptr);
  lv_img_set_src(btnYOpen, "F:/bmp_endstop_on.bin");
  lv_obj_align(btnYOpen, btnXOpen, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
  // btnZOpen = lv_imgbtn_create(scr, "F:/bmp_endstop_on.bin", event_handler, ID_ENDSTOP_Z_OPEN);
  btnZOpen = lv_img_create(scr,nullptr);
  lv_img_set_src(btnZOpen, "F:/bmp_endstop_on.bin");
  lv_obj_align(btnZOpen, btnYOpen, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
  // btnMDOpen = lv_imgbtn_create(scr, "F:/bmp_endstop_on.bin", event_handler, ID_ENDSTOP_MD_OPEN);
  btnMDOpen = lv_img_create(scr,nullptr);
  lv_img_set_src(btnMDOpen, "F:/bmp_endstop_on.bin");
  lv_obj_align(btnMDOpen, btnZOpen, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);

  //create X-Endstop label
  labelXEndstop = lv_label_create_empty(scr);
  lv_label_set_text(labelXEndstop,tool_menu.x_endstop);
  lv_obj_set_style(labelXEndstop, &tft_style_preHeat_label_BLACK);
  // lv_obj_set_pos(labelXEndstop, 35, 120);
  lv_obj_align(labelXEndstop, btnXTrigger, LV_ALIGN_OUT_LEFT_MID, -125, 00);

  //create Y-Endstop label
  labelYEndstop = lv_label_create_empty(scr);
  lv_label_set_text(labelYEndstop,tool_menu.y_endstop);
  lv_obj_set_style(labelYEndstop, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelYEndstop, labelXEndstop, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

  //create Z-Endstop label
  labelZEndstop = lv_label_create_empty(scr);
  lv_label_set_text(labelZEndstop,tool_menu.z_endstop);
  lv_obj_set_style(labelZEndstop, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelZEndstop, labelYEndstop, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 25);
  
  //create material detection label
  labelMaterialDetection = lv_label_create_empty(scr);
  lv_label_set_text(labelMaterialDetection,tool_menu.material_detection);
  lv_obj_set_style(labelMaterialDetection, &tft_style_preHeat_label_BLACK);
  lv_obj_align(labelMaterialDetection, labelZEndstop, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 25);

  lv_endstop_temp_refr();
}

void lv_clear_endstop() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

void lv_endstop_temp_refr()
{
  //X endstop
  #if X_MIN_ENDSTOP_INVERTING
    if(READ(X_STOP_PIN)){
      lv_img_set_src(btnXTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnXOpen, "F:/bmp_endstop_on.bin");
    }
    else{
      lv_img_set_src(btnXTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnXOpen, "F:/bmp_endstop_off.bin");
    }
  #else
    if(!READ(X_STOP_PIN)){
      lv_img_set_src(btnXTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnXOpen, "F:/bmp_endstop_on.bin");
    }
    else{
      lv_img_set_src(btnXTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnXOpen, "F:/bmp_endstop_off.bin");
    }
  #endif

  //Y endstop
  #if Y_MIN_ENDSTOP_INVERTING
    if(READ(Y_STOP_PIN)){
      lv_img_set_src(btnYTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnYOpen, "F:/bmp_endstop_on.bin");
    }
    else{
      lv_img_set_src(btnYTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnYOpen, "F:/bmp_endstop_off.bin");
    }
  #else
    if(!READ(Y_STOP_PIN)){
      lv_img_set_src(btnYTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnYOpen, "F:/bmp_endstop_on.bin");
    }
    else{
      lv_img_set_src(btnYTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnYOpen, "F:/bmp_endstop_off.bin");
    }
  #endif

  //Z endstop
  #if Z_MIN_ENDSTOP_INVERTING
    if(READ(Z_MIN_PIN)){
      lv_img_set_src(btnZTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnZOpen, "F:/bmp_endstop_on.bin");
    }
    else{
      lv_img_set_src(btnZTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnZOpen, "F:/bmp_endstop_off.bin");
    }
  #else
    if(!READ(Z_MIN_PIN)){
      lv_img_set_src(btnZTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnZOpen, "F:/bmp_endstop_on.bin");
    }
    else{
      lv_img_set_src(btnZTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnZOpen, "F:/bmp_endstop_off.bin");
    }
  #endif

  #if PIN_EXISTS(MT_DET_2)
    if (READ(MT_DET_2_PIN) == MT_DET_PIN_STATE)
    {
      lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_off.bin");
      lv_img_set_src(btnMDOpen, "F:/bmp_endstop_on.bin");
    }
    else
    {
      lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_on.bin");
      lv_img_set_src(btnMDOpen, "F:/bmp_endstop_off.bin");
    }
  #endif
  
  // #if FILAMENT_RUNOUT_SENSOR
  //   if(FIL_RUNOUT_STATE == LOW)
  //   {
  //     if(READ(PA4)){
  //       lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_off.bin");
  //       lv_img_set_src(btnMDOpen, "F:/bmp_endstop_on.bin");
  //     }
  //     else{
  //       lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_on.bin");
  //       lv_img_set_src(btnMDOpen, "F:/bmp_endstop_off.bin");
  //     }
  //   }
  //   else
  //   {
  //     if(!READ(PA4)){
  //       lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_off.bin");
  //       lv_img_set_src(btnMDOpen, "F:/bmp_endstop_on.bin");
  //     }
  //     else{
  //       lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_on.bin");
  //       lv_img_set_src(btnMDOpen, "F:/bmp_endstop_off.bin");
  //     }
  //   }
  // #endif
  
  lv_refr_now(lv_refr_get_disp_refreshing());
  lv_imgbtn_set_src_both(buttonReturn,"F:/bmp_preHeat_return.bin");
}
#endif // HAS_TFT_LVGL_UI
