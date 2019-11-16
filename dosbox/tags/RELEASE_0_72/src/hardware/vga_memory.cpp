/*
 *  Copyright (C) 2002-2007  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
	
#include <stdlib.h>
#include <string.h>
#include "dosbox.h"
#include "mem.h"
#include "vga.h"
#include "paging.h"
#include "pic.h"
#include "inout.h"

#ifdef VGA_KEEP_CHANGES
#define MEM_CHANGED( _MEM ) vga.changes.map[ (_MEM) >> VGA_CHANGE_SHIFT ] |= vga.changes.writeMask;
//#define MEM_CHANGED( _MEM ) vga.changes.map[ (_MEM) >> VGA_CHANGE_SHIFT ] = 1;
#else
#define MEM_CHANGED( _MEM ) 
#endif

#define TANDY_VIDBASE(_X_)  &MemBase[ 0x80000 + (_X_)]

template <class Size>
static INLINE void hostWrite(HostPt off, Bitu val) {
	if ( sizeof( Size ) == 1)
		host_writeb( off, (Bit8u)val );
	else if ( sizeof( Size ) == 2)
		host_writew( off, (Bit16u)val );
	else if ( sizeof( Size ) == 4)
		host_writed( off, (Bit32u)val );
}

template <class Size>
static INLINE Bitu  hostRead(HostPt off ) {
	if ( sizeof( Size ) == 1)
		return host_readb( off );
	else if ( sizeof( Size ) == 2)
		return host_readw( off );
	else if ( sizeof( Size ) == 4)
		return host_readd( off );
	return 0;
}


void VGA_MapMMIO(void);
//Nice one from DosEmu
INLINE static Bit32u RasterOp(Bit32u input,Bit32u mask) {
	switch (vga.config.raster_op) {
	case 0x00:	/* None */
		return (input & mask) | (vga.latch.d & ~mask);
	case 0x01:	/* AND */
		return (input | ~mask) & vga.latch.d;
	case 0x02:	/* OR */
		return (input & mask) | vga.latch.d;
	case 0x03:	/* XOR */
		return (input & mask) ^ vga.latch.d;
	};
	return 0;
}

INLINE static Bit32u ModeOperation(Bit8u val) {
	Bit32u full;
	switch (vga.config.write_mode) {
	case 0x00:
		// Write Mode 0: In this mode, the host data is first rotated as per the Rotate Count field, then the Enable Set/Reset mechanism selects data from this or the Set/Reset field. Then the selected Logical Operation is performed on the resulting data and the data in the latch register. Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register. Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory. 
		val=((val >> vga.config.data_rotate) | (val << (8-vga.config.data_rotate)));
		full=ExpandTable[val];
		full=(full & vga.config.full_not_enable_set_reset) | vga.config.full_enable_and_set_reset; 
		full=RasterOp(full,vga.config.full_bit_mask);
		break;
	case 0x01:
		// Write Mode 1: In this mode, data is transferred directly from the 32 bit latch register to display memory, affected only by the Memory Plane Write Enable field. The host data is not used in this mode. 
		full=vga.latch.d;
		break;
	case 0x02:
		//Write Mode 2: In this mode, the bits 3-0 of the host data are replicated across all 8 bits of their respective planes. Then the selected Logical Operation is performed on the resulting data and the data in the latch register. Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register. Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory. 
		full=RasterOp(FillTable[val&0xF],vga.config.full_bit_mask);
		break;
	case 0x03:
		// Write Mode 3: In this mode, the data in the Set/Reset field is used as if the Enable Set/Reset field were set to 1111b. Then the host data is first rotated as per the Rotate Count field, then logical ANDed with the value of the Bit Mask field. The resulting value is used on the data obtained from the Set/Reset field in the same way that the Bit Mask field would ordinarily be used. to select which bits come from the expansion of the Set/Reset field and which come from the latch register. Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
		full=RasterOp(vga.config.full_set_reset,ExpandTable[val] & vga.config.full_bit_mask);
		break;
	default:
		LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:Unsupported write mode %d",vga.config.write_mode);
		full=0;
		break;
	}
	return full;
}

