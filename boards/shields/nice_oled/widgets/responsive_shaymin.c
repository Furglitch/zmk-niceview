/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <lvgl.h>
#include "responsive_shaymin.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static lv_anim_t walk_anim;
static lv_timer_t *idle_check_timer = NULL;

// Include the shaymin pet images
#include "../assets/shaymin_pet.h"

#define WALK_FRAMES 4
#define WALK_ANIM_TIME_SLOW 2000  // 2 seconds for full walk cycle when idle
#define WALK_ANIM_TIME_FAST 400   // 0.4 seconds for full walk cycle when typing
#define IDLE_TIMEOUT_MS 500       // Return to slow walk after 500ms of no keypresses
#define IDLE_CHECK_PERIOD 100     // Check for idle every 100ms

struct responsive_shaymin_state {
    bool key_pressed;
    uint32_t last_tap;
    lv_obj_t *obj;
    bool is_fast;
};

static struct responsive_shaymin_state current_state = {
    .key_pressed = false, .last_tap = 0, .obj = NULL, .is_fast = false};

static void set_walk_frame(void *var, int32_t val) {
    LOG_DBG("SHAYMIN: Walk animation frame: %d", val);
    lv_obj_t *img = (lv_obj_t *)var;
    int frame = val % WALK_FRAMES;
    lv_img_set_src(img, shaymin_pet_images[frame]);
}

static void start_walk_animation(lv_obj_t *obj, bool fast) {
    if (current_state.is_fast == fast && lv_anim_get(obj, set_walk_frame) != NULL) {
        return; // Don't restart if already in same speed animation
    }

    LOG_DBG("SHAYMIN: Starting %s walk animation", fast ? "fast" : "slow");
    current_state.is_fast = fast;

    lv_anim_del(obj, set_walk_frame); // Stop any existing animation

    lv_anim_init(&walk_anim);
    lv_anim_set_var(&walk_anim, obj);
    lv_anim_set_values(&walk_anim, 0, WALK_FRAMES - 1);
    lv_anim_set_time(&walk_anim, fast ? WALK_ANIM_TIME_FAST : WALK_ANIM_TIME_SLOW);
    lv_anim_set_exec_cb(&walk_anim, set_walk_frame);
    lv_anim_set_repeat_count(&walk_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&walk_anim);
}

static void check_idle_timeout(lv_timer_t *timer) {
    uint32_t now = k_uptime_get_32();
    uint32_t time_since_last_tap = now - current_state.last_tap;

    if (time_since_last_tap >= IDLE_TIMEOUT_MS && current_state.obj != NULL &&
        current_state.is_fast) {
        LOG_DBG("SHAYMIN: Idle timeout reached, slowing walk animation");
        start_walk_animation(current_state.obj, false);
    }
}

static void update_responsive_shaymin_anim(struct zmk_widget_responsive_shaymin *widget,
                                           struct responsive_shaymin_state state) {
    if (!widget || !widget->obj) {
        LOG_ERR("SHAYMIN: Widget or object is NULL!");
        return;
    }

    current_state.obj = widget->obj; // Update the global state

    if (state.key_pressed) {
        LOG_DBG("SHAYMIN: Key pressed, starting fast walk");
        start_walk_animation(widget->obj, true);
    }
}

static struct responsive_shaymin_state responsive_shaymin_get_state(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev != NULL && ev->state) { // Only update on key press, not release
        current_state.key_pressed = true;
        current_state.last_tap = k_uptime_get_32();
    } else {
        current_state.key_pressed = false;
    }

    return current_state;
}

static void responsive_shaymin_update_cb(struct responsive_shaymin_state state) {
    struct zmk_widget_responsive_shaymin *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        update_responsive_shaymin_anim(widget, state);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_responsive_shaymin, struct responsive_shaymin_state,
                            responsive_shaymin_update_cb, responsive_shaymin_get_state)
ZMK_SUBSCRIPTION(widget_responsive_shaymin, zmk_keycode_state_changed);

int zmk_widget_responsive_shaymin_init(struct zmk_widget_responsive_shaymin *widget,
                                       lv_obj_t *parent) {
    widget->obj = lv_img_create(parent);

    // Initialize idle check timer
    if (idle_check_timer == NULL) {
        idle_check_timer = lv_timer_create(check_idle_timeout, IDLE_CHECK_PERIOD, NULL);
    }

    // Start with slow walk animation
    current_state.obj = widget->obj;
    current_state.is_fast = true; // Set to true so initial animation will start
    start_walk_animation(widget->obj, false);

    sys_slist_append(&widgets, &widget->node);

    widget_responsive_shaymin_init();

    return 0;
}

lv_obj_t *zmk_widget_responsive_shaymin_obj(struct zmk_widget_responsive_shaymin *widget) {
    return widget->obj;
}
