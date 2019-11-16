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

#include <string.h>
#include "dosbox.h"
#include "keyboard.h"
#include "inout.h"
#include "pic.h"
#include "mixer.h"

#define KEYBUFSIZE 32

enum KeyCommands {
	CMD_NONE,
	CMD_SETLEDS,
	CMD_SETTYPERATE
};


struct KeyEvent {
	Bitu type;
	Bitu state;
	KEYBOARD_EventHandler * handler;
	KeyEvent * next;
};

struct KeyBlock {
	Bit8u buf[KEYBUFSIZE];
	Bitu buf_pos;
	Bitu buf_used;
	Bitu write_state;

	KeyCommands command;
	bool read_active;
	bool enabled;
	bool active;
};

static KeyBlock keyb;
static Bitu shift_state=0;
static Bit8u cur_scancode;
static Bit8u port_61_data;
//TODO Are these initialized at 0 at startup? Hope so :)
static KeyEvent * event_handlers[KBD_LAST];


void KEYBOARD_ClrBuffer(void) {
	keyb.buf_used=0;
	keyb.buf_pos=0;
	keyb.read_active=false;
	PIC_DeActivateIRQ(1);
}


void KEYBOARD_ReadBuffer(void) {
	if (keyb.read_active) return;
	if (keyb.buf_used) {
		keyb.buf_used--;
		keyb.buf_pos++;
		if (keyb.buf_pos>=KEYBUFSIZE) keyb.buf_pos=0;
	}
	if (keyb.buf_used) {
		keyb.read_active=true;
		PIC_ActivateIRQ(1);
	}
}

void KEYBOARD_AddCode(Bit8u code) {
	if (keyb.buf_used<KEYBUFSIZE) {
		Bitu start=keyb.buf_pos+keyb.buf_used;
		keyb.buf_used++;
		if (start>=KEYBUFSIZE) start-=KEYBUFSIZE;
		keyb.buf[start]=code;
		keyb.read_active=true;
	}
	if (keyb.buf_used==1) PIC_AddIRQ(1,0);
}

static Bit8u read_p60(Bit32u port) {
	/* Reading this port signals that IRQ can be lowered */
	Bit8u val=keyb.buf[keyb.buf_pos];
	keyb.read_active=false;
	return keyb.buf[keyb.buf_pos];
}

static void write_p60(Bit32u port,Bit8u val) {
	switch (keyb.command) {
	case CMD_NONE:	/* None */
		KEYBOARD_ClrBuffer();
		switch (val) {
		case 0xed:	/* Set Leds */
			keyb.command=CMD_SETLEDS;
			KEYBOARD_AddCode(0xfa);	/* Acknowledge */
			break;
		case 0xee:	/* Echo */
			KEYBOARD_AddCode(0xee);
			break;
		case 0xf2:	/* Identify keyboard */
			/* AT's just send acknowledge */
			KEYBOARD_AddCode(0xfa);	/* Acknowledge */
			break;
		case 0xf3: /* Typematic rate programming */
			keyb.command=CMD_SETTYPERATE;
			KEYBOARD_AddCode(0xfa);	/* Acknowledge */
			break;
		default:
			LOG_DEBUG("KEYB:60:Unhandled command %X",val);
		}
		return;
	case CMD_SETTYPERATE:
	case CMD_SETLEDS:
		keyb.command=CMD_NONE;
		KEYBOARD_AddCode(0xfa);	/* Acknowledge */
		break;
	}
}

static Bit8u read_p61(Bit32u port) {
	port_61_data^=0x20;
	return port_61_data;

};

static void KEYBOARD_IRQHandler(void) {
	if (!keyb.read_active) KEYBOARD_ReadBuffer();
}

static void write_p61(Bit32u port,Bit8u val) {
	port_61_data=val;
	if (val & 128) if (!keyb.read_active) KEYBOARD_ReadBuffer();
	if ((val & 3)==3) {
		PCSPEAKER_Enable(true);
	} else {
		PCSPEAKER_Enable(false);
	}
}

