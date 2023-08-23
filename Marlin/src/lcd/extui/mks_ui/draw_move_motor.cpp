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
#include "..\Marlin\src\module\planner.h"
#include "..\Marlin\src\module\stepper.h"

#include "../../../gcode/queue.h"
#include "../../../module/motion.h"
#include "../../../inc/MarlinConfig.h"
#include "../../../gcode/gcode.h"

extern lv_group_t *g;

#ifndef USE_NEW_LVGL_CONF
static lv_obj_t *scr;
#endif

static lv_obj_t *labelV, *buttonV, *labelX, *labelY, *labelZ;
static lv_task_t *updatePosTask;
static char cur_label = 'Z';
static float cur_pos = 0;
static bool move_or_zero = false;

enum {
  ID_M_X_P = 1,
  ID_M_X_N,
  ID_M_Y_P,
  ID_M_Y_N,
  ID_M_Z_P,
  ID_M_Z_N,
  ID_M_STEP,
  ID_M_RETURN,
  ID_M_ZEROALL,
  ID_M_CLOSEMOTOR
};

void disp_cur_pos() {
  char str_1[16];
  // sprintf_P(public_buf_l, PSTR("%c:%s mm"), cur_label, dtostrf(cur_pos, 1, 1, str_1));
  // if(!move_or_zero)
  // {
    sprintf_P(public_buf_l, PSTR("X:%smm"), dtostrf((current_position.x - X_MIN_POS), 1, 1, str_1));//  cur_pos
    lv_label_set_text(labelX, public_buf_l);

    sprintf_P(public_buf_l, PSTR("Y:%smm"), dtostrf((current_position.y - Y_MIN_POS), 1, 1, str_1));//  cur_pos
    lv_label_set_text(labelY, public_buf_l);

    sprintf_P(public_buf_l, PSTR("Z:%smm"), dtostrf(current_position.z, 1, 1, str_1));//  cur_pos
    lv_label_set_text(labelZ, public_buf_l);
  // }
  // else{
  //   lv_label_set_text(labelX, PSTR("X:0.0mm"));

  //   lv_label_set_text(labelY, PSTR("Y:0.0mm"));

  //   lv_label_set_text(labelZ, PSTR("Z:0.0mm"));
  // }
  
  // switch(cur_label)
  // {
  //   case 'X':
  //     if (labelX) {
  //       lv_label_set_text(labelX, public_buf_l);
  //     }
  //     break;
  //   case 'Y': 
  //     if (labelY) {
  //       lv_label_set_text(labelY, public_buf_l);
  //     }
  //     break;
  //   case 'Z':
  //     if (labelZ) {
  //       lv_label_set_text(labelZ, public_buf_l);
  //     } 
  //     break;
  // }
}

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  char str_1[16];
  bool is_rb_full = true;

  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW); 

  if(!queue.ring_buffer.full(1)) {
    is_rb_full = true;
  }else{
    is_rb_full = false;
  }

  if (!queue.ring_buffer.full(1)) {
    bool do_inject = true;
    float dist = uiCfg.move_dist;
    switch (obj->mks_obj_id) {
      case ID_M_X_N: dist *= -1; case ID_M_X_P: cur_label = 'X'; move_or_zero = false; break;
      case ID_M_Y_N: dist *= -1; case ID_M_Y_P: cur_label = 'Y'; move_or_zero = false; break;
      case ID_M_Z_N: dist *= -1; case ID_M_Z_P: cur_label = 'Z'; move_or_zero = false; break;
      default: do_inject = false;
    }
    if (do_inject) {
      sprintf_P(public_buf_l, PSTR("G91\nG1 %c%s F%d\nG90"), cur_label, dtostrf(dist, 1, 3, str_1), uiCfg.moveSpeed);
      queue.inject(public_buf_l);
    }
  }

  switch (obj->mks_obj_id) {
    case ID_M_CLOSEMOTOR:
        Emergemcy_flog = true;
        planner.quick_stop();
        gcode.process_subcommands_now(PSTR("M84"));
      break;
    case ID_M_ZEROALL:
      move_or_zero = true;
      queue.inject_P(G28_STR);
      break;
    case ID_M_STEP:
      if (ABS(10 * (int)uiCfg.move_dist) == 100)
        uiCfg.move_dist = 0.1;
      else
        uiCfg.move_dist *= 10.0f;
      disp_move_dist();
      break;
    case ID_M_RETURN:
      move_or_zero = false;
      clear_cur_ui();
      draw_return_ui();
      return;
  }
  disp_cur_pos();//刷新label
}

