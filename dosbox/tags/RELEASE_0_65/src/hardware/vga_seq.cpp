/*
 *  Copyright (C) 2002-2006  The DOSBox Team
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

#include "dosbox.h"
#include "inout.h"
#include "vga.h"

#define seq(blah) vga.seq.blah

Bitu read_p3c4(Bitu port,Bitu iolen) {
	return seq(index);
}

void write_p3c4(Bitu port,Bitu val,Bitu iolen) {
	seq(index)=val;
};

void write_p3c5(Bitu port,Bitu val,Bitu iolen) {
	if (seq(index)>0x8 && vga.s3.pll.lock!=0x6) return;
//	LOG_MSG("SEQ WRITE reg %X val %X",seq(index),val);
	switch(seq(index)) {
	case 0:		/* Reset */
		seq(reset)=val;
		break;
	case 1:		/* Clocking Mode */
		if (val!=seq(clocking_mode)) {
			seq(clocking_mode)=val;
			VGA_StartResize();
		}
		/* TODO Figure this out :)
			0	If set character clocks are 8 dots wide, else 9.
			2	If set loads video serializers every other character
				clock cycle, else every one.
			3	If set the Dot Clock is Master Clock/2, else same as Master Clock
				(See 3C2h bit 2-3). (Doubles pixels). Note: on some SVGA chipsets
				this bit also affects the Sequencer mode.
			4	If set loads video serializers every fourth character clock cycle,
				else every one.
			5	if set turns off screen and gives all memory cycles to the CPU
				interface.
		*/
		break;
	case 2:		/* Map Mask */
		seq(map_mask)=val & 15;
		vga.config.full_map_mask=FillTable[val & 15];
		vga.config.full_not_map_mask=~vga.config.full_map_mask;
		/*
			0  Enable writes to plane 0 if set
			1  Enable writes to plane 1 if set
			2  Enable writes to plane 2 if set
			3  Enable writes to plane 3 if set
		*/
		break;
	case 3:		/* Character Map Select */
		{
			seq(character_map_select)=val;
			Bit8u font1=((val & 0x3) << 1) | ((val & 0x10) >> 4);
			vga.draw.font_tables[0]=&vga.draw.font[font1*8*1024];
			Bit8u font2=((val & 0xc) >> 1) | ((val & 0x20) >> 5);
			vga.draw.font_tables[1]=&vga.draw.font[font2*8*1024];
		}
		/*
			0,1,4  Selects VGA Character Map (0..7) if bit 3 of the character
					attribute is clear.
			2,3,5  Selects VGA Character Map (0..7) if bit 3 of the character
					attribute is set.
			Note: Character Maps are placed as follows:
			Map 0 at 0k, 1 at 16k, 2 at 32k, 3: 48k, 4: 8k, 5: 24k, 6: 40k, 7: 56k
		*/
		break;
	case 4:	/* Memory Mode */
		/* 
			0  Set if in an alphanumeric mode, clear in graphics modes.
			1  Set if more than 64kbytes on the adapter.
			2  Enables Odd/Even addressing mode if set. Odd/Even mode places all odd
				bytes in plane 1&3, and all even bytes in plane 0&2.
			3  If set address bit 0-1 selects video memory planes (256 color mode),
				rather than the Map Mask and Read Map Select Registers.
		*/
		seq(memory_mode)=val;
		/* Changing this means changing the VGA Memory Read/Write Handler */
		if (val&0x08) vga.config.chained=true;
		else vga.config.chained=false;
		VGA_SetupHandlers();
		break;
	default:
		switch (svgaCard) {
		case SVGA_S3Trio:
			SVGA_S3_WriteSEQ( seq(index), val, iolen );
			break;
		default:
			LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:SEQ:Write to illegal index %2X",seq(index));
		}
	}
}


Bitu read_p3c5(Bitu port,Bitu iolen) {
//	LOG_MSG("VGA:SEQ:Read from index %2X",seq(index));
	if (seq(index)>0x8 && vga.s3.pll.lock!=0x6) return seq(index);
	switch(seq(index)) {
	case 0:			/* Reset */
		return seq(reset);
		break;
	case 1:			/* Clocking Mode */
		return seq(clocking_mode);
		break;
	case 2:			/* Map Mask */
		return seq(map_mask);
		break;
	case 3:			/* Character Map Select */
		return seq(character_map_select);
		break;
	case 4:			/* Memory Mode */
		return seq(memory_mode);
	default:
		switch (svgaCard) {
		case SVGA_S3Trio:
			return SVGA_S3_ReadSEQ( seq(index), iolen );
		}
	}
	return 0;
}


void VGA_SetupSEQ(void) {
	if (machine==MCH_VGA) {
		IO_RegisterWriteHandler(0x3c4,write_p3c4,IO_MB);
		IO_RegisterWriteHandler(0x3c5,write_p3c5,IO_MB);
		IO_RegisterReadHandler(0x3c4,read_p3c4,IO_MB);
		IO_RegisterReadHandler(0x3c5,read_p3c5,IO_MB);
	}
}

