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
#pragma once

#ifdef __cplusplus
  extern "C" {
#endif

#define MIN_EXT_SPEED_PERCENT    10
#define MAX_EXT_SPEED_PERCENT   999

#define MIN_PRINT_SPEED          20
#define MAX_PRINT_SPEED          200

#define MIN_EXT_SPEED            0
#define MAX_EXT_SPEED            900


#define MIN_EXT_HEAT            0
#define MAX_EXT_HEAT            400

#define MIN_BED_HEAT            0
#define MAX_BED_HEAT            120

void lv_draw_change_speed();
void lv_clear_change_speed();
void disp_speed_step();
void disp_print_speed();
void disp_speed_type();
void lv_draw_print_speed_change();

#ifdef __cplusplus
  } /* C-declarations for C++ */
#endif
