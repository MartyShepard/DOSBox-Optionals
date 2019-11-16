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

#ifndef __CPU_H
#define __CPU_H

#include "dosbox.h" 
#include "regs.h"
#include "mem.h"

/* CPU Cycle Timing */
extern Bits CPU_Cycles;
extern Bits CPU_CycleLeft;
extern Bits CPU_CycleMax;

/* Some common Defines */
/* A CPU Handler */
typedef Bits (CPU_Decoder)(void);
extern CPU_Decoder * cpudecoder;

Bits CPU_Core_Normal_Run(void);
Bits CPU_Core_Normal_Trap_Run(void);
Bits CPU_Core_Full_Run(void);
Bits CPU_Core_Dyn_X86_Run(void);

//CPU Stuff

extern Bit16u parity_lookup[256];

void CPU_LLDT(Bitu selector);
void CPU_LTR(Bitu selector);
void CPU_LIDT(Bitu limit,Bitu base);
void CPU_LGDT(Bitu limit,Bitu base);

void CPU_STR(Bitu & selector);
void CPU_SLDT(Bitu & selector);
void CPU_SIDT(Bitu & limit,Bitu & base);
void CPU_SGDT(Bitu & limit,Bitu & base);


void CPU_ARPL(Bitu & dest_sel,Bitu src_sel);
void CPU_LAR(Bitu selector,Bitu & ar);
void CPU_LSL(Bitu selector,Bitu & limit);

bool CPU_SET_CRX(Bitu cr,Bitu value);
Bitu CPU_GET_CRX(Bitu cr);

void CPU_SMSW(Bitu & word);
bool CPU_LMSW(Bitu word);

void CPU_VERR(Bitu selector);
void CPU_VERW(Bitu selector);

void CPU_JMP(bool use32,Bitu selector,Bitu offset,Bitu opLen=0);
void CPU_CALL(bool use32,Bitu selector,Bitu offset,Bitu opLen=0);
void CPU_RET(bool use32,Bitu bytes,Bitu opLen=0);

#define CPU_INT_SOFTWARE		0x1
#define CPU_INT_EXCEPTION		0x2
#define CPU_INT_HAS_ERROR		0x4


void CPU_Interrupt(Bitu num,Bitu type,Bitu opLen=0);
INLINE void CPU_HW_Interrupt(Bitu num) {
	CPU_Interrupt(num,0);
}
INLINE void CPU_SW_Interrupt(Bitu num,Bitu OpLen) {
	CPU_Interrupt(num,CPU_INT_SOFTWARE,OpLen);
}

void CPU_Exception(Bitu which,Bitu error=0);
void CPU_StartException(void);
void CPU_SetupException(Bitu which,Bitu error=0);

void CPU_IRET(bool use32);
bool CPU_SetSegGeneral(SegNames seg,Bitu value);
void CPU_HLT(Bitu opLen);

void CPU_CPUID(void);
Bitu CPU_Pop16(void);
Bitu CPU_Pop32(void);
void CPU_Push16(Bitu value);
void CPU_Push32(Bitu value);

void CPU_SetFlags(Bitu word,Bitu mask);

// *********************************************************************
// Descriptor
// *********************************************************************

#define CR0_PROTECTION		0x00000001
#define CR0_FPUENABLED		0x00000002
#define CR0_FPUMONITOR		0x00000004
#define CR0_TASKSWITCH		0x00000008
#define CR0_FPUPRESENT		0x00000010
#define CR0_PAGING			0x80000000


#define DESC_INVALID				0x00
#define DESC_286_TSS_A				0x01
#define DESC_LDT					0x02
#define DESC_286_TSS_B				0x03
#define DESC_286_CALL_GATE			0x04
#define DESC_TASK_GATE				0x05
#define DESC_286_INT_GATE			0x06
#define DESC_286_TRAP_GATE			0x07

#define DESC_386_TSS_A				0x09
#define DESC_386_TSS_B				0x0b
#define DESC_386_CALL_GATE			0x0c
#define DESC_386_INT_GATE			0x0e
#define DESC_386_TRAP_GATE			0x0f

/* EU/ED Expand UP/DOWN RO/RW Read Only/Read Write NA/A Accessed */
#define DESC_DATA_EU_RO_NA			0x10
#define DESC_DATA_EU_RO_A			0x11
#define DESC_DATA_EU_RW_NA			0x12
#define DESC_DATA_EU_RW_A			0x13
#define DESC_DATA_ED_RO_NA			0x14
#define DESC_DATA_ED_RO_A			0x15
#define DESC_DATA_ED_RW_NA			0x16
#define DESC_DATA_ED_RW_A			0x17

