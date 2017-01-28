#include <pebble.h>
#include "Info.h"
#include "Setup.h"

// --------------------------------------------------------

char* read_stored_text(char* text, const size_t text_size, const uint32_t key, const char* defaultValue) {
    if (persist_exists(key)) {
        persist_read_string(key, text, text_size);
    } else {
        strncpy(text, defaultValue, text_size);
    }
    if (FAKE_READ_STORED)
        strncpy(text, defaultValue, text_size);
    return text;
}

void store_text(const uint32_t key, const char* text) {
    if (text == NULL || strlen(text) <= 0) {
        persist_delete(key);
    } else {
        persist_write_string(key, text);
    }
}

// --------------------------------------------------------

static int lesson_short = 45;
static int lesson_long = 0;

int strpos(char *str, int c) {
    if (str == NULL)
        return -1;
    int len = strlen(str);
    for (int i = 0; i < len; i++)
        if (str[i] == c)
            return i;
    return -1;
}

int find_integer_end(char *s, int pos, int len, bool use_signum) {
    int pos1 = pos;
    while (pos1 < len
            &&
            ((s[pos1] >= '0' && s[pos1] <= '9') || (use_signum && (s[pos] == '+' || s[pos] == '-'))
            )
            )
        pos1++;
    return pos1;
}

int read_integer(char *str, int start, int end, int defaultValue) {
    if (str == NULL || (end >= 0 && end <= start))
        return defaultValue;
    int len = strlen(str);
    if (len <= start)
        return defaultValue;
    int signum = 1;
    if (str[start] == '+') {
        signum = 1;
        start++;
    } else
        if (str[start] == '-') {
        signum = -1;
        start++;
    }
    if (end < 0)
        end = len;
    if (end <= start)
        return defaultValue;
    int res = 0;
    for (int i = start; i < end; i++) {
        if (str[i] < '0' || str[i] > '9')

            return defaultValue;
        res *= 10;
        res += str[i] - '0';
    }
    return res*signum;
}

int read_lesson(char *lesson) {
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_lesson: start");
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "lesson = \"%s\"", lesson);
    if (lesson == NULL || strlen(lesson) <= 0) {
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_lesson: end, empty");
        return 0;
    }
    int pos = strpos(lesson, ',');
    if (pos > 0) {
        lesson_short = read_integer(lesson, 0, pos, -1);
        lesson_long = read_integer(lesson, pos + 1, -1, -1);
    } else {
        lesson_short = read_integer(lesson, 0, -1, -1);
        lesson_long = 0;
    }
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "lesson_short = %d", lesson_short);
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "lesson_long = %d", lesson_long);
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_lesson: end");
    if (lesson_short > 0 && lesson_long >= 0)
        return 1;

    else
        return 0;
}

int read_day(char *day, int which) {
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: start", which);
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "day = \"%s\"", day);
    struct day * d = &days[which];
    d->start = -1;
    for (int i = 0; i < LESSON_COUNT; i++) {
        d->lessons[i] = -1;
        d->breaks[i] = -1;
    }
    if (day == NULL || strlen(day) <= 0) {
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: end, empty", which);
        return 0;
    }
    int hour = read_integer(day, 0, 2, -1);
    int minute = read_integer(day, 2, 4, -1);
    if (hour < 0 || minute < 0) {
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: end, wrong start", which);
        return 0;
    }
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "hour =  %d", hour);
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "minute =  %d", minute);
    int len = strlen(day);
    int pos = 4;
    int total = 0;
    for (int i = 0; i < LESSON_COUNT; i++) {
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: hour %d, pos %d: \"%c\"", which, i, pos, day[pos]);
        if (pos >= len)
            break;
        if (day[pos] == 'N')
            d->lessons[i] = lesson_short;
        else
            if (day[pos] == 'L' && lesson_long > 0)
            d->lessons[i] = lesson_long;
        else {
            if (DBG_BASIC)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: end, wrong lesson %d: \"%c\"", which, i, day[pos]);
            return 0;
        }
        pos++;
        total++;
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: lesson %d = %d", which, i, d->lessons[i]);
        if (pos >= len)
            break;
        int pos1 = find_integer_end(day, pos, len, false);
        if (pos1 == pos) {
            if (DBG_BASIC)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: no break %d", which, i);
            break;
        }
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: hour %d, pos %d: \"%c\", pos1 %d", which, i, pos, day[pos], pos1);
        d->breaks[i] = read_integer(day, pos, pos1, -1);
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: break %d = %d", which, i, d->breaks[i]);
        pos = pos1;
    }
    if (total > 0)
        d->start = hour * 60 + minute;
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_day %d: end", which);
    if (total > 0)
        return 1;

    else
        return 0;
}