/* Gonna assume that whoever maps vga memory, maps it on 32/64kb boundary */

#define VGA_PAGES		(128/4)
#define VGA_PAGE_A0		(0xA0000/4096)
#define VGA_PAGE_B0		(0xB0000/4096)
#define VGA_PAGE_B8		(0xB8000/4096)

static struct {
	Bitu base, mask;
} vgapages;
	
class VGA_UnchainedRead_Handler : public PageHandler {
public:
	Bitu readHandler(PhysPt start) {
		vga.latch.d=((Bit32u*)vga.mem.linear)[start];
		switch (vga.config.read_mode) {
		case 0:
			return (vga.latch.b[vga.config.read_map_select]);
		case 1:
			VGA_Latch templatch;
			templatch.d=(vga.latch.d &	FillTable[vga.config.color_dont_care]) ^ FillTable[vga.config.color_compare & vga.config.color_dont_care];
			return (Bit8u)~(templatch.b[0] | templatch.b[1] | templatch.b[2] | templatch.b[3]);
		}
		return 0;
	}
public:
	Bitu readb(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return readHandler(addr);
	}
	Bitu readw(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return 
			(readHandler(addr+0) << 0) |
			(readHandler(addr+1) << 8);
	}
	Bitu readd(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return 
			(readHandler(addr+0) << 0)  |
			(readHandler(addr+1) << 8)  |
			(readHandler(addr+2) << 16) |
			(readHandler(addr+3) << 24);
	}
};

class VGA_ChainedEGA_Handler : public PageHandler {
public:
	Bitu readHandler(PhysPt addr) {
		return vga.mem.linear[addr];
	}
	void writeHandler(PhysPt start, Bit8u val) {
		Bit32u data=ModeOperation(val);
		/* Update video memory and the pixel buffer */
		VGA_Latch pixels;
		vga.mem.linear[start] = val;
		start >>= 2;
		pixels.d=((Bit32u*)vga.mem.linear)[start];

		Bit8u * write_pixels=&vga.mem.linear[VGA_CACHE_OFFSET+(start<<3)];

		Bit32u colors0_3, colors4_7;
		VGA_Latch temp;temp.d=(pixels.d>>4) & 0x0f0f0f0f;
		colors0_3 = 
			Expand16Table[0][temp.b[0]] |
			Expand16Table[1][temp.b[1]] |
			Expand16Table[2][temp.b[2]] |
			Expand16Table[3][temp.b[3]];
		*(Bit32u *)write_pixels=colors0_3;
		temp.d=pixels.d & 0x0f0f0f0f;
		colors4_7 = 
			Expand16Table[0][temp.b[0]] |
			Expand16Table[1][temp.b[1]] |
			Expand16Table[2][temp.b[2]] |
			Expand16Table[3][temp.b[3]];
		*(Bit32u *)(write_pixels+4)=colors4_7;
	}
public:	
	VGA_ChainedEGA_Handler()  {
		flags=PFLAG_NOCODE;
	}
	void writeb(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr << 3);
		writeHandler(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr << 3);
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr << 3);
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
		writeHandler(addr+2,(Bit8u)(val >> 16));
		writeHandler(addr+3,(Bit8u)(val >> 24));
	}
	Bitu readb(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return readHandler(addr);
	}
	Bitu readw(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return 
			(readHandler(addr+0) << 0) |
			(readHandler(addr+1) << 8);
	}
	Bitu readd(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return 
			(readHandler(addr+0) << 0)  |
			(readHandler(addr+1) << 8)  |
			(readHandler(addr+2) << 16) |
			(readHandler(addr+3) << 24);
	}
};

