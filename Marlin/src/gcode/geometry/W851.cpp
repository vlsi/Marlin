/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2023 Vladimir Sitnikov.
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

#include "../../inc/MarlinConfig.h"

#if ENABLED(EDITABLE_HOME_POS)

#include "../gcode.h"
#include "../../feature/bedlevel/bedlevel.h"
#include "../../module/probe.h"

/**
 * W851: Set the nozzle-to-bed offsets when endstops trigger
 */
void GcodeSuite::W851() {
  // No parameters? Show current state.
  if (!parser.seen("XYZ")) return W851_report();

  // Start with current offsets and modify
  xyz_pos_t pos = base_home_pos_p;

  // Assume no errors
  bool ok = true;

  if (parser.seenval('X')) {
    const float x = parser.value_float();
    if (WITHIN(x, EDITABLE_HOME_X_RANGE_MIN, EDITABLE_HOME_X_RANGE_MAX)) {
      pos.x = x;
    } else {
      SERIAL_ECHOLNPGM("?X out of range (-", EDITABLE_HOME_X_RANGE_MIN, " to ", EDITABLE_HOME_X_RANGE_MAX, ")");
      ok = false;
    }
  }

  #if HAS_Y_AXIS
    if (parser.seenval('Y')) {
      const float y = parser.value_float();
      if (WITHIN(y, EDITABLE_HOME_Y_RANGE_MIN, EDITABLE_HOME_Y_RANGE_MAX)) {
        pos.y = y;
      } else {
        SERIAL_ECHOLNPGM("?Y out of range (-", EDITABLE_HOME_Y_RANGE_MIN, " to ", EDITABLE_HOME_Y_RANGE_MAX, ")");
        ok = false;
      }
    }
  #endif

  #if HAS_Z_AXIS
    if (parser.seenval('Z')) {
      const float z = parser.value_float();
      if (WITHIN(z, EDITABLE_HOME_Z_RANGE_MIN, EDITABLE_HOME_Z_RANGE_MAX)) {
        pos.z = z;
      } else {
        SERIAL_ECHOLNPGM("?Z out of range (-", EDITABLE_HOME_Z_RANGE_MIN, " to ", EDITABLE_HOME_Z_RANGE_MAX, ")");
        ok = false;
      }
    }
  #endif

  // Save the new offsets
  if (ok) base_home_pos_p = pos;
}

void GcodeSuite::W851_report(const bool forReplay/*=true*/) {
  report_heading_etc(forReplay, F(STR_HOME_POSITION));
  SERIAL_ECHOPGM_P(
    PSTR("  W851 X")
    , LINEAR_UNIT(base_home_pos(X_AXIS))
    #if HAS_Y_AXIS
      , PSTR(" Y")
      , LINEAR_UNIT(base_home_pos(Y_AXIS))
    #endif
    #if HAS_Z_AXIS
      , PSTR(" Z")
      , LINEAR_UNIT(base_home_pos(Z_AXIS))
    #endif
    , PSTR(" ;")
  );
  say_units();
}

#endif // EDITABLE_HOME_POS
