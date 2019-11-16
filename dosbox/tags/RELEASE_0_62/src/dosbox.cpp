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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* $Id: dosbox.cpp,v 1.80 2004-09-21 19:32:15 qbix79 Exp $ */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "dosbox.h"
#include "debug.h"
#include "cpu.h"
#include "video.h"
#include "pic.h"
#include "cpu.h"
#include "callback.h"
#include "inout.h"
#include "mixer.h"
#include "timer.h"
#include "dos_inc.h"
#include "setup.h"
#include "cross.h"
#include "programs.h"
#include "support.h"

Config * control;
MachineType machine;

/* The whole load of startups for all the subfunctions */
void MSG_Init(Section_prop *);
void LOG_StartUp(void);
void MEM_Init(Section *);
void PAGING_Init(Section *);
void IO_Init(Section * );
void CALLBACK_Init(Section*);
void PROGRAMS_Init(Section*);
void CREDITS_Init(Section*);
void RENDER_Init(Section*);
void VGA_Init(Section*);

void DOS_Init(Section*);


void CPU_Init(Section*);

#if C_FPU
void FPU_Init(Section*);
#endif

void DMA_Init(Section*);
void MIXER_Init(Section*);
void MIDI_Init(Section*);
void HARDWARE_Init(Section*);


void KEYBOARD_Init(Section*);	//TODO This should setup INT 16 too but ok ;)
void JOYSTICK_Init(Section*);
void MOUSE_Init(Section*);
void SBLASTER_Init(Section*);
void GUS_Init(Section*);
void MPU401_Init(Section*);
void PCSPEAKER_Init(Section*);
void TANDYSOUND_Init(Section*);
void DISNEY_Init(Section*);
void SERIAL_Init(Section*); 

#if C_MODEM
void MODEM_Init(Section*); 
#endif

#if C_IPX
void IPX_Init(Section*);
#endif

#if C_DIRECTSERIAL
void DIRECTSERIAL_Init(Section* sec);
#endif
void SID_Init(Section* sec);

void PIC_Init(Section*);
void TIMER_Init(Section*);
void BIOS_Init(Section*);
void DEBUG_Init(Section*);
void CMOS_Init(Section*);

void MSCDEX_Init(Section*);

/* Dos Internal mostly */
void EMS_Init(Section*);
void XMS_Init(Section*);

void AUTOEXEC_Init(Section*);
void SHELL_Init(void);

void INT10_Init(Section*);

static LoopHandler * loop;

bool SDLNetInited;

Bits RemainTicks;
Bits LastTicks;

static Bitu Normal_Loop(void) {
	Bits ret,NewTicks;
	while (1) {
		if (PIC_RunQueue()) {
			ret=(*cpudecoder)();
			if (ret<0) return 1;
			if (ret>0) {
				Bitu blah=(*CallBack_Handlers[ret])();
				if (blah) return blah;
			}
#if C_DEBUG
			if (DEBUG_ExitLoop()) return 0;
#endif
		} else {
			if (RemainTicks>0) {
				TIMER_AddTick();
				RemainTicks--;
				GFX_Events();
			} else goto increaseticks;
		}
	}
increaseticks:
	NewTicks=GetTicks();
	if (NewTicks>LastTicks) {
		RemainTicks=NewTicks-LastTicks;
		if (RemainTicks>20) {
//			LOG_MSG("Ticks to handle overflow %d",RemainTicks);
			RemainTicks=20;
		}
		LastTicks=NewTicks;
	}
	//TODO Make this selectable in the config file, since it gives some lag */
	if (RemainTicks<=0) {
		SDL_Delay(1);
		return 0;
	}
	return 0;
}

void DOSBOX_SetLoop(LoopHandler * handler) {
	loop=handler;
}

void DOSBOX_SetNormalLoop() {
	loop=Normal_Loop;
}

void DOSBOX_RunMachine(void){
	Bitu ret;
	do {
		ret=(*loop)();
	} while (!ret);
}

