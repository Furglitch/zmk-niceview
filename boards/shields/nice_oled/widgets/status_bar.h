#pragma once

#include "util.h"
#include <lvgl.h>
#include <zmk/endpoints.h>

/**
 * @brief Draws a combined one-line status bar with the connection symbol
 *        (USB / BT) followed directly by the battery percentage.
 *
 * No "SIG" or "BAT" labels are rendered — just the icon and the number.
 * Icon origin : (CONFIG_NICE_OLED_WIDGET_STATUS_BAR_ICON_X,
 *                CONFIG_NICE_OLED_WIDGET_STATUS_BAR_ICON_Y)
 * Text origin : (CONFIG_NICE_OLED_WIDGET_STATUS_BAR_TEXT_X,
 *                CONFIG_NICE_OLED_WIDGET_STATUS_BAR_TEXT_Y)
 */
void draw_status_bar(lv_obj_t *canvas, const struct status_state *state);
