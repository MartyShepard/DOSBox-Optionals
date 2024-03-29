/*
 *  Copyright (C) 2002-2021  The DOSBox Team
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


#ifndef DOSBOX_DOSBOX_H
#define DOSBOX_DOSBOX_H

#include <string>
#include "config.h"
/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/
#include "rgb24.h"
/* Custom S3 VGA /////////////////////////////////////////////////////////////////////////////////////////////*/


GCC_ATTRIBUTE(noreturn) void E_Exit(const char * message,...) GCC_ATTRIBUTE( __format__(__printf__, 1, 2));

void MSG_Add(const char*,const char*); //add messages to the internal languagefile
const char* MSG_Get(char const *);     //get messages from the internal languagefile

class Section;

typedef Bitu (LoopHandler)(void);

void DOSBOX_RunMachine();
void DOSBOX_SetLoop(LoopHandler * handler);
void DOSBOX_SetNormalLoop();

void DOSBOX_Init(void);

void Set_Window_HMenu(void);

void MenuBrowseCDImage(char drive, int num);
void MenuBrowseFDImage(char drive, int num);

class Config;
extern Config * control;

enum MachineType {
	MCH_HERC,
	MCH_CGA,
	MCH_TANDY,
	MCH_PCJR,
	MCH_EGA,
	MCH_VGA,
	MCH_AMSTRAD
};

enum SVGACards {
	SVGA_None,
	SVGA_S3Trio,
	SVGA_TsengET4K,
	SVGA_TsengET3K,
	SVGA_ParadisePVGA1A
}; 


extern SVGACards svgaCard;
extern MachineType machine;

extern bool SDLNetInited;
extern bool mono_cga;
extern bool CPU_FastForward;

extern int nCurrentDisplay;

/* Für die Intros */
extern int nCurrent_Memory;
extern int nCurrent_VidSize_S3Trio;
extern int nCurrent_VidSize_S3ET3K;
extern int nCurrent_VidSize_S3ET4K;
extern int nCurrent_VidSize_PVGA1A;
extern std::string nCurrentVBEMode;
extern std::string nCurrentCPUCore;
extern std::string nCurrentCPUType;
extern std::string nCurrent_Voodoo;
extern std::string nCurrent_SBType;
extern std::string nCurrentGUSType;
extern std::string nCurrentInnovtn;
extern std::string nCurrentSnTandy;
extern std::string nCurrent_Disney;
extern std::string nCurrent_Ps1SND;
extern std::string nCurrent_Ne2000;

extern bool bVoodooInUse;
extern bool bVoodooOpen;
extern bool bVoodooUpdateWinRes;
extern bool bVoodooUseHighRatio;

/* Menu Bools */
extern bool useSoundGus;
extern bool useSoundSB;
extern bool useSoundAdlib;
extern bool useSoundMT32;
extern bool useSoundDisney;
extern bool useSoundTandy;
extern bool useSoundSSI2k1;
extern bool useSoundPS1;

/* Virtueller Modus */
extern bool isVirtualModus;

/* WarteZeit für das Label */
extern bool MediaLabelChanged;

/* Vooodoo Structure to Syncronize mit SDL Main */
struct ExtVoodooMaschine{
	int   pciH;
	int   pciW;
	int   pciFSH;
	int   pciFSW;
	
	int   GL_filtering;
	int   GL_shade;
	
	int   gl_wrap_s;		
	int   gl_wrap_t;

	bool  bLFBFixBack;
	bool  bLFBFixFrnt;	
	
	bool  bForceWindowUpdate;
	bool  bForceFullSnUpdate;
	
	bool  Pic_Alternate;
	
	bool  gl_QuadsDraw;
	
	bool  bLFBDebugLg;
	
	bool  ChacheDelete;
	bool  Use3DFXCycles;
	bool  voodoo_aspect;
	
	bool  sh_FbzcpCca_Sw2;
	bool  glP_Smoth_flag;
	bool  glL_Smoth_flag;
	bool  glBlendFc_flag;
	bool  glGMipMap_flag;
	bool  glPersCor_flag;
	bool  glG_Smoth_flag;
	bool  gl_GLFog__flag;
	
	bool  compatibleFlag;
	bool  glScissor_flag;
	
	int RGB_Type;
	int RGB_Format;

	int GLDark;
	int GLScan;

};
extern void FX_Menu(int /*Menu*/Option);
extern void Restart(bool pressed);
extern void UI_Run(bool);

#define IS_TANDY_ARCH ((machine==MCH_TANDY) || (machine==MCH_PCJR))
#define IS_EGAVGA_ARCH ((machine==MCH_EGA) || (machine==MCH_VGA))
#define IS_VGA_ARCH (machine==MCH_VGA)
#define TANDY_ARCH_CASE MCH_TANDY: case MCH_PCJR
#define EGAVGA_ARCH_CASE MCH_EGA: case MCH_VGA
#define VGA_ARCH_CASE MCH_VGA

/* Added from DOSBox-X /////////////////////////////////////////////////////*/
#define GetHWND() (0)
/* Added from DOSBox-X /////////////////////////////////////////////////////*/

#ifndef DOSBOX_LOGGING_H
#include "logging.h"
#endif // the logging system.

#endif /* DOSBOX_DOSBOX_H */
