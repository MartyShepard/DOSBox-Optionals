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

/* $Id: mouse.cpp,v 1.24 2003-09-24 19:35:01 qbix79 Exp $ */

#include <string.h>
#include "dosbox.h"
#include "callback.h"
#include "mem.h"
#include "regs.h"
#include "mouse.h"
#include "pic.h"
#include "inout.h"
#include "int10.h"
#include "bios.h"

static Bitu call_int33,call_int74;

// forward
void WriteMouseIntVector(void);

struct button_event {
	Bit16u type;
	Bit16u buttons;
};

#define QUEUE_SIZE 32
#define MOUSE_BUTTONS 3
#define MOUSE_IRQ 12
#define POS_X (Bit16s)(mouse.x)
#define POS_Y (Bit16s)(mouse.y)

#define CURSORX 16
#define CURSORY 16
#define HIGHESTBIT (1<<(CURSORX-1))

static Bit16u defaultTextAndMask = 0x77FF;
static Bit16u defaultTextXorMask = 0x7700;

static Bit16u defaultScreenMask[CURSORY] = {
		0x3FFF, 0x1FFF, 0x0FFF, 0x07FF,
		0x03FF, 0x01FF, 0x00FF, 0x007F,
		0x003F, 0x001F, 0x01FF, 0x00FF,
		0x30FF, 0xF87F, 0xF87F, 0xFCFF
};

static Bit16u defaultCursorMask[CURSORY] = {
		0x0000, 0x4000, 0x6000, 0x7000,
		0x7800, 0x7C00, 0x7E00, 0x7F00,
		0x7F80, 0x7C00, 0x6C00, 0x4600,
		0x0600, 0x0300, 0x0300, 0x0000
};

static Bit16u userdefScreenMask[CURSORY];
static Bit16u userdefCursorMask[CURSORY];

static struct {
	Bit16u buttons;
	Bit16u times_pressed[MOUSE_BUTTONS];
	Bit16u times_released[MOUSE_BUTTONS];
	Bit16u last_released_x[MOUSE_BUTTONS];
	Bit16u last_released_y[MOUSE_BUTTONS];
	Bit16u last_pressed_x[MOUSE_BUTTONS];
	Bit16u last_pressed_y[MOUSE_BUTTONS];
	Bit16s shown;
	float add_x,add_y;
	Bit16u min_x,max_x,min_y,max_y;
	float mickey_x,mickey_y;
	float x,y;
	button_event event_queue[QUEUE_SIZE];
	Bit32u events;
	Bit16u sub_seg,sub_ofs;
	Bit16u sub_mask;

	bool	background;
	Bit16s	backposx, backposy;
	Bit8u	backData[CURSORX*CURSORY];
	Bit16u*	screenMask;
	Bit16u* cursorMask;
	Bit16s	clipx,clipy;
	Bit16s  hotx,hoty;
	Bit16u  textAndMask, textXorMask;

	float	mickeysPerPixel_x;
	float	mickeysPerPixel_y;
	float	pixelPerMickey_x;
	float	pixelPerMickey_y;

} mouse;

#define X_MICKEY 8
#define Y_MICKEY 8

#define MOUSE_MOVED 1
#define MOUSE_LEFT_PRESSED 2
#define MOUSE_LEFT_RELEASED 4
#define MOUSE_RIGHT_PRESSED 8
#define MOUSE_RIGHT_RELEASED 16
#define MOUSE_MIDDLE_PRESSED 32
#define MOUSE_MIDDLE_RELEASED 64

INLINE void Mouse_AddEvent(Bit16u type) {
	if (mouse.events<QUEUE_SIZE) {
		mouse.event_queue[mouse.events].type=type;
		mouse.event_queue[mouse.events].buttons=mouse.buttons;
		mouse.events++;
	}
	PIC_ActivateIRQ(12);
}

// ***************************************************************************
// Mouse cursor - text mode
// ***************************************************************************