void init_one_vibration(struct one_vibration* v) {
    v->count = 0;
}

void init_vibration(struct vibration* v) {
    init_one_vibration(&v->start);
    init_one_vibration(&v->end);
}

void show_one_vibration(const char*label, struct one_vibration* v) {
    if (v->count == 0)
        return;
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "      %s:", label);
    for (int i = 0; i < v->count; i++)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "         %d", v->positions[i]);
}

void show_vibration(const char*label, struct vibration* v) {
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "   %s:", label);
    show_one_vibration("Start", &v->start);
    show_one_vibration("End", &v->end);
}

void show_vibrations(struct vibrations* v) {
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "vibrations:");
    show_vibration("Lesson", &v->lesson);
    show_vibration("Free Break", &v->break_free);
    show_vibration("Busy Break", &v->break_busy);
}

int read_vibration(char *s) {
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration: start");
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "vibration = \"%s\"", s);
    init_vibration(&vibr.lesson);
    init_vibration(&vibr.break_free);
    init_vibration(&vibr.break_busy);
    if (s == NULL || strlen(s) <= 0) {
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration: end, empty");
        return 0;
    }
    int len = strlen(s);
    int pos = 0;
    struct vibration* v = NULL;
    struct one_vibration* ov = NULL;
    bool new_vibr;
    while (pos < len) {
        new_vibr = false;
        if (s[pos] == 'L') {
            v = &vibr.lesson;
            new_vibr = true;
        } else
            if (s[pos] == 'B') {
            v = &vibr.break_free;
            new_vibr = true;
        } else
            if (s[pos] == 'S') {
            v = &vibr.break_busy;
            new_vibr = true;
        }
        if (new_vibr) {
            pos++;
            if (pos >= len)
                break;
            if (s[pos] != ':') {
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE,
                        "read_vibration: end, wrong missing colon after vibration at %d: %c", pos, s[pos]);
                return 0;
            }
            pos++;
            if (pos >= len)
                break;
        }
        if (s[pos] == ';') {
            v = NULL;
            pos++;
            continue;
        }
        if (v == NULL) {
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration: end, no vibration at %d", pos);
            return 0;
        }
        if (s[pos] == 's') {
            ov = &v->start;
        } else
            if (s[pos] == 'e') {
            ov = &v->end;
        } else {
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration: end, no one vibration at %d: %c", pos, s[pos]);
            return 0;
        }
        pos++;
        if (pos >= len)
            break;
        if (ov == NULL) {
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration: end, no one vibration at %d", pos);
            return 0;
        }
        // find number, possible negative
        int pos1 = find_integer_end(s, pos, len, true);
        if (pos1 == pos) {
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration end, no vibration size at %d", pos);
            return 0;
        }
        int n = read_integer(s, pos, pos1, 0);
        pos = pos1;
        if (ov->count < MAX_VIBRATIONS) {
            ov->positions[ov->count] = n;
            ov->count++;
        }
    }
    if (DBG_VIBRATION)
        show_vibrations(&vibr);
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_vibration: end");
    return 1;
}

void show_vibration_array(char*label, char*label2, int a[ARR_SIZE]) {
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           one_vibration %s %s:", label, label2);
    for (int j = 0; j < ARR_SIZE; j++) {
        if (a[j] < 0)
            break;
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "               %d:  %02d:%02d", j, a[j] / 60, a[j] % 60);
    }
}

