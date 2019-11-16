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



	
#include <stdlib.h>
#include <string.h>
#include "dosbox.h"
#include "mem.h"
#include "vga.h"


Bit8u VGA_ChainedReadHandler(Bit32u start) {
	return vga.mem.linear[start];
}

void VGA_ChainedWriteHandler(Bit32u start,Bit8u val) {
	 vga.mem.linear[start]=val;
};


Bit8u VGA_NormalReadHandler(PhysPt start) {
	start-=0xa0000;
	vga.latch.d=vga.mem.latched[start].d;
	switch (vga.config.read_mode) {
	case 0:
		return(vga.latch.b[vga.config.read_map_select]);
	case 1:
		VGA_Latch templatch;
		templatch.d=(vga.latch.d &	FillTable[vga.config.color_dont_care]) ^ FillTable[vga.config.color_compare & vga.config.color_dont_care];
		return ~(templatch.b[0] | templatch.b[1] | templatch.b[2] | templatch.b[3]);
	}
	return 0;
}

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
		LOG_ERROR("VGA:Unsupported write mode %d",vga.config.write_mode);
	}
	return full;
}

Bit8u VGA_GFX_4_ReadHandler(Bit32u start) {
	return vga.mem.linear[start];
}

void VGA_GFX_4_WriteHandler(Bit32u start,Bit8u val) {
	start-=0xa0000;
	vga.mem.linear[start]=val;
	Bitu line=start / 320;
	Bitu x=start % 320;
	Bit8u * draw=&vga.buffer[start<<2];
	/* TODO Could also use a Bit32u lookup table for this */
	*(draw+0)=(val>>6)&3;
	*(draw+1)=(val>>4)&3;
	*(draw+2)=(val>>2)&3;
	*(draw+3)=(val)&3;
}

void VGA_GFX_16_WriteHandler(Bit32u start,Bit8u val) {
	start-=0xa0000;
	Bit32u data=ModeOperation(val);
	/* Update video memory and the pixel buffer */
	VGA_Latch pixels;
	pixels.d=vga.mem.latched[start].d;
	pixels.d&=vga.config.full_not_map_mask;
	pixels.d|=(data & vga.config.full_map_mask);
	vga.mem.latched[start].d=pixels.d;
	Bit8u * write_pixels=&vga.buffer[start<<3];

	Bit32u colors0_3, colors4_7;
	VGA_Latch temp;temp.d=(pixels.d>>4) & 0x0f0f0f0f;
		colors0_3 = 
		Expand16Table[0][temp.b[0]] |
		Expand16Table[1][temp.b[1]] |
		Expand16Table[2][temp.b[2]] |
		Expand16Table[3][temp.b[3]];
	*(Bit32u *)write_pixels=colors0_3;
	*(Bit32u *)(write_pixels+512*1024)=colors0_3;
	temp.d=pixels.d & 0x0f0f0f0f;
	colors4_7 = 
		Expand16Table[0][temp.b[0]] |
		Expand16Table[1][temp.b[1]] |
		Expand16Table[2][temp.b[2]] |
		Expand16Table[3][temp.b[3]];
	*(Bit32u *)(write_pixels+4)=colors4_7;
	*(Bit32u *)(write_pixels+512*1024+4)=colors4_7;

}

void VGA_GFX_256U_WriteHandler(Bit32u start,Bit8u val) {
	start-=0xa0000;
	Bit32u data=ModeOperation(val);
	VGA_Latch pixels;
	pixels.d=vga.mem.latched[start].d;
	pixels.d&=vga.config.full_not_map_mask;
	pixels.d|=(data & vga.config.full_map_mask);
	vga.mem.latched[start].d=pixels.d;
	vga.mem.latched[start+64*1024].d=pixels.d;
};
















void VGA_SetupMemory() {
	memset((void *)&vga.mem,0,256*1024);
	/* Alocate Video Memory */
	/* Not needed for VGA memory it gets allocated together with emulator maybe
	   later for VESA memory */

}
