/*
 *  Copyright (C) 2002  The DOSBox Team
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

#ifndef __CALLBACK_H
#define __CALLBACK_H

#include <mem.h>

typedef Bitu (*CallBack_Handler)(void);
extern CallBack_Handler CallBack_Handlers[];

enum { CB_RETF,CB_IRET };

#define CB_MAX 1024
#define CB_SEG 0xC800

enum {	
	CBRET_NONE=0,CBRET_STOP=1
};

extern Bit8u lastint;
INLINE RealPt CALLBACK_RealPointer(Bitu callback) {
	return RealMake(CB_SEG,callback << 4);
}

Bitu CALLBACK_Allocate();

void CALLBACK_Idle(void);


void CALLBACK_RunRealInt(Bit8u intnum);
void CALLBACK_RunRealFar(Bit16u seg,Bit16u off);

bool CALLBACK_Setup(Bitu callback,CallBack_Handler handler,Bitu type);

bool CALLBACK_Free(Bitu callback);

void CALLBACK_SCF(bool val);
void CALLBACK_SZF(bool val);
#endif

