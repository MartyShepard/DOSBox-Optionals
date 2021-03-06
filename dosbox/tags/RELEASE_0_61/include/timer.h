/*
 *  Copyright (C) 2002-2004  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _TIMER_H_
#define _TIMER_H_
/* underlying clock rate in HZ */
#include <SDL.h>

#define PIT_TICK_RATE 1193182

#define GetTicks() SDL_GetTicks()

typedef void (*TIMER_TickHandler)(Bitu ticks);
typedef void (*TIMER_MicroHandler)(void);

typedef void TIMER_Block;


/* Register a function that gets called everytime if 1 or more ticks pass */
TIMER_Block * TIMER_RegisterTickHandler(TIMER_TickHandler handler);
/* Register a function to be called every x microseconds */
TIMER_Block * TIMER_RegisterMicroHandler(TIMER_MicroHandler handler,Bitu micro);

/* Set the microseconds value to a new value */
void TIMER_SetNewMicro(TIMER_Block * block,Bitu micro);

/* This will add 1 milliscond to all timers */
void TIMER_AddTick(void);

#endif