void refresh_pos(lv_task_t *) {
  switch (cur_label) {
    case 'X': cur_pos = current_position.x; break;
    case 'Y': cur_pos = current_position.y; break;
    case 'Z': cur_pos = current_position.z; break;
    default: return;
  }
  disp_cur_pos();
}

void lv_draw_move_motor() {

  move_or_zero = Emergemcy_flog;

#ifdef USE_NEW_LVGL_CONF
  mks_ui.src_main = lv_set_scr_id_title(mks_ui.src_main, MOVE_MOTOR_UI, "");

  lv_big_button_create(mks_ui.src_main, "F:/bmp_xAdd.bin", move_menu.x_add, INTERVAL_V, titleHeight, event_handler, ID_M_X_P);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_xDec.bin", move_menu.x_dec, INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_X_N);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_yAdd.bin", move_menu.y_add, BTN_X_PIXEL + INTERVAL_V * 2, titleHeight, event_handler, ID_M_Y_P);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_yDec.bin", move_menu.y_dec, BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_Y_N);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_zAdd.bin", move_menu.z_add, BTN_X_PIXEL * 2 + INTERVAL_V * 3, titleHeight, event_handler, ID_M_Z_P);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_zDec.bin", move_menu.z_dec, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_Z_N);

#else 
  scr = lv_screen_create(MOVE_MOTOR_UI);

  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);
  lv_refr_now(lv_refr_get_disp_refreshing());
  lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_M_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);
  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, main_menu.move);
  lv_obj_set_style(labelname, &label_dialog_white);
  lv_obj_set_pos(labelname, 69, 13);
  // lv_refr_now(lv_refr_get_disp_refreshing());

  lv_obj_t *buttonXAdd = lv_imgbtn_create(scr, "F:/bmp_xAdd.bin", event_handler, ID_M_X_P);
  lv_obj_set_pos(buttonXAdd, 32, 90);
  lv_obj_clear_protect(buttonXAdd, LV_PROTECT_FOLLOW);
  lv_obj_t *buttonXDec = lv_imgbtn_create(scr, "F:/bmp_xDec.bin", event_handler, ID_M_X_N);
  lv_obj_set_pos(buttonXDec, 32, 201);
  lv_obj_t *buttonYAdd = lv_imgbtn_create(scr, "F:/bmp_yAdd.bin", event_handler, ID_M_Y_P);
  lv_obj_set_pos(buttonYAdd, 152, 52);
  lv_obj_t *buttonYDec = lv_imgbtn_create(scr, "F:/bmp_yDec.bin", event_handler, ID_M_Y_N);
  lv_obj_set_pos(buttonYDec, 152, 241);
  lv_obj_t *buttonZAdd = lv_imgbtn_create(scr, "F:/bmp_zAdd.bin", event_handler, ID_M_Z_P);
  lv_obj_set_pos(buttonZAdd, 274, 90);
  lv_obj_t *buttonZDec = lv_imgbtn_create(scr, "F:/bmp_zDec.bin", event_handler, ID_M_Z_N);
  lv_obj_set_pos(buttonZDec, 274, 201);
  lv_obj_t *buttonZeroAll = lv_imgbtn_create(scr, "F:/bmp_zeroAll.bin", event_handler, ID_M_ZEROALL);
  lv_obj_set_pos(buttonZeroAll, 147, 142);
  lv_obj_t *buttonCloseMotor = lv_imgbtn_create(scr, "F:/bmp_stop_allMotor.bin", event_handler, ID_M_CLOSEMOTOR);
  lv_obj_set_pos(buttonCloseMotor, 382, 216);
#endif

  // button with image and label changed dynamically by disp_move_dist
