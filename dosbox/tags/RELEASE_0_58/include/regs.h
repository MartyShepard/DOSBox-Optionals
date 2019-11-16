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

#if !defined __REGS_H
#define __REGS_H

#include <mem.h>

struct Flag_Info {
    union {
		Bit8u b;
		Bit16u w;
		Bit32u d;
	} var1,var2,result;
	Bitu type;
	Bitu prev_type;
	bool cf,sf,pf,af,zf,of,df,tf,intf;
	bool nt;
	Bit8u io;
	bool oldcf;
};

struct Segment {
	Bit16u val;
	PhysPt phys;							/* The phyiscal address start in emulated machine */
};


enum SegNames { cs=0,ds,es,fs,gs,ss};

union GenReg32 {
	Bit32u dword[1];
	Bit16u word[2];
	Bit8u byte[4];
};

#ifdef WORDS_BIGENDIAN

#define DW_INDEX 0
#define W_INDEX 1
#define BH_INDEX 2
#define BL_INDEX 3

#else

#define DW_INDEX 0
#define W_INDEX 0
#define BH_INDEX 1
#define BL_INDEX 0

#endif

struct CPU_Regs {
	 GenReg32 regs[8],ip;
};

extern Segment Segs[6];
extern Flag_Info flags;
extern CPU_Regs cpu_regs;


//#define SegPhys(index) Segs[index].phys
//#define SegValue(index) Segs[index].val

INLINE PhysPt SegPhys(SegNames index) {
	return Segs[index].phys;
}

INLINE Bit16u SegValue(SegNames index) {
	return Segs[index].val;
}

	
INLINE RealPt RealMakeSeg(SegNames index,Bit16u off) {
	return RealMake(SegValue(index),off);	
}


INLINE void SegSet16(Bitu index,Bit16u val) {
	Segs[index].val=val;
	Segs[index].phys=val << 4;
}


enum REG_NUM {
	REG_NUM_AX, REG_NUM_CX, REG_NUM_DX, REG_NUM_BX,
	REG_NUM_SP, REG_NUM_BP, REG_NUM_SI, REG_NUM_DI
};

//macros to convert a 3-bit register index to the correct register
#define reg_8l(reg) (cpu_regs.regs[(reg)].byte[BL_INDEX])
#define reg_8h(reg) (cpu_regs.regs[(reg)].byte[BH_INDEX])
#define reg_8(reg) ((reg) & 4 ? reg_8h((reg) & 3) : reg_8l((reg) & 3))
#define reg_16(reg) (cpu_regs.regs[(reg)].word[W_INDEX])
#define reg_32(reg) (cpu_regs.regs[(reg)].dword[DW_INDEX])


#define reg_al cpu_regs.regs[REG_NUM_AX].byte[BL_INDEX]
#define reg_ah cpu_regs.regs[REG_NUM_AX].byte[BH_INDEX]
#define reg_ax cpu_regs.regs[REG_NUM_AX].word[W_INDEX]
#define reg_eax cpu_regs.regs[REG_NUM_AX].dword[DW_INDEX]

#define reg_bl cpu_regs.regs[REG_NUM_BX].byte[BL_INDEX]
#define reg_bh cpu_regs.regs[REG_NUM_BX].byte[BH_INDEX]
#define reg_bx cpu_regs.regs[REG_NUM_BX].word[W_INDEX]
#define reg_ebx cpu_regs.regs[REG_NUM_BX].dword[DW_INDEX]

#define reg_cl cpu_regs.regs[REG_NUM_CX].byte[BL_INDEX]
#define reg_ch cpu_regs.regs[REG_NUM_CX].byte[BH_INDEX]
#define reg_cx cpu_regs.regs[REG_NUM_CX].word[W_INDEX]
#define reg_ecx cpu_regs.regs[REG_NUM_CX].dword[DW_INDEX]

#define reg_dl cpu_regs.regs[REG_NUM_DX].byte[BL_INDEX]
#define reg_dh cpu_regs.regs[REG_NUM_DX].byte[BH_INDEX]
#define reg_dx cpu_regs.regs[REG_NUM_DX].word[W_INDEX]
#define reg_edx cpu_regs.regs[REG_NUM_DX].dword[DW_INDEX]

#define reg_si cpu_regs.regs[REG_NUM_SI].word[W_INDEX]
#define reg_esi cpu_regs.regs[REG_NUM_SI].dword[DW_INDEX]

#define reg_di cpu_regs.regs[REG_NUM_DI].word[W_INDEX]
#define reg_edi cpu_regs.regs[REG_NUM_DI].dword[DW_INDEX]

#define reg_sp cpu_regs.regs[REG_NUM_SP].word[W_INDEX]
#define reg_esp cpu_regs.regs[REG_NUM_SP].dword[DW_INDEX]

#define reg_bp cpu_regs.regs[REG_NUM_BP].word[W_INDEX]
#define reg_ebp cpu_regs.regs[REG_NUM_BP].dword[DW_INDEX]

#define reg_ip cpu_regs.ip.word[W_INDEX]
#define reg_eip cpu_regs.ip.dword[DW_INDEX]

#endif

