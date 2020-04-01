/*
 *  Copyright (C) 2002-2019  The DOSBox Team
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


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
#include "ide.h"
#include "callback.h"
#include "inout.h"
#include "mixer.h"
#include "timer.h"
#include "dos_inc.h"
#include "setup.h"
#include "control.h"
#include "cross.h"
#include "programs.h"
#include "support.h"
#include "mapper.h"
#include "ints/int10.h"
#include "render.h"
#include "pci_bus.h"

Config * control;
MachineType machine;
SVGACards svgaCard;

/* The whole load of startups for all the subfunctions */
void MSG_Init(Section_prop *);
void LOG_StartUp(void);
void MEM_Init(Section *);
void PAGING_Init(Section *);
void IO_Init(Section * );
void CALLBACK_Init(Section*);
void PROGRAMS_Init(Section*);
//void CREDITS_Init(Section*);
void RENDER_Init(Section*);
void VGA_Init(Section*);
/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/
void vesa_refresh_Init(Section*);
void vesa_Patches_Init(Section*);
/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/
void DOS_Init(Section*);


void CPU_Init(Section*);

#if C_FPU
void FPU_Init(Section*);
#endif

void DMA_Init(Section*);

void MIXER_Init(Section*);
void MIDI_Init(Section*);
void HARDWARE_Init(Section*);

#if defined(PCI_FUNCTIONALITY_ENABLED)
void PCI_Init(Section*);
/* Voodoo /////////////////////////////////////////////////////////////////////////////////////////////*/
void VOODOO_Init(Section*);	
					   
#endif

/* IDE Emulation /////////////////////////////////////////////////////////////////////////////////////////////*/
void IDE_Primary_Init(Section*);
void IDE_Secondary_Init(Section*);
void IDE_Tertiary_Init(Section*);
void IDE_Quaternary_Init(Section*);
void IDE_Quinternary_Init(Section*);
void IDE_Sexternary_Init(Section*);
void IDE_Septernary_Init(Section*);
void IDE_Octernary_Init(Section*);
/* IDE Emulation /////////////////////////////////////////////////////////////////////////////////////////////*/


void KEYBOARD_Init(Section*);	//TODO This should setup INT 16 too but ok ;)
void JOYSTICK_Init(Section*);
void MOUSE_Init(Section*);
void SBLASTER_Init(Section*);
void GUS_Init(Section*);
void MPU401_Init(Section*);
void PCSPEAKER_Init(Section*);
void TANDYSOUND_Init(Section*);
void INNOVA_Init(Section*);
void DISNEY_Init(Section*);
void SERIAL_Init(Section*);


#if C_IPX
void IPX_Init(Section*);
#endif

#ifdef C_NE2000
void NE2K_Init(Section* sec);
#endif

void SID_Init(Section* sec);

void PIC_Init(Section*);
void TIMER_Init(Section*);
void BIOS_Init(Section*);
void DEBUG_Init(Section*);
void CMOS_Init(Section*);

void MSCDEX_Init(Section*);
void DRIVES_Init(Section*);
void CDROM_Image_Init(Section*);

/* Dos Internal mostly */
void EMS_Init(Section*);
void XMS_Init(Section*);

void DOS_KeyboardLayout_Init(Section*);

void AUTOEXEC_Init(Section*);
void SHELL_Init(void);

void INT10_Init(Section*);

static LoopHandler * loop;

bool SDLNetInited;

static Bit32u ticksRemain;
static Bit32u ticksLast;
static Bit32u ticksAdded;
Bit32s ticksDone;
Bit32u ticksScheduled;
bool ticksLocked;
void increaseticks();
bool mono_cga=false;

bool CPU_FastForward = false;

const char* GFX_Type="Not Specified";

extern void GFX_SetTitle(Bit32s cycles,int frameskip,bool paused);

static Bitu Normal_Loop(void) {
	Bits ret;
	while (1) {
		if (PIC_RunQueue()) {
			ret = (*cpudecoder)();
			if (GCC_UNLIKELY(ret<0)) return 1;
			if (ret>0) {
				if (GCC_UNLIKELY(ret >= CB_MAX)) return 0;
				Bitu blah = (*CallBack_Handlers[ret])();
				if (GCC_UNLIKELY(blah)) return blah;
			}
#if defined(C_DEBUG)
			if (DEBUG_ExitLoop()) return 0;
#endif
		} else {
			GFX_Events();
			if (ticksRemain>0) {
				TIMER_AddTick();
				ticksRemain--;
			} else {increaseticks();return 0;}
		}
	}
}

//For trying other delays
#define wrap_delay(a) SDL_Delay(a)