void use_one_vibration(struct one_vibration* v, char*label, char*label2, int minute, int min_minute, int max_minute, int a[ARR_SIZE]) {
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           use_one_vibration %s %s: %02d:%02d", label, label2, minute / 60, minute % 60);
    if (min_minute > 0 && minute < min_minute) {
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           end, before min %02d:%02d", min_minute / 60, min_minute % 60);
        return;
    }
    if (max_minute > 0 && minute > max_minute) {
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           end, after max %02d:%02d", max_minute / 60, max_minute % 60);
        return;
    }
    if (v->count <= 0) {
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           end, no data");
        return;
    }
    int last = -1;
    for (int j = 0; j < ARR_SIZE; j++) {
        if (a[j] < 0)
            break;
        last = j;
    }
    int len = last + 1;
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           go, len = %d", len);
    if (DBG_VIBRATION)
        show_vibration_array(label, "start", a);
    for (int i = 0; i < v->count; i++) {
        int time = minute + v->positions[i];
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "             time %02d:%02d", time / 60, time % 60);
        int pos = -1;
        for (int j = 0; j < ARR_SIZE; j++) {
            pos = j;
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "                pos %d", pos);
            if (a[j] < 0) {
                if (DBG_VIBRATION)
                    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "                break");
                break;
            }
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "                at %02d:%02d", a[pos] / 60, a[pos] % 60);
            if (a[j] >= time) {
                if (DBG_VIBRATION)
                    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "                break");
                break;
            }
        }
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "             found pos %d", pos);
        if (len == ARR_SIZE) {// no more fits
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "               no more fits");
            break;
        }
        if (a[pos] == time) { // already there
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "               aleady there");
            continue;
        }
        if (a[pos] < 0) { // last
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "               last");
            a[pos] = time;
        } else { // shift
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "               shift");
            for (int j = ARR_SIZE - 1; j >= pos + 1; j--) {
                if (a[j - 1] < 0)
                    break;
                a[j] = a[j - 1];
            }
            a[pos] = time;
        }
        len++;
    }
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           show, len = %d", len);
    if (DBG_VIBRATION)
        show_vibration_array(label, "end", a);
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "           use_one_vibration: end, len = %d", len);
}

void use_vibration(struct vibrations* vibrations) {
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "use_vibration: start");
    for (int day = 0; day < DAY_COUNT; day++) {
        if (DBG_VIBRATION)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "  day %d", day);
        int min_minute = -1;
        int max_minute = -1;
        for (int pos = 0; pos < ARR_SIZE; pos++) {
            if (want_week[day][pos] < 0) {
                break;
            }
            bool is_free = want_mark[day][pos] == 0;
            if (is_free)
                continue;
            if (min_minute < 0)
                min_minute = want_week[day][pos];
            if (max_minute < want_week[day][pos])
                max_minute = want_week[day][pos];
        }
        for (int pos = 0; pos < ARR_SIZE; pos++) {
            if (want_week[day][pos] < 0) {
                break;
            }
            bool is_lesson = pos % 2 == 0;
            bool is_free = want_mark[day][pos] == 0;
            if (DBG_VIBRATION)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "        pos %d, lesson %d, free %d", pos, is_lesson ? 1 : 0, is_free ? 1 : 0);
            struct vibration*v;
            char * label;
            if (is_lesson) {
                if (is_free) {
                    v = NULL;
                } else {
                    v = &vibrations->lesson;
                    label = "Lesson";
                }
            } else {
                if (is_free) {
                    v = &vibrations->break_free;
                    label = "Free Break";
                } else {
                    v = &vibrations->break_busy;
                    label = "Busy Break";
                }
            }
            if (v == NULL)
                continue;
            use_one_vibration(&v->start, label, "Start", want_week[day][pos], min_minute, max_minute, want_vibrate[day]);
            if (pos + 1 < ARR_SIZE) {
                use_one_vibration(&v->end, label, "End", want_week[day][pos + 1], min_minute, max_minute, want_vibrate[day]);
            }
            if (DBG_VIBRATION)
                show_vibration_array(label, "", want_vibrate[day]);
        }
        // show that day
        if (DBG_VIBRATION) {
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "  day %d final setup:", day);
            for (int pos = 0; pos < ARR_SIZE; pos++) {
                int minute = want_vibrate[day][pos];
                if (minute < 0)
                    break;
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "         %02d:%02d", minute / 60, minute % 60);
            }
        }
    }
    if (DBG_VIBRATION)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "use_vibration: end");
}

int read_weekday(char *s, int pos, int len) {
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_weekday at %d: start", pos);
    if (pos + 2 >= len) {
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_weekday, end, too short");
        return -1;
    }
    if (s[pos + 2] != ':') {
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_weekday, end, missing :");
        return -1;
    }
    int a = s[pos];
    int b = s[pos + 1];
    int res = -1;
    if (a == 'M' && b == 'O')
        res = 1;
    else
        if (a == 'T' && b == 'U')
        res = 2;
    else
        if (a == 'W' && b == 'E')
        res = 3;
    else
        if (a == 'T' && b == 'H')
        res = 4;
    else
        if (a == 'F' && b == 'R')
        res = 5;
    else
        if (a == 'S' && b == 'A')
        res = 6;
    else
        if (a == 'S' && b == 'U')
        res = 0;
    if (res < 0) {

    } else {

    }
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_weekday, end, %c%c : %d", a, b, res);

    return res;
}

