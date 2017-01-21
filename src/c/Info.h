//sof
#pragma once
#ifndef INFO_H
#define	INFO_H
#include "Const.h"

// info business interface

#define DBG_SHOW_INFO false
#define DBG_SHOW_INFO_DETAIL false

// Setting values

enum infoSettingKeys {
    INFO_SETTING_INFO = 10
};

static enum infoKeys {
    INFO_WHITE = 0, INFO_BLACK, INFO_CRAPE
} current_info;

#define MAX_DELTA_VISIBLE 90
#define MAX_DELTA 60

static int week[DAY_COUNT][ARR_SIZE];
static int mark[DAY_COUNT][ARR_SIZE];
static int vibrate[DAY_COUNT][ARR_SIZE];

void init_week();
void copy_week(int want_week[DAY_COUNT][ARR_SIZE], int want_mark[DAY_COUNT][ARR_SIZE], int want_vibrate[DAY_COUNT][ARR_SIZE]);

// design and basic interface

void force_update_info();
void window_load_info(Window *window);
void window_unload_info(Window *window);
void handle_tick_info(struct tm *t);

#endif	/* INFO_H */
//eof