void increaseticks() { //Make it return ticksRemain and set it in the function above to remove the global variable.
	if (GCC_UNLIKELY(ticksLocked)) { // For Fast Forward Mode
		ticksRemain=5;
		/* Reset any auto cycle guessing for this frame */
		ticksLast = GetTicks();
		ticksAdded = 0;
		ticksDone = 0;
		ticksScheduled = 0;
		return;
	}

	static Bit32s lastsleepDone = -1;
	static Bitu sleep1count = 0;
	
	Bit32u ticksNew;
	ticksNew = GetTicks();
	ticksScheduled += ticksAdded;
	if (ticksNew <= ticksLast) { //lower should not be possible, only equal.
		ticksAdded = 0;

		if (!CPU_CycleAutoAdjust || CPU_SkipCycleAutoAdjust || sleep1count < 3) {
			wrap_delay(1);
		} else {
			/* Certain configurations always give an exact sleepingtime of 1, this causes problems due to the fact that
			   dosbox keeps track of full blocks.
			   This code introduces some randomness to the time slept, which improves stability on those configurations
			 */
			static const Bit32u sleeppattern[] = { 2, 2, 3, 2, 2, 4, 2};
			static Bit32u sleepindex = 0;
			if (ticksDone != lastsleepDone) sleepindex = 0;
			wrap_delay(sleeppattern[sleepindex++]);
			sleepindex %= sizeof(sleeppattern) / sizeof(sleeppattern[0]);
		}
		Bit32s timeslept = GetTicks() - ticksNew;
		// Count how many times in the current block (of 250 ms) the time slept was 1 ms
		if (CPU_CycleAutoAdjust && !CPU_SkipCycleAutoAdjust && timeslept == 1) sleep1count++;
		lastsleepDone = ticksDone;

		// Update ticksDone with the time spent sleeping
		ticksDone -= timeslept;
		if (ticksDone < 0)
			ticksDone = 0;
		return; //0

		// If we do work this tick and sleep till the next tick, then ticksDone is decreased, 
		// despite the fact that work was done as well in this tick. Maybe make it depend on an extra parameter.
		// What do we know: ticksRemain = 0 (condition to enter this function)
		// ticksNew = time before sleeping
		
		// maybe keep track of sleeped time in this frame, and use sleeped and done as indicators. (and take care of the fact there
		// are frames that have both.
	}

	//TicksNew > ticksLast
	ticksRemain = ticksNew-ticksLast;
	ticksLast = ticksNew;
	ticksDone += ticksRemain;
	if ( ticksRemain > 20 ) {
//		LOG(LOG_MISC,LOG_ERROR)("large remain %d",ticksRemain);
		ticksRemain = 20;
	}
	ticksAdded = ticksRemain;

	// Is the system in auto cycle mode guessing ? If not just exit. (It can be temporary disabled)
	if (!CPU_CycleAutoAdjust || CPU_SkipCycleAutoAdjust) return;
	
	if (ticksScheduled >= 250 || ticksDone >= 250 || (ticksAdded > 15 && ticksScheduled >= 5) ) {
		if(ticksDone < 1) ticksDone = 1; // Protect against div by zero
		/* ratio we are aiming for is around 90% usage*/
		Bit32s ratio = (ticksScheduled * (CPU_CyclePercUsed*90*1110/100/100+1)) / ticksDone;
		Bit32s new_cmax = CPU_CycleMax;

		Bit64s cproc = (Bit64s)CPU_CycleMax * (Bit64s)ticksScheduled;
		double ratioremoved = 0.0; //increase scope for logging
		if (cproc > 0) {
			/* ignore the cycles added due to the IO delay code in order
			   to have smoother auto cycle adjustments */
			ratioremoved = (double) CPU_IODelayRemoved / (double) cproc;
			if (ratioremoved < 1.0) {
				double ratio_not_removed = 1 - ratioremoved;
				ratio = (Bit32s)((double)ratio * ratio_not_removed);
				/* Don't allow very high ratio which can cause us to lock as we don't scale down
				 * for very low ratios. High ratio might result because of timing resolution */
				if (ticksScheduled >= 250 && ticksDone < 10 && ratio > 16384)
					ratio = 16384;

				// Limit the ratio even more when the cycles are already way above the realmode default.
				if (ticksScheduled >= 250 && ticksDone < 10 && ratio > 5120 && CPU_CycleMax > 50000)
					ratio = 5120;
				
				// When downscaling multiple times in a row, ensure a minimum amount of downscaling
				if (ticksAdded > 15 && ticksScheduled >= 5 && ticksScheduled <= 20 && ratio > 800)
					ratio = 800;
				
				if (ratio <= 1024) {
					// ratio_not_removed = 1.0; //enabling this restores the old formula
					double r = (1.0 + ratio_not_removed) /(ratio_not_removed + 1024.0/(static_cast<double>(ratio)));
					new_cmax = 1 + static_cast<Bit32s>(CPU_CycleMax * r);
				} else {
					Bit64s ratio_with_removed = (Bit64s) ((((double)ratio - 1024.0) * ratio_not_removed) + 1024.0);
					Bit64s cmax_scaled = (Bit64s)CPU_CycleMax * ratio_with_removed;
					new_cmax = (Bit32s)(1 + (CPU_CycleMax >> 1) + cmax_scaled / (Bit64s)2048);
				}
			}
		}

		if (new_cmax < CPU_CYCLES_LOWER_LIMIT)
			new_cmax = CPU_CYCLES_LOWER_LIMIT;
		/*
		LOG(LOG_MISC,LOG_ERROR)("cyclelog: current %06d   cmax %06d   ratio  %05d  done %03d   sched %03d Add %d rr %4.2f",
 			CPU_CycleMax,
 			new_cmax,
 			ratio,
 			ticksDone,
 			ticksScheduled,
			ticksAdded,
			ratioremoved);			
		*/

		/* ratios below 1% are considered to be dropouts due to
		   temporary load imbalance, the cycles adjusting is skipped */
		if (ratio > 10) {
			/* ratios below 12% along with a large time since the last update
			   has taken place are most likely caused by heavy load through a
			   different application, the cycles adjusting is skipped as well */
			if ((ratio > 120) || (ticksDone < 700)) {
				CPU_CycleMax = new_cmax;
				if (CPU_CycleLimit > 0) {
					if (CPU_CycleMax > CPU_CycleLimit) CPU_CycleMax = CPU_CycleLimit;
				} else if (CPU_CycleMax > 2000000) CPU_CycleMax = 2000000; //Hardcoded limit was 2000000, if no limit was specified. 
			}
		}

		//Reset cycleguessing parameters.		
		CPU_IODelayRemoved = 0;
		ticksDone = 0;
		ticksScheduled = 0;
		lastsleepDone = -1;
		sleep1count = 0;		
	} else if (ticksAdded > 15) {
		/* ticksAdded > 15 but ticksScheduled < 5, lower the cycles
		   but do not reset the scheduled/done ticks to take them into
		   account during the next auto cycle adjustment */
		CPU_CycleMax /= 3;
		if (CPU_CycleMax < CPU_CYCLES_LOWER_LIMIT)
			CPU_CycleMax = CPU_CYCLES_LOWER_LIMIT;
	} //if (ticksScheduled >= 250 || ticksDone >= 250 || (ticksAdded > 15 && ticksScheduled >= 5) )
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

static void DOSBOX_UnlockSpeed( bool pressed ) {
	static bool autoadjust = false;
	if (pressed) {
		CPU_FastForward = true;
		LOG_MSG("SPEED: Fast Forward ON");
		ticksLocked = true;
		if (CPU_CycleAutoAdjust) {
			autoadjust = true;
			CPU_CycleAutoAdjust = false;
			CPU_CycleMax /= 3;
			if (CPU_CycleMax<1000) CPU_CycleMax=1000;
		}
	} else {
		LOG_MSG("SPEED: Fast Forward OFF");
		ticksLocked = false;
		CPU_FastForward = false;
		if (autoadjust) {
			autoadjust = false;
			CPU_CycleAutoAdjust = true;
		}
	}
	GFX_SetTitle(-1,-1,false);
}

static void DOSBOX_UnlockSpeed2( bool pressed ) {
	if (pressed) {
		CPU_FastForward =! CPU_FastForward;
		ticksLocked =! ticksLocked;
		DOSBOX_UnlockSpeed(ticksLocked?true:false);
	}
	GFX_SetTitle(-1,-1,false);	
}


static void DOSBOX_RealInit(Section * sec) {
	Section_prop * section=static_cast<Section_prop *>(sec);
	/* Initialize some dosbox internals */

	ticksRemain=0;
	ticksLast=GetTicks();
	ticksLocked = false;
	DOSBOX_SetLoop(&Normal_Loop);
	MSG_Init(section);

	MAPPER_AddHandler(DOSBOX_UnlockSpeed, MK_f12, MMOD2,"speedlock","Speedlock");
	MAPPER_AddHandler(DOSBOX_UnlockSpeed2, MK_f12, MMOD1|MMOD2,"speedlock2","Speedlock2");	
	std::string cmd_machine;
	if (control->cmdline->FindString("-machine",cmd_machine,true)){
		//update value in config (else no matching against suggested values
		section->HandleInputline(std::string("machine=") + cmd_machine);
	}

	std::string mtype(section->Get_string("machine"));
	svgaCard = SVGA_None;
	machine = MCH_VGA;
	int10.vesa_nolfb = false;
	int10.vesa_oldvbe = false;
	/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/
	int10.vesa_no24bpp = false;
	/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/
	if      (mtype == "cga")      { machine = MCH_CGA; mono_cga = false; }
	else if (mtype == "cga_mono") { machine = MCH_CGA; mono_cga = true; }
	else if (mtype == "tandy")    { machine = MCH_TANDY; }
	else if (mtype == "pcjr")     { machine = MCH_PCJR; }
	else if (mtype == "hercules") { machine = MCH_HERC; }
	else if (mtype == "ega")      { machine = MCH_EGA; }
//	else if (mtype == "vga")          { svgaCard = SVGA_S3Trio; }
	else if (mtype == "svga_s3")       { svgaCard = SVGA_S3Trio; }
	else if (mtype == "vesa_nolfb")   { svgaCard = SVGA_S3Trio; int10.vesa_nolfb = true;}
	else if (mtype == "vesa_oldvbe")   { svgaCard = SVGA_S3Trio; int10.vesa_oldvbe = true;}
	else if (mtype == "svga_et4000")   { svgaCard = SVGA_TsengET4K; }
	else if (mtype == "svga_et3000")   { svgaCard = SVGA_TsengET3K; }
//	else if (mtype == "vga_pvga1a")   { svgaCard = SVGA_ParadisePVGA1A; }
	else if (mtype == "svga_paradise") { svgaCard = SVGA_ParadisePVGA1A; }
	else if (mtype == "vgaonly")      { svgaCard = SVGA_None; }
	/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/
	else if (mtype == "vesa_no24bpp") { svgaCard = SVGA_S3Trio; int10.vesa_no24bpp = true;}
	else if (mtype == "vesa_nolfb_no24bpp") { svgaCard = SVGA_S3Trio; int10.vesa_nolfb = true; int10.vesa_no24bpp = true;}
	else E_Exit("DOSBOX:Unknown machine type %s",mtype.c_str());
	
		static char name[14];	
		name[13] = 0;
		if (!strlen(name)) strcpy(name,mtype.c_str());
		for(Bitu i = 0;i < 12;i++) { //Don't put garbage in the title bar. Mac OS X doesn't like it
			if (name[i] == 0) break;
			if ( !isprint(*reinterpret_cast<unsigned char*>(&name[i])) ) name[i] = '?';
		}
		GFX_Type = name;
	GFX_SetTitle(-1,-1,false);
	}


void DOSBOX_Init(void) {
	Section_prop * secprop;
	Section_line * secline;
	Prop_int* Pint;
	Prop_hex* Phex;
	Prop_string* Pstring;
	Prop_float* Pfloat;
	Prop_double* Pdouble;	
	Prop_bool* Pbool;
	Prop_multival* Pmulti;
	Prop_multival_remain* Pmulti_remain;

	SDLNetInited = false;

	// Some frequently used option sets
	const char *rates[] = 	 {   "49716", "48000", "44100", "32000","22050", "16000", "11025", "8000", 0 };
	const char *oplrates[] = {   "49716", "48000", "44100", "32000","22050", "16000", "11025", "8000", 0 };
	const char *fluidrates[]= {  "96000", "49716", "48000", "44100", "32000","22050", 0 };	
	const char *ios[] = { "220", "240", "260", "280", "2a0", "2c0", "2e0", "300", 0 };
	const char *irqssb[] = { "7", "5", "3", "9", "10", "11", "12", 0 };
	const char *dmassb[] = { "1", "5", "0", "3", "6", "7", 0 };
	const char *iosgus[] = { "240", "220", "260", "280", "2a0", "2c0", "2e0", "300", 0 };
	const char *irqsgus[] = { "5", "3", "7", "9", "10", "11", "12", 0 };
	const char *dmasgus[] = { "3", "0", "1", "5", "6", "7", 0 };
	const char *innoqal[] = { "0" , "1", "2", "3", 0};
	const char *innobas[] = { "280", "2a0", "2c0", "2e0", 0 };
	const char *pcmode[] = { "old", "new", 0 };		
	const char *cgapalmodes[] = { "green", "amber", "grey", "paperwhite", 0 };

	

	/* Setup all the different modules making up DOSBox */
	const char* machines[] = {
		"hercules", "cga", "cga_mono", "tandy", "pcjr", "ega",
		"vgaonly", "svga_s3", "svga_et3000", "svga_et4000",
		"svga_paradise", "vesa_nolfb", "vesa_oldvbe", "vesa_no24bpp","vesa_nolfb_no24bpp",0 };
	secprop=control->AddSection_prop("dosbox",&DOSBOX_RealInit);
	Pstring = secprop->Add_path("language",Property::Changeable::Always,"");
	Pstring->Set_help(	"================================================================================================\n"
						"Select another language file.");

	Pstring = secprop->Add_string("machine",Property::Changeable::OnlyAtStart,"svga_s3");
	Pstring->Set_values(machines);
	Pstring->Set_help(  "================================================================================================\n"
	                    "The type of machine DOSBox tries to emulate.");				

	Pstring = secprop->Add_path("captures",Property::Changeable::Always,"\\CAPTURE");
	Pstring->Set_help(  "================================================================================================\n"
	                    "Directory where things like wave, midi, screenshot get captured.");

#if defined(C_DEBUG)
	LOG_StartUp();
#endif

	secprop->AddInitFunction(&IO_Init);//done
	secprop->AddInitFunction(&PAGING_Init);//done
	secprop->AddInitFunction(&MEM_Init);//done
	secprop->AddInitFunction(&HARDWARE_Init);//done
	Pint = secprop->Add_int("memsize", Property::Changeable::WhenIdle,16);
	Pint->SetMinMax(1,3072);
	Pint->Set_help(     "================================================================================================\n"
		                "Amount of memory DOSBox has in megabytes.\n"
		                "This value is best left at its default to avoid problems with some games, though few games\n"
		                "might require a higher value. There is generally no speed advantage when raising this value.\n"
						"DOSBox Current Support Max Memory MS-DOS    : 256MB\n"
						"DOSBox Current Support Max Memory Windows 95: 512MB\n"
						"DOSBox Current Support Max Memory Windows 98: 1GB (1024MB)\n"
						"Max Memory for DOS Games (The don't like more):\n"
						"- Goblins 3             : ~32MB\n"
						"- Sleepwalker           : ~ 4MB");
		

	Pint = secprop->Add_int("memsvga3", Property::Changeable::WhenIdle,0);	
	Pint->SetMinMax(0,32);
	Pint->Set_help(     "================================================================================================\n"
		                "Amount of memory for the machine SVGA_S3. Possible Values are 0 (Dosbox Default) / 1 >> 32mb\n"
						"Note: If a Program/ Game or Scene Demo comes with a Message e.q. Not enough Video Memory. You can\n"
						"force this to use amount Memory. Note: The bigger the memory, the longer the Resolution Index list\n"
						"Tip: If the screen is Garbage or looks not normal try vgaonly. If the screen flicker use vesa_nolfb\n"
						"or the tool on Z: named nolfb.com");
			
	
	Pstring = secprop->Add_string("colormode_cga_mono", Property::Changeable::Always,"paperwhite");
	Pstring->Set_values(cgapalmodes);
	Pstring->Set_help(  "================================================================================================\n"
	                    "Only for Machine: CGA Mono. Start with your favorite Pal CGA Mono Color Mode.");	
		
	Pint = secprop->Add_int("cobrimode_cga_mono", Property::Changeable::WhenIdle,0);
	Pint->SetMinMax(0,1);
	Pint->Set_help(     "================================================================================================\n"
	                    "Only for Machine: CGA Mono. Change Contrast/Brigthness Variant. Values are 0 or 1.");
		
	Pstring = secprop->Add_string("colormode_hercules", Property::Changeable::Always,"paperwhite");
	Pstring->Set_values(cgapalmodes);
	Pstring->Set_help(  "================================================================================================\n"
	                    "Only for Machine: Hercules. Start with your favorite Hercules Color.");	

	Pint = secprop->Add_int("CutVesaResolutionIndex",Property::Changeable::Always,0);
	Pint->SetMinMax(0,32);	
	Pint->Set_help(		  "================================================================================================\n"
						  "If nonzero, the VESA modelist is capped so that it contains no more than the specified number of\n"
						  "video modes did you set. Set this option to a value between 8 to 32 if the DOS application has\n"
						  "problems with long modelists or a fixed buffer for querying modes. Such programs may crash if\n"
						  "given the entire modelist supported by DOSBox.\n"
						  "Warcraft II by Blizzard: ..Set to a value between 8 and 16. This game has a fixed buffer that it\n"
						  "                           reads the modelist into. DOSBox's normal modelist is too long and the\n"
			              "                           game will overrun the buffer and crash without this setting.\n"
						  "Pyl (Software Mode)      ..Set a Value between 24 and 34\n"
						  "Quake 1.06 (DOS)         ..Set 150\n"
						  "I'm Include a tool from VesaLib Archiv, CHKVESA.COM. You can list the Resolution with this tool");					  
	
	Pbool = secprop->Add_bool("LogVesaResolutionIndex",Property::Changeable::Always,false);	
	Pbool->Set_help(		  "================================================================================================\n"
						  "Log the VESA Video mode List to 'dosbox_log'.");
						  				 
	Pbool = secprop->Add_bool("S3Screen_Fix_320x240x8",Property::Changeable::Always,false);
	Pbool->Set_help(     "================================================================================================\n"
	                     "This Patch and Change the Resolution at index 0x153. From 320x240x4 bit to 320x200x8 Bit. Use\n"
						 "this if your Game or Scene Demo Crashed with this Mode. Example? This fixed: (Default false)\n"
						 "- Scene Demo: RealTech 'Countdown'");	
						 
	Pbool = secprop->Add_bool("S3Screen_Fix_400x300x8",Property::Changeable::Always,false);
	Pbool->Set_help(     "================================================================================================\n"
	                     "This Patch and Change the Resolution at index 0x166. From 400x300x8 bit to 320x480x8 Bit. Use\n"
						 "this if your Game or Scene Demo Crashed with this Mode. Example? This fixed: (Default false)\n"
						 "- Scene Demo: Habitual Demo");

	Pbool = secprop->Add_bool("S3Screen_Fix_640x480x15",Property::Changeable::Always,false);
	Pbool->Set_help(     "================================================================================================\n"
	                     "This Patch and Change the Resolution Stretch at ndex 0x110. Example? This fixed: (Default false)\n"
						 "- Game: Alien Trilogy [DOS, 16Bit Screenmode]");	
						 
	Pbool = secprop->Add_bool("S3Screen_Fix_640x480x16",Property::Changeable::Always,false);
	Pbool->Set_help(     "================================================================================================\n"
	                     "This Patch and Change the Resolution at index 0x211. From 640x480x16 bit to 320x480x8 Bit. Use\n"
						 "this if your Game or Scene Demo is stretched with this Mode. Example? This fixed: (Default false)\n"
						 "- Game: 3 Skulls of the Toltecs [Gold, Windows]");							 

	Pbool = secprop->Add_bool("VesaVbe1.2_32Bit_Modes",Property::Changeable::Always,true);
	Pbool->Set_help(     "================================================================================================\n"
	                     "This Change the Vesa 1.2 High Color Modes to 8:8:8 (24-bit) (0x10F, 0x112, 0x115, 0x118).\n"
						 "Note: Version 1.2 of the VESA BIOS Extensions explicitly states 24Bit Color Modes not 32Bit. Set\n"
						 "this to false use 24Bit Modes. Default is True, DOSBox Original Settings. Example? This fixed:\n"
						 "- Scene Demo: COMA 'Parhaat' 1997 demo");						 
		
		
	secprop->AddInitFunction(&CALLBACK_Init);
	secprop->AddInitFunction(&PIC_Init);//done
	secprop->AddInitFunction(&PROGRAMS_Init);
	secprop->AddInitFunction(&TIMER_Init);//done
	secprop->AddInitFunction(&CMOS_Init);//done

	secprop=control->AddSection_prop("render",&RENDER_Init,true);
	Pint = secprop->Add_int("frameskip",Property::Changeable::Always,0);
	Pint->SetMinMax(0,10);
	Pint->Set_help(     "================================================================================================\n"
	                    "How many frames DOSBox skips before drawing one.");

	Pbool = secprop->Add_bool("aspect",Property::Changeable::Always,true);
	Pbool->Set_help(    "================================================================================================\n"
	                    "Do aspect correction, if your output method doesn't support scaling this can slow things down!\n"
	                    "Note: For 3DFX set to false");

	Pmulti = secprop->Add_multi("scaler",Property::Changeable::Always," ");
	Pmulti->SetValue("none");
	Pmulti->Set_help(   "================================================================================================\n"
	                    "Scaler used to enlarge/enhance low resolution modes. If 'forced' is appended, then the scaler\n"
	                    "will be used even if the result might not be desired.To use with output method: surface, texture\n"
					    "texturenb\n"
					    "Opengl/Openglnb only for shader. For Scanline Filter (scan, tv, rgb) set Aspect to False");
	Pstring = Pmulti->GetSection()->Add_string("type",Property::Changeable::Always,"none");
	
	Pbool = secprop->Add_bool("debug",Property::Changeable::Always,false);
	Pbool->Set_help(    "================================================================================================\n"
	                    "Debug Output for Modes, Aspect Ratios und Resoultion Handle. For the Developer");	
						
	/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/	
	secprop=control->AddSection_prop("vesa_refresh",&vesa_refresh_Init,true);
	Pint = secprop->Add_int("Vesa_Graphic_Mode_512x384",Property::Changeable::Always,70);
	Pint->Set_help(     "================================================================================================\n"
	                    "Set the Refresh rates for VESA graphics modes (Default Refresh Rate: 70)");
	
	Pint = secprop->Add_int("Vesa_Graphic_Mode_640x350",Property::Changeable::Always,70);
	Pint->Set_help(    "Default Refresh Rate: 70");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_640x400",Property::Changeable::Always,70);
	Pint->Set_help(    "Default Refresh Rate: 70");	
	Pint = secprop->Add_int("Vesa_Graphic_Mode_640x480",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_720x480",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60");	
	Pint = secprop->Add_int("Vesa_Graphic_Mode_800x600",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1024x768",Property::Changeable::Always,60);	
	Pint->Set_help(    "Default Refresh Rate: 50");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1188x344",Property::Changeable::Always,70);
	Pint->Set_help(    "Default Refresh Rate: 70 --> Note: VESA text mode  132x43 (9x8 character cell)");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1188x400",Property::Changeable::Always,70);
	Pint->Set_help(    "Default Refresh Rate: 70 --> Note: VESA text modes 132x25 & 132x50 (9x16 & 9x8 character cells).");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1188x480",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60 --> Note: VESA text mode 132x60 (9x8 character cell).");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1152x864",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60");
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1280x960",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60");	
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1280x1024",Property::Changeable::Always,60);	
	Pint->Set_help(    "Default Refresh Rate: 60");	
	Pint = secprop->Add_int("Vesa_Graphic_Mode_1600x1200",Property::Changeable::Always,60);
	Pint->Set_help(    "Default Refresh Rate: 60\n"
	                   "================================================================================================");


	/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/			
	const char *scalers[] = {
		"none", "normal2x", "normal3x",
#if RENDER_USE_ADVANCED_SCALERS>2
		"advmame2x", "advmame3x", "advinterp2x", "advinterp3x", "hq2x", "hq3x", "2xsai", "super2xsai", "supereagle",
#endif
#if RENDER_USE_ADVANCED_SCALERS>0
		"tv2x", "tv3x", "rgb2x", "rgb3x", "scan2x", "scan3x",
#endif
		0 };
	Pstring->Set_values(scalers);

	const char* force[] = { "", "forced", 0 };
	Pstring = Pmulti->GetSection()->Add_string("force",Property::Changeable::Always,"");
	Pstring->Set_values(force);

	secprop=control->AddSection_prop("cpu",&CPU_Init,true);//done
	const char* cores[] = { "auto",
#if (C_DYNAMIC_X86) || (C_DYNREC)
		"dynamic",
#endif
		"normal", "simple",0 };
	Pstring = secprop->Add_string("core",Property::Changeable::WhenIdle,"auto");
	Pstring->Set_values(cores);
	Pstring->Set_help(   "================================================================================================\n"
	                     "CPU Core used in emulation. 'Auto' will switch to Dynamic if available and appropriate.");

	const char* cputype_values[] = { "auto", "386", "386_prefetch", "386_slow", "486_slow", "pentium_slow", "pentiumpro_slow" , 0};
	Pstring = secprop->Add_string("cputype",Property::Changeable::Always,"auto");
	Pstring->Set_values(cputype_values);
	Pstring->Set_help(   "================================================================================================\n"
	                     "CPU Type used in emulation. auto is the fastest choice.\n"
						 "Pentium Slow   : CPUID 513, Support FPU + TimeStamp/RDTSC\n"
						 "PentiumPro Slow: CPUID <look cpuident and set>. Additional set Features\n"
						 "If Win9x at the beginning crasht. You have set the false cpuident.");

	Pint = secprop->Add_int("cpuident",Property::Changeable::Always,612);
	Pint->SetMinMax(1,1000000);
	Pint->Set_help(         "================================================================================================\n"
	                        "CPU Ident for Pentium Pro Settings (Default 612)\n"
							"Intel Pentium          : 513, 543*\n"
							"Intel Pentium pro      : 610, 611, 612, 616\n"
							"AMD Athlon             : 621\n"	
							"AMD Duron              : 631\n"
							"Intel Pentium II       : 634, 650\n"
							"AMD Athlon             : 642\n"	
							"Intel Celeron          : 660\n"
							"Intel Pentium III      : 672, 673\n"
							"Intel Celeron/Xeon     : 683\n"									
							"Intel Pentium Overdrive: 1531\n");
		
	
	Pmulti_remain = secprop->Add_multiremain("cycles",Property::Changeable::Always," ");
	Pmulti_remain->Set_help("================================================================================================\n"
							"Amount of instructions DOSBox tries to emulate each millisecond. Setting this value too high\n"
							"results in sound dropouts and lags. Cycles can be set in 3 ways:\n"
							"  'auto'          tries to guess what a game needs. It usually works, but can fail for certain\n"
							"                  games.\n"
							"  'fixed #number' will set a fixed amount of cycles. This is what you usually need if'auto'fails\n"
							"                  (Example: fixed 4000).\n"
							"  'max'           will allocate as much cycles as your computer is able to handle. The next\n"
							"                  values based on The Speed Test Program, v1.14. Note: These values give an\n"
							"                  approximate guide value to CPU_CycleMax aka Max Cyles\n"
							"  'max 200%'      Max Cycles can be overpowered or slowed downed. The Default Max Cycles is 100%\n"
							"                  Max Cycles Values: 0 to 500%."
							"  'i8088_477'     CPU 8088    with a speed from 4,77mhz\n"
							"  'i8088_716'     CPU 8088    with a speed from 7,16mhz\n"
							"  'i8088_954'     CPU 8088    with a speed from 9,54mhz\n"
							"  'i286_10'       CPU 268     with a speed from 10mhz\n"		
							"  'i286_12'       CPU 268     with a speed from 12mhz\n"
							"  'i286_16'       CPU 268     with a speed from 16mhz\n"
							"  'i286_20'       CPU 268     with a speed from 20mhz\n"		
							"  'i286_25'       CPU 268     with a speed from 25mhz\n"			
							"  'i386dx_25'     CPU 368DX   with a speed from 25mhz\n"
							"  'i386dx_33'     CPU 368DX   with a speed from 33mhz\n"		
							"  'i386dx_40'     CPU 368DX   with a speed from 40mhz\n"		
							"  'i486sx_25'     CPU 468SX   with a speed from 25mhz\n"		
							"  'i486dx_33'     CPU 468DX   with a speed from 33mhz\n"		
							"  'i486sx_33'     CPU 468SX   with a speed from 33mhz\n"
							"  'i486sx_40'     CPU 468SX   with a speed from 33mhz (40mhz) SL Enhanced\n"
							"  'i486dx_50'     CPU 468DX   with a speed from 50mhz\n"
							"  'i486dx2_66'    CPU 468DX   with a speed from 66mhz\n"	
							"  'i486sx2_80'    CPU 468DX2  with a speed from 50mhz (80mhz)\n"
							"  'i486dx2_100'   CPU 468DX2  with a speed from 80mhz (100mhz)\n"
							"  'i486dx4_100'   CPU 468DX4  with a speed from 100mhz\n"
							"  'i486dx4_120'   CPU 468DX4  with a speed from 120mhz\n"		
							"  'p60'           CPU Pentium with a speed from 60mhz (66mhz)\n"			
							"  'p75'           CPU Pentium with a speed from 75mhz (90mhz)\n"			
							"  'p100'          CPU Pentium with a speed from 100mhz\n");

	const char* cyclest[] = { "auto","fixed","max","i8088_477","i8088_716","i8088_954","i286_10","i286_12","i286_16","i286_20","i286_25",												   
							  "i386dx_25","i386dx_33","i386dx_40","i486sx_25","i486dx_33","i486sx_33","i486sx_40","i486dx_50","i486dx2_66",												   
							  "i486sx2_80","i486dx2_100","i486dx4_100","i486dx4_120","p60","p75","p100","%u",0 };
	Pstring = Pmulti_remain->GetSection()->Add_string("type",Property::Changeable::Always,"auto");
	Pmulti_remain->SetValue("auto");
	Pstring->Set_values(cyclest);

	Pstring = Pmulti_remain->GetSection()->Add_string("parameters",Property::Changeable::Always,"");

	Pint = secprop->Add_int("cycleup",Property::Changeable::Always,10);
	Pint->SetMinMax(1,1000000);
	Pint->Set_help(         "================================================================================================\n"
	                        "Amount of cycles to decrease/increase with keycombos.(CTRL-F11/CTRL-F12)");

	Pint = secprop->Add_int("cycledown",Property::Changeable::Always,20);
	Pint->SetMinMax(1,1000000);
	Pint->Set_help(         "================================================================================================\n"
	                        "Setting it lower than 100 will be a percentage.");

	Pbool = secprop->Add_bool("speedmeter",Property::Changeable::Always,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "A simple speed meter. Allows to tune cycles very reliably to the maximum possible, while having\n"
					        "no impact on performance. Sourceforge: #75 speed meter, Author Moe");
														
	Pbool = secprop->Add_bool("Enable-MMX",Property::Changeable::Always,false);
	Pbool->Set_help(        "================================================================================================\n"
							"Only use this with CPUID 543. MMX Added from Daum's Version. Set this to true can slowly things");
							
	Pbool = secprop->Add_bool("Enable-SSE1",Property::Changeable::Always,false);
	Pbool->Set_help(        "================================================================================================\n"
							"Only use this with CPUID 543. Testing Only");

	Pbool = secprop->Add_bool("Enable-SSE2",Property::Changeable::Always,false);
	Pbool->Set_help(        "================================================================================================\n"
							"Only use this with CPUID 543. Testing Only");							
							
	Pbool = secprop->Add_bool("Enable-OPTS",Property::Changeable::Always,false);
	Pbool->Set_help(        "================================================================================================\n"
							"Only use this with CPUID 543. Testing Only (POPCNT, MOVBE, CMPXCHG16B, SEP");	
							
	Pbool = secprop->Add_bool("Enable-3DNOW",Property::Changeable::Always,false);
	Pbool->Set_help(        "================================================================================================\n"
							"Only use this with CPUID 543. Testing Only");								
	