void init_want_week() {
    for (int i = 0; i < DAY_COUNT; i++)
        for (int j = 0; j < ARR_SIZE; j++) {

            want_week[i][j] = -1;
            want_mark[i][j] = 0;
            want_vibrate[i][j] = -1;
        }
}

int read_table(char *table) {
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: start");
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "\"%s\"", table);
    if (table == NULL || strlen(table) <= 0) {
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: end, empty");
        return 0;
    }
    int len = strlen(table);
    int pos = 0;
    while (pos < len) {
        int weekday = read_weekday(table, pos, len);
        if (weekday < 0)
            break;
        pos += 3;
        if (pos >= len)
            break;
        int day = 0;
        if (table[pos] == 'D') { // different day;
            pos++;
            if (pos + 1 >= len)
                break;
            int d = table[pos] - '0';
            if (d < 0 || d > 4) {
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: end, wrong day %c", table[pos]);
                return 0;
            }
            day = d;
            pos++;
        }
        struct day *d = &days[day];
        if (d->start < 0) {
            if (DBG_BASIC)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: end, day %d not set up", day);
            return 0;
        }
        int minute = d->start;
        int lessons = 0;
        if (DBG_BASIC)
            APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: weekday %d, day %d ... go", weekday, day);
        for (int lesson = 0; lesson < LESSON_COUNT; lesson++) {
            int teach;
            if (table[pos] == '1')
                teach = 1;
            else
                if (table[pos] == '0')
                teach = 0;
            else {
                break;
            }
            pos++;
            int busy_break = 0;
            if (pos < len) {
                if (table[pos] == 'f') {
                    busy_break = 0;
                    pos++;
                } else if (table[pos] == 's') {
                    busy_break = 1;
                    pos++;
                }
            }
            if (DBG_BASIC)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: lesson %d, teach %d, busy break %d", lesson, teach, busy_break);
            if (DBG_BASIC)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: lesson start %02d:%02d", minute / 60, minute % 60);
            want_week[weekday][lesson * 2] = minute;
            minute += d->lessons[lesson];
            want_week[weekday][lesson * 2 + 1] = minute;
            if (DBG_BASIC)
                APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: break start %02d:%02d", minute / 60, minute % 60);
            if (teach) {
                want_mark[weekday][lesson * 2] = 1;
            }
            if (busy_break) {
                want_mark[weekday][lesson * 2 + 1] = 1;
            }
            minute += d->breaks[lesson];
            lessons++;
        }
        if (pos < len && table[pos] == ',') {
            pos++;
        }
    }
    if (DBG_BASIC)
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "read_table: end");

    return 1;
}

void set_data(char *lesson,
        char *day0, char *day1, char *day2, char *day3, char *day4,
        char *vibration, char *breaks, char *table) {
    if (lesson == NULL || day0 == NULL || table == NULL) {
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: some null");
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: start");
    // -----------------------------------
    // lesson
    if (read_lesson(lesson) == 0) {
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: end, missing lesson");
        return;
    }
    // -----------------------------------
    // days
    if (read_day(day0, 0) == 0) {
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: end, missing basic day0");
        return;
    }
    read_day(day1, 1);
    read_day(day2, 2);
    read_day(day3, 3);
    read_day(day4, 4);
    // -----------------------------------
    // breaks
    // TODO
    // -----------------------------------
    // table
    init_want_week();
    if (read_table(table) == 0) {
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: end, missing table");

        return;
    }
    // -----------------------------------
    // vibration
    if (read_vibration(vibration)) {
        use_vibration(&vibr);
    }
    // -----------------------------------
    // use it
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: show it");
    copy_week(want_week, want_mark, want_vibrate);
    force_update_info();
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "set_data: end");
}

void set_current_data() {

    set_data(current_lessons, current_day0, current_day1, current_day2, current_day3, current_day4,
            current_vibration, current_breaks, current_table);
}

// --------------------------------------------------------

/*
  Handle data
 */