void RestoreCursorBackgroundText()
{
	if (mouse.shown<0) return;

	if (mouse.background) {
		// Save old Cursorposition
		Bit8u oldx = CURSOR_POS_ROW(0);
		Bit8u oldy = CURSOR_POS_COL(0);
		// Restore background
		INT10_SetCursorPos	((Bit8u)mouse.backposy,(Bit8u)mouse.backposx,0);
		INT10_WriteChar		(mouse.backData[0],mouse.backData[1],0,1,true);
		// Restore old cursor position
		INT10_SetCursorPos	(oldx,oldy,0);
		mouse.background = false;
	}
};

void DrawCursorText()
{	
	// Restore Background
	RestoreCursorBackgroundText();

	// Save old Cursorposition
	Bit8u oldx = CURSOR_POS_ROW(0);
	Bit8u oldy = CURSOR_POS_COL(0);

	// Save Background
	Bit16u result;
	INT10_SetCursorPos	(POS_Y>>3,POS_X>>3,0);
	INT10_ReadCharAttr	(&result,0);
	mouse.backData[0]	= result & 0xFF;
	mouse.backData[1]	= result>>8;
	mouse.backposx		= POS_X>>3;
	mouse.backposy		= POS_Y>>3;
	mouse.background	= true;

	// Write Cursor
	result = (result & mouse.textAndMask) ^ mouse.textXorMask;
	INT10_WriteChar	(result&0xFF,result>>8,0,1,true);

	// Restore old cursor position
	INT10_SetCursorPos (oldx,oldy,0);
};

// ***************************************************************************
// Mouse cursor - graphic mode
// ***************************************************************************

static Bit8u gfxReg[9];

void SaveVgaRegisters()
{
	for (int i=0; i<9; i++) {
		IO_Write	(0x3CE,i);
		gfxReg[i] = IO_Read(0x3CF);
	}
	/* Setup some default values in GFX regs that should work */
	IO_Write	(0x3CE,3);IO_Write(0x3Cf,0);		//disable rotate and operation
	IO_Write	(0x3CE,5);IO_Write(0x3Cf,0);		//Force read/write mode 0
}

void RestoreVgaRegisters()
{
	for (int i=0; i<9; i++) {
		IO_Write(0x3CE,i);
		IO_Write(0x3CF,gfxReg[i]);
	}
}

void ClipCursorArea(Bit16s& x1, Bit16s& x2, Bit16s& y1, Bit16s& y2, Bit16u& addx1, Bit16u& addx2, Bit16u& addy)
{
	addx1 = addx2 = addy = 0;
	// Clip up
	if (y1<0) {
		addy += (-y1);
		y1 = 0;
	}
	// Clip down
	if (y2>mouse.clipy) {
		y2 = mouse.clipy;		
	};
	// Clip left
	if (x1<0) {
		addx1 += (-x1);
		x1 = 0;
	};
	// Clip right
	if (x2>mouse.clipx) {
		addx2 = x2 - mouse.clipx;
		x2 = mouse.clipx;
	};
};

void RestoreCursorBackground()
{
	if (mouse.shown<0) return;

	SaveVgaRegisters();
	if (mouse.background) {
		// Restore background
		Bit16s x,y;
		Bit16u addx1,addx2,addy;
		Bit16u dataPos	= 0;
		Bit16s x1		= mouse.backposx;
		Bit16s y1		= mouse.backposy;
		Bit16s x2		= x1 + CURSORX - 1;
		Bit16s y2		= y1 + CURSORY - 1;	

		ClipCursorArea(x1, x2, y1, y2, addx1, addx2, addy);

		dataPos = addy * CURSORX;
		for (y=y1; y<=y2; y++) {
			dataPos += addx1;
			for (x=x1; x<=x2; x++) {
				INT10_PutPixel(x,y,0,mouse.backData[dataPos++]);
			};
			dataPos += addx2;
		};
		mouse.background = false;
	};
	RestoreVgaRegisters();
};

