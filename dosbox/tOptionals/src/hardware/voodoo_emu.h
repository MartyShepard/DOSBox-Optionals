 /*
 *  Copyright (C) 2002-2011  The DOSBox Team
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


#ifndef DOSBOX_VOODOO_EMU_H
#define DOSBOX_VOODOO_EMU_H

#include <stdlib.h>

#include "dosbox.h"

#include "SDL.h"

#include "voodoo_types.h"
#include "voodoo_data.h"

extern voodoo_state *v;

void voodoo_w(UINT32 offset, UINT32 data, UINT32 mask);
UINT32 voodoo_r(UINT32 offset);

void voodoo_init(int type);
void voodoo_shutdown();
extern void voodoo_leave(void);

extern void voodoo_activate(void);
void voodoo_update_dimensions(void);
void voodoo_set_window(void);
void voodoo_set_fullscreen(void);

void voodoo_vblank_flush(void);
void voodoo_swap_buffers(voodoo_state *v);

extern bool voodoo_ogl_init(voodoo_state *v);

extern void Voodoo_UpdateScreenStart();
extern void Voodoo_Output_Enable(bool enabled);
extern bool Voodoo_GetRetrace();
extern double Voodoo_GetVRetracePosition();
extern double Voodoo_GetHRetracePosition();

extern void CPU_Core_Dyn_X86_SaveDHFPUState(void);
extern void CPU_Core_Dyn_X86_RestoreDHFPUState(void);

extern ExtVoodooMaschine extVoodoo;

struct SDL_Block_Voodoo {
	SDL_Window   *surface;
	SDL_Window   *Dosbox_Surface;
	
	struct {
		SDL_GLContext GLContext;
		int 	minFilter;

		const char*	sglshademdl;	
		const char*	sfiltering;	
		const char* sgl_wrap_s;	
		const char* sgl_wrap_t;

		int	  GL_filtering;		
		int	  GL_ShadeModel;		
		int   n_ClipLowYHigh;
		int   glZoomFaktor_W;
		int   glZoomFaktor_H;	
		int   gl_Major_Version;
		int   gl_Minor_Version;	
		int   gl_wrap_s;		
		int   gl_wrap_t;
		int   nLFBMode;		
		
		bool  compatibleFlag;
		bool  glScissor_flag;
		bool  glP_Smoth_flag;
		bool  glL_Smoth_flag;
		bool  glBlendFc_flag;
		bool  gl_GLFog__flag;				
		bool  glGMipMap_flag;
		bool  glPersCor_flag;
		bool  glG_Smoth_flag;	
		bool  a_ClipLowYHigh;
		bool  gl_QuadsDraw;
		bool  voodoo_aspect;
		bool  sh_FbzcpCca_Sw2;		
		bool  gl_PointSize_use;
		bool  bLFBFixBack;
		bool  bLFBFixFrnt;
		bool  bLFBDebugLg;
		float gl_PointSize_num;
		float Anisotropic_Level;		
		float gl_ortho_zNear;
		float gl_ortho_zFar;
	
		void *framebuf;
	}opengl;
	
	Uint32 ScreenType;
	Uint32 sdl_WindowFlags;
	Uint32 sdl_FullS_Flags;
	Uint32 ScrOpenGL_Flags;
	SDL_PixelFormat *pixelFormat;
	
	bool OpenGLDesktopFullScreen;
	bool fullscreen;
	bool full_fixed;
			
	struct {
		bool fullscreen_desktop;		
		int wTBH;
		int Index;		
	}desktop;
	
	int pciW;
	int pciH;
	int pciFSW;
	int pciFSH;	
	int posX;
	int posY;
	int posX_Old;
	int posY_Old;	
	int windowstaskbaradjust;
	int displaynumber;
	
	struct {
		const char* output;
		const char* texture;
	}dosbox;	
	
	Uint32 dosboxScreenId;	
};
static SDL_Block_Voodoo sdlVoodoo;

#endif