class VGA_UnchainedEGA_Handler : public VGA_UnchainedRead_Handler {
public:
	template< bool wrapping>
	void writeHandler(PhysPt start, Bit8u val) {
		Bit32u data=ModeOperation(val);
		/* Update video memory and the pixel buffer */
		VGA_Latch pixels;
		pixels.d=((Bit32u*)vga.mem.linear)[start];
		pixels.d&=vga.config.full_not_map_mask;
		pixels.d|=(data & vga.config.full_map_mask);
		((Bit32u*)vga.mem.linear)[start]=pixels.d;
		Bit8u * write_pixels=&vga.mem.linear[VGA_CACHE_OFFSET+(start<<3)];

		Bit32u colors0_3, colors4_7;
		VGA_Latch temp;temp.d=(pixels.d>>4) & 0x0f0f0f0f;
			colors0_3 = 
			Expand16Table[0][temp.b[0]] |
			Expand16Table[1][temp.b[1]] |
			Expand16Table[2][temp.b[2]] |
			Expand16Table[3][temp.b[3]];
		*(Bit32u *)write_pixels=colors0_3;
		temp.d=pixels.d & 0x0f0f0f0f;
		colors4_7 = 
			Expand16Table[0][temp.b[0]] |
			Expand16Table[1][temp.b[1]] |
			Expand16Table[2][temp.b[2]] |
			Expand16Table[3][temp.b[3]];
		*(Bit32u *)(write_pixels+4)=colors4_7;
		if (wrapping && GCC_UNLIKELY( start < 512)) {
			*(Bit32u *)(write_pixels+512*1024)=colors0_3;
			*(Bit32u *)(write_pixels+512*1024+4)=colors4_7;
		}
	}
public:	
	VGA_UnchainedEGA_Handler()  {
		flags=PFLAG_NOCODE;
	}
	void writeb(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr << 3);
		writeHandler<true>(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr << 3);
		writeHandler<true>(addr+0,(Bit8u)(val >> 0));
		writeHandler<true>(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr << 3);
		writeHandler<true>(addr+0,(Bit8u)(val >> 0));
		writeHandler<true>(addr+1,(Bit8u)(val >> 8));
		writeHandler<true>(addr+2,(Bit8u)(val >> 16));
		writeHandler<true>(addr+3,(Bit8u)(val >> 24));
	}
};

//Slighly unusual version, will directly write 8,16,32 bits values
class VGA_ChainedVGA_Handler : public PageHandler {
public:
	VGA_ChainedVGA_Handler()  {
		flags=PFLAG_NOCODE;
	}
	template <class Size>
	static INLINE Bitu readHandler(PhysPt addr ) {
		return hostRead<Size>( &vga.mem.linear[((addr&~3)<<2)+(addr&3)] );
	}
	template <class Size>
	static INLINE void writeCache(PhysPt addr, Bitu val) {
		hostWrite<Size>( &vga.mem.linear[VGA_CACHE_OFFSET+addr], val );
		if (GCC_UNLIKELY(addr < 320)) {
			// And replicate the first line
			hostWrite<Size>( &vga.mem.linear[VGA_CACHE_OFFSET+addr+64*1024], val );
		}
	}
	template <class Size>
	static INLINE void writeHandler(PhysPt addr, Bitu val) {
		// No need to check for compatible chains here, this one is only enabled if that bit is set
		hostWrite<Size>( &vga.mem.linear[((addr&~3)<<2)+(addr&3)], val );
	}
	Bitu readb(PhysPt addr ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return readHandler<Bit8u>( addr );
	}
	Bitu readw(PhysPt addr ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return readHandler<Bit16u>( addr );
	}
	Bitu readd(PhysPt addr ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		return readHandler<Bit32u>( addr );
	}
	bool readw_checked( PhysPt addr,Bitu *val ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if ( addr & 1) {
			*val =
				(readHandler<Bit8u>( addr+0 ) << 0 ) | 
				(readHandler<Bit8u>( addr+1 ) << 8 );
		} else {
			*val = readHandler<Bit16u>( addr ); 
		}
		return false;
	}
	bool readd_checked( PhysPt addr,Bitu *val ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if ( addr & 3) {
			*val =
				(readHandler<Bit8u>( addr+0 ) << 0 ) | 
				(readHandler<Bit8u>( addr+1 ) << 8 ) | 
				(readHandler<Bit8u>( addr+2 ) << 16 ) | 
				(readHandler<Bit8u>( addr+3 ) << 24 );
		} else {
			*val = readHandler<Bit32u>( addr ); 
		}
		return false;
	}
	void writeb(PhysPt addr, Bitu val ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr );
		writeHandler<Bit8u>( addr, val );
		writeCache<Bit8u>( addr, val );
	}
	void writew(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr );
		writeHandler<Bit16u>( addr, val );
		writeCache<Bit16u>( addr, val );
	}
	void writed(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		MEM_CHANGED( addr );
		writeHandler<Bit32u>( addr, val );
		writeCache<Bit32u>( addr, val );
	}
	bool writew_checked( PhysPt addr,Bitu val ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if ( addr & 1 ) {
			MEM_CHANGED( addr );
			MEM_CHANGED( addr + 1);
			writeHandler<Bit8u>( addr+0, val >> 0 );
			writeHandler<Bit8u>( addr+1, val >> 8 );
			writeCache<Bit16u>( addr, val );
		} else {
			MEM_CHANGED( addr );
			writeHandler<Bit16u>( addr, val );
			writeCache<Bit16u>( addr, val );
		}
		return false;
	}
	bool writed_checked( PhysPt addr,Bitu val ) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if ( addr & 3) {
			MEM_CHANGED( addr );
			MEM_CHANGED( addr + 3);
			writeHandler<Bit8u>( addr+0, val >> 0 );
			writeHandler<Bit8u>( addr+1, val >> 8 );
			writeHandler<Bit8u>( addr+2, val >> 16 );
			writeHandler<Bit8u>( addr+3, val >> 24 );
			writeCache<Bit32u>( addr, val );
		} else {
			MEM_CHANGED( addr );
			writeHandler<Bit32u>( addr, val );
			writeCache<Bit32u>( addr, val );
		}
		return false;
	}
};