static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "tuple_changed_callback: start, key = %d", (int) key);
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "\"%s\"", tuple_new->value->cstring);

    switch (key) {
        case SETTING_DAY0:
            strncpy(current_day0, tuple_new->value->cstring, sizeof (current_day0));
            break;
        case SETTING_DAY1:
            strncpy(current_day1, tuple_new->value->cstring, sizeof (current_day1));
            break;
        case SETTING_DAY2:
            strncpy(current_day2, tuple_new->value->cstring, sizeof (current_day2));
            break;
        case SETTING_DAY3:
            strncpy(current_day3, tuple_new->value->cstring, sizeof (current_day3));
            break;
        case SETTING_DAY4:
            strncpy(current_day4, tuple_new->value->cstring, sizeof (current_day4));
            break;
        case SETTING_LESSONS:
            strncpy(current_lessons, tuple_new->value->cstring, sizeof (current_lessons));
            break;
        case SETTING_VIBRATION:
            strncpy(current_vibration, tuple_new->value->cstring, sizeof (current_vibration));
            break;
        case SETTING_BREAKS:
            strncpy(current_breaks, tuple_new->value->cstring, sizeof (current_breaks));
            break;
        case SETTING_TABLE:
            strncpy(current_table, tuple_new->value->cstring, sizeof (current_table));

            break;
    }
    store_text(key, tuple_new->value->cstring);
    set_current_data();
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "tuple_changed_callback: end, key = %d", (int) key);
}

/*
  Handle errors
 */
static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

void init_setup(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "init_setup: start");

    read_stored_text(current_day0, sizeof (current_day0), SETTING_DAY0, DEFAULT_D);
    read_stored_text(current_day1, sizeof (current_day1), SETTING_DAY1, DEFAULT_DX);
    read_stored_text(current_day2, sizeof (current_day2), SETTING_DAY2, DEFAULT_DX);
    read_stored_text(current_day3, sizeof (current_day3), SETTING_DAY3, DEFAULT_DX);
    read_stored_text(current_day4, sizeof (current_day4), SETTING_DAY4, DEFAULT_DX);
    read_stored_text(current_lessons, sizeof ( current_lessons), SETTING_LESSONS, DEFAULT_L);
    read_stored_text(current_vibration, sizeof ( current_vibration), SETTING_VIBRATION, DEFAULT_V);
    read_stored_text(current_breaks, sizeof ( current_breaks), SETTING_BREAKS, DEFAULT_B);
    read_stored_text(current_table, sizeof ( current_table), SETTING_TABLE, DEFAULT_T);

    if (COMM) {
        // Initial settings
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "init_setup: COMM is true");
        Tuplet initial_values[] = {
            TupletCStringFixed(SETTING_DAY0, current_day0),
            TupletCStringFixed(SETTING_DAY1, current_day1),
            TupletCStringFixed(SETTING_DAY2, current_day2),
            TupletCStringFixed(SETTING_DAY3, current_day3),
            TupletCStringFixed(SETTING_DAY4, current_day4),
            TupletCStringFixed(SETTING_LESSONS, current_lessons),
            TupletCStringFixed(SETTING_VIBRATION, current_vibration),
            TupletCStringFixed(SETTING_BREAKS, current_breaks),
            TupletCStringFixed(SETTING_TABLE, current_table)
        };

        // Open AppMessage to transfers
        app_message_open(sizeof (sync_buffer), sizeof (sync_buffer));

        // Initialize AppSync
        app_sync_init(&app, sync_buffer
                , sizeof (sync_buffer)
                , initial_values
                , ARRAY_LENGTH(initial_values)
                , tuple_changed_callback
                , app_error_callback
                , NULL
                );

        // Perform sync
        //app_sync_set(&app, initial_values, ARRAY_LENGTH(initial_values));
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "init_setup: set_current_data");
    set_current_data();
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "init_setup: end");
}

static int have_data = 0;
static long start = -1;

void handle_tick_setup(struct tm *t) {
    if (have_data > 0)
        return;
    long now = t->tm_hour;
    now *= 60;
    now += t->tm_min;
    now *= 60;
    now += t->tm_sec;
    if (start < 0) {
        start = now;
        return;
    }
    if (now - start < 3)

        return;
    have_data = 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "handle_tick_setup: set_current_data");
    set_current_data();
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "handle_tick_setup: end");
}

void deinit_setup(void) {
    // Clean up AppSync system
    if (COMM) {
        app_sync_deinit(&app);
    }
}

// eof
