 /*
 *  Copyright (C) 2002-2003  The DOSBox Team
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

#ifndef VGA_H_
#define VGA_H_

#include <mem.h>
#include "dosbox.h"

enum VGAModes {
	M_TEXT16,
	M_CGA2,M_CGA4,
	M_TANDY16,
	M_EGA2,M_EGA4,M_EGA16,
	M_VGA,
	M_LIN8,
	M_ERROR,
};

#define CLK_25 25175
#define CLK_28 28322

#define MIN_VCO	180000
#define MAX_VCO 360000

#define S3_CLOCK_REF	14318	/* KHz */
#define S3_CLOCK(_M,_N,_R)	((S3_CLOCK_REF * ((_M) + 2)) / (((_N) + 2) * (1 << (_R))))
#define S3_MAX_CLOCK	150000	/* KHz */

/* Different functions that should be handle by memory handler */

#define MH_ROTATEOP		0x0001;
#define MH_SETRESET		0x0002;
#define MH_BITMASK		0x0004;


typedef Bit8u VGA_ReadHandler(PhysPt off);
typedef void VGA_WriteHandler(PhysPt off,Bit8u val);

typedef struct {
	bool attrindex;
} VGA_Internal;

typedef struct {
/* Memory handlers */
	Bitu mh_mask;

/* Video drawing */
	Bitu display_start;
	Bitu real_start;
	bool retrace;					/* A retrace is active */
	Bitu scan_len;
	Bitu cursor_start;

/* Some other screen related variables */
	Bitu line_compare;

	bool chained;					/* Enable or Disabled Chain 4 Mode */
	bool blinking;					/* Attribute bit 7 is blinking */

	bool vline_double;
	Bit8u vline_height;

	/* Pixel Scrolling */
	Bit8u pel_panning;				/* Amount of pixels to skip when starting horizontal line */
	Bit8u hlines_skip;
	Bit8u bytes_skip;

/* Specific stuff memory write/read handling */

	VGA_ReadHandler * readhandler;
	VGA_WriteHandler * writehandler;
	
	Bit8u read_mode;
	Bit8u write_mode;
	Bit8u read_map_select;
	Bit8u color_dont_care;
	Bit8u color_compare;
	Bit8u data_rotate;
	Bit8u raster_op;

	Bit32u full_bit_mask;
	Bit32u full_map_mask;
	Bit32u full_not_map_mask;
	Bit32u full_set_reset;
	Bit32u full_not_enable_set_reset;
	Bit32u full_enable_set_reset;
	Bit32u full_enable_and_set_reset;

} VGA_Config;

typedef struct {
	bool resizing;
	Bitu width;
	Bitu height;
	Bitu pitch;
	Bitu blank;
	bool double_width;
	bool double_height;
	Bitu lines;
	Bit8u font_height;
	Bit8u font[64*1024];
	Bitu font1_start;
	Bitu font2_start;
	Bitu rows,cols;
	struct {
		Bit8u sline,eline;
		Bit8u count,delay;
		Bit8u enabled;
	} cursor;
} VGA_Draw;

typedef struct {
	Bit8u bank;
	Bit8u reg_lock1;
	Bit8u reg_lock2;
	Bit8u reg_31;
	Bit8u reg_35;
	Bit8u reg_43;
	Bit8u reg_58;
	Bit8u reg_51;
	Bit8u reg_55;
	Bit8u ex_hor_overflow;
	Bit8u ex_ver_overflow;
	Bit16u la_window;
	struct {
		Bit8u r;
		Bit8u n;
		Bit8u m;
	} clk[4],mclk;
	struct {
		Bit8u lock;
		Bit8u cmd;
	} pll;
} VGA_S3;

typedef struct {
	Bit8u color_select;
} VGA_CGA;

typedef struct {
	Bit8u mem_bank;
	Bit8u disp_bank;
	Bit8u reg_index;
} VGA_TANDY;

typedef struct {
	Bit8u index;

	Bit8u reset;
	Bit8u clocking_mode;
	Bit8u map_mask;
	Bit8u character_map_select;
	Bit8u memory_mode;
} VGA_Seq;

