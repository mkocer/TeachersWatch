#include <pebble.h>
#include "Info.h"
#include <pebble-effect-layer/pebble-effect-layer.h> 

#define DIAL_COLOR GColorWhite
#define TEXT_COLOR GColorBlack

#define LABEL_TEXT_Y 14
#define LABEL_TEXT_H 48

#define LABEL_SECONDS_TEXT_H 32
#define LABEL_SECONDS_TEXT_W 30

#define TOP_RIM_HEIGHT 2

#define R 30
#define D 25

static const GPathInfo CRAPE_PATH_INFO = {
    .num_points = 4,
    .points = (GPoint [])
    {
        {0, R},
        {R, 0},
        {R + D, 0},
        {0, R + D}
    }
};

// design data

static Layer *layer;
static InverterLayer *inverter_layer;
static TextLayer *label;
static char buffer[10];
static TextLayer *seconds_label;
static char seconds_buffer[5];
static Layer *vorsicht_layer;
static GPath *crape_path = NULL;

// current values

static int current_weekday; // since Sunday 0 .. 6
static int current_hours;
static int current_minutes;
static int current_seconds;

// info business interface

int read_stored_int(const uint32_t key, int defaultValue) {
    int value;
    if (persist_exists(key)) {
        value = persist_read_int(key);
    } else {
        value = defaultValue;
    }
    return value;
}

void setInfo(int x, bool force) {
    if (current_info == x && !force)
        return;
    current_info = x;
    persist_write_int(INFO_SETTING_INFO, current_info);
    switch (current_info) {
        case INFO_WHITE:
            if (DBG_SHOW_INFO)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "setInfo: INFO_WHITE");
            layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
            layer_set_hidden(vorsicht_layer, true);
            break;
        case INFO_BLACK:
            if (DBG_SHOW_INFO)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "setInfo: INFO_BLACK");
            layer_set_hidden(inverter_layer_get_layer(inverter_layer), false);
            layer_set_hidden(vorsicht_layer, true);
            break;
        case INFO_CRAPE:
            if (DBG_SHOW_INFO)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "setInfo: INFO_CRAPE");
            layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
            layer_set_hidden(vorsicht_layer, false);
            break;
    }
}

void init_week() {
    for (int i = 0; i < DAY_COUNT; i++)
        for (int j = 0; j < ARR_SIZE; j++) {
            week[i][j] = -1;
            mark[i][j] = 0;
            vibrate[i][j] = -1;
        }
}

void copy_week(int want_week[DAY_COUNT][ARR_SIZE], int want_mark[DAY_COUNT][ARR_SIZE], int want_vibrate[DAY_COUNT][ARR_SIZE]) {
    //void copy_week(int *want_week[], int *want_mark[], int *want_vibrate[]) {
    for (int i = 0; i < DAY_COUNT; i++)
        for (int j = 0; j < ARR_SIZE; j++) {
            week[i][j] = want_week[i][j];
            mark[i][j] = want_mark[i][j];
            vibrate[i][j] = want_vibrate[i][j];
        }
}

// design and basic interface

void clear_info() {
    current_weekday = -1;
    current_hours = -1;
    current_minutes = -1;
    current_seconds = -1;
}

void show_info_as_minutes(int minutes) {
    if (minutes < 0) {
        strcpy(buffer, "");
        layer_set_hidden(text_layer_get_layer(seconds_label), true);
    } else {
        snprintf(buffer, sizeof (buffer), "%d", minutes);
        layer_set_hidden(text_layer_get_layer(seconds_label), false);
    }
    text_layer_set_text(label, buffer);
}

void show_info_as_time(int hour, int minute) {
    snprintf(buffer, sizeof (buffer), "%02d:%02d", hour, minute);
    text_layer_set_text(label, buffer);
    layer_set_hidden(text_layer_get_layer(seconds_label), true);
}

void show_time_info() {
    if (DBG_SHOW_INFO)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: weekday %d, lesson start %02d:%02d", current_weekday, current_hours, current_minutes);
    int minute = current_hours * 60 + current_minutes;
    for (int j = 0; j < ARR_SIZE; j++) {
        if (vibrate[current_weekday][j] < 0)
            break;
        if (DBG_SHOW_INFO)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "  vibration %d: %02d:%02d", j, vibrate[current_weekday][j] / 60, vibrate[current_weekday][j] % 60);
        if (minute < vibrate[current_weekday][j]) {
            if (DBG_SHOW_INFO)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "  early");
            break;
        }
        if (vibrate[current_weekday][j] == minute) {
            if (DBG_SHOW_INFO)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "  now");
            vibes_short_pulse();
            break;
        }
    }
    int pos = -1;
    for (int j = 0; j < ARR_SIZE; j++) {
        if (DBG_SHOW_INFO_DETAIL)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: week[%d] = %d", j, week[current_weekday][j]);
        if (week[current_weekday][j] < 0)
            break;
        if (week[current_weekday][j] == 0)
            continue;
        if (DBG_SHOW_INFO_DETAIL)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: test %02d:%02d", week[current_weekday][j] / 60, week[current_weekday][j] % 60);
        if (minute >= week[current_weekday][j]) {
            continue;
        }
        pos = j;
        break;
    }
    if (DBG_SHOW_INFO)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: pos = %d", pos);
    if (pos < 0) {
        // nothing today
        show_info_as_minutes(-1);
        setInfo(INFO_WHITE, false);
        return;
    }
    int is_lesson = (pos % 2 == 1) ? 1 : 0;
    int is_busy = (pos > 0) ? mark[current_weekday][pos - 1] : 0;
    int end_minute = week[current_weekday][pos];
    if (DBG_SHOW_INFO)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: lesson = %d, busy = %d, end = %02d:%02d", is_lesson, is_busy, end_minute / 60, end_minute % 60);
    if (!is_busy) {
        // find possible different end_minute
        int pos1 = pos;
        while (pos1 < ARR_SIZE && mark[current_weekday][pos1] == 0)
            pos1++;
        if (DBG_SHOW_INFO)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: pos1 = %d", pos1);
        if (pos1 == ARR_SIZE) {
            // nothing more today
            show_info_as_minutes(-1);
            setInfo(INFO_WHITE, false);
            return;
        }
        if (pos1 > pos) {
            end_minute = week[current_weekday][pos1];
            if (DBG_SHOW_INFO)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: end moved = %02d:%02d", end_minute / 60, end_minute % 60);
        }
    }
    int delta = end_minute - minute;
    if (DBG_SHOW_INFO)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: delta %d", delta);
    if (delta > MAX_DELTA_VISIBLE && pos == 0) {
        if (DBG_SHOW_INFO)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: delta too big before first lesson");
        show_info_as_minutes(-1);
        setInfo(INFO_WHITE, false);
        return;
    } else if (delta > MAX_DELTA) {
        if (DBG_SHOW_INFO)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: delta big");
        show_info_as_time(end_minute / 60, end_minute % 60);
    } else {
        delta--; // we display seconds
        if (DBG_SHOW_INFO)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "show_time_info: show delta %d", delta);
        show_info_as_minutes(delta);
    }
    if (is_busy == 0) {
        setInfo(INFO_WHITE, false);
    } else if (is_lesson == 1) {
        setInfo(INFO_BLACK, false);
    } else {
        setInfo(INFO_CRAPE, false);
    }
}