class VGA_UnchainedVGA_Handler : public VGA_UnchainedRead_Handler {
public:
	void writeHandler( PhysPt addr, Bit8u val ) {
		Bit32u data=ModeOperation(val);
		VGA_Latch pixels;
		pixels.d=((Bit32u*)vga.mem.linear)[addr];
		pixels.d&=vga.config.full_not_map_mask;
		pixels.d|=(data & vga.config.full_map_mask);
		((Bit32u*)vga.mem.linear)[addr]=pixels.d;
		if(vga.config.compatible_chain4)
			((Bit32u*)vga.mem.linear)[addr+64*1024]=pixels.d; 
	}
public:
	VGA_UnchainedVGA_Handler()  {
		flags=PFLAG_NOCODE;
	}
	void writeb(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		MEM_CHANGED( addr << 2 );
		writeHandler(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		MEM_CHANGED( addr << 2);
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		MEM_CHANGED( addr << 2);
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
		writeHandler(addr+2,(Bit8u)(val >> 16));
		writeHandler(addr+3,(Bit8u)(val >> 24));
	}
};

class VGA_TEXT_PageHandler : public PageHandler {
public:
	VGA_TEXT_PageHandler() {
		flags=PFLAG_NOCODE;
	}
	Bitu readb(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		return vga.draw.font[addr];
	}
	void writeb(PhysPt addr,Bitu val){
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		if (vga.seq.map_mask & 0x4) {
			vga.draw.font[addr]=(Bit8u)val;
		}
	}
};

class VGA_Map_Handler : public PageHandler {
public:
	VGA_Map_Handler() {
		flags=PFLAG_READABLE|PFLAG_WRITEABLE|PFLAG_NOCODE;
	}
	HostPt GetHostReadPt(Bitu phys_page) {
 		phys_page-=vgapages.base;
		return &vga.mem.linear[vga.s3.svga_bank.fullbank+phys_page*4096];
	}
	HostPt GetHostWritePt(Bitu phys_page) {
 		phys_page-=vgapages.base;
		return &vga.mem.linear[vga.s3.svga_bank.fullbank+phys_page*4096];
	}
};

class VGA_Changes_Handler : public PageHandler {
public:
	VGA_Changes_Handler() {
		flags=PFLAG_NOCODE;
	}
	Bitu readb(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		return hostRead<Bit8u>( &vga.mem.linear[addr] );
	}
	Bitu readw(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		return hostRead<Bit16u>( &vga.mem.linear[addr] );
	}
	Bitu readd(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		return hostRead<Bit32u>( &vga.mem.linear[addr] );
	}
	void writeb(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		MEM_CHANGED( addr );
		hostWrite<Bit8u>( &vga.mem.linear[addr], val );
	}
	void writew(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		MEM_CHANGED( addr );
		hostWrite<Bit16u>( &vga.mem.linear[addr], val );
	}
	void writed(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) & 0xffff;
		addr += vga.s3.svga_bank.fullbank;
		MEM_CHANGED( addr );	
		hostWrite<Bit32u>( &vga.mem.linear[addr], val );
	}
};

