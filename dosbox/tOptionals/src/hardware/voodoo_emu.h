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

#if defined(_MSC_VER)
#include "SDL2/include/SDL.h"
#else
#include "SDL2/SDL.h"
#endif

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

struct READ_BUFFER_BLOCK {
	struct {
		struct {
			INT32   Y; 		// cached_line_front_y
			INT32   W; 		// cached_line_front_width
			INT32   L; 		// cached_line_front_length
			INT32   P; 		// cached_line_front_pixels
			int   CountA;
			int   CountB;
			int	  r;
			int	  g;
			int	  b;
			int	  a;
			UINT32* Buffer; // cached_line_front_data
		}Frnt;
		struct {
			INT32   Y; 		// cached_line_back_y
			INT32   W; 		// cached_line_back_width
			INT32   L; 		// cached_line_back_length
			INT32   P; 		// cached_line_back_pixels
			int  CountA;
			int  CountB;
			int	  r;
			int	  g;
			int	  b;
			int	  a;
			UINT32* Buffer; // cached_line_back_data
		}Back;
	}Line;
};
static READ_BUFFER_BLOCK Cache;

typedef struct vidmode_s
{
	const char* description;
	int width, height;
	int mode;
	int htotal;
	int hvis;
	int hbp;
	int vtotal;
	int vvis;
	int vbp;

} vidmode_t;


struct OpenGL_VertexData
{
	INT32 iterR;
	INT32 iterG;
	INT32 iterB;
	INT32 iterA;
	INT32 iterZ;
	INT64 iterW;
	INT32 VertX; /*dx*/
	INT32 VertY; /*dy*/
	INT32 d;	 /*??*/
};
static OpenGL_VertexData VertexData;
typedef unsigned int GLenum;

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
		/*
		int   n_ClipLowYHigh;
		*/	
		int   glZoomFaktor_W; /*Temporary Allowed*/
		int   glZoomFaktor_H; /*Temporary Allowed*/

		int   gl_Major_Version;
		int   gl_Minor_Version;	
		int   gl_wrap_s;		
		int   gl_wrap_t;
		int   nLFBMode;		
		int   LogCntY;			
		int   LogCntX;	

		bool  compatibleFlag;

		bool  glScissor_flag; /*Temporary True*/
		
		bool  glP_Smoth_flag;
		bool  glL_Smoth_flag;
		bool  glBlendFc_flag;
		bool  gl_GLFog__flag;				
		bool  glGMipMap_flag;
		bool  glPersCor_flag;
		bool  glG_Smoth_flag;
		/*
		bool  a_ClipLowYHigh;
		*/
		bool  gl_QuadsDraw;
		bool  voodoo_aspect;
		bool  voodoo_aspect_Save;
		bool  sh_FbzcpCca_Sw2;		
		bool  bLFBFixBack;
		bool  bLFBFixFrnt;
		bool  bLFBDebugLg;
		
		float Anisotropic_Level;		
		float gl_ortho_zNear;
		float gl_ortho_zFar;
		/*
		float CoordsY;
		float CoordsX;	
		*/
		void *framebuf;

		bool ChacheDelete;
		bool Use3DFXCycles;
		GLenum RGB_Type;
		GLenum RGB_Format;
		const char* sRGBType;
		const char* sRGBFormat;

		int Bright;
		int ScanMode;

		int a_ClipLowYHigh; /*Obsolete*/
		int n_ClipLowYHigh; /*Obsolete*/
	}opengl;
	
	Uint32 ScreenType;
	Uint32 sdl_WindowFlags;
	Uint32 sdl_FullS_Flags;
	Uint32 ScrOpenGL_Flags;
	SDL_PixelFormat *pixelFormat;
	
	bool OpenGLDesktopFullScreen;
	bool fullscreen;
	bool full_fixed;
	bool isFullscreen;
			
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
	struct {
		bool bDrawPixelLFB0;
		bool bDrawPixelLFB1;
		bool bDrawPixelLFB2;
		int  iDrawPixelLFB0;
		int  iDrawPixelLFB1;
		int  iDrawPixelLFB2;
	}count;
	struct {
		Uint32 AlphaMode;
		Uint32 r_fbzMode;
	}extra;
};
static SDL_Block_Voodoo sdlVoodoo;

#endif