void show_seconds() {
    snprintf(seconds_buffer, sizeof (seconds_buffer), "%02d", 59 - current_seconds);
    text_layer_set_text(seconds_label, seconds_buffer);
}

void set_seconds(int seconds) {
    if (current_seconds == seconds)
        return;
    current_seconds = seconds;
    show_seconds();
}

void set_time(int weekday, int hours, int minutes) {
    if (current_weekday == weekday && current_hours == hours && current_minutes == minutes)
        return;
    current_weekday = weekday;
    current_hours = hours;
    current_minutes = minutes;
    show_time_info();
}

void update_info(struct tm *t) {
    set_time(t->tm_wday, t->tm_hour, t->tm_min);
    set_seconds(t->tm_sec);
}

void force_update_info() {
    clear_info();
}

void handle_tick_info(struct tm *t) {
    update_info(t);
}

void vorsicht_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    graphics_context_set_fill_color(ctx, TEXT_COLOR);
    graphics_context_set_stroke_color(ctx, TEXT_COLOR);

    GRect topRim = (GRect){
        { bounds.origin.x, bounds.origin.y},
        { bounds.size.w, TOP_RIM_HEIGHT}
    };
    graphics_fill_rect(ctx, topRim, 0, GCornerNone);

    gpath_draw_filled(ctx, crape_path);
}

void window_load_info(Window *window) {
    init_week();
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int half = bounds.size.h / 2;

    GRect layerBounds = (GRect){
        { bounds.origin.x, bounds.origin.y + half},
        { bounds.size.w, half}
    };

    layer = layer_create(layerBounds);

    GFont font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    GFont seconds_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

    // time label
    strcpy(buffer, "14");
    label = text_layer_create(GRect(layerBounds.origin.x, LABEL_TEXT_Y, layerBounds.size.w, LABEL_TEXT_H));
    text_layer_set_font(label, font);
    text_layer_set_text_alignment(label, GTextAlignmentCenter);
    text_layer_set_text(label, buffer);
    text_layer_set_background_color(label, DIAL_COLOR);
    text_layer_set_text_color(label, TEXT_COLOR);
    layer_add_child(layer, text_layer_get_layer(label));

    // seconds label
    strcpy(seconds_buffer, "14");
    seconds_label = text_layer_create(GRect(layerBounds.size.w - LABEL_SECONDS_TEXT_W - 1, layerBounds.size.h - LABEL_SECONDS_TEXT_H - 1, LABEL_SECONDS_TEXT_W, LABEL_SECONDS_TEXT_H));
    text_layer_set_font(seconds_label, seconds_font);
    text_layer_set_text_alignment(seconds_label, GTextAlignmentRight);
    text_layer_set_text(seconds_label, seconds_buffer);
    text_layer_set_background_color(seconds_label, DIAL_COLOR);
    text_layer_set_text_color(seconds_label, TEXT_COLOR);
    layer_add_child(layer, text_layer_get_layer(seconds_label));

    // add info layer
    layer_add_child(window_layer, layer);

    // init vorsicht
    crape_path = gpath_create(&CRAPE_PATH_INFO);
    vorsicht_layer = layer_create(layerBounds);
    layer_set_update_proc(vorsicht_layer, vorsicht_update_proc);
    layer_add_child(window_layer, vorsicht_layer);

    // inverter
    inverter_layer = inverter_layer_create(layerBounds);
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
    layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));

    // rest
    setInfo(read_stored_int(INFO_SETTING_INFO, INFO_WHITE), true);
    force_update_info();
}

void window_unload_info(Window *window) {
    layer_remove_from_parent(layer);
    layer_destroy(layer);
    text_layer_destroy(label);
    text_layer_destroy(seconds_label);
    layer_remove_from_parent(inverter_layer_get_layer(inverter_layer));
    inverter_layer_destroy(inverter_layer);
    layer_destroy(vorsicht_layer);
    gpath_destroy(crape_path);
}

// eof