/* N/R Readable  NC/C Confirming A/NA Accessed */
#define DESC_CODE_N_NC_A			0x18
#define DESC_CODE_N_NC_NA			0x19
#define DESC_CODE_R_NC_A			0x1a
#define DESC_CODE_R_NC_NA			0x1b
#define DESC_CODE_N_C_A				0x1c
#define DESC_CODE_N_C_NA			0x1d
#define DESC_CODE_R_C_A				0x1e
#define DESC_CODE_R_C_NA			0x1f

#ifdef _MSC_VER
#pragma pack (1)
#endif

struct S_Descriptor {
#ifdef WORDS_BIGENDIAN
	Bit32u base_0_15	:16;
	Bit32u limit_0_15	:16;
	Bit32u base_24_31	:8;
	Bit32u g			:1;
	Bit32u big			:1;
	Bit32u r			:1;
	Bit32u avl			:1;
	Bit32u limit_16_19	:4;
	Bit32u p			:1;
	Bit32u dpl			:2;
	Bit32u type			:5;
	Bit32u base_16_23	:8;
#else
	Bit32u limit_0_15	:16;
	Bit32u base_0_15	:16;
	Bit32u base_16_23	:8;
	Bit32u type			:5;
	Bit32u dpl			:2;
	Bit32u p			:1;
	Bit32u limit_16_19	:4;
	Bit32u avl			:1;
	Bit32u r			:1;
	Bit32u big			:1;
	Bit32u g			:1;
	Bit32u base_24_31	:8;
#endif
}GCC_ATTRIBUTE(packed);

struct G_Descriptor {
#ifdef WORDS_BIGENDIAN
	Bit32u selector:	16;
	Bit32u offset_0_15	:16;
	Bit32u offset_16_31	:16;
	Bit32u p			:1;
	Bit32u dpl			:2;
	Bit32u type			:5;
	Bit32u reserved		:3;
	Bit32u paramcount	:5;
#else
	Bit32u offset_0_15	:16;
	Bit32u selector		:16;
	Bit32u paramcount	:5;
	Bit32u reserved		:3;
	Bit32u type			:5;
	Bit32u dpl			:2;
	Bit32u p			:1;
	Bit32u offset_16_31	:16;
#endif
} GCC_ATTRIBUTE(packed);

struct TSS_16 {	
    Bit16u back;                 /* Back link to other task */
    Bit16u sp0;				     /* The CK stack pointer */
    Bit16u ss0;					 /* The CK stack selector */
	Bit16u sp1;                  /* The parent KL stack pointer */
    Bit16u ss1;                  /* The parent KL stack selector */
	Bit16u sp2;                  /* Unused */
    Bit16u ss2;                  /* Unused */
    Bit16u ip;                   /* The instruction pointer */
    Bit16u flags;                /* The flags */
    Bit16u ax, cx, dx, bx;       /* The general purpose registers */
    Bit16u sp, bp, si, di;       /* The special purpose registers */
    Bit16u es;                   /* The extra selector */
    Bit16u cs;                   /* The code selector */
    Bit16u ss;                   /* The application stack selector */
    Bit16u ds;                   /* The data selector */
    Bit16u ldt;                  /* The local descriptor table */
} GCC_ATTRIBUTE(packed);

struct TSS_32 {	
    Bit32u back;                /* Back link to other task */
	Bit32u esp0;		         /* The CK stack pointer */
    Bit32u ss0;					 /* The CK stack selector */
	Bit32u esp1;                 /* The parent KL stack pointer */
    Bit32u ss1;                  /* The parent KL stack selector */
	Bit32u esp2;                 /* Unused */
    Bit32u ss2;                  /* Unused */
	Bit32u cr3;                  /* The page directory pointer */
    Bit32u eip;                  /* The instruction pointer */
    Bit32u eflags;               /* The flags */
    Bit32u eax, ecx, edx, ebx;   /* The general purpose registers */
    Bit32u esp, ebp, esi, edi;   /* The special purpose registers */
    Bit32u es;                   /* The extra selector */
    Bit32u cs;                   /* The code selector */
    Bit32u ss;                   /* The application stack selector */
    Bit32u ds;                   /* The data selector */
    Bit32u fs;                   /* And another extra selector */
    Bit32u gs;                   /* ... and another one */
    Bit32u ldt;                  /* The local descriptor table */
} GCC_ATTRIBUTE(packed);

#ifdef _MSC_VER
#pragma pack()
#endif
class Descriptor
{
public:
	Descriptor() { saved.fill[0]=saved.fill[1]=0; }

