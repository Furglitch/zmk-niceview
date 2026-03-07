/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <lvgl.h>
#include "responsive_shaymin.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

// Include the shaymin pet images
#include "../assets/shaymin_pet.h"

// Helper macro for passing arrays to lv_animimg_set_src
#define SRC(array) (const void **)array, sizeof(array) / sizeof(lv_img_dsc_t *)

// Animation speeds (duration in ms for full cycle)
#define ANIMATION_SPEED_IDLE 10000  // 10 seconds - very slow walk when idle
#define ANIMATION_SPEED_SLOW 2000   // 2 seconds - slow walk
#define ANIMATION_SPEED_MID 1000    // 1 second - normal walk
#define ANIMATION_SPEED_FAST 400    // 0.4 seconds - fast walk

// WPM thresholds
#define WPM_THRESHOLD_IDLE 5
#define WPM_THRESHOLD_SLOW 30
#define WPM_THRESHOLD_MID 70

// Animation state tracking
enum anim_state {
    anim_state_none,
    anim_state_idle,
    anim_state_slow,
    anim_state_mid,
    anim_state_fast
};

static enum anim_state current_anim_state = anim_state_none;

struct wpm_shaymin_state {
    uint8_t wpm;
};

static struct wpm_shaymin_state shaymin_get_state(const zmk_event_t *eh) {
    struct wpm_shaymin_state state;
    const struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    if (ev == NULL) {
        state.wpm = 0;
    } else {
        state.wpm = ev->state;
    }
    return state;
}

static void set_animation(lv_obj_t *animimg, struct wpm_shaymin_state state) {
    enum anim_state new_state;
    uint32_t duration;

    // Determine animation state based on WPM
    if (state.wpm < WPM_THRESHOLD_IDLE) {
        new_state = anim_state_idle;
        duration = ANIMATION_SPEED_IDLE;
    } else if (state.wpm < WPM_THRESHOLD_SLOW) {
        new_state = anim_state_slow;
        duration = ANIMATION_SPEED_SLOW;
    } else if (state.wpm < WPM_THRESHOLD_MID) {
        new_state = anim_state_mid;
        duration = ANIMATION_SPEED_MID;
    } else {
        new_state = anim_state_fast;
        duration = ANIMATION_SPEED_FAST;
    }

    // Only update if state changed
    if (new_state == current_anim_state) {
        return;
    }

    current_anim_state = new_state;
    LOG_DBG("SHAYMIN: WPM=%d, switching to %s animation (%d ms)",
            state.wpm,
            new_state == anim_state_idle ? "idle" :
            new_state == anim_state_slow ? "slow" :
            new_state == anim_state_mid ? "mid" : "fast",
            duration);

    // Set the animation source and duration
    lv_animimg_set_src(animimg, SRC(shaymin_pet_images));
    lv_animimg_set_duration(animimg, duration);
    lv_animimg_set_repeat_count(animimg, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(animimg);
}

static void shaymin_update_cb(struct wpm_shaymin_state state) {
    struct zmk_widget_responsive_shaymin *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_animation(widget->obj, state);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_responsive_shaymin, struct wpm_shaymin_state,
                            shaymin_update_cb, shaymin_get_state)
ZMK_SUBSCRIPTION(widget_responsive_shaymin, zmk_wpm_state_changed);

int zmk_widget_responsive_shaymin_init(struct zmk_widget_responsive_shaymin *widget,
                                       lv_obj_t *parent) {
    widget->obj = lv_animimg_create(parent);
    lv_obj_center(widget->obj);

    // Set the animation source and duration
    lv_animimg_set_src(widget->obj, SRC(shaymin_pet_images));
    lv_animimg_set_duration(widget->obj, ANIMATION_SPEED_IDLE);
    lv_animimg_set_repeat_count(widget->obj, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(widget->obj);

    current_anim_state = anim_state_idle;

    sys_slist_append(&widgets, &widget->node);

    widget_responsive_shaymin_init();

    return 0;
}

lv_obj_t *zmk_widget_responsive_shaymin_obj(struct zmk_widget_responsive_shaymin *widget) {
    return widget->obj;
}