static void write_p64(Bit32u port,Bit8u val) {
	switch (val) {
	case 0xad:		/* Activate keyboard */
		keyb.active=true;
		break;
	case 0xae:		/* Deactivate keyboard */
		keyb.active=false;
		break;
	default:
		LOG_DEBUG("Port 64 write with val %d",val);
		break;
	}
}

static Bit8u read_p64(Bit32u port) {
	return 0x1c | (keyb.read_active ? 0x1 : 0x0);
}


void KEYBOARD_AddEvent(Bitu keytype,Bitu state,KEYBOARD_EventHandler * handler) {
	KeyEvent * newevent=new KeyEvent;
/* Add the event in the correct key structure */
	if (keytype>=KBD_LAST) {
		LOG_ERROR("KEYBOARD:Illegal key %d for handler",keytype);
	}
	newevent->next=event_handlers[keytype];
	event_handlers[keytype]=newevent;
	newevent->type=keytype;
	newevent->state=state;
	newevent->handler=handler;
}

void KEYBOARD_AddKey(Bitu keytype,bool pressed) {
	bool extend=false;
	Bit8u ret=0;
	switch (keytype) {
	case KBD_esc:ret=1;break;
	case KBD_1:ret=2;break;
	case KBD_2:ret=3;break;
	case KBD_3:ret=4;break;		
	case KBD_4:ret=5;break;
	case KBD_5:ret=6;break;
	case KBD_6:ret=7;break;		
	case KBD_7:ret=8;break;
	case KBD_8:ret=9;break;
	case KBD_9:ret=10;break;		
	case KBD_0:ret=11;break;

	case KBD_minus:ret=12;break;
	case KBD_equals:ret=13;break;
	case KBD_backspace:ret=14;break;
	case KBD_tab:ret=15;break;

	case KBD_q:ret=16;break;		
	case KBD_w:ret=17;break;
	case KBD_e:ret=18;break;		
	case KBD_r:ret=19;break;
	case KBD_t:ret=20;break;		
	case KBD_y:ret=21;break;
	case KBD_u:ret=22;break;		
	case KBD_i:ret=23;break;
	case KBD_o:ret=24;break;		
	case KBD_p:ret=25;break;

	case KBD_leftbracket:ret=26;break;
	case KBD_rightbracket:ret=27;break;
	case KBD_enter:ret=28;break;
	case KBD_leftctrl:ret=29;
		shift_state=(shift_state&~CTRL_PRESSED)|(pressed ? CTRL_PRESSED:0);
		break;

	case KBD_a:ret=30;break;
	case KBD_s:ret=31;break;
	case KBD_d:ret=32;break;
	case KBD_f:ret=33;break;
	case KBD_g:ret=34;break;		
	case KBD_h:ret=35;break;		
	case KBD_j:ret=36;break;
	case KBD_k:ret=37;break;		
	case KBD_l:ret=38;break;

	case KBD_semicolon:ret=39;break;
	case KBD_quote:ret=40;break;
	case KBD_grave:ret=41;break;
	case KBD_leftshift:ret=42;
		shift_state=(shift_state&~SHIFT_PRESSED)|(pressed ? SHIFT_PRESSED:0);
		break;
	case KBD_backslash:ret=43;break;
	case KBD_z:ret=44;break;
	case KBD_x:ret=45;break;
	case KBD_c:ret=46;break;
	case KBD_v:ret=47;break;
	case KBD_b:ret=48;break;
	case KBD_n:ret=49;break;
	case KBD_m:ret=50;break;

	case KBD_comma:ret=51;break;
	case KBD_period:ret=52;break;
	case KBD_slash:ret=53;break;
	case KBD_rightshift:ret=54;break;
	case KBD_kpmultiply:ret=55;break;
	case KBD_leftalt:ret=56;
		shift_state=(shift_state&~ALT_PRESSED)|(pressed ? ALT_PRESSED:0);
		break;
	case KBD_space:ret=57;break;
	case KBD_capslock:ret=58;break;

	case KBD_f1:ret=59;break;
	case KBD_f2:ret=60;break;
	case KBD_f3:ret=61;break;
	case KBD_f4:ret=62;break;
	case KBD_f5:ret=63;break;
	case KBD_f6:ret=64;break;
	case KBD_f7:ret=65;break;
	case KBD_f8:ret=66;break;
	case KBD_f9:ret=67;break;
	case KBD_f10:ret=68;break;

	case KBD_numlock:ret=69;break;
	case KBD_scrolllock:ret=70;break;

	case KBD_kp7:ret=71;break;
	case KBD_kp8:ret=72;break;
	case KBD_kp9:ret=73;break;
	case KBD_kpminus:ret=74;break;
	case KBD_kp4:ret=75;break;
	case KBD_kp5:ret=76;break;
	case KBD_kp6:ret=77;break;
	case KBD_kpplus:ret=78;break;
	case KBD_kp1:ret=79;break;
	case KBD_kp2:ret=80;break;
	case KBD_kp3:ret=81;break;
	case KBD_kp0:ret=82;break;
	case KBD_kpperiod:ret=83;break;

	case KBD_f11:ret=87;break;
	case KBD_f12:ret=88;break;

	//The Extended keys

	case KBD_kpenter:extend=true;ret=28;break;
	case KBD_rightctrl:extend=true;ret=29;
		shift_state=(shift_state&~CTRL_PRESSED)|(pressed ? CTRL_PRESSED:0);
		break;
	case KBD_kpslash:extend=true;ret=53;break;
	case KBD_rightalt:extend=true;ret=56;
		shift_state=(shift_state&~ALT_PRESSED)|(pressed ? ALT_PRESSED:0);
		break;
	case KBD_home:extend=true;ret=71;break;
	case KBD_up:extend=true;ret=72;break;
	case KBD_pageup:extend=true;ret=73;break;
	case KBD_left:extend=true;ret=75;break;
	case KBD_right:extend=true;ret=77;break;
	case KBD_end:extend=true;ret=79;break;
	case KBD_down:extend=true;ret=80;break;
	case KBD_pagedown:extend=true;ret=81;break;
	case KBD_insert:extend=true;ret=82;break;
	case KBD_delete:extend=true;ret=83;break;
	default:
		E_Exit("Unsopperted key press");
		break;
	}
	/* check for active key events */
	KeyEvent * checkevent=event_handlers[keytype];
	while (checkevent) {
		if ((shift_state & checkevent->state)==checkevent->state) {
			if (checkevent->type==keytype && pressed) {
				(*checkevent->handler)();
				return;
			}
			if (checkevent->type==keytype) return;
		}
		checkevent=checkevent->next;
	}
	if (extend) KEYBOARD_AddCode(224);
	if (!pressed) ret+=128;
	KEYBOARD_AddCode(ret);
}

void KEYBOARD_Init(Section* sec) {
	IO_RegisterWriteHandler(0x60,write_p60,"Keyboard");
	IO_RegisterReadHandler(0x60,read_p60,"Keyboard");
	IO_RegisterWriteHandler(0x61,write_p61,"Keyboard");
	IO_RegisterReadHandler(0x61,read_p61,"Keyboard");
	IO_RegisterWriteHandler(0x64,write_p64,"Keyboard");
	IO_RegisterReadHandler(0x64,read_p64,"Keyboard");

	port_61_data=1;				/* Speaker control through PIT and speaker disabled */
//	memset(&event_handlers,0,sizeof(event_handlers));
	/* Clear the keyb struct */
	keyb.enabled=true;
	keyb.command=CMD_NONE;
	KEYBOARD_ClrBuffer();
	PIC_RegisterIRQ(1,KEYBOARD_IRQHandler,"KEYBOARD");
}