void DrawCursor() {
	
	if (mouse.shown<0) return;

	// Get Clipping ranges

	// In Textmode ?
	if (CurMode->type==M_TEXT16) {
		DrawCursorText();
		return;
	}

	mouse.clipx = CurMode->swidth-1;	/* Get from bios ? */
	mouse.clipy = CurMode->sheight-1;
	Bit16s xratio = 640 / CurMode->swidth;  /* might be mouse.max_x-.mouse.min_x+1/swidth */
											/* might even be vidmode == 0x13?2:1 */
	
	RestoreCursorBackground();

	SaveVgaRegisters();

	// Save Background
	Bit16s x,y;
	Bit16u addx1,addx2,addy;
	Bit16u dataPos	= 0;
	Bit16s x1		= POS_X / xratio - mouse.hotx;
	Bit16s y1		= POS_Y - mouse.hoty;
	Bit16s x2		= x1 + CURSORX - 1;
	Bit16s y2		= y1 + CURSORY - 1;	

	ClipCursorArea(x1,x2,y1,y2, addx1, addx2, addy);

	dataPos = addy * CURSORX;
	for (y=y1; y<=y2; y++) {
		dataPos += addx1;
		for (x=x1; x<=x2; x++) {
			INT10_GetPixel(x,y,0,&mouse.backData[dataPos++]);
		};
		dataPos += addx2;
	};
	mouse.background= true;
	mouse.backposx	= POS_X / xratio - mouse.hotx;
	mouse.backposy	= POS_Y - mouse.hoty;

	// Draw Mousecursor
	dataPos = addy * CURSORX;
	for (y=y1; y<=y2; y++) {
		Bit16u scMask = mouse.screenMask[addy+y-y1];
		Bit16u cuMask = mouse.cursorMask[addy+y-y1];
		if (addx1>0) { scMask<<=addx1; cuMask<<=addx1; dataPos += addx1; };
		for (x=x1; x<=x2; x++) {
			Bit8u pixel = 0;
			// ScreenMask
			if (scMask & HIGHESTBIT) pixel = mouse.backData[dataPos];
			scMask<<=1;
			// CursorMask
			if (cuMask & HIGHESTBIT) pixel = pixel ^ 0x0F;
			cuMask<<=1;
			// Set Pixel
			INT10_PutPixel(x,y,0,pixel);
			dataPos++;
		};
		dataPos += addx2;
	};
	RestoreVgaRegisters();
}

void Mouse_CursorMoved(float x,float y) {

	float dx = x * mouse.pixelPerMickey_x;
	float dy = y * mouse.pixelPerMickey_y;

	mouse.mickey_x += dx;
	mouse.mickey_y += dy;

	mouse.x += dx;
	if (mouse.x>mouse.max_x) mouse.x=mouse.max_x;
	if (mouse.x<mouse.min_x) mouse.x=mouse.min_x;
	mouse.y += dy;
	if (mouse.y>mouse.max_y) mouse.y=mouse.max_y;
	if (mouse.y<mouse.min_y) mouse.y=mouse.min_y;
	Mouse_AddEvent(MOUSE_MOVED);
	DrawCursor();
}

void Mouse_CursorSet(float x,float y) {
	mouse.x=x;
	mouse.y=y;
	DrawCursor();
}

void Mouse_ButtonPressed(Bit8u button) {
	switch (button) {
	case 0:
		mouse.buttons|=1;
		Mouse_AddEvent(MOUSE_LEFT_PRESSED);
		break;
	case 1:
		mouse.buttons|=2;
		Mouse_AddEvent(MOUSE_RIGHT_PRESSED);
		break;
	case 2:
		mouse.buttons|=4;
		Mouse_AddEvent(MOUSE_MIDDLE_PRESSED);
		break;
	}
	mouse.times_pressed[button]++;
	mouse.last_pressed_x[button]=POS_X;
	mouse.last_pressed_y[button]=POS_Y;
}

void Mouse_ButtonReleased(Bit8u button) {
	switch (button) {
	case 0:
		mouse.buttons&=~1;
		Mouse_AddEvent(MOUSE_LEFT_RELEASED);
		break;
	case 1:
		mouse.buttons&=~2;
		Mouse_AddEvent(MOUSE_RIGHT_RELEASED);
		break;
	case 2:
		mouse.buttons&=~4;
		Mouse_AddEvent(MOUSE_MIDDLE_RELEASED);
		break;
	}
	mouse.times_released[button]++;	
	mouse.last_released_x[button]=POS_X;
	mouse.last_released_y[button]=POS_Y;
}

