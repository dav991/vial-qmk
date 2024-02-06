// Copyright 2024 David A. (@David A.)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#ifdef RGBLIGHT_ENABLE
#define RGBLIGHT_SLEEP
#define RGBLIGHT_TIMEOUT 600
#define RGBLIGHT_LAYERS
#endif
/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT
