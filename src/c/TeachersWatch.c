// sof
#include <pebble.h>
#include "Watch.h"
#include "Info.h"
#include "Setup.h"
#include "TeachersWatch.h"

static Window *window;

static void window_load(Window *window) {
    window_load_watch(window);
    window_load_info(window);
}

static void window_unload(Window *window) {
    window_unload_watch(window);
    window_unload_info(window);
}

static time_t delta_time = 0;
static time_t start_time = 0;

//static void show(const char* label, struct tm *t) {
//    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE,
//            "handle_tick: %s, W %d, %d.%d.%d %02d:%02d:%02d",
//            label,
//            t->tm_wday,
//            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
//            t->tm_hour, t->tm_min, t->tm_sec);
//
//}

static void handle_tick(struct tm *t, TimeUnits units_changed) {
    if (start_time > 0) {
        time_t now = time(NULL);
        //        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "handle_tick:        now = %ld", now);
        //        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "handle_tick: start_time = %ld", start_time);
        //        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "handle_tick: delta_time = %ld", delta_time);
        //        show("org now", t);
        now += delta_time;
        //        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "handle_tick:    new now = %ld", now);
        t = localtime(&now);
        //        show("new now", t);
        //        struct tm *t1 = localtime(&start_time);
        //        show("  start", t1);
    }
    handle_tick_watch(t);
    handle_tick_info(t);
    handle_tick_setup(t);
}

static void init(void) {
    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    const bool animated = true;
    window_stack_push(window, animated);
    tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
    init_setup();
}

static void deinit(void) {
    deinit_setup();
    tick_timer_service_unsubscribe();
    window_destroy(window);
}

static void set_start_time(int weekday, int hour, int minute, int second) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    long d = weekday - t->tm_wday;
    if (d != 0) {
        if (d < 0)
            d += 7L;
        d *= 24L;
        delta_time += d;
    }
    d = hour - t->tm_hour;
    if (d != 0) {
        delta_time += d;
    }
    delta_time *= 3600L;
    d = minute - t->tm_min;
    if (d != 0) {
        d *= 60L;
        delta_time += d;
    }
    d = second - t->tm_sec;
    if (d != 0) {
        delta_time += d;
    }
    start_time = now + delta_time;
}

int main(void) {
    init();
    // ---------------------------------------------------
    // use this to start at given weekday and time
    //
    // set_start_time(3, 10, 39, 45);
    // set_start_time(5, 18, 39, 45);
    // set_start_time(2, 9, 19, 45);
    // set_start_time(1, 6, 10, 45);
    // set_start_time(1, 7, 10, 45);
    // ---------------------------------------------------
    app_event_loop();
    deinit();
}
// eof