#if C_FPU
	secprop->AddInitFunction(&FPU_Init);
#endif
	secprop->AddInitFunction(&DMA_Init);//done
	secprop->AddInitFunction(&VGA_Init);
	secprop->AddInitFunction(&KEYBOARD_Init);

	/*************************************** IDE emulation options and setup ***************************************************/
	for (size_t i=0;i < MAX_IDE_CONTROLLERS;i++) {
		secprop=control->AddSection_prop(ide_names[i],ide_inits[i],false);//done
					/* Primary and Secondary are on by default, Teritary and Quaternary are off by default.
					 * Throughout the life of the IDE interface it was far more common for a PC to have just
					 * a Primary and Secondary interface */		
					
					/* ( Property::Changeable::OnlyAtStart,(i < 2) false?true: )
					 * Dont Enbale IDE by Default
					 */
					 
		Pbool = secprop->Add_bool("enable",Property::Changeable::OnlyAtStart,false);
		
		if (i == 0) {
		Pbool->Set_help(        "================================================================================================\n"
								"Enable IDE Interface");
		}
		
		Pbool = secprop->Add_bool("ISA-PNP Enumerate IDE",Property::Changeable::OnlyAtStart,false);
		if ( i == 0 ) {
			Pbool->Set_help(    "================================================================================================\n"
								"List IDE device in ISA PnP BIOS enumeration");	
		} 

		Pint = secprop->Add_int("IDE IRQ",Property::Changeable::WhenIdle,0/*use IDE default*/);
		if ( i == 0 ) {
			Pint->Set_help(     "================================================================================================\n"
							    "IRQ used by IDE controller. Set to 0 for default. WARNING: Setting the IRQ to non-standard values\n"
								"will not work unless the guest OS is using the ISA PnP BIOS to detect the IDE controller.\n"
								"Setting the IRQ to one already occupied by another device or IDE controller will trigger \"resource\n"
								"conflict\" errors in Windows 95. Using IRQ 9, 12, 13, or IRQ 2-7 may cause problems with MS-DOS\n"
								"CD-ROM drivers.");
		}
		
		Phex = secprop->Add_hex("IDE Base",Property::Changeable::WhenIdle,0/*use IDE default*/);
		if (i == 0) {
			Pint->Set_help(     "================================================================================================\n"
							    "Base I/O port for IDE controller. Set to 0 for default. WARNING: Setting the I/O port to non-\n"
								"standard values will not work unless the guest OS is using the ISA PnP BIOS to detect the IDE\n"
						        "controller. Using any port other than 1F0, 170, 1E8 or 168 can prevent MS-DOS CD-ROM drivers from\n"
								"detecting the IDE controller.");										
		}
		
		
		Phex = secprop->Add_hex("IDE Base (alt)",Property::Changeable::WhenIdle,0/*use IDE default*/);
		if (i == 0) {
			Pint->Set_help(     "================================================================================================\n"
							    "Alternate I/O port for IDE controller (alt status, etc). Set to 0 for default. WARNING: Setting\n"
								"the I/O port to non-standard values will not work unless the guest OS is using the ISA PnP BIOS\n"
								"to detect the IDE controller. For best compatability set this value to io+0x106.\n"
								"For example \"IDE Base=1F0\" and \"IDE Base (alt)=3F6\".");
		}
		Pbool = secprop->Add_bool("Int13FakeIO",Property::Changeable::WhenIdle,false);
		if ( i == 0 ) {
		Pbool->Set_help(        "================================================================================================\n"
								"If set, force IDE state change on certain INT 13h commands. IDE registers will be changed as if\n"
								"BIOS had carried out the action. If you are running Windows 3.11 or Windows 3.11 Windows for\n"
								"Workgroups you must enable this option (and use -reservecyl 1) if you want 32-bit disk access\n"
								"to work correctly in DOSBox.");						
		}
				
		Pbool = secprop->Add_bool("Int13Fakev86IO",Property::Changeable::WhenIdle,false);
		if ( i == 0 ) {
		Pbool->Set_help(        "================================================================================================\n"
								"If set, and int13fakeio is set, certain INT 13h commands will cause IDE emulation to issue fake CPU\n"
								"I/O traps (GPF) in  virtual 8086 mode and a fake IRQ signal. you must enable this option if you want\n"
								"32-bit disk access in Windows 95 to work with DOSBox.");
		}
		
		Pbool = secprop->Add_bool("Enable PIO32",Property::Changeable::WhenIdle,false);
		if ( i == 0 ) {
		Pbool->Set_help(        "================================================================================================\n"
								"If set, 32-bit I/O reads and writes are handled directly (much like PCI IDE implementations)\n"
				                "If clear, 32-bit I/O will be handled as if two 16-bit I/O (much like ISA IDE implementations)");
		}

		Pbool = secprop->Add_bool("Ignore PIO32",Property::Changeable::WhenIdle,false);
		if ( i == 0 ) {
		Pbool->Set_help(        "================================================================================================\n"
								"If 32-bit I/O is enabled, attempts to read/write 32-bit I/O will be ignored entirely.\n"
				                "In this way, you can have DOSBox emulate one of the strange quirks of 1995-1997 era laptop hardware");
		}

				
		Pint = secprop->Add_int("CDROM Spinup Time",Property::Changeable::WhenIdle,0			/*use IDE or CD-ROM default*/);
		
		if ( i == 0 ) {
		Pint->Set_help(        "================================================================================================\n"
							   "Emulated CD-ROM time in ms to spin up if CD is stationary. Set to 0 to use controller or CD-ROM\n"
							   "drive-specific default.");
		}

		Pint = secprop->Add_int("CDROM Spindown Timeout",Property::Changeable::WhenIdle,0		/*use IDE or CD-ROM default*/);
		if ( i == 0 ) {
		Pint->Set_help(        "================================================================================================\n"
							   "Emulated CD-ROM time in ms that drive will spin down automatically when not in use. Set to 0 to\n"
							   "use controller or CD-ROM drive-specific default.");
		}

		Pint = secprop->Add_int("CDROM Insertion Delay",Property::Changeable::WhenIdle,0		/*use IDE or CD-ROM default*/);
		
		if ( i == 0 ) {
		Pint->Set_help(       "================================================================================================\n"
						      "Emulated CD-ROM time in ms that drive will report \"medium not present\" to emulate the time it\n"
							  "takes for someone to take out a CD and insert a new one when DOSBox is instructed to swap or change\n"
							  "CDs. When running Windows 95 or higher a delay of 4000ms is recommended to ensure that auto-insert\n"
							  "notification triggers properly. Set to 0 to use controller or CD-ROM drive-specific default.");
		}							  
	}					
	
	
	