static void SetMickeyPixelRate(Bit16s px, Bit16s py)
{
	if ((px!=0) && (py!=0)) {
		mouse.mickeysPerPixel_x	 = (float)px/X_MICKEY;
		mouse.mickeysPerPixel_y  = (float)py/Y_MICKEY;
		mouse.pixelPerMickey_x	 = X_MICKEY/(float)px;
		mouse.pixelPerMickey_y 	 = Y_MICKEY/(float)py;	
	}
};

static void mouse_reset_hardware(void){
	mouse.sub_mask=0;
	mouse.sub_seg=0;
	mouse.sub_ofs=0;
};

static void  mouse_reset(void) 
{
	WriteMouseIntVector();
	real_writed(0,(0x74<<2),CALLBACK_RealPointer(call_int74));
	mouse.shown=-1;
	/* Get the correct resolution from the current video mode */
	Bitu mode=mem_readb(BIOS_VIDEO_MODE);
	switch (mode) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x0d:
	case 0x0e:
	case 0x13:
		mouse.max_y=199;
		break;
	case 0x0f:
	case 0x10:
		mouse.max_y=349;
		break;
	case 0x11:
	case 0x12:
		mouse.max_y=479;
		break;
	default:
		mouse.max_y=199;
		LOG(LOG_MOUSE,LOG_ERROR)("Unhandled videomode %X on reset",mode);
		break;
	} 
	mouse.max_x=639;
	mouse.min_x=0;
	mouse.min_y=0;
	// Dont set max coordinates here. it is done by SetResolution!
	mouse.x=0;				// civ wont work otherwise
	mouse.y=static_cast<float>(mouse.max_y/2);
	mouse.events=0;
	mouse.mickey_x=0;
	mouse.mickey_y=0;

	mouse.hotx		 = 0;
	mouse.hoty		 = 0;
	mouse.background = false;
	mouse.screenMask = defaultScreenMask;
	mouse.cursorMask = defaultCursorMask;
	mouse.textAndMask= defaultTextAndMask;
	mouse.textXorMask= defaultTextXorMask;

	SetMickeyPixelRate(8,16);
}

void Mouse_NewVideoMode(void)
{
	//mouse.shown = -1;	
	mouse_reset();
	//Added this for cd-v19
}

