/*
 * Custom status screens for the Rocco Corne.
 *
 * Left (central): bongo cat driven by typing, battery, output, WPM and layer.
 * Right (peripheral): the Rocco Offroad emblem.
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/output_status.h>
#include <zmk/display/widgets/wpm_status.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/wpm.h>

#include "bongo_frames.h"

static struct zmk_widget_battery_status battery_widget;
static struct zmk_widget_output_status output_widget;
static struct zmk_widget_wpm_status wpm_widget;

static lv_obj_t *bongo_img;

struct bongo_state {
    uint8_t wpm;
    uint32_t taps;
};

static uint32_t tap_count;

static struct bongo_state bongo_get_state(const zmk_event_t *eh) {
    if (eh != NULL) {
        const struct zmk_position_state_changed *pos = as_zmk_position_state_changed(eh);
        if (pos != NULL && pos->state) {
            tap_count++;
        }
    }
    return (struct bongo_state){.wpm = zmk_wpm_get_state(), .taps = tap_count};
}

static void bongo_update_cb(struct bongo_state state) {
    static uint32_t last_taps;

    if (bongo_img == NULL) {
        return;
    }
    if (state.taps != last_taps) {
        last_taps = state.taps;
        lv_img_set_src(bongo_img, (state.taps % 2) ? &bongo_tap_left : &bongo_tap_right);
    } else if (state.wpm == 0) {
        lv_img_set_src(bongo_img, &bongo_idle);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(bongo_cat, struct bongo_state, bongo_update_cb, bongo_get_state)
ZMK_SUBSCRIPTION(bongo_cat, zmk_position_state_changed);
ZMK_SUBSCRIPTION(bongo_cat, zmk_wpm_state_changed);

/* Compact layer indicator: the keymap's display-name, no icon prefix. */
static lv_obj_t *layer_label;

struct layer_num_state {
    uint8_t index;
};

static struct layer_num_state layer_num_get_state(const zmk_event_t *eh) {
    return (struct layer_num_state){.index = zmk_keymap_highest_layer_active()};
}

static void layer_num_update_cb(struct layer_num_state state) {
    if (layer_label == NULL) {
        return;
    }
    const char *name = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(state.index));
    if (name != NULL && strlen(name) > 0) {
        lv_label_set_text(layer_label, name);
    } else {
        char text[4];
        snprintf(text, sizeof(text), "L%u", state.index);
        lv_label_set_text(layer_label, text);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(layer_num, struct layer_num_state, layer_num_update_cb,
                            layer_num_get_state)
ZMK_SUBSCRIPTION(layer_num, zmk_layer_state_changed);

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);

    bongo_img = lv_img_create(screen);
    lv_img_set_src(bongo_img, &bongo_idle);
    lv_obj_align(bongo_img, LV_ALIGN_LEFT_MID, 0, 0);
    bongo_cat_init();

    zmk_widget_battery_status_init(&battery_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_widget), LV_ALIGN_TOP_RIGHT, 0, 0);

    zmk_widget_output_status_init(&output_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_widget), LV_ALIGN_TOP_LEFT, 52, 0);

    layer_label = lv_label_create(screen);
    lv_obj_align(layer_label, LV_ALIGN_BOTTOM_LEFT, 52, 0);
    layer_num_init();

    /* The WPM widget re-aligns itself to the bottom right on every update. */
    zmk_widget_wpm_status_init(&wpm_widget, screen);

    return screen;
}

#else /* peripheral: right half shows the Rocco emblem plus battery and link */

#include "rocco_logo.h"

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
#include <zmk/display/widgets/battery_status.h>
static struct zmk_widget_battery_status battery_widget;
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
#include <zmk/display/widgets/peripheral_status.h>
static struct zmk_widget_peripheral_status peripheral_widget;
#endif

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_t *logo = lv_img_create(screen);
    lv_img_set_src(logo, &rocco_logo);
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, 0);

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
    zmk_widget_battery_status_init(&battery_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_widget), LV_ALIGN_TOP_LEFT, 0, 0);
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
    zmk_widget_peripheral_status_init(&peripheral_widget, screen);
    lv_obj_align(zmk_widget_peripheral_status_obj(&peripheral_widget), LV_ALIGN_BOTTOM_RIGHT, 0,
                 0);
#endif

    return screen;
}

#endif
