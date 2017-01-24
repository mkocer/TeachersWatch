// sof
#include <pebble.h>
#include "Watch.h"

#define LETTER_HEIGHT 70
#define LETTER_WIDTH 28
#define COLON_WIDTH 16
#define SPACING_WIDTH 2

// 144 - 16 - 4 * 4 = 112
// 112 / 4 = 28

// 144 - 8 - 4 * 2 = 128
// 128 / 4 = 32

#define LABEL_X 2
#define LABEL_Y 8

static Layer * layer;
static TextLayer * label[5];
static TextLayer * label_date;
static char buffer[5][3]; // just for sure
static char buf[5]; // hour

static int current_hours;
static int current_minutes;
static int current_odd_second;
static int current_year;
static int current_month;
static int current_day;
static int current_dow;

static const char *dow_to_str[] = {"ne","po","út","st","čt","pá","so","ne"};

void clear_watch() {
    current_hours = -1;
    current_minutes = -1;
    current_odd_second = -1;
    current_year = 1970;
    current_month = 1;
    current_day = 1;
    current_dow = 0;
}

void show_time_watch() {
    snprintf(buf, sizeof (buf), "%02d%02d", current_hours, current_minutes);
    buffer[0][0] = buf[0];
    buffer[1][0] = buf[1];
    buffer[3][0] = buf[2];
    buffer[4][0] = buf[3];
    for (int i = 0; i < 5; i++) {
        text_layer_set_text(label[i], buffer[i]);
    }
}

void show_colon_watch() {
    if (current_odd_second == 0)
        buffer[2][0] = ':';
    else
        buffer[2][0] = ' ';
    text_layer_set_text(label[2], buffer[2]);
}

void show_date_watch() {
    static char date_s[18];
    
    snprintf(date_s, sizeof(date_s), "%s %02d.%02d.%04d", dow_to_str[current_dow], current_day, current_month, current_year);
    text_layer_set_text(label_date, date_s);
}

void set_time_watch(int hours, int minutes) {
    if (current_hours == hours && current_minutes == minutes)
        return;
    current_hours = hours;
    current_minutes = minutes;
    show_time_watch();
}

void set_colon_watch(int seconds) {
    int odd = seconds % 2;
    if (current_odd_second == odd) {
        return;
    }
    current_odd_second = odd;
    show_colon_watch();
}

void set_date_watch(int day, int month, int year, int dow) {
    current_year = year;
    current_day= day;
    current_month= month;
    current_dow = dow;
    show_date_watch();
}

void update_watch(struct tm *t) {
    set_time_watch(t->tm_hour, t->tm_min);
    set_colon_watch(t->tm_sec);
    set_date_watch(t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_wday);
}

void force_update_watch() {
    clear_watch();
}

void handle_tick_watch(struct tm *t) {
    update_watch(t);
}

void window_load_watch(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int half = bounds.size.h / 2;

    layer = layer_create((GRect) {
        bounds.origin,
        { bounds.size.w, half}
    });

    GFont font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIGITAL_56));
    int x = LABEL_X;
    for (int i = 0; i < 5; i++) {
        strcpy(buffer[i], "0");
        int w;
        if (i == 2)
            w = COLON_WIDTH;
        else
            w = LETTER_WIDTH;
        label[i] = text_layer_create(GRect(x, LABEL_Y, w, LETTER_HEIGHT));
        text_layer_set_font(label[i], font);
        text_layer_set_text_alignment(label[i], GTextAlignmentRight);
        text_layer_set_text(label[i], buffer[i]);
        text_layer_set_background_color(label[i], DIAL_COLOR);
        text_layer_set_text_color(label[i], TEXT_COLOR);
        layer_add_child(layer, text_layer_get_layer(label[i]));
        x += w;
        x += SPACING_WIDTH;
    }
    bounds = layer_get_bounds(layer);
    label_date = text_layer_create(GRect(
             0, (bounds.origin.y - 15 + bounds.size.h), 
             bounds.size.w, 14
    ));
    text_layer_set_font(label_date, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(label_date, GTextAlignmentCenter);
    text_layer_set_text(label_date, "01.02.2013");
    text_layer_set_background_color(label_date, DIAL_COLOR);
    text_layer_set_text_color(label_date, TEXT_COLOR);
    layer_add_child(layer, text_layer_get_layer(label_date));
    layer_add_child(window_layer, layer);
    force_update_watch();
}

void window_unload_watch(Window *window) {
    layer_remove_from_parent(layer);
    layer_destroy(layer);
    text_layer_destroy(label_date);
    for (int i = 0; i < 5; i++) {
        text_layer_destroy(label[i]);
    }
}


// eof

