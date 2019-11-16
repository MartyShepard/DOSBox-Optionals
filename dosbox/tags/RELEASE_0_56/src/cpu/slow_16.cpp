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


#include "dosbox.h"
#include "mem.h"
#include "cpu.h"
#include "inout.h"
#include "callback.h"
#include "pic.h"
#include "fpu.h"

#if C_DEBUG
#include "debug.h"
#endif

typedef PhysPt EAPoint;
#define SegBase(c)	SegPhys(c)
#define LoadMb(off) mem_readb(off)
#define LoadMw(off) mem_readw(off)
#define LoadMd(off) mem_readd(off)

#define LoadMbs(off) (Bit8s)(LoadMb(off))
#define LoadMws(off) (Bit16s)(LoadMw(off))
#define LoadMds(off) (Bit32s)(LoadMd(off))

#define SaveMb(off,val)	mem_writeb(off,val)
#define SaveMw(off,val)	mem_writew(off,val)
#define SaveMd(off,val)	mem_writed(off,val)

#define LoadRb(reg) reg
#define LoadRw(reg) reg
#define LoadRd(reg) reg

#define SaveRb(reg,val)	reg=val
#define SaveRw(reg,val)	reg=val
#define SaveRd(reg,val)	reg=val

extern Bitu cycle_count;

/* Enable parts of the cpu emulation */
#define CPU_386							//Enable 386 instructions
#if C_FPU
#define CPU_FPU							//Enable FPU escape instructions
#endif

#include "core_16/support.h"
static Bitu CPU_Real_16_Slow_Decode_Special(Bitu count);

static Bitu CPU_Real_16_Slow_Decode(Bits count) {
#include "core_16/start.h"		
	do {
#if C_DEBUG
		cycle_count++;		
#endif
#if C_HEAVY_DEBUG
		SAVEIP;
		if (DEBUG_HeavyIsBreakpoint()) return CBRET_NONE;
#endif
		#include "core_16/main.h"	
	} while (--count>0);
	#include "core_16/stop.h"		
	return CBRET_NONE;
}

static Bitu CPU_Real_16_Slow_Decode_Special(Bits count) {
	while (count>0) {
		if (flags.tf) {
			Interrupt(3);
			cpudecoder=&CPU_Real_16_Slow_Decode;
			return CBRET_NONE;
		}
		CPU_Real_16_Slow_Decode(1);
		if (!flags.tf) {
			cpudecoder=&CPU_Real_16_Slow_Decode;
			return CBRET_NONE;
		};
		count--;
	}

	return CBRET_NONE;
}

void CPU_Real_16_Slow_Start(void) {

	cpudecoder=&CPU_Real_16_Slow_Decode;
	EAPrefixTable[0]=&GetEA_16_n;
	EAPrefixTable[1]=&GetEA_16_s;
	EAPrefixTable[2]=&GetEA_32_n;
	EAPrefixTable[3]=&GetEA_32_s;
	PrefixReset;
};