static Bitu INT33_Handler(void) {

//	LOG(LOG_MOUSE,LOG_NORMAL)("MOUSE: %04X",reg_ax);
	switch (reg_ax) {
	case 0x00:	/* Reset Driver and Read Status */
		mouse_reset_hardware(); /* fallthrough */
	case 0x21:	/* Software Reset */
		reg_ax=0xffff;
		reg_bx=MOUSE_BUTTONS;
		mouse_reset();
		Mouse_AutoLock(true);
		break;
	case 0x01:	/* Show Mouse */
		mouse.shown++;
		Mouse_AutoLock(true);
		if (mouse.shown>0) mouse.shown=0;
		DrawCursor();
		break;
	case 0x02:	/* Hide Mouse */
		{
			if (CurMode->type!=M_TEXT16)	RestoreCursorBackground();
			else							RestoreCursorBackgroundText();
			mouse.shown--;
		}
		break;
	case 0x03:	/* Return position and Button Status */
		reg_bx=mouse.buttons;
		reg_cx=POS_X;
		reg_dx=POS_Y;
		break;
	case 0x04:	/* Position Mouse */
		mouse.x = static_cast<float>(((reg_cx > mouse.max_x) ? mouse.max_x : reg_cx));
		mouse.y = static_cast<float>(((reg_dx > mouse.max_y) ? mouse.max_y : reg_dx));
		DrawCursor();
		break;
	case 0x05:	/* Return Button Press Data */
		{
			Bit16u but=reg_bx;
			reg_ax=mouse.buttons;
			if (but>=MOUSE_BUTTONS) break;
			reg_cx=mouse.last_pressed_x[but];
			reg_dx=mouse.last_pressed_y[but];
			reg_bx=mouse.times_pressed[but];
			mouse.times_pressed[but]=0;
			break;
		}
	case 0x06:	/* Return Button Release Data */
		{
			Bit16u but=reg_bx;
			reg_ax=mouse.buttons;
			if (but>=MOUSE_BUTTONS) break;
			reg_cx=mouse.last_released_x[but];
			reg_dx=mouse.last_released_y[but];
			reg_bx=mouse.times_released[but];
			mouse.times_released[but]=0;
			break;
		}
	case 0x07:	/* Define horizontal cursor range */
		{	//lemmings set 1-640 and wants that. iron seeds set 0-640 but doesn't like 640
			Bits max,min;
			if ((Bit16s)reg_cx<(Bit16s)reg_dx) { min=(Bit16s)reg_cx;max=(Bit16s)reg_dx;}
			else { min=(Bit16s)reg_dx;max=(Bit16s)reg_cx;}
			//if(max - min + 1 > 640) max = min + 640 - 1;
			mouse.min_x=min;
			mouse.max_x=max;
			LOG(LOG_MOUSE,LOG_NORMAL)("Define Hortizontal range min:%d max:%d",min,max);
		}
		break;
	case 0x08:	/* Define vertical cursor range */
		{	// not sure what to take instead of the CurMode (see case 0x07 as well)
			// especially the cases where sheight= 400 and we set it with the mouse_reset to 200
			//disabled it at the moment. Seems to break syndicate who want 400 in mode 13
			Bits max,min;
			if ((Bit16s)reg_cx<(Bit16s)reg_dx) { min=(Bit16s)reg_cx;max=(Bit16s)reg_dx;}
			else { min=(Bit16s)reg_dx;max=(Bit16s)reg_cx;}
		//	if(static_cast<Bitu>(max - min + 1) > CurMode->sheight) max = min + CurMode->sheight - 1;
			mouse.min_y=min;
			mouse.max_y=max;
			LOG(LOG_MOUSE,LOG_NORMAL)("Define Vertical range min:%d max:%d",min,max);
		}
		break;
	case 0x09:	/* Define GFX Cursor */
		{
			PhysPt src = SegPhys(es)+reg_dx;
			MEM_BlockRead(src          ,userdefScreenMask,CURSORY*2);
			MEM_BlockRead(src+CURSORY*2,userdefCursorMask,CURSORY*2);
			mouse.screenMask = userdefScreenMask;
			mouse.cursorMask = userdefCursorMask;
			mouse.hotx		 = reg_bx;
			mouse.hoty		 = reg_cx;
			DrawCursor();
		}
		break;
	case 0x0a:	/* Define Text Cursor */
		mouse.textAndMask = reg_cx;
		mouse.textXorMask = reg_dx;
		break;
	case 0x0c:	/* Define interrupt subroutine parameters */
		mouse.sub_mask=reg_cx;
		mouse.sub_seg=SegValue(es);
		mouse.sub_ofs=reg_dx;
		break;
	case 0x0f:	/* Define mickey/pixel rate */
		SetMickeyPixelRate(reg_cx,reg_dx);
		break;
	case 0x0B:	/* Read Motion Data */
		reg_cx=(Bit16s)(mouse.mickey_x*mouse.mickeysPerPixel_x);
		reg_dx=(Bit16s)(mouse.mickey_y*mouse.mickeysPerPixel_y);
		mouse.mickey_x=0;
		mouse.mickey_y=0;
		break;
	case 0x14: /* Exchange event-handler */ 
		{	
			Bit16u oldSeg = mouse.sub_seg;
			Bit16u oldOfs = mouse.sub_ofs;
			Bit16u oldMask= mouse.sub_mask;
			// Set new values
			mouse.sub_mask= reg_cx;
			mouse.sub_seg = SegValue(es);
			mouse.sub_ofs = reg_dx;
			// Return old values
			reg_cx = oldMask;
			reg_dx = oldOfs;
			SegSet16(es,oldSeg);
		}
		break;		
	case 0x1a:	/* Set mouse sensitivity */
		SetMickeyPixelRate(reg_bx,reg_cx);
		// ToDo : double mouse speed value
		break;
	case 0x1b:	/* Get mouse sensitivity */
		reg_bx = Bit16s(X_MICKEY * mouse.mickeysPerPixel_x);
		reg_cx = Bit16s(Y_MICKEY * mouse.mickeysPerPixel_y);
		// ToDo : double mouse speed value
		reg_dx = 64;
		break;
	case 0x1c:	/* Set interrupt rate */
		/* Can't really set a rate this is host determined */
		break;
	case 0x24:	/* Get Software version and mouse type */
		reg_bx=0x805;	//Version 8.05 woohoo 
		reg_ch=0xff;	/* Unkown type */
		reg_cl=0;		/* Hmm ps2 irq dunno */
		break;
	default:
		LOG(LOG_MOUSE,LOG_ERROR)("Mouse Function %2X",reg_ax);
	}
	return CBRET_NONE;
}