#if defined(PCI_FUNCTIONALITY_ENABLED)
	secprop=control->AddSection_prop("pci",&PCI_Init,false); //PCI bus

	secprop->AddInitFunction(&VOODOO_Init,true);
	const char* voodoo_settings[] = {
		"false",
		"software",
#if C_OPENGL
		"opengl",
		"vulkan",
#endif
		"auto",
		0
	};
	Pstring = secprop->Add_string("voodoo",Property::Changeable::WhenIdle,"software");
	Pstring->Set_values(voodoo_settings);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Enable VOODOO support. Use the Voodoo Emulation with the Output method (Cat: sdl) Recommend:\n"
							"Texture(nb)/OpengGL(nb). Performance Setting: Set the exture Renderer to OpenGL. Acceleration\n"
					        "is disabled on auto, direct3d or Software NOTE: If you use SOFTWARE Mode. Aspect Ratio must set\n"
							"to FALSE or you become gfx glitches. ( Default: opengl");
	
	const char* voodoo_memory[] = {"standard","max",0};
	Pstring = secprop->Add_string("Voodoo_Memory",Property::Changeable::OnlyAtStart,"standard");
	Pstring->Set_values(voodoo_memory);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Specify VOODOO card memory size.\n"
		                     "   standard  : 4MB  card (2MB front buffer + 1x2MB texture unit)\n"
					         "   max       : 12MB card (4MB front buffer + 2x4MB texture units)");
					  
	Pint = secprop->Add_int("Voodoo_MemXtra",Property::Changeable::OnlyAtStart,0);
	Pint->SetMinMax(0,512);
	Pint->Set_help(         "================================================================================================\n"
	                        "Amount Of Voodoo Videomemory. Set this greater as 0 will be Disable the Option above. (Default: 0)\n");					  
							
	const char* voodoo_filter[] = {"none","point","bilinear","trilinear","anisotropic","testmode",0};		
	Pstring = secprop->Add_string("Voodoo_Filter",Property::Changeable::WhenIdle,"trilinear");
	Pstring->Set_values(voodoo_filter);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Filtering in OpenGL Mode                                                   (Default: anisotropic)\n"
	                        "   none           : Using System Default Settings\n"
					        "   point          : No Filtering. \n"
					        "   bilinear       : Full Filtering (cheapest)\n"
							"   trilinear      : Good Filtering Mode\n"
							"   anisotropic    : Best Filter Mode\n"
							"   testmode	   : Testing Only");	
										
	const char* voodoo_glshade[] = {"none", "flat", "smooth",0};						  
	Pstring = secprop->Add_string("Voodoo_GLShade",Property::Changeable::WhenIdle,"none");
	Pstring->Set_values(voodoo_glshade);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Filtering in OpenGL Mode                                                       (Default: smooth)\n"
	                        "   none       : Default Value \n"
					        "   flat       : GL_ShadeModel using flat\n"
					        "   smooth     : GL_ShadeModel using smooth");
							
	const char* voodoo_glwraps[] = {"gl_repeat", "gl_mirrored_repeat", "gl_clamp_to_edge","gl_clamp_to_border",0};						  
	Pstring = secprop->Add_string("Texture_Wrap_S",Property::Changeable::WhenIdle,"gl_repeat");
	Pstring->Set_values(voodoo_glwraps);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Change this ONLY if you Problems with textures. Sets the wrap parameter for texture coordinate\n"
	                        "   gl_repeat         : Default Value \n"
					        "   gl_mirrored_repeat:\n"
					        "   gl_clamp_to_edge  :\n"
							"   gl_clamp_to_border:\n"
							"Settings gl_clamp_to_edge will Fix F1'97 Texture Screen Beginners level");			

	const char* voodoo_glwrapt[] = {"gl_repeat", "gl_mirrored_repeat", "gl_clamp_to_edge","gl_clamp_to_border",0};						  
	Pstring = secprop->Add_string("Texture_Wrap_R",Property::Changeable::WhenIdle,"gl_repeat");
	Pstring->Set_values(voodoo_glwrapt);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Change this ONLY if you Problems with textures. Sets the wrap parameter for texture coordinate\n"
	                        "   gl_repeat         : Default Value \n"
					        "   gl_mirrored_repeat: \n"
					        "   gl_clamp_to_edge  : \n"
							"   gl_clamp_to_border:");								
							
	Pstring = secprop->Add_string("voodoo_Window",Property::Changeable::OnlyAtStart,"512x384");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Change Voodoo 3DFX Resolution in Window Mode. NOTE: 3DFX Games under DOS use 512x384/640x480\n"
							"Windows 9x Games begins at 512x384 or 640x480. Blood 1 3DFX must be set equal to the Screen\n"
							"Config how in the Blood,cfg. This option works only if [sdl] --> VoodooUseOwnWindowRes = true");
					  
	Pstring = secprop->Add_string("voodoo_Fullscreen",Property::Changeable::OnlyAtStart,"512x384");	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Change Voodoo 3DFX Resolution in Fullscreen Mode. NOTE: 3DFX Games under DOS use 512x384/640x480\n"
							"Windows 9x Games begins at 512x384 or 640x480. Blood 1 3DFX must be set equal to the Screen\n"
							"Config how in the Blood,cfg. This option works only if [sdl] --> VoodooUseOwnFullScRes = true");
	
	Pbool = secprop->Add_bool("voodoo_Aspect",Property::Changeable::Always,true);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "Dont Stretch the Mode on Widescreen Resolutions. (Default: true)" );
							
	Pint = secprop->Add_int("ZoomScreen_Width",Property::Changeable::OnlyAtStart,0);
	Pint->SetMinMax(-4096,4096);
	Pint->Set_help(         "================================================================================================\n"
	                        "Zoom Voodoo Game Screen (Width Aspect) (Default: 0) (Can used in combination with voodoo_Aspect)");

	Pint = secprop->Add_int("ZoomScreen_Height",Property::Changeable::OnlyAtStart,0);
	Pint->SetMinMax(-4096,4096);
	Pint->Set_help(         "================================================================================================\n"
	                        "Zoom Voodoo Game Screen (Height Aspect) (Default: 0)");							
	
	Pfloat = secprop->Add_float("Anisotropic_Level",Property::Changeable::OnlyAtStart,16.0);
	Pfloat->SetMinMax(2.0,16.0);
	Pfloat->Set_help(       "================================================================================================\n"
	                        "Anisotropic Filtering Levels. Only for Anisotropic Filter Mode (Default is Value 16)");
														
	Pbool = secprop->Add_bool("a_ClipLowYHigh",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"OpenGL Only, Alternate Height Patch for VooDoo's ClipLow Y High.\n"
							"On some game the Height is on Higher Resolutions not normal. (Default false)"                );

	Pint = secprop->Add_int("n_ClipLowYHigh",Property::Changeable::OnlyAtStart,0);
	Pint->SetMinMax(0,100);
	Pint->Set_help(         "================================================================================================\n"
	                        "OpenGL Only. Pitch VooDoo's ClipLow Y High (Default: 0)");
							
							
	Pbool = secprop->Add_bool("compatible_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "This flag is intended to put the GL Into a Software (Default False)");	
							
						
	Pbool = secprop->Add_bool("gl_QuadsDraw_use",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "OpenGL Only. Modify the 2D Texture on Higher Resolution and use GL_QUADS (Default False)");
							
	Pbool = secprop->Add_bool("gl_PointSize_use",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "OpenGL Only. Modify the 2D Texture on Higher Resolution. (Default False)");
							
	Pfloat = secprop->Add_float("gl_PointSize_num",Property::Changeable::OnlyAtStart,0.0);
	Pfloat->SetMinMax(0.0,1000.0);
	Pfloat->Set_help(       "================================================================================================\n"
	                        "OpenGL Only. Modify Amount the Pointsize the 2D Texture on Higher Resolution. (Default is Value 0)");
							
							
	Pbool = secprop->Add_bool("glScissor_flag",Property::Changeable::Always,true);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "If you have on High Resolution a problem with the Viewport and this is Cutted to Left/Bottom or.\n"
							"the Screen looks ugly the rest from the Screen is Blank or Parts is Flickering/Tearing.\n"
							"Set this to False forces to not use the glScissor calculation and free the fixed the Resolution:\n"
							"- Argosy 2325\n"
							"- Pyl\n"
							"- Warhammer: Dark Omen (Fixed Cutscenes) and i think on other 3DFX Games. (Default true)");
							
	Pbool = secprop->Add_bool("glP_Smoth_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "OpenGL Only, Enable Antialiasing for points (Default false) (Just For Testing)" );
							
	Pbool = secprop->Add_bool("glL_Smoth_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "OpenGL Only, Enable Antialiasing for lines (Default false) (Just For Testing)"  );

	Pbool = secprop->Add_bool("glG_Smoth_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"OpenGL Only, Enable Antialiasing for Polygons (Default false) (Just For Testing)" );
							
	Pbool = secprop->Add_bool("glBlendFc_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"OpenGL Only, Enable blending (Default false) (Just For Testing)");

	Pbool = secprop->Add_bool("glPersCor_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"OpenGL Only, Enable GL_PERSPECTIVE_CORRECTION_HINT (Default false)" );
							
	Pbool = secprop->Add_bool("glGMipMap_flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"OpenGL Only, Enable GL_GENERATE_MIPMAP_HINT (Default false) (Just For Testing)" );

	Pbool = secprop->Add_bool("gl_GLFog__flag",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"OpenGL Only, Enable GL_FOG_HINT (Default false)  (Just For Testing)" );							

	Pfloat = secprop->Add_float("gl_ortho_zNear",Property::Changeable::OnlyAtStart,0.0);
	Pfloat->SetMinMax(-200.0,200.0);
	Pfloat->Set_help(         "================================================================================================\n"
	                        "Modify the zNear Value (Default 0.0). Specify the distances to the nearer clipping planes");	

	Pfloat = secprop->Add_float("gl_ortho_zFar",Property::Changeable::OnlyAtStart,-1.0);
	Pfloat->SetMinMax(-200.0,200.0);
	Pfloat->Set_help(         "================================================================================================\n"
	                        "OpenGL Only. Modify the zFar Value (Default -1.0) Specify the dists to the farther depth\n"
							"clipping planes Change this to negative ~9.0 will fix the Far Objects in Pyl on High Resolution");	
	
	Pint = secprop->Add_int("gl_Major_Version",Property::Changeable::OnlyAtStart,2);
	Pint->SetMinMax(0,100);
	Pint->Set_help(         "================================================================================================\n"
	                        "Change the OpengGL Context Major Version. Do not set too high otherwise Opengl crashes. (default 2)");

	Pint = secprop->Add_int("gl_Minor_Version",Property::Changeable::OnlyAtStart,1);
	Pint->SetMinMax(0,100);
	Pint->Set_help(         "================================================================================================\n"
	                        "Change the OpengGL Context Minor Version.Do not set too high otherwise Opengl crashes. (default 1)");	

	Pbool = secprop->Add_bool("sh_FbzcpCca_Sw2",Property::Changeable::Always,true);		
	Pbool->Set_help(        "================================================================================================\n"
							"Set this to False Fix F1'97 Massive Grafic Glitch\n"
							"Shader: The FBZCOLORPATH in the switch FBZCP_CC_ASELECT will be not used (default true)");												
	
	Pbool = secprop->Add_bool("LFB_ScreenFixFrnt",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"Set this to true will be Fix the LFB Front Buffer Static Draw Screen\n"
							"This is a Temporary Fix and will not work on all Resolutions except eq 1280x960. (default false)\n"
							"Games:\n"
							"-> Road To India (Movie Stutter and Zoom Mismatch)");												

	Pbool = secprop->Add_bool("LFB_ScreenFixBack",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"Set this to true will be Fix the LFB Back Buffer Static Draw Screen\n"
							"This is a Temporary Fix and will not work on all Resolutions except eq 1280x960. (default false)\n"
							"Games:\n"
							"->Prince Of Perisa 3D\n"							
							"-> Tomb Raider\n"							
							"-> Warhammer - Dark Omen");								
							
	Pbool = secprop->Add_bool("LFB_LogRegisterNr",Property::Changeable::Always,false);		
	Pbool->Set_help(        "================================================================================================\n"
							"Set this to true will be Logging the Frame Buffer Mode and Register Nr. (default false)");
							
#endif


	secprop=control->AddSection_prop("mixer",&MIXER_Init);
	Pbool = secprop->Add_bool("nosound",Property::Changeable::OnlyAtStart,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable silent mode, sound is still emulated though.");

	Pint = secprop->Add_int("rate",Property::Changeable::OnlyAtStart,44100);
	Pint->Set_values(rates);
	Pint->Set_help(         "================================================================================================\n"
	                        "Mixer sample rate, setting any device's rate higher than this will probably lower their sound quality.");

	const char *blocksizes[] = {
		"128", "256", "512","1024", "2048", "4096", "8192", "32768","65536", "131072", 0};
	Pint = secprop->Add_int("blocksize",Property::Changeable::OnlyAtStart,1024);
	Pint->Set_values(blocksizes);
	Pint->Set_help(         "================================================================================================\n"
	                        "Mixer block size, larger blocks might help sound stuttering but sound will also be more lagged.");

	Pint = secprop->Add_int("prebuffer",Property::Changeable::OnlyAtStart,25);
	Pint->SetMinMax(0,10000);
	Pint->Set_help(         "================================================================================================\n"
	                        "How many milliseconds of data to keep on top of the blocksize.");

	
	Pbool = secprop->Add_bool("UseMediaKeys",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Allow and set Default to use the media hotkeys instead of keys for use tzte Mixer Volume + and -.\n"
							"The Media Keys is System Wide. If you change the Volume this will change the Volume System Wide");
							
	secprop=control->AddSection_prop("midi",&MIDI_Init,true);//done
	secprop->AddInitFunction(&MPU401_Init,true);//done

	const char* mputypes[] = { "intelligent", "uart", "none",0};
	// FIXME: add some way to offer the actually available choices.
	//
	const char *devices[] = {"default","win32","alsa","oss","coreaudio","coremidi","mt32",
#ifdef C_FLUIDSYNTH
		"fluidsynth",
#endif
		"none",
		0};

	Pstring = secprop->Add_string("mpu401",Property::Changeable::WhenIdle,"intelligent");
	Pstring->Set_values(mputypes);
	Pstring->Set_help(       "================================================================================================\n"
	                         "Type of MPU-401 to emulate.");

	Pstring = secprop->Add_string("mididevice",Property::Changeable::WhenIdle,"default");
	Pstring->Set_values(devices);
	Pstring->Set_help(       "================================================================================================\n"
	                         "Device that will receive the MIDI data from MPU-401.");

	Pstring = secprop->Add_string("midiconfig",Property::Changeable::WhenIdle,"\n");
	Pstring->Set_help(       "================================================================================================\n"
	                         "Special configuration options for the device driver. This is usually the id or part of the name\n"
					         "of the device you want to use Find the id/name in DOsbox with the command 'mixer /listmidi' or\n"
	                         "in the case of coreaudio, you can specify a soundfont here. When using a Roland MT-32 rev. 0 as\n"
	                         "midi output device, some games may require a delay in order to prevent 'buffer overflow' issues.\n"
	                         "In that case, add 'delaysysex', for example: 'midiconfig=2 delaysysex' See the README/Manual for\n"
	                         "more details.");						 

#ifdef C_FLUIDSYNTH
	const char *fluiddrivers[] = {"pulseaudio", "alsa", "oss", "coreaudio", "dsound", "portaudio", "sndman", "jack", "file", "default",0};
	Pstring = secprop->Add_string("fluid.driver",Property::Changeable::WhenIdle,"default");
	Pstring->Set_values(fluiddrivers);	
	Pstring->Set_help(       "================================================================================================\n"
	                         "Driver to use with Fluidsynth (Linux/Mac), Leave blank under Windows.");

	Pstring = secprop->Add_string("fluid.soundfont",Property::Changeable::WhenIdle,".\\DATA\\SOUNDFONT\\8MBGMSFX.SF2");
	Pstring->Set_help(       "================================================================================================\n"
	                         "Soundfont to use with Fluidsynth. One must be specified.");

	Pstring = secprop->Add_string("fluid.samplerate",Property::Changeable::WhenIdle,"44100");
	Pstring->Set_values(fluidrates);	
	Pstring->Set_help(       "================================================================================================\n"
	                         "The sample rate of the audio generated by the synthesizer");

	Pstring = secprop->Add_string("fluid.gain",Property::Changeable::WhenIdle,"0.5");	
	Pstring->Set_help(       "================================================================================================\n"
	                         "The gain is applied to the final or master output of the synthesizer. It is set to a low value\n" 
					         "by default to avoid the saturation of the output when many notes are played.\n"
					         "Min:  0.00\n"
							 "Max: 10.00 (Default 0.5)");

	Pint = secprop->Add_int("fluid.polyphony",Property::Changeable::WhenIdle,256);	
	Pint->SetMinMax(1,65535);		
	Pint->Set_help(          "================================================================================================\n"
	                         "The polyphony defines how many voices can be played in parallel. A note event produces one or\n"
				             "more voices. Its good to set this to a value which the system can handle and will thus limit \n"
				             "FluidSynth's CPU usage. When FluidSynth runs out of voices it will begin terminating lower priority \n"
				             "voices for new note events."
							 "Min:     1\n"
							 "Max: 65535 (Default 256)");

	Pstring = secprop->Add_string("fluid.cores",Property::Changeable::WhenIdle,"default");
	Pstring->Set_help(       "================================================================================================\n"
	                         "Fluidsynth CPU cores to use:"
							 "Min:   1\n"
							 "Max: 256 (Default is \"default\")\n");

	Pstring = secprop->Add_string("fluid.periods",Property::Changeable::WhenIdle,"8");
	Pstring->Set_help(       "================================================================================================\n"
	                         "The number of the audio buffers used by the driver. This number of buffers, multiplied by the\n"
					         "buffer size (see setting audio.period-size), determines the maximum latency of the audio driver.\n"
					         "System Defaults: 16 (Linux, Mac OS X), 8 (Windows).\n"
							 "Min:  2\n"
							 "Max: 64 (Default 8)");

	Pstring = secprop->Add_string("fluid.periodsize",Property::Changeable::WhenIdle,"512");	
	Pstring->Set_help(       "================================================================================================\n"
	                         "The size of the audio buffers (in frames). System Defaults 64 (Linux, Mac OS X), 512 (Windows).\n"
					         "Min:   64\n"
							 "Max: 8192 (Default 512)");

	const char *fluidreverb[] = {"no", "yes",0};
	Pstring = secprop->Add_string("fluid.reverb",Property::Changeable::WhenIdle,"yes");
	Pstring->Set_values(fluidreverb);	
	Pstring->Set_help(       "================================================================================================\n"
	                         "When set to \"yes\" the reverb effects module is activated. Otherwise, no reverb will be added\n"
					         "to the output signal. Note that the amount of signal sent to the reverb module depends on the\n"
					         "\"reverb send\" generator defined in the SoundFont.");

	const char *fluidchorus[] = {"no", "yes",0};
	Pstring = secprop->Add_string("fluid.chorus",Property::Changeable::WhenIdle,"yes");
	Pstring->Set_values(fluidchorus);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "When set to \"yes\" the chorus effects module is activated. Otherwise, no chorus will be added\n"
					        "to the output signal. Note that the amount of signal sent to the chorus module depends on the\n"
					        "\"chorus send\" generator defined in the SoundFont.");

	Pstring = secprop->Add_string("fluid.reverb,roomsize",Property::Changeable::WhenIdle,".2");	
	Pstring->Set_help(      "================================================================================================\n"
							"Min: 0.00\n"
							"Max: 1.20 (Default 0.2)");

	Pstring = secprop->Add_string("fluid.reverb.damping",Property::Changeable::WhenIdle,"0");
	Pstring->Set_help(      "================================================================================================\n"
							"Min: 0.00\n"
							"Max: 0.99 (Default 0)");

	Pstring = secprop->Add_string("fluid.reverb.width",Property::Changeable::WhenIdle,"1");
	Pstring->Set_help(      "================================================================================================\n"
							"Min:   0\n"
							"Max: 100  (Default 0.1)");

	Pstring = secprop->Add_string("fluid.reverb.level",Property::Changeable::WhenIdle,"0.9");
	Pstring->Set_help(      "================================================================================================\n"
							"Min: 0.00\n"
							"Max: 1.00 (Default 0.9)");

	Pint = secprop->Add_int("fluid.chorus.number",Property::Changeable::WhenIdle,3);
	Pint->SetMinMax(0,99);	
	Pint->Set_help(         "================================================================================================\n"
							"Min:  0\n"
							"Max: 99 (Default 3)");

	Pstring = secprop->Add_string("fluid.chorus.level",Property::Changeable::WhenIdle,"1");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Min: 0.00\n"
							"Max: 1.00 (Default 1)");

	Pstring = secprop->Add_string("fluid.chorus.speed",Property::Changeable::WhenIdle,"0.3");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Min: 0.30\n"
							"Max: 5.00 (Default 0.3)");

	Pstring = secprop->Add_string("fluid.chorus.depth",Property::Changeable::WhenIdle,"8.0");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Min:  0.0\n"
							"Max: 21.0 (Default 8.0)");

	const char *fluidchorustypes[] = {"0", "1",0};
	Pint = secprop->Add_int("fluid.chorus.type",Property::Changeable::WhenIdle,0);
	Pint->Set_values(fluidchorustypes);
	Pint->Set_help(         "================================================================================================\n"
	                        "Fluidsynth Chorus Type. 0 = Sine Wave / 1 =  Triangle Wave.");
#endif

#include "mt32options.h"

#if defined(C_DEBUG)
	secprop=control->AddSection_prop("debug",&DEBUG_Init);
#endif

	secprop=control->AddSection_prop("sblaster",&SBLASTER_Init,true);//done

	const char* sbtypes[] = { "sb1", "sb2", "sbpro1", "sbpro2", "sb16", "gb", "none", 0 };
	Pstring = secprop->Add_string("sbtype",Property::Changeable::WhenIdle,"sb16");
	Pstring->Set_values(sbtypes);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Type of Soundblaster to emulate. gb is Gameblaster.");

	Phex = secprop->Add_hex("sbbase",Property::Changeable::WhenIdle,0x220);
	Phex->Set_values(ios);
	Phex->Set_help(         "================================================================================================\n"
	                        "The IO address of the soundblaster.");

	Pint = secprop->Add_int("irq",Property::Changeable::WhenIdle,7);
	Pint->Set_values(irqssb);
	Pint->Set_help(         "================================================================================================\n"
	                        "The IRQ number of the soundblaster.");

	Pint = secprop->Add_int("dma",Property::Changeable::WhenIdle,1);
	Pint->Set_values(dmassb);
	Pint->Set_help(         "================================================================================================\n"
	                        "The DMA number of the soundblaster.");

	Pint = secprop->Add_int("hdma",Property::Changeable::WhenIdle,5);
	Pint->Set_values(dmassb);
	Pint->Set_help(         "================================================================================================\n"
	                        "The High DMA number of the soundblaster.");

	Pbool = secprop->Add_bool("sbmixer",Property::Changeable::WhenIdle,true);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Allow the soundblaster mixer to modify the DOSBox mixer.");

	const char* oplmodes[]={ "auto", "cms", "opl2", "dualopl2", "opl3", "opl3gold", "none", 0};
	Pstring = secprop->Add_string("oplmode",Property::Changeable::WhenIdle,"auto");
	Pstring->Set_values(oplmodes);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Type of OPL emulation. On 'auto' the mode is determined by sblaster type. All OPL modes are\n"
	                        "Adlib-compatible, except for 'cms'.");

	const char* oplemus[]={ "default", "compat", "fast", "mame","nuked", 0};
	Pstring = secprop->Add_string("oplemu",Property::Changeable::WhenIdle,"default");
	Pstring->Set_values(oplemus);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Provider for the OPL emulation.\n"
					        "  default\n"
					        "  compat : Might provide a Better Quality (see oplrate as well)\n"
					        "  fast\n"
					        "  mame   : Activate the MAME FM OPL, OPL2 and OPL3 Music Synthesizer\n"
					        "  nuked  : This is more accurate than current default, fast or compat");

	Pint = secprop->Add_int("oplrate",Property::Changeable::WhenIdle,44100);
	Pint->Set_values(oplrates);	
	Pint->Set_help(         "================================================================================================\n"
	                        "Samplerate of OPL Music Emulation. Use 49716 for highest Quality (set the mixer rate accordingly).");

	Pint = secprop->Add_int("fmstrength",Property::Changeable::WhenIdle,150);
	Pint->SetMinMax(1,1000);	
	Pint->Set_help(         "================================================================================================\n"
	                        "Strength of the FM playback volume in percent, in relation to PCM playback volume Default is 150.\n"
	                        "Possible Values: 1 to 1000 (0.01x to 10x)");
	
	secprop=control->AddSection_prop("gus",&GUS_Init,true); //done
	Pbool = secprop->Add_bool("gus",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable the Gravis Ultrasound emulation.");

	Pint = secprop->Add_int("gusrate",Property::Changeable::WhenIdle,44100);
	Pint->Set_values(rates);
	Pint->Set_help(         "================================================================================================\n"
	                        "Sample rate of Ultrasound emulation.");

	Phex = secprop->Add_hex("gusbase",Property::Changeable::WhenIdle,0x240);
	Phex->Set_values(iosgus);
	Phex->Set_help(         "================================================================================================\n"
	                        "The IO base address of the Gravis Ultrasound.");

	Pint = secprop->Add_int("gusirq",Property::Changeable::WhenIdle,5);
	Pint->Set_values(irqsgus);
	Pint->Set_help(         "================================================================================================\n"
	                        "The IRQ number of the Gravis Ultrasound.");

	Pint = secprop->Add_int("gusdma",Property::Changeable::WhenIdle,3);
	Pint->Set_values(dmasgus);
	Pint->Set_help(         "================================================================================================\n"
	                        "The DMA channel of the Gravis Ultrasound.");

	Pstring = secprop->Add_string("ultradir",Property::Changeable::WhenIdle,"C:\\ULTRASND");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Path to Ultrasound directory. In this directory there should be a MIDI directory that contains\n"
		                    "the patch files for GUS playback. Patch sets used with Timidity should work fine.");

	secprop	= control->AddSection_prop("innova",&INNOVA_Init,true);
	Pbool 	= secprop->Add_bool("innova",Property::Changeable::WhenIdle,false);
	
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable Innovation SSI-2001 Sound. Activate MOS6581 (SID) Sound Chip Card");
	
	Pint 	= secprop->Add_int("samplerate",Property::Changeable::WhenIdle,44100);
	Pint->Set_values(oplrates);	
	Pint->Set_help(         "================================================================================================\n"
	                        "Sample rate of the emulation.");
	
	Phex 	= secprop->Add_hex("sidbase",Property::Changeable::WhenIdle,0x280);
	Phex->Set_values(innobas);
	Phex->Set_help(         "================================================================================================\n"
	                        "SID Base Port (default 280h).");	
	
	Pint 	= secprop->Add_int("quality", Property::Changeable::WhenIdle,0);
	Pint->Set_values(innoqal);
	Pint->Set_help(         "================================================================================================\n"
	                        "Set reSID quality/interpolation  level (0 to 3).\n"
					        "    0: Off\n"
					        "    1: Interpolate\n"
					        "    2: Resample Fast\n"
					        "    3: Resample & Interpolate");
	secprop = control->AddSection_prop("speaker",&PCSPEAKER_Init,true);//done
	Pbool = secprop->Add_bool("pcspeaker",Property::Changeable::WhenIdle,true);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable PC-Speaker emulation.");

	Pstring = secprop->Add_string("pcspeaker.mode",Property::Changeable::WhenIdle,"old");
	Pstring->Set_values(pcmode);	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Set the PC Speaker Emulation Compatibility:\n"
					        "   old: Original Dosbox\n"
					        "   new: For Games that didnt work with how Starcontrol etc..");
	
	Pint = secprop->Add_int("pcrate",Property::Changeable::WhenIdle,44100);
	Pint->Set_values(rates);
	Pint->Set_help(         "================================================================================================\n"
	                        "Sample rate of the PC-Speaker sound generation.");

	secprop->AddInitFunction(&TANDYSOUND_Init,true);//done
	const char* tandys[] = { "auto", "on", "off", 0};
	Pstring = secprop->Add_string("tandy",Property::Changeable::WhenIdle,"auto");
	Pstring->Set_values(tandys);
	
	Pstring->Set_help(      "================================================================================================\n"
	                        "Enable Tandy Sound System emulation. For 'auto', emulation is present only if machine is set to\n"
	                        "'tandy'.");

	Pint = secprop->Add_int("tandyrate",Property::Changeable::WhenIdle,44100);
	Pint->Set_values(rates);
	Pint->Set_help(         "================================================================================================\n"
	                        "Sample rate of the Tandy 3-Voice generation.");

	secprop->AddInitFunction(&DISNEY_Init,true);//done

	Pbool = secprop->Add_bool("disney",Property::Changeable::WhenIdle,true);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable Disney Sound Source emulation. (Covox Voice Master and Speech Thing compatible).");

	secprop=control->AddSection_prop("joystick",&BIOS_Init,false);//done
	secprop->AddInitFunction(&INT10_Init);
	secprop->AddInitFunction(&MOUSE_Init); //Must be after int10 as it uses CurMode
	secprop->AddInitFunction(&JOYSTICK_Init,true);
	const char* joytypes[] = { "auto", "2axis", "4axis", "4axis_2", "fcs", "fcslw", "ch", "chvp", "chgs", "qshot6", "capcom", "inter6", "wheel","none",0};
	Pstring = secprop->Add_string("joysticktype",Property::Changeable::WhenIdle,"4axis");
	Pstring->Set_values(joytypes);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Type of joystick to emulate: auto (default)\n"
		                    "    2axis  : Supports two joysticks [Axes: 2, Buttons: 2]\n"
		                    "    4axis  : Supports one joystick  [Axes: 4, Buttons: 4] (First  joystick used)\n"
		                    "    4axis_2: Supports one joystick  [Axes: 4, Buttons: 4] (Second joystick used)\n"
							"    qshot6 : QuickShot6 Windows Pad [Axes: 2, Buttons: 6]\n"
							"    capcom : Capcom 6 Button (DOS)  [Axes: 2, Buttons: 6]\n"
                            "    inter6 : InterACT PcPad (Pro)   [Axes: 2, Buttons: 6]\n"							
		                    "    fcs    : Thrustmaster           [Axes: 3, Buttons: 4]\n"
		                    "    fcslw  : Thrustmaster, Throttle [Axes: 3, Buttons: 4]\n"							
		                    "    ch     : CH Flightstick         [Axes: 4, Buttons: 6]\n"
							"    chvp   : CH Virtual Pilot Pro   [Axes: 6, Buttons:10]\n"							
							"             For Saitek Flight Systems or XInput on Host in Combination with Win9x CH Drivers\n"
							"             CH Virtual Pilot/ CH Flight Stick Pro\n"
							"    chgs   : CH Gamestick 1.4       [Axes: 6, Buttons:10]\n"							
							"             For XBox, F710 Controller on Host in Combination with Win9x CH GSC14 Drivers\n"
							"    wheel  : Driving Wheel          [Axes: 4, Buttons:4]\n"
		                    "    none   : Disable joystick(s) Emulation\n"
		                    "    auto   : Chooses emulation depending on real joystick(s).\n"
		                    "(Remember to reset/ delet dosbox's mapperfile if you saved it earlier)");

	const char* hosttypes[] = { "default", "saitekx45", "lgex3dpro","tfhotasone" ,"ldriveforcepro",0};
	Pstring = secprop->Add_string("joystickhost",Property::Changeable::WhenIdle,"default");
	Pstring->Set_values(hosttypes);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Specifed Type of joystick on your Host System in Combination with chvp\n"
							"It Changes the Axes Values for Rudder /Pedals/ Throttle Axes\n"
		                    "    default       : The Standard and default\n"
		                    "    SaitekX45     : Using Saitek-X45 Specs (Same are Default)\n"
		                    "    LgEx3DPro     : Using Logitech 3D Pro\n"
							"    ldriveforcepro: Using Logitech Driving Force (Pro)\n"
							"    TFHotasOne    : Using Thrustmaster Flight Hotas One");
							
	Pbool = secprop->Add_bool("timed",Property::Changeable::WhenIdle,false);		
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable timed intervals for axis. Experiment with this option, if your joystick drifts (away).");

	Pbool = secprop->Add_bool("autofire",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Continuously fires as long as you keep the button pressed.");

	Pbool = secprop->Add_bool("swap34",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Wrap the 3rd and the 4th axis. can be useful for certain joysticks.");

	Pbool = secprop->Add_bool("buttonwrap",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable button wrapping at the number of emulated buttons.");


	Pbool = secprop->Add_bool("circularinput",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
							"enable translation of circular input to square output.\n"
							"Try enabling this if your left analog stick can only move in a circle.");

	Pint = secprop->Add_int("deadzone",Property::Changeable::WhenIdle,10);
	Pint->SetMinMax(0,100);
	Pint->Set_help(        "================================================================================================\n"
	                       "the percentage of motion to ignore. 100 turns the stick into a digital one.");
						   
	Pint = secprop->Add_int("buttons",Property::Changeable::WhenIdle,0);
	Pint->SetMinMax(0,10);
	Pint->Set_help(        "================================================================================================\n"
	                       "Overwrite the Default Emulated Buttons in DOSBox for 2axis,4axis,4axis_2");						   
							
	secprop=control->AddSection_prop("serial",&SERIAL_Init,true);
	const char* serials[] = { "dummy", "disabled", "modem", "nullmodem",
	                          "directserial",0 };

	Pmulti_remain = secprop->Add_multiremain("serial1",Property::Changeable::WhenIdle," ");
	Pstring = Pmulti_remain->GetSection()->Add_string("type",Property::Changeable::WhenIdle,"dummy");
	Pmulti_remain->SetValue("dummy");
	Pstring->Set_values(serials);
	Pstring = Pmulti_remain->GetSection()->Add_string("parameters",Property::Changeable::WhenIdle,"");
	Pmulti_remain->Set_help( "================================================================================================\n"
	                         "Set type of device connected to com port. Can be disabled, dummy, modem, nullmodem, directserial.\n"
		                     "Additional parameters must be in the same line in the form of parameter:value. Parameter for all\n"
		                     "types is irq (optional).\n"
		                     "    directserial: realport (required), rxdelay (optional) (realport:COM1 realport:ttyS0)\n"
		                     "    modem       : listenport (optional).\n"
		                     "    nullmodem   : server, rxdelay, txdelay, telnet, usedtr, transparent, port, inhsocket\n"
		                     "                 (all optional)\n"
							 "Example: serial1=modem listenport:5000");

	Pmulti_remain = secprop->Add_multiremain("serial2",Property::Changeable::WhenIdle," ");
	Pstring = Pmulti_remain->GetSection()->Add_string("type",Property::Changeable::WhenIdle,"dummy");
	Pmulti_remain->SetValue("dummy");
	Pstring->Set_values(serials);
	Pstring = Pmulti_remain->GetSection()->Add_string("parameters",Property::Changeable::WhenIdle,"");
	Pmulti_remain->Set_help( "================================================================================================\n"
	                         "see serial1");

	Pmulti_remain = secprop->Add_multiremain("serial3",Property::Changeable::WhenIdle," ");
	Pstring = Pmulti_remain->GetSection()->Add_string("type",Property::Changeable::WhenIdle,"disabled");
	Pmulti_remain->SetValue("disabled");
	Pstring->Set_values(serials);
	Pstring = Pmulti_remain->GetSection()->Add_string("parameters",Property::Changeable::WhenIdle,"");
	Pmulti_remain->Set_help( "================================================================================================\n"
	                         "see serial1");

	Pmulti_remain = secprop->Add_multiremain("serial4",Property::Changeable::WhenIdle," ");
	Pstring = Pmulti_remain->GetSection()->Add_string("type",Property::Changeable::WhenIdle,"disabled");
	Pmulti_remain->SetValue("disabled");
	Pstring->Set_values(serials);
	Pstring = Pmulti_remain->GetSection()->Add_string("parameters",Property::Changeable::WhenIdle,"");
	Pmulti_remain->Set_help( "================================================================================================\n"
	                         "see serial1");


	/* All the DOS Related stuff, which will eventually start up in the shell */
	secprop=control->AddSection_prop("dos",&DOS_Init,false);//done
	secprop->AddInitFunction(&XMS_Init,true);//done
	Pbool = secprop->Add_bool("xms",Property::Changeable::WhenIdle,true);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable XMS support.");

	secprop->AddInitFunction(&EMS_Init,true);//done
	const char* ems_settings[] = { "true", "emsboard", "emm386", "false", 0};
	Pstring = secprop->Add_string("ems",Property::Changeable::WhenIdle,"true");
	Pstring->Set_values(ems_settings);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Enable EMS support. The default (=true) provides the best compatibility but certain applications\n"
		                    "may run better with other choices, or require EMS support to be disabled (=false) to work at all.\n");

	Pbool = secprop->Add_bool("umb",Property::Changeable::WhenIdle,true);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable UMB support.");

	Pint = secprop->Add_int("files",Property::Changeable::OnlyAtStart,127);
	Pint->Set_help(         "================================================================================================\n"
	                         "Number Of File Handles For DOS programs. (equivalent to \"files=\" in config.sys) (Default: 127)");
	
	const char* ldz_settings[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s",
	                               "t", "u", "v", "w", "x", "y", "z", 0};
	Pstring = secprop->Add_string("LastDrive",Property::Changeable::WhenIdle,"e");
	Pstring->Set_values(ldz_settings);
	Pstring->Set_help(      "================================================================================================\n"
	                        "Increase and set max Logical Drives (equivalent to \"lastdrive=\" in config.sys (Default is e:)\n");
							
	secprop->AddInitFunction(&DOS_KeyboardLayout_Init,true);
	Pstring = secprop->Add_string("keyboardlayout",Property::Changeable::WhenIdle, "auto");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Language code of the keyboard layout (or none).");
							
	Pbool = secprop->Add_bool("Disable_DOS4GW",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Disable DOS/4GW Version 2.01a on Drive Z:\n"
							"You can Unbind an old DOS/4GW e.q 1.97 from the Executbale by usinf the command DOS4GU.exe. Yes,\n"
							"I have this renamed. Use a newer DOS/4GW speed up your Games. Example? Terminator: FS Skynet");

	Pbool = secprop->Add_bool("Disable_DOS32A",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Disable the DOS32A Extender Version 9.1.2 on Drive Z:");	

	Pbool = secprop->Add_bool("Disable_CWSDPMI",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Disable the CWSDPMI on Drive Z: (For compatibility)");	

	Pbool = secprop->Add_bool("DOS4GW_As_DOS32",Property::Changeable::WhenIdle,false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Use DOS32A Extender 9.12 instead of DOS4GW 2.01 (e.q. as Benchmark and Compare)");							
							
							
	// Mscdex
	secprop->AddInitFunction(&MSCDEX_Init);
	secprop->AddInitFunction(&DRIVES_Init);
	secprop->AddInitFunction(&CDROM_Image_Init);
#if C_IPX
	secprop=control->AddSection_prop("ipx",&IPX_Init,true);
	Pbool = secprop->Add_bool("ipx",Property::Changeable::WhenIdle, false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable ipx over UDP/IP emulation.");
#endif

#ifdef C_NE2000
	secprop=control->AddSection_prop("ne2000",&NE2K_Init,true);
	Pbool = secprop->Add_bool("ne2000", Property::Changeable::WhenIdle, false);
	Pbool->Set_help(        "================================================================================================\n"
	                        "Enable Ethernet passthrough. Requires [Win]Pcap.");	

	Phex = secprop->Add_hex("nicbase", Property::Changeable::WhenIdle, 0x300);
	Phex->Set_help(        "================================================================================================\n"
	                        "The base address of the NE2000 board.");	

	Pint = secprop->Add_int("nicirq", Property::Changeable::WhenIdle, 6);
	Pint->Set_help(        "================================================================================================\n"
	                        "The interrupt it uses. Note serial2 uses IRQ3 as default and remember IRQ 10 use IDE on more as\n"
							"3 Interfaces");		

	Pstring = secprop->Add_string("macaddr", Property::Changeable::WhenIdle,"AC:DE:48:88:99:AA");	
	Pstring->Set_help(      "================================================================================================\n"
	                        "The physical address the emulator will use on your network. If you have multiple DOSBoxes running\n"
							"on your network this has to be changed for each. AC:DE:48 is an address range reserved for private\n"
                            "use, so modify the last three number blocks I.e. AC:DE:48:88:99:AB.");	
	
	Pstring = secprop->Add_string("realnic", Property::Changeable::WhenIdle,"list");
	Pstring->Set_help(      "================================================================================================\n"
	                        "Specifies which of your network interfaces is used. Write \'list\' here to see the list of devices\n"
                            "in the log File. Then make your choice and put either the interface number (2 or something) or\n"
                            "a part of your adapters name, e.g. VIA here.");		
#endif // C_NE2000


//	secprop->AddInitFunction(&CREDITS_Init);

	//TODO ?
	secline=control->AddSection_line("autoexec",&AUTOEXEC_Init);
	MSG_Add("AUTOEXEC_CONFIGFILE_HELP",
		"Lines in this section will be run at startup.\n"
		"=================================================================================\n"		
		"CONFIG -WRITECONF\n"
		"EXIT\n"
		"=================================================================================\n"
		"ECHO OFF\n"
		"MOUSECAP\n"
		"MIXER MASTER 20\n"		
		"=================================================================================\n"
		"SET PATH=Z:\\;C:\\;C:\\DOS;C:\\WINDOWS\n\n"
		);
	MSG_Add("EXEC_MOUNT_HD",		
        "-- Example: Image Mount Options HD (The Simplest Way)\n"
		"MOUNT    C \".\\HDD-C\" freesize 250\n"
		"MOUNT    D \".\\HDD-D\" freesize 250\n\n\n"
        "-- Example: Image Mount Options HD (As Image variant): (Using 250MB Harddrive)\n"
		"SET HD_DIR=.\\HDD_IMAGE\n"
		"SET HDIMG1=MS-DOS 5.00.img\n"
		"SET HDIMG2=MS-DOS 6.22.img\n"
		"IMGMOUNT 2 \"%HD_DIR%\\%HDIMG1%\" -size 512,63,16,507 -fs none\n\n\n"
		"=================================================================================\n"
		);
	MSG_Add("EXEC_MOUNT_CD",		
        "-- Example: Image Mount Options CD:\n"
        "SET CD_DIR=.\\HDD_CDROM\n"
		"SET CDROM1=GAME1.CUE\n"
		"SET CDROM2=GAME2.CUE\n\n"
        "-- Example: As Directory or Image (Iso, Bin, Cue)\n"
		"MOUNT    E \"%CD_DIR%\" -t cdrom\n"
		"IMGMOUNT E \"%CD_DIR%\\%CDROM1%\"  \"%CD_DIR%\\%CDROM2%\" -t cdrom\n\n"
		"=================================================================================\n"
		);
	MSG_Add("EXEC_MOUNT_DD",		
        "-- Example: Image Mount Options Floppy:\n"
        "SET DSKDIR=.\\HDD_FLOPPYS\n"
		"SET DISK01=%DSKDIR%\\DISK1.IMA\n"
		"SET DISK02=%DSKDIR%\\DISK2.IMA\n"
		"SET DISK03=%DSKDIR%\\DISK3.IMA\n"
		"SET DISK04=%DSKDIR%\\DISK4.IMA\n"
		"SET DISK05=%DSKDIR%\\DISK5.IMA\n"
		"SET DISK06=%DSKDIR%\\DISK6.IMA\n\n"
        "-- Example: Image, Boot, Multi (Multiples Floppy in One Drive) (img, ima)\n"
		"IMGMOUNT A \"%DISK01%\" \"%DISK02%\" \"%DISK03%\" \"%DISK04%\" \"%DISK05%\" \"%DISK06%\" -t floppy\n"
		"BOOT       \"%DISK01%\" \"%DISK02%\" \"%DISK03%\" \"%DISK04%\" \"%DISK05%\" \"%DISK06%\" -l A\n\n"
		"REM More Mount Examples: http:////www.dosbox.com//wiki//IMGMOUNT\n\n\n"
		"=================================================================================\n"
		);
	MSG_Add("EXEC_MOUNT_SB",		
		"SET BLASTER=A220 I5 D1 T3 P330 H6 E620\n"
		" |    |     |    |  |  |  |    |  |\n"
		" |    |     |    |  |  |  |    |  |_______ AWE 32 Only Parameter\n"
		" |    |     |    |  |  |  |    |__________ \"High\" DMA Channel\n"
		" |    |     |    |  |  |  |_______________ MIDI Port\n"
		" |    |     |    |  |  |__________________ Type of Card\n"
		" |    |     |    |  |_____________________ DMA Channel\n"
		" |    |     |    |________________________ Interrupt\n"
		" |    |     |_____________________________ Port Address\n"
		" |    |___________________________________ Environment Variable\n"
		" |________________________________________ DOS Command\n\n"
		"SET MIDI=SYNTH:1 MAP:E\n\n"
		"=================================================================================\n"
	);
	MSG_Add("CONFIGFILE_INTRO",	
	        "# This is the configuration file for DOSBox %s. Lines starting with a # are comment lines and\n"
	        "# are ignored by DOSBox. They are used to (briefly) document the effect of each option.\n");
	MSG_Add("CONFIG_SUGGESTED_VALUES", "Possible values");

	control->SetStartUp(&SHELL_Init);
}