class VGA_LIN4_Handler : public VGA_UnchainedEGA_Handler {
public:
	VGA_LIN4_Handler() {
		flags=PFLAG_NOCODE;
	}
	void writeb(PhysPt addr,Bitu val) {
		addr = vga.s3.svga_bank.fullbank + (PAGING_GetPhysicalAddress(addr) & 0xffff);
		addr &= (128*1024-1);
		MEM_CHANGED( addr << 3 );
		writeHandler<false>(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bitu val) {
		addr = vga.s3.svga_bank.fullbank + (PAGING_GetPhysicalAddress(addr) & 0xffff);
		addr &= (128*1024-1);
		MEM_CHANGED( addr << 3 );
		writeHandler<false>(addr+0,(Bit8u)(val >> 0));
		writeHandler<false>(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bitu val) {
		addr = vga.s3.svga_bank.fullbank + (PAGING_GetPhysicalAddress(addr) & 0xffff);
		addr &= (128*1024-1);
		MEM_CHANGED( addr << 3 );
		writeHandler<false>(addr+0,(Bit8u)(val >> 0));
		writeHandler<false>(addr+1,(Bit8u)(val >> 8));
		writeHandler<false>(addr+2,(Bit8u)(val >> 16));
		writeHandler<false>(addr+3,(Bit8u)(val >> 24));
	}
	Bitu readb(PhysPt addr) {
		addr = vga.s3.svga_bank.fullbank + (PAGING_GetPhysicalAddress(addr) & 0xffff);
		addr &= (128*1024-1);
		return readHandler(addr);
	}
	Bitu readw(PhysPt addr) {
		addr = vga.s3.svga_bank.fullbank + (PAGING_GetPhysicalAddress(addr) & 0xffff);
		addr &= (128*1024-1);
		return 
			(readHandler(addr+0) << 0) |
			(readHandler(addr+1) << 8);
	}
	Bitu readd(PhysPt addr) {
		addr = vga.s3.svga_bank.fullbank + (PAGING_GetPhysicalAddress(addr) & 0xffff);
		addr &= (128*1024-1);
		return 
			(readHandler(addr+0) << 0)  |
			(readHandler(addr+1) << 8)  |
			(readHandler(addr+2) << 16) |
			(readHandler(addr+3) << 24);
	}
};


class VGA_LFBChanges_Handler : public PageHandler {
public:
	VGA_LFBChanges_Handler() {
		flags=PFLAG_NOCODE;
	}
	Bitu readb(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		return hostRead<Bit8u>( &vga.mem.linear[addr] );
	}
	Bitu readw(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		return hostRead<Bit16u>( &vga.mem.linear[addr] );
	}
	Bitu readd(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		return hostRead<Bit32u>( &vga.mem.linear[addr] );
	}
	void writeb(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		hostWrite<Bit8u>( &vga.mem.linear[addr], val );
		MEM_CHANGED( addr );
	}
	void writew(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		hostWrite<Bit16u>( &vga.mem.linear[addr], val );
		MEM_CHANGED( addr );
	}
	void writed(PhysPt addr,Bitu val) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		hostWrite<Bit32u>( &vga.mem.linear[addr], val );
		MEM_CHANGED( addr );
	}
	bool writed_checked( PhysPt addr,Bitu val ) {
		addr = PAGING_GetPhysicalAddress(addr) - vga.lfb.addr;
		hostWrite<Bit32u>( &vga.mem.linear[addr], val );
		MEM_CHANGED( addr );
		return false;
	}

};

class VGA_LFB_Handler : public PageHandler {
public:
	VGA_LFB_Handler() {
		flags=PFLAG_READABLE|PFLAG_WRITEABLE|PFLAG_NOCODE;
	}
	HostPt GetHostReadPt( Bitu phys_page ) {
		phys_page -= vga.lfb.page;
		return &vga.mem.linear[phys_page * 4096];
	}
	HostPt GetHostWritePt( Bitu phys_page ) {
		return GetHostReadPt( phys_page );
	}
};

class VGA_MMIO_Handler : public PageHandler {
public:
	Bit16u regmem[16384];
	VGA_MMIO_Handler() {
		flags=PFLAG_NOCODE;
		//memset(&regmem[0], 0, sizeof(regmem));
	}
	void writeb(PhysPt addr,Bitu val) {
		Bitu port = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if(port >= 0x82E8) IO_WriteB(port, val);
		if(port <= 0x4000) {
			if(port == 0x0000) {
				IO_WriteB(0xe2e0, val);
			} else {
				IO_WriteB(0xe2e8, val);
			}
		}
		//LOG_MSG("MMIO: Write byte to %x with %x", addr, val);
	}
	void writew(PhysPt addr,Bitu val) {
		Bitu port = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if(port >= 0x82E8) IO_WriteW(port, val);
		if(port == 0x8118) IO_WriteW(0x9ae8, val);
		if(port <= 0x4000) {
			if(port == 0x0000) {
                IO_WriteW(0xe2e0, val);
			} else {
			IO_WriteW(0xe2e8, val);
		}
		}
		//LOG_MSG("MMIO: Write word to %x with %x", addr, val);	
	}
	void writed(PhysPt addr,Bitu val) {
		Bitu port = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if(port >= 0x82E8) IO_WriteD(port, val);
		if(port == 0x8100) {
			IO_WriteW(0x86e8, (val >> 16));
			IO_WriteW(0x82e8, (val & 0xffff));
		}
		if(port == 0x8148) {
			IO_WriteW(0x96e8, (val >> 16));
			IO_WriteW(0xbee8, (val & 0xffff));
		}
		if(port <= 0x4000) {
			if(port == 0x0000) {
				IO_WriteW(0xe2e0, (val & 0xffff));
				IO_WriteW(0xe2e8, (val >> 16));
			} else {
				IO_WriteW(0xe2e8, (val & 0xffff));
				IO_WriteW(0xe2e8, (val >> 16));
			}
		}

		//LOG_MSG("MMIO: Write dword to %x with %x", addr, val);
	}

