//sof
#pragma once
#ifndef WATCH_H
#define	WATCH_H
#include "Const.h"

void force_update_watch();
void window_load_watch(Window *window);
void window_unload_watch(Window *window);
void handle_tick_watch(struct tm *t);

#endif	/* WATCH_H */
//eof