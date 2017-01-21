#pragma once
#ifndef SETUP_H
#define	SETUP_H
#include "Const.h"

#define COMM true
#define DBG_BASIC false
#define DBG_VIBRATION false
#define FAKE_READ_STORED true

// defaults

#define DEFAULT_L "45,90"
#define MAX_D 48
//#define DEFAULT_D "0655N15N10N20N10N10N10N10N10N10N10N10N10"
#define DEFAULT_D "0740N5N5N10N20N10N10N5N10N5N10N5"
#define DEFAULT_DX ""
#define MAX_T 160
//#define DEFAULT_T "MO:0s1f1f1f1f1f1,TU:0f1f1f1s1f1s,WE:0f0f0f1s1f1f1,TH:0f1f1s0s1f1f0f0f1,FR:1f1f1f1f1"
#define DEFAULT_T "MO:1s1f1f1f1f1f1,TU:1s1f1f1f1f1f1f1f1f1,WE:1s1f1f1f1f1f1,TH:1s1f1f1f1f1f1f1f1,FR:1s1f1f1f1f1f1"
//#define DEFAULT_T "WE:0f0f0f1s1f1f1"
#define MAX_V 16
#define DEFAULT_V "L:s0e0;S:s1e-1"
#define MAX_B 16
#define DEFAULT_B "0b,Bb,Sb"

struct day {
    int start;
    int lessons[LESSON_COUNT];
    int breaks[LESSON_COUNT];
};

#define MAX_VIBRATIONS 4

struct one_vibration {
    int count;
    int positions[MAX_VIBRATIONS];
};

struct vibration {
    struct one_vibration start;
    struct one_vibration end;
};

struct vibrations {
    struct vibration lesson;
    struct vibration break_free;
    struct vibration break_busy;
};

static struct day days[5];
static struct vibrations vibr;

static int want_week[DAY_COUNT][ARR_SIZE];
static int want_mark[DAY_COUNT][ARR_SIZE];
static int want_vibrate[DAY_COUNT][ARR_SIZE];

// Setting keys

enum settingKeys {
    SETTING_DAY0 = 0,
    SETTING_DAY1 = 1,
    SETTING_DAY2 = 2,
    SETTING_DAY3 = 3,
    SETTING_DAY4 = 4,
    SETTING_LESSONS = 5,
    SETTING_VIBRATION = 6,
    SETTING_BREAKS = 7,
    SETTING_TABLE = 8
};

#define TupletCStringFixed(_key, _cstring) \
((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = _cstring!=NULL ? strlen(_cstring) + 1 : 0 }})

static AppSync app;
static uint8_t sync_buffer[1024];
static char current_day0[MAX_D];
static char current_day1[MAX_D];
static char current_day2[MAX_D];
static char current_day3[MAX_D];
static char current_day4[MAX_D];
static char current_lessons[16];
static char current_vibration[MAX_V];
static char current_breaks[MAX_B];
static char current_table[MAX_T];

void init_setup(void);
void deinit_setup(void);
void handle_tick_setup(struct tm *t);

#endif	/* SETUP_H */
//eof