#ifdef USE_NEW_LVGL_CONF
  buttonV = lv_imgbtn_create(mks_ui.src_main, nullptr, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_M_STEP);
#else
  buttonV = lv_imgbtn_create(scr, nullptr, 387, 65, event_handler, ID_M_STEP);
#endif
  labelV = lv_label_create_empty(buttonV);
  lv_label_set_style(labelV, LV_LABEL_STYLE_MAIN, &label_dialog_white);
  // #if HAS_ROTARY_ENCODER
  //   if (gCfgItems.encoder_enable) {
  //     lv_group_add_obj(g, buttonXAdd);
  //     lv_group_add_obj(g, buttonXDec);
  //     lv_group_add_obj(g, buttonYAdd);
  //     lv_group_add_obj(g, buttonYDec);
  //     lv_group_add_obj(g, buttonZAdd);
  //     lv_group_add_obj(g, buttonZDec);
  //     lv_group_add_obj(g, buttonZeroAll);
  //     lv_group_add_obj(g, buttonCloseMotor);
  //     lv_group_add_obj(g, buttonV);
  //   }
  // #endif


#ifdef USE_NEW_LVGL_CONF
  lv_big_button_create(mks_ui.src_main, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_RETURN);
  // lv_obj_t * title = lv_obj_get_child_back(mks_ui.src_main, nullptr);
#else
  // lv_big_button_create(scr, "F:/bmp_preHeat_return.bin", nullptr, 9, 6, event_handler, ID_M_RETURN);
  // lv_obj_t * title = lv_obj_get_child_back(scr, nullptr);
#endif
  // We need to patch the title to leave some space on the right for displaying the status
  
  // if (title != nullptr) lv_obj_set_width(title, TFT_WIDTH - 101);
#ifdef USE_NEW_LVGL_CONF
  labelP = lv_label_create(mks_ui.src_main, TFT_WIDTH - 100, TITLE_YPOS, "Z:0.0mm");
#else
  labelX = lv_label_create(scr, 145, 17, "X:-000.0mm");
  lv_label_set_style(labelX, LV_LABEL_STYLE_MAIN, &label_dialog_white);
  labelY = lv_label_create(scr, 260, 17, "Y:-000.0mm");
  lv_label_set_style(labelY, LV_LABEL_STYLE_MAIN, &label_dialog_white);
  labelZ = lv_label_create(scr, 370, 17, "Z:-000.0mm");
  lv_label_set_style(labelZ, LV_LABEL_STYLE_MAIN, &label_dialog_white);
#endif
  if (labelZ != nullptr)
    updatePosTask = lv_task_create(refresh_pos, 300, LV_TASK_PRIO_LOWEST, 0);

  // lv_refr_now(lv_refr_get_disp_refreshing());
  // lv_imgbtn_set_src_both(buttonReturn,"F:/bmp_preHeat_return.bin");

  disp_move_dist();
  disp_cur_pos();
}

void disp_move_dist() {
  // if ((int)(10 * uiCfg.move_dist) == 1)//0.1
  //   lv_imgbtn_set_src_both(buttonV, "F:/bmp_step_move1.bin");
  // else if ((int)(10 * uiCfg.move_dist) == 10)
  //   lv_imgbtn_set_src_both(buttonV, "F:/bmp_step_move1.bin");
  // else if ((int)(10 * uiCfg.move_dist) == 100)//10
    lv_imgbtn_set_src_both(buttonV, "F:/bmp_step_move1.bin");

  if (gCfgItems.multiple_language) {
    if ((int)(10 * uiCfg.move_dist) == 1) {
      lv_label_set_text(labelV, move_menu.step_01mm);
      lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else if ((int)(10 * uiCfg.move_dist) == 10) {
      lv_label_set_text(labelV, move_menu.step_1mm);
      lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else if ((int)(10 * uiCfg.move_dist) == 100) {
      lv_label_set_text(labelV, move_menu.step_10mm);
      lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
  }
}

void lv_clear_move_motor() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_task_del(updatePosTask);
#ifdef USE_NEW_LVGL_CONF
  lv_obj_clean(mks_ui.src_main);
#else
  lv_obj_del(scr);
#endif
}

#endif // HAS_TFT_LVGL_UI
