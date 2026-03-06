/**
 * @file status_bar.c
 * @brief Combined one-line status bar: connection icon + battery percentage.
 *
 * Replaces the two-line SIG / BAT layout with a single compact row.
 * Enable via CONFIG_NICE_OLED_WIDGET_STATUS_BAR=y.
 */

#include "status_bar.h"
#include <fonts.h>
#include <zephyr/kernel.h>

/* ── image assets ─────────────────────────────────────────────────── */
LV_IMG_DECLARE(bt_no_signal);
LV_IMG_DECLARE(bt_unbonded);
LV_IMG_DECLARE(bt);
LV_IMG_DECLARE(usb);
LV_IMG_DECLARE(bolt);

/* ── signal icon helpers ──────────────────────────────────────────── */

static void draw_sb_icon(lv_obj_t *canvas, const lv_img_dsc_t *icon) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    lv_canvas_draw_img(canvas,
                       CONFIG_NICE_OLED_WIDGET_STATUS_BAR_ICON_X,
                       CONFIG_NICE_OLED_WIDGET_STATUS_BAR_ICON_Y,
                       icon, &img_dsc);
}

/* ── public draw function ─────────────────────────────────────────── */

void draw_status_bar(lv_obj_t *canvas, const struct status_state *state) {

    /* --- connection icon --- */
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        draw_sb_icon(canvas, &usb);
        break;
    case ZMK_TRANSPORT_BLE:
        if (state->active_profile_bonded) {
            if (state->active_profile_connected) {
                draw_sb_icon(canvas, &bt);
            } else {
                draw_sb_icon(canvas, &bt_no_signal);
            }
        } else {
            draw_sb_icon(canvas, &bt_unbonded);
        }
        break;
    }
#else
    /* Peripheral: only connected / disconnected states */
    if (state->connected) {
        draw_sb_icon(canvas, &bt);
    } else {
        draw_sb_icon(canvas, &bt_no_signal);
    }
#endif

    /* --- battery percentage --- */
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono_16, LV_TEXT_ALIGN_RIGHT);

    char text[10] = {};
    sprintf(text, "%i%%", state->battery);

    if (state->charging) {
        lv_draw_img_dsc_t img_dsc;
        lv_draw_img_dsc_init(&img_dsc);

        /* Measure rendered text width so bolt can be placed flush to its left */
        lv_point_t sz;
        lv_txt_get_size(&sz, text, label_dsc.font, label_dsc.letter_space,
                        label_dsc.line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

        /* bolt (5×9) vertically centred against the 16px font: offset = (16-9)/2 = 3 */
        lv_canvas_draw_img(canvas,
                           CANVAS_WIDTH - sz.x - 7,
                           CONFIG_NICE_OLED_WIDGET_STATUS_BAR_TEXT_Y + 3,
                           &bolt, &img_dsc);

        /* percentage text right-aligned to canvas edge */
        lv_canvas_draw_text(canvas, 0,
                            CONFIG_NICE_OLED_WIDGET_STATUS_BAR_TEXT_Y,
                            CANVAS_WIDTH, &label_dsc, text);
    } else {
        /* percentage text right-aligned to canvas edge */
        lv_canvas_draw_text(canvas, 0,
                            CONFIG_NICE_OLED_WIDGET_STATUS_BAR_TEXT_Y,
                            CANVAS_WIDTH, &label_dsc, text);
    }
}