static void DOSBOX_RealInit(Section * sec) {
	Section_prop * section=static_cast<Section_prop *>(sec);
	/* Initialize some dosbox internals */

	RemainTicks=0;LastTicks=GetTicks();
	DOSBOX_SetLoop(&Normal_Loop);
	MSG_Init(section);

	machine=MCH_VGA;std::string cmd_machine;
	const char * mtype;
	if (control->cmdline->FindString("-machine",cmd_machine,true)) mtype=cmd_machine.c_str();
	else mtype=section->Get_string("machine");
	if (strcasecmp(mtype,"cga")==0) machine=MCH_CGA;
	else if (strcasecmp(mtype,"tandy")==0) machine=MCH_TANDY;
	else if (strcasecmp(mtype,"hercules")==0) machine=MCH_HERC;
	else if (strcasecmp(mtype,"vga")==0) machine=MCH_VGA;
	else LOG_MSG("DOSBOX:Unknown machine type %s",mtype);
}


void DOSBOX_Init(void) {
	Section_prop * secprop;
	Section_line * secline;

	SDLNetInited = false;

	/* Setup all the different modules making up DOSBox */
	
	secprop=control->AddSection_prop("dosbox",&DOSBOX_RealInit);
	secprop->Add_string("language","");
	secprop->Add_string("machine","vga");
	secprop->Add_string("captures","capture");

#if C_DEBUG	
	LOG_StartUp();
#endif
	
	secprop->AddInitFunction(&IO_Init);
	secprop->AddInitFunction(&PAGING_Init);
	secprop->AddInitFunction(&MEM_Init);
	secprop->AddInitFunction(&HARDWARE_Init);
	secprop->Add_int("memsize",16);
	secprop->AddInitFunction(&CALLBACK_Init);
	secprop->AddInitFunction(&PIC_Init);
	secprop->AddInitFunction(&PROGRAMS_Init);
	secprop->AddInitFunction(&TIMER_Init);
	secprop->AddInitFunction(&CMOS_Init);
	secprop->AddInitFunction(&SERIAL_Init); 
		
	MSG_Add("DOSBOX_CONFIGFILE_HELP",
		"language -- Select another language file.\n"
		"memsize -- Amount of memory dosbox has in megabytes.\n"
		"machine -- The type of machine tries to emulate:hercules,cga,tandy,vga.\n"
		"captures -- Directory where things like wave,midi,screenshot get captured.\n"
	);

	secprop=control->AddSection_prop("render",&RENDER_Init);
	secprop->Add_int("frameskip",0);
	secprop->Add_bool("aspect",false);
	secprop->Add_string("scaler","normal2x");
	MSG_Add("RENDER_CONFIGFILE_HELP",
		"frameskip -- How many frames dosbox skips before drawing one.\n"
		"aspect -- Do aspect correction.\n"
		"scaler -- Scaler used to enlarge/enhance low resolution modes.\n"
		"          Supported are none,normal2x,advmame2x,advmame3x,advinterp2x,interp2x,tv2x.\n"
	);

	secprop=control->AddSection_prop("cpu",&CPU_Init);
	secprop->Add_string("core","normal");
	secprop->Add_int("cycles",3000);
	secprop->Add_int("cycleup",500);
	secprop->Add_int("cycledown",20);
	MSG_Add("CPU_CONFIGFILE_HELP",
		"core -- CPU Core used in emulation: simple,normal,full"
#if (C_DYNAMIC_X86)
		",dynamic"
#endif
		".\n"
		"cycles -- Amount of instructions dosbox tries to emulate each millisecond.\n"
		"          Setting this higher than your machine can handle is bad!\n"
		"cycleup   -- Amount of cycles to increase/decrease with keycombo.\n"
		"cycledown    Setting it lower than 100 will be a percentage.\n"
	);
#if C_FPU
	secprop->AddInitFunction(&FPU_Init);
#endif
	secprop->AddInitFunction(&DMA_Init);
	secprop->AddInitFunction(&VGA_Init);
	secprop->AddInitFunction(&KEYBOARD_Init);
	secprop->AddInitFunction(&MOUSE_Init);
	secprop->AddInitFunction(&JOYSTICK_Init);

	secprop=control->AddSection_prop("mixer",&MIXER_Init);
	secprop->Add_bool("nosound",false);
	secprop->Add_int("rate",22050);
	secprop->Add_int("blocksize",2048);
	secprop->Add_int("prebuffer",10);

	MSG_Add("MIXER_CONFIGFILE_HELP",
		"nosound -- Enable silent mode, sound is still emulated though.\n"
		"rate -- Mixer sample rate, setting any devices higher than this will\n"
		"        probably lower their sound quality.\n"
		"blocksize -- Mixer block size, larger blocks might help sound stuttering\n"
		"             but sound will also be more lagged.\n"
		"prebuffer -- How many milliseconds of data to keep on top of the blocksize.\n"
	);
	
	secprop=control->AddSection_prop("midi",&MIDI_Init);
	secprop->AddInitFunction(&MPU401_Init);
	secprop->Add_bool("mpu401",true);
	secprop->Add_bool("intelligent",true);   
	secprop->Add_string("device","default");
	secprop->Add_string("config","");
	
	MSG_Add("MIDI_CONFIGFILE_HELP",
		"mpu401      -- Enable MPU-401 Emulation.\n"
		"intelligent -- Operate in Intelligent mode.\n"
		"device      -- Device that will receive the MIDI data from MPU-401.\n"
		"               This can be default,alsa,oss,win32,coreaudio,none.\n"
		"config      -- Special configuration options for the device.\n"
	);

#if C_DEBUG
	secprop=control->AddSection_prop("debug",&DEBUG_Init);
#endif
	secprop=control->AddSection_prop("sblaster",&SBLASTER_Init);
	secprop->Add_string("type","sb16");
	secprop->Add_hex("base",0x220);
	secprop->Add_int("irq",7);
	secprop->Add_int("dma",1);
	secprop->Add_int("hdma",5);
	secprop->Add_bool("mixer",true);
	secprop->Add_string("oplmode","auto");
	secprop->Add_int("oplrate",22050);

	MSG_Add("SBLASTER_CONFIGFILE_HELP",
		"type -- Type of sblaster to emulate:none,sb1,sb2,sbpro1,sbpro2,sb16.\n"
		"base,irq,dma,hdma -- The IO/IRQ/DMA/High DMA address of the soundblaster.\n"
		"mixer -- Allow the soundblaster mixer to modify the dosbox mixer.\n"
		"oplmode -- Type of OPL emulation: auto,cms,opl2,dualopl2,opl3.\n"
		"           On auto the mode is determined by sblaster type.\n"
		"oplrate -- Sample rate of OPL music emulation.\n"
		);

	secprop=control->AddSection_prop("gus",&GUS_Init); 
	secprop->Add_bool("gus",true); 	
	secprop->Add_int("rate",22050);
	secprop->Add_hex("base",0x240);
	secprop->Add_int("irq1",5);
	secprop->Add_int("irq2",5);
	secprop->Add_int("dma1",3);
	secprop->Add_int("dma2",3);
	secprop->Add_string("ultradir","C:\\ULTRASND");

	MSG_Add("GUS_CONFIGFILE_HELP",
		"gus -- Enable the Gravis Ultrasound emulation.\n"
		"base,irq1,irq2,dma1,dma2 -- The IO/IRQ/DMA addresses of the \n"
		"           Gravis Ultrasound. (Same IRQ's and DMA's are OK.)\n"
		"rate -- Sample rate of Ultrasound emulation.\n"
		"ultradir -- Path to Ultrasound directory.  In this directory\n"
		"            there should be a MIDI directory that contains\n"
		"            the patch files for GUS playback.  Patch sets used\n"
		"            with Timidity should work fine.\n"
	);

	secprop=control->AddSection_prop("speaker",&PCSPEAKER_Init);
	secprop->Add_bool("pcspeaker",true);
	secprop->Add_int("pcrate",22050);
	secprop->AddInitFunction(&TANDYSOUND_Init);
	secprop->Add_int("tandyrate",22050);
	secprop->AddInitFunction(&DISNEY_Init);
	secprop->Add_bool("disney",true);

	MSG_Add("SPEAKER_CONFIGFILE_HELP",
		"pcspeaker -- Enable PC-Speaker emulation.\n"
		"pcrate -- Sample rate of the PC-Speaker sound generation.\n"
		"tandyrate -- Sample rate of the Tandy 3-Voice generation.\n"
		"             Tandysound emulation is present if machine is set to tandy.\n"
		"disney -- Enable Disney Sound Source emulation.\n"
	);
	secprop=control->AddSection_prop("bios",&BIOS_Init);
	secprop->AddInitFunction(&INT10_Init);

	/* All the DOS Related stuff, which will eventually start up in the shell */
	//TODO Maybe combine most of the dos stuff in one section like ems,xms
	secprop=control->AddSection_prop("dos",&DOS_Init);
	secprop->AddInitFunction(&XMS_Init);
	secprop->Add_bool("xms",true);
	secprop->AddInitFunction(&EMS_Init);
	secprop->Add_bool("ems",true);
	MSG_Add("DOS_CONFIGFILE_HELP",
		"xms -- Enable XMS support.\n"
		"ems -- Enable EMS support.\n"
	);
	// Mscdex
	secprop->AddInitFunction(&MSCDEX_Init);
#if C_MODEM
	secprop=control->AddSection_prop("modem",&MODEM_Init); 
	secprop->Add_bool("modem",false); 	
	secprop->Add_hex("comport",2); 
	secprop->Add_int("listenport",23);
	
	MSG_Add("MODEM_CONFIGFILE_HELP",
		"modem -- Enable virtual modem emulation.\n"
		"comport -- COM Port modem is connected to.\n"
		"listenport -- TCP Port the modem listens on for incoming connections.\n"
	);
#endif
#if C_DIRECTSERIAL
	secprop=control->AddSection_prop("directserial",&DIRECTSERIAL_Init);
	secprop->Add_bool("directserial", false);
	secprop->Add_hex("comport",1); 
	secprop->Add_string("realport", "COM1");
	secprop->Add_int("defaultbps", 1200);
	secprop->Add_string("parity", "N"); // Could be N, E, O
	secprop->Add_int("bytesize", 8); // Could be 5 to 8
	secprop->Add_int("stopbit", 1); // Could be 1 or 2
	MSG_Add("DIRECTSERIAL_CONFIGFILE_HELP",
		"directserial -- Enable serial passthrough support.\n"
		"comport -- COM Port inside DOSBox.\n"
		"realport -- COM Port on the Host.\n"
		"defaultbps -- Default BPS.\n"
		"parity -- Parity of the packets. This can be N, E or O.\n"
		"bytesize -- Size of each packet. This can be 5 or 8.\n"
		"stopbit -- The number of stopbits. This can be 1 or 2.\n"
	);
#endif
#if C_IPX
	secprop=control->AddSection_prop("ipx",&IPX_Init);
	secprop->Add_bool("ipx", false);
	MSG_Add("IPX_CONFIGFILE_HELP",
		"ipx -- Enable ipx over UDP/IP emulation.\n"
	);
#endif
	secline=control->AddSection_line("autoexec",&AUTOEXEC_Init);
	MSG_Add("AUTOEXEC_CONFIGFILE_HELP",
		"Lines in this section will be run at startup.\n"
	);
	control->SetStartUp(&SHELL_Init);
}