typedef struct {
	Bit8u palette[16];
	Bit8u mode_control;
	Bit8u horizontal_pel_panning;
	Bit8u overscan_color;
	Bit8u color_plane_enable;
	Bit8u color_select;
	Bit8u index;
	Bit8u enabled;
} VGA_Attr;

typedef struct {
	Bit8u horizontal_total;
	Bit8u horizontal_display_end;
	Bit8u start_horizontal_blanking;
	Bit8u end_horizontal_blanking;
	Bit8u start_horizontal_retrace;
	Bit8u end_horizontal_retrace;
	Bit8u vertical_total;
	Bit8u overflow;
	Bit8u preset_row_scan;
	Bit8u maximum_scan_line;
	Bit8u cursor_start;
	Bit8u cursor_end;
	Bit8u start_address_high;
	Bit8u start_address_low;
	Bit8u cursor_location_high;
	Bit8u cursor_location_low;
	Bit8u vertical_retrace_start;
	Bit8u vertical_retrace_end;
	Bit8u vertical_display_end;
	Bit8u offset;
	Bit8u underline_location;
	Bit8u start_vertical_blanking;
	Bit8u end_vertical_blanking;
	Bit8u mode_control;
	Bit8u line_compare;

	Bit8u index;
	bool read_only;
} VGA_Crtc;

typedef struct {
	Bit8u index;
	Bit8u set_reset;
	Bit8u enable_set_reset;
	Bit8u color_compare;
	Bit8u data_rotate;
	Bit8u read_map_select;
	Bit8u mode;
	Bit8u miscellaneous;
	Bit8u color_dont_care;
	Bit8u bit_mask;
} VGA_Gfx;

struct RGBEntry {
	Bit8u red;
	Bit8u green;
	Bit8u blue;
};

typedef struct {
	Bit8u bits;						/* DAC bits, usually 6 or 8 */
	Bit8u pel_mask;
	Bit8u pel_index;	
	Bit8u state;
	Bit8u write_index;
	Bit8u read_index;
	Bitu first_changed;
	RGBEntry rgb[0x100];
	Bit8u attr[16];
} VGA_Dac;

union VGA_Latch {
	Bit32u d;
	Bit8u b[4];
};

union VGA_Memory {
	Bit8u linear[512*1024*4];
	Bit8u paged[512*1024][4];
	VGA_Latch latched[512*1024];
};	

typedef struct {
	VGAModes mode;								/* The mode the vga system is in */
	Bit8u misc_output;
	VGA_Draw draw;
	VGA_Config config;
	VGA_Internal internal;
/* Internal module groups */
	VGA_Seq seq;
	VGA_Attr attr;
	VGA_Crtc crtc;
	VGA_Gfx gfx;
	VGA_Dac dac;
	VGA_Latch latch;
	VGA_S3 s3;
	VGA_CGA cga;
	VGA_TANDY tandy;
	VGA_Memory mem;
} VGA_Type;



/* Functions for different resolutions */
void VGA_SetMode(VGAModes mode);
void VGA_SetupHandlers(void);
void VGA_StartResize(void);
void VGA_SetupDrawing(void);

/* Some DAC/Attribute functions */
void VGA_DAC_CombineColor(Bit8u attr,Bit8u pal);
void VGA_ATTR_SetPalette(Bit8u index,Bit8u val);

/* The VGA Subfunction startups */
void VGA_SetupAttr(void);
void VGA_SetupMemory(void);
void VGA_SetupDAC(void);
void VGA_SetupCRTC(void);
void VGA_SetupMisc(void);
void VGA_SetupGFX(void);
void VGA_SetupSEQ(void);

/* Some Support Functions */
void VGA_SetClock(Bitu which,Bitu target);
void VGA_DACSetEntirePalette(void);
void VGA_StartRetrace(void);
void VGA_StartUpdateLFB(void);

extern VGA_Type vga;

extern Bit32u ExpandTable[256];
extern Bit32u FillTable[16];
extern Bit32u CGA_4_Table[256];
extern Bit32u Expand16Table[4][16];
extern Bit32u Expand16BigTable[0x10000];


#endif

