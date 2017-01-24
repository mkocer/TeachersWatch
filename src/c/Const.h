//sof
#pragma once
#ifndef CONST_H
#define	CONST_H

#define DAY_COUNT 7
#define LESSON_COUNT 12
#define ARR_SIZE 24 // = 2 * LESSON_COUNT

#ifdef PBL_COLOR
#define DIAL_COLOR GColorWhite
#define TEXT_COLOR GColorBlack
#define VORSIGHT_COLOR GColorRed
#else //****************************
#define DIAL_COLOR GColorBlack
#define TEXT_COLOR GColorWhite
#define VORSIGHT_COLOR GColorWhite
#endif

#endif	/* CONST_H */
//eof