#pragma once

#include <stdint.h>

void Timer_Init();
void Timer_SetTimer(uint16_t time);
extern void Timer_onTimerOverflow();