	Bitu readb(PhysPt addr) {
		Bitu port = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if(port >= 0x82E8) return IO_ReadB(port);
		//LOG_MSG("MMIO: Read byte from %x", addr);
		return 0x00;
	}
	Bitu readw(PhysPt addr) {
		Bitu port = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if(port >= 0x82E8) return IO_ReadW(port);
		//LOG_MSG("MMIO: Read word from %x", addr);
		return 0x00;
	}
	Bitu readd(PhysPt addr) {
		Bitu port = PAGING_GetPhysicalAddress(addr) & 0xffff;
		if(port >= 0x82E8) return IO_ReadD(port);
		//LOG_MSG("MMIO: Read dword from %x", addr);
		return 0x00;
	}

};

class VGA_TANDY_PageHandler : public PageHandler {
public:
	VGA_TANDY_PageHandler() {
		flags=PFLAG_READABLE|PFLAG_WRITEABLE;
//			|PFLAG_NOCODE;
	}
	HostPt GetHostReadPt(Bitu phys_page) {
		if (vga.tandy.mem_bank & 1) 
			phys_page&=0x03;
		else 
			phys_page&=0x07;
		return vga.tandy.mem_base + (phys_page * 4096);
	}
	HostPt GetHostWritePt(Bitu phys_page) {
		return GetHostReadPt( phys_page );
	}
};


class VGA_PCJR_Handler : public PageHandler {
public:
	VGA_PCJR_Handler() {
		flags=PFLAG_READABLE|PFLAG_WRITEABLE;
	}
	HostPt GetHostReadPt(Bitu phys_page) {
		phys_page-=0xb8;
		//test for a unaliged bank, then replicate 2x16kb
		if (vga.tandy.mem_bank & 1) 
			phys_page&=0x03;
		return vga.tandy.mem_base + (phys_page * 4096);
	}
	HostPt GetHostWritePt(Bitu phys_page) {
		return GetHostReadPt( phys_page );
	}
};

class VGA_Empty_Handler : public PageHandler {
public:
	VGA_Empty_Handler() {
		flags=PFLAG_NOCODE;
	}
	Bitu readb(PhysPt addr) {
//		LOG(LOG_VGA, LOG_NORMAL ) ( "Read from empty memory space at %x", addr );
		return 0xff;
	} 
	void writeb(PhysPt addr,Bitu val) {
//		LOG(LOG_VGA, LOG_NORMAL ) ( "Write %x to empty memory space at %x", val, addr );
	}
};

static struct vg {
	VGA_Map_Handler				map;
	VGA_Changes_Handler			changes;
	VGA_TEXT_PageHandler		text;
	VGA_TANDY_PageHandler		tandy;
	VGA_ChainedEGA_Handler		cega;
	VGA_ChainedVGA_Handler		cvga;
	VGA_UnchainedEGA_Handler	uega;
	VGA_UnchainedVGA_Handler	uvga;
	VGA_PCJR_Handler			pcjr;
	VGA_LIN4_Handler			lin4;
	VGA_LFB_Handler				lfb;
	VGA_LFBChanges_Handler		lfbchanges;
	VGA_MMIO_Handler			mmio;
	VGA_Empty_Handler			empty;
} vgaph;

void VGA_ChangedBank(void) {
#ifndef VGA_LFB_MAPPED
	//If the mode is accurate than the correct mapper must have been installed already
	if ( vga.mode >= M_LIN4 && vga.mode <= M_LIN32 ) {
		return;
	}
#endif
	VGA_SetupHandlers();
}

void VGA_SetupHandlers(void) {
	PageHandler *newHandler;
	switch (machine) {
	case MCH_CGA:
	case MCH_PCJR:
		MEM_SetPageHandler( VGA_PAGE_B8, 8, &vgaph.pcjr );
		goto range_done;
	case MCH_HERC:
		vgapages.base=VGA_PAGE_B0;
		if (vga.herc.enable_bits & 0x2) {
			vgapages.mask=0xffff;
			MEM_SetPageHandler(VGA_PAGE_B0,16,&vgaph.map);
		} else {
			vgapages.mask=0x7fff;
			/* With hercules in 32kb mode it leaves a memory hole on 0xb800 */
			MEM_SetPageHandler(VGA_PAGE_B0,8,&vgaph.map);
			MEM_SetPageHandler(VGA_PAGE_B8,8,&vgaph.empty);
		}
		goto range_done;
	case MCH_TANDY:
		/* Always map 0xa000 - 0xbfff, might overwrite 0xb800 */
		vgapages.base=VGA_PAGE_A0;
		vgapages.mask=0x1ffff;
		MEM_SetPageHandler(VGA_PAGE_A0, 32, &vgaph.map );
		if ( vga.tandy.extended_ram & 1 ) {
			//You seem to be able to also map different 64kb banks, but have to figure that out
			//This seems to work so far though
			vga.tandy.draw_base = vga.mem.linear;
			vga.tandy.mem_base = vga.mem.linear;
		} else {
			vga.tandy.draw_base = TANDY_VIDBASE( vga.tandy.draw_bank * 16 * 1024);
			vga.tandy.mem_base = TANDY_VIDBASE( vga.tandy.mem_bank * 16 * 1024);
			MEM_SetPageHandler( 0xb8, 8, &vgaph.tandy );
		}
		goto range_done;
//		MEM_SetPageHandler(vga.tandy.mem_bank<<2,vga.tandy.is_32k_mode ? 0x08 : 0x04,range_handler);
	case MCH_VGA:
		break;
	default:
		LOG_MSG("Illegal machine type %d", machine );
		return;
	}

	/* This should be vga only */
	switch (vga.mode) {
	case M_ERROR:
		return;
	case M_LIN4:
		newHandler = &vgaph.lin4;
		break;	
	case M_LIN15:
	case M_LIN16:
	case M_LIN32:
#ifdef VGA_LFB_MAPPED
		newHandler = &vgaph.map;
#else
		newHandler = &vgaph.changes;
#endif
		break;
	case M_LIN8:
	case M_VGA:
		if (vga.config.chained) {
			if(vga.config.compatible_chain4)
				newHandler = &vgaph.cvga;
			else 
#ifdef VGA_LFB_MAPPED
				newHandler = &vgaph.map;
#else
				newHandler = &vgaph.changes;
#endif
		} else {
			newHandler = &vgaph.uvga;
		}
		break;
	case M_EGA:
		if (vga.config.chained) 
			newHandler = &vgaph.cega;
		else
			newHandler = &vgaph.uega;
		break;	
	case M_TEXT:
		/* Check if we're not in odd/even mode */
		if (vga.gfx.miscellaneous & 0x2) newHandler = &vgaph.map;
		else newHandler = &vgaph.text;
		break;
	case M_CGA4:
	case M_CGA2:
		newHandler = &vgaph.map;
		break;
	}
	switch ((vga.gfx.miscellaneous >> 2) & 3) {
	case 0:
		vgapages.base = VGA_PAGE_A0;
		vgapages.mask = 0x1ffff;
		MEM_SetPageHandler(VGA_PAGE_A0, 32, newHandler );
		break;
	case 1:
		vgapages.base = VGA_PAGE_A0;
		vgapages.mask = 0xffff;
		MEM_SetPageHandler( VGA_PAGE_A0, 16, newHandler );
		MEM_ResetPageHandler( VGA_PAGE_B0, 16);
		break;
	case 2:
		vgapages.base = VGA_PAGE_B0;
		vgapages.mask = 0x7fff;
		MEM_SetPageHandler( VGA_PAGE_B0, 8, newHandler );
		MEM_ResetPageHandler( VGA_PAGE_A0, 16 );
		MEM_ResetPageHandler( VGA_PAGE_B8, 8 );
		break;
	case 3:
		vgapages.base = VGA_PAGE_B8;
		vgapages.mask = 0x7fff;
		MEM_SetPageHandler( VGA_PAGE_B8, 8, newHandler );
		MEM_ResetPageHandler( VGA_PAGE_A0, 16 );
		MEM_ResetPageHandler( VGA_PAGE_B0, 8 );
		break;
	}
	if(((vga.s3.ext_mem_ctrl & 0x10) != 0x00) /*&& (vga.mode == M_LIN8)*/)
		MEM_SetPageHandler(VGA_PAGE_A0, 16, &vgaph.mmio);
range_done:
	PAGING_ClearTLB();
}

void VGA_StartUpdateLFB(void) {
	vga.lfb.page = vga.s3.la_window << 4;
	vga.lfb.addr = vga.s3.la_window << 16;
#ifdef VGA_LFB_MAPPED
	vga.lfb.handler = &vgaph.lfb;
#else
	vga.lfb.handler = &vgaph.lfbchanges;
#endif
	MEM_SetLFB(vga.s3.la_window << 4 ,sizeof(vga.mem.linear)/4096, vga.lfb.handler );
}

void VGA_MapMMIO(void) {
	MEM_SetPageHandler(VGA_PAGE_A0, 16, &vgaph.mmio);
}

void VGA_UnmapMMIO(void) {
	//MEM_SetPageHandler(VGA_PAGE_A0, &ram_page_handler);
}


void VGA_SetupMemory() {
	memset( &vga.mem, 0, VGA_MEMORY );
#ifdef VGA_KEEP_CHANGES
	memset( &vga.changes, 0, sizeof( vga.changes ));
#endif
	vga.s3.svga_bank.fullbank=0;
	if (machine==MCH_PCJR) {
		/* PCJr does not have dedicated graphics memory but uses
		   conventional memory below 128k */
		//TODO map?	
	} 
}