static Bitu INT74_Handler(void) {
	if (mouse.events>0) {
		mouse.events--;
		/* Check for an active Interrupt Handler that will get called */
		if (mouse.sub_mask & mouse.event_queue[mouse.events].type) {
			/* Save lot's of registers */
			Bit32u oldeax,oldebx,oldecx,oldedx,oldesi,oldedi,oldebp,oldesp;
			Bit16u oldds,oldes,oldss;
			oldeax=reg_eax;oldebx=reg_ebx;oldecx=reg_ecx;oldedx=reg_edx;
			oldesi=reg_esi;oldedi=reg_edi;oldebp=reg_ebp;oldesp=reg_esp;
			oldds=SegValue(ds); oldes=SegValue(es);	oldss=SegValue(ss); // Save segments
			reg_ax=mouse.event_queue[mouse.events].type;
			reg_bx=mouse.event_queue[mouse.events].buttons;
			reg_cx=POS_X;
			reg_dx=POS_Y;
			reg_si=(Bit16s)(mouse.mickey_x*mouse.mickeysPerPixel_x);
			reg_di=(Bit16s)(mouse.mickey_y*mouse.mickeysPerPixel_y);
			// Hmm... this look ok, but moonbase wont work with it
			/*if (mouse.event_queue[mouse.events].type==MOUSE_MOVED) {
				mouse.mickey_x=0;
				mouse.mickey_y=0;
			}*/
			CALLBACK_RunRealFar(mouse.sub_seg,mouse.sub_ofs);
			reg_eax=oldeax;reg_ebx=oldebx;reg_ecx=oldecx;reg_edx=oldedx;
			reg_esi=oldesi;reg_edi=oldedi;reg_ebp=oldebp;reg_esp=oldesp;
			SegSet16(ds,oldds); SegSet16(es,oldes); SegSet16(ss,oldss); // Save segments

		}
	}
	IO_Write(0xa0,0x20);
	IO_Write(0x20,0x20);
	/* Check for more Events if so reactivate IRQ */
	if (mouse.events) {
		PIC_ActivateIRQ(12);
	}
	return CBRET_NONE;
}

void WriteMouseIntVector(void)
{
	// Create a mouse vector with weird address 
	// for strange mouse detection routines in Sim City & Wasteland
	real_writed(0,0x33<<2,RealMake(CB_SEG+1,(call_int33<<4)-0x10+1));	// +1 = Skip NOP 
};

void CreateMouseCallback(void)
{
	// Create callback
	call_int33=CALLBACK_Allocate();
	CALLBACK_Setup(call_int33,&INT33_Handler,CB_IRET);
	// Create a mouse vector with weird address 
	// for strange mouse detection routines in Sim City & Wasteland
	Bit16u ofs = call_int33<<4;
	phys_writeb(CB_BASE+ofs+0,(Bit8u)0x90);	//NOP
	phys_writeb(CB_BASE+ofs+1,(Bit8u)0xFE);	//GRP 4
	phys_writeb(CB_BASE+ofs+2,(Bit8u)0x38);	//Extra Callback instruction
	phys_writew(CB_BASE+ofs+3,call_int33);	//The immediate word
	phys_writeb(CB_BASE+ofs+5,(Bit8u)0xCF);	//An IRET Instruction
	// Write weird vector
	WriteMouseIntVector();
};

void MOUSE_Init(Section* sec) {
	
	// Callback 0x33
	CreateMouseCallback();

	call_int74=CALLBACK_Allocate();
	CALLBACK_Setup(call_int74,&INT74_Handler,CB_IRET);
	real_writed(0,(0x74<<2),CALLBACK_RealPointer(call_int74));

	memset(&mouse,0,sizeof(mouse));
	mouse_reset_hardware();
	mouse_reset();
}