	void Load(PhysPt address) {
		Bit32u* data = (Bit32u*)&saved;
		*data	  = mem_readd(address);
		*(data+1) = mem_readd(address+4);
	}
	void Save(PhysPt address) {
		Bit32u* data = (Bit32u*)&saved;
		mem_writed(address,*data);
		mem_writed(address+4,*(data+1));
	}
	PhysPt GetBase (void) { 
		return (saved.seg.base_24_31<<24) | (saved.seg.base_16_23<<16) | saved.seg.base_0_15; 
	}
	Bitu GetLimit (void) {
		Bitu limit = (saved.seg.limit_16_19<<16) | saved.seg.limit_0_15;
		if (saved.seg.g)	return (limit<<12) | 0xFFF;
		return limit;
	}
	Bitu GetOffset(void) {
		return (saved.gate.offset_16_31 << 16) | saved.gate.offset_0_15;
	}
	Bitu GetSelector(void) {
		return saved.gate.selector;
	}
	Bitu Type(void) {
		return saved.seg.type;
	}
	Bitu Conforming(void) {
		return saved.seg.type & 8;
	}
	Bitu DPL(void) {
		return saved.seg.dpl;
	}
	Bitu Big(void) {
		return saved.seg.big;
	}
public:
	union {
		S_Descriptor seg;
		G_Descriptor gate;
		Bit32u fill[2];
	} saved;
};

class DescriptorTable {
public:
	PhysPt	GetBase			(void)			{ return table_base;	}
	Bitu	GetLimit		(void)			{ return table_limit;	}
	void	SetBase			(PhysPt _base)	{ table_base = _base;	}
	void	SetLimit		(Bitu _limit)	{ table_limit= _limit;	}

	bool GetDescriptor	(Bitu selector, Descriptor& desc) {
		selector&=~7;
		if (selector>=table_limit) return false;
		desc.Load(table_base+(selector));
		return true;
	}
protected:
	PhysPt table_base;
	Bitu table_limit;
};

class GDTDescriptorTable : public DescriptorTable {
public:
	bool GetDescriptor(Bitu selector, Descriptor& desc) {
		Bitu address=selector & ~7;
		if (selector & 4) {
			if (address>=ldt_limit) return false;
			desc.Load(ldt_base+address);
			return true;
		} else {
			if (address>=table_limit) return false;
			desc.Load(table_base+address);
			return true;
		}
	}
	bool SetDescriptor(Bitu selector, Descriptor& desc) {
		Bitu address=selector & ~7;
		if (selector & 4) {
			if (address>=ldt_limit) return false;
			desc.Save(ldt_base+address);
			return true;
		} else {
			if (address>=table_limit) return false;
			desc.Save(table_base+address);
			return true;
		}
	} 
	Bitu SLDT(void)	{
		return ldt_value;
	}
	bool LLDT(Bitu value)	{
//TODO checking
		Descriptor desc;
		GetDescriptor(value,desc);
		ldt_base=desc.GetBase();
		ldt_limit=desc.GetLimit();
		ldt_value=value;
		return true;
	}
private:
	PhysPt ldt_base;
	Bitu ldt_limit;
	Bitu ldt_value;
};

class TSS_Descriptor : public Descriptor {
public:
	Bitu IsBusy(void) {
		return saved.seg.type & 2;
	}
	Bitu Is386(void) {
		return saved.seg.type & 8;
	}
	void SetBusy(bool busy) {
		if (busy) saved.seg.type|=2;
		else saved.seg.type&=~2;
	}
};


struct CPUBlock {
	Bitu cpl;							/* Current Privilege */
	Bitu cr0;
	bool pmode;							/* Is Protected mode enabled */
	GDTDescriptorTable gdt;
	DescriptorTable idt;
	struct {
		Bitu mask;
		bool big;
	} stack;
	struct {
		bool big;
	} code;
	struct {
		Bitu cs,eip;
		CPU_Decoder * old_decoder;
	} hlt;
	struct {
		Bitu which,error;
	} exception;
	Bits direction;
};

extern CPUBlock cpu;

INLINE void CPU_SetFlagsd(Bitu word) {
	Bitu mask=cpu.cpl ? FMASK_NORMAL : FMASK_ALL;
	CPU_SetFlags(word,mask);
};

INLINE void CPU_SetFlagsw(Bitu word) {
	Bitu mask=(cpu.cpl ? FMASK_NORMAL : FMASK_ALL) & 0xffff;
	CPU_SetFlags(word,mask);
};

#endif

