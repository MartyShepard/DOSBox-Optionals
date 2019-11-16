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

/* $Id: sdlmain.cpp,v 1.51 2003-10-14 23:32:32 harekiet Exp $ */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "dosbox.h"
#include "video.h"
#include "keyboard.h"
#include "mouse.h"
#include "joystick.h"
#include "pic.h"
#include "timer.h"
#include "setup.h"
#include "debug.h"

//#define DISABLE_JOYSTICK
#define C_GFXTHREADED 1						//Enabled by default

#if !(ENVIRON_INCLUDED)
extern char** environ;
#endif


#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define STDOUT_FILE	TEXT("stdout.txt")
#define STDERR_FILE	TEXT("stderr.txt")
#endif

struct SDL_Block {
	volatile bool active;							//If this isn't set don't draw
	volatile bool drawing;
	Bitu width;
	Bitu height;
	Bitu bpp;
	Bitu flags;
	GFX_ModeCallBack mode_callback;
	bool full_screen;
	bool wait_on_error;
	SDL_Surface * surface;
	SDL_Surface * blit_surface;
	SDL_Joystick * joy;
	SDL_cond *cond;
#if C_GFXTHREADED
	SDL_mutex *mutex;
	SDL_Thread *thread;
	SDL_sem *sem;
	volatile bool kill_thread;
#endif
	GFX_DrawCallBack draw_callback;
	struct {
		bool autolock;
		bool autoenable;
		bool requestlock;
		bool locked;
		Bitu sensitivity;
	} mouse;
};

static SDL_Block sdl;
static void CaptureMouse(void);

void GFX_SetTitle(Bits cycles,Bits frameskip){
	char title[200]={0};
	static Bits internal_cycles=0;
	static Bits internal_frameskip=0;
	if(cycles != -1) internal_cycles = cycles;
	if(frameskip != -1) internal_frameskip = frameskip;
	sprintf(title,"DOSBox %s, Cpu Cycles: %8d, Frameskip %2d",VERSION,internal_cycles,internal_frameskip);
	SDL_WM_SetCaption(title,VERSION);
}

/* Reset the screen with current values in the sdl structure */
static void ResetScreen(void) {
	GFX_Stop();
	if (sdl.full_screen) { 
		if (sdl.flags & GFX_SHADOW) {
			withshadow:
			/* TODO Maybe allocate a shadow surface and blit yourself and do real double buffering too */
			sdl.surface=SDL_SetVideoMode(sdl.width,sdl.height,sdl.bpp,SDL_SWSURFACE|SDL_FULLSCREEN);
		} else {
			Bitu test_bpp=SDL_VideoModeOK(sdl.width,sdl.height,sdl.bpp,SDL_HWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN|SDL_DOUBLEBUF);
			if (test_bpp != sdl.bpp) 
				goto withshadow;
			sdl.surface=SDL_SetVideoMode(sdl.width,sdl.height,sdl.bpp,SDL_HWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN|SDL_DOUBLEBUF);
		}
	} else {
		if (sdl.flags & GFX_FIXED_BPP) {
			withfixed:
			sdl.surface=SDL_SetVideoMode(sdl.width,sdl.height,sdl.bpp,SDL_HWSURFACE);
		} else {
			sdl.surface=SDL_SetVideoMode(sdl.width,sdl.height,0,SDL_HWSURFACE);
			if (sdl.surface->format->BitsPerPixel==24) 
				goto withfixed;
		}
	}
	if (sdl.surface==0) {
		E_Exit("SDL:Can't get a surface error:%s.",SDL_GetError());
	}
	/* Create special surface for direct blitting if needed */
	if (sdl.blit_surface) SDL_FreeSurface(sdl.blit_surface);
	sdl.blit_surface=SDL_CreateRGBSurfaceFrom(0,0,0,sdl.surface->format->BytesPerPixel,0,
		sdl.surface->format->Rmask,sdl.surface->format->Gmask,sdl.surface->format->Bmask,sdl.surface->format->Amask
	);
			

	GFX_SetTitle(-1,-1);

	Bitu flags=MODE_SET;
	if (sdl.full_screen) flags|=MODE_FULLSCREEN;
	if (sdl.mode_callback) sdl.mode_callback(sdl.surface->w,sdl.surface->h,sdl.surface->format->BitsPerPixel,sdl.surface->pitch,flags);
	GFX_Start();
}


void GFX_SetSize(Bitu width,Bitu height,Bitu bpp,Bitu flags,GFX_ModeCallBack mode_callback, GFX_DrawCallBack draw_callback){
	GFX_Stop();
	sdl.width=width;
	sdl.height=height;
	sdl.bpp=bpp;
	sdl.flags=flags;
	sdl.mode_callback=mode_callback;
	sdl.draw_callback=draw_callback;
	ResetScreen();
}


static void CaptureMouse(void) {
	sdl.mouse.locked=!sdl.mouse.locked;
	if (sdl.mouse.locked) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
	}
}

static void SwitchFullScreen(void) {
	sdl.full_screen=!sdl.full_screen;
	if (sdl.full_screen) {
//TODO Give an resize event
		if (!sdl.mouse.locked) CaptureMouse();
	} else {
		if (sdl.mouse.locked) CaptureMouse();
	}
	ResetScreen();
}

void GFX_SwitchFullScreen(void) {
    SwitchFullScreen();
}

/* Special render part handler that blits directly to the output screen */
void GFX_Render_Blit(Bit8u * src,Bitu x,Bitu y,Bitu dx,Bitu dy) {
	sdl.blit_surface->w=dx;
	sdl.blit_surface->h=dy;
	sdl.blit_surface->pitch=dx;
	sdl.blit_surface->pixels=src;
	SDL_UnlockSurface(sdl.surface);
	SDL_Rect dest;
	dest.x=x;
	dest.y=y;
	SDL_BlitSurface(sdl.blit_surface,0,sdl.surface,&dest);
}

static void SDL_DrawScreen(void) {
		sdl.drawing=true;
		if (SDL_MUSTLOCK(sdl.surface)) {
			if (SDL_LockSurface(sdl.surface)) {
				sdl.drawing=false;
				return;
			}
		}
		sdl.draw_callback(sdl.surface->pixels);
		if (SDL_MUSTLOCK(sdl.surface)) {
			SDL_UnlockSurface(sdl.surface);
		}
		SDL_Flip(sdl.surface);
		sdl.drawing=false;	
}

#if C_GFXTHREADED
int SDL_DisplayThread(void * data) {
	while (!SDL_SemWait(sdl.sem)) {
		if (sdl.kill_thread) return 0;
		if (!sdl.active) continue;
		if (sdl.drawing) continue;
		SDL_mutexP(sdl.mutex);
		SDL_DrawScreen();
		SDL_mutexV(sdl.mutex);
	}
	return 0;
}
#endif

void GFX_DoUpdate(void) {
	if (!sdl.active) 
		return;
	if (sdl.drawing)return;
#if C_GFXTHREADED
	SDL_SemPost(sdl.sem);
#else 
	SDL_DrawScreen();
#endif

}


void GFX_SetPalette(Bitu start,Bitu count,GFX_PalEntry * entries) {
#if C_GFXTHREADED
	if (SDL_mutexP(sdl.mutex)) {
		E_Exit("SDL:Can't lock Mutex");
	};
#endif
	/* I should probably not change the GFX_PalEntry :) */
	if (sdl.full_screen) {
		if (!SDL_SetPalette(sdl.surface,SDL_PHYSPAL,(SDL_Color *)entries,start,count)) {
			E_Exit("SDL:Can't set palette");
		}
	} else {
		if (!SDL_SetPalette(sdl.surface,SDL_LOGPAL,(SDL_Color *)entries,start,count)) {
			E_Exit("SDL:Can't set palette");
		}
	}
#if C_GFXTHREADED
	if (SDL_mutexV(sdl.mutex)) {
		E_Exit("SDL:Can't release Mutex");
	};
#endif
}

Bitu GFX_GetRGB(Bit8u red,Bit8u green,Bit8u blue) {
	return SDL_MapRGB(sdl.surface->format,red,green,blue);
}

void GFX_Stop() {
#if C_GFXTHREADED
	SDL_mutexP(sdl.mutex);
#endif
	sdl.active=false;
#if C_GFXTHREADED
	SDL_mutexV(sdl.mutex);
#endif

}

void GFX_Start() {
	sdl.active=true;
}

static void GUI_ShutDown(Section * sec) {
	GFX_Stop();
	if (sdl.mouse.locked) CaptureMouse();
	if (sdl.full_screen) SwitchFullScreen();
#if C_GFXTHREADED
	sdl.kill_thread=true;
	SDL_SemPost(sdl.sem);
	SDL_WaitThread(sdl.thread,0);
	SDL_DestroyMutex(sdl.mutex);
	SDL_DestroySemaphore(sdl.sem);
#endif

}

static void KillSwitch(void){
	throw 1;
}

static void GUI_StartUp(Section * sec) {
	sec->AddDestroyFunction(&GUI_ShutDown);
	Section_prop * section=static_cast<Section_prop *>(sec);
	sdl.active=false;
	sdl.full_screen=false;
	sdl.wait_on_error=section->Get_bool("waitonerror");
	sdl.mouse.locked=false;
	sdl.mouse.requestlock=false;
	sdl.mouse.autoenable=section->Get_bool("autolock");
	sdl.mouse.autolock=false;
	sdl.mouse.sensitivity=section->Get_int("sensitivity");
	sdl.blit_surface=0;
#if C_GFXTHREADED
	sdl.kill_thread=false;
	sdl.mutex=SDL_CreateMutex();
	sdl.sem=SDL_CreateSemaphore(0);
	sdl.thread=SDL_CreateThread(&SDL_DisplayThread,0);
#endif
	/* Initialize screen for first time */
	sdl.surface=SDL_SetVideoMode(640,400,0,0);
	if (sdl.surface->format->BitsPerPixel==24) {
		LOG_MSG("SDL:You are running in 24 bpp mode, this will slow down things!");
	}
	GFX_SetSize(640,400,8,0,0,0);
	SDL_EnableKeyRepeat(250,30);
	SDL_EnableUNICODE(1);
/* Get some Keybinds */
	KEYBOARD_AddEvent(KBD_f9,KBD_MOD_CTRL,KillSwitch);
	KEYBOARD_AddEvent(KBD_f10,KBD_MOD_CTRL,CaptureMouse);
	KEYBOARD_AddEvent(KBD_enter,KBD_MOD_ALT,SwitchFullScreen);

}

void Mouse_AutoLock(bool enable) {
	sdl.mouse.autolock=enable;
	if (enable && sdl.mouse.autoenable) sdl.mouse.requestlock=true;
	else sdl.mouse.requestlock=false;
}

static void HandleKey(SDL_KeyboardEvent * key) {
	KBD_KEYS code;
	switch (key->keysym.sym) {
	case SDLK_1:code=KBD_1;break;
	case SDLK_2:code=KBD_2;break;
	case SDLK_3:code=KBD_3;break;
	case SDLK_4:code=KBD_4;break;
	case SDLK_5:code=KBD_5;break;
	case SDLK_6:code=KBD_6;break;
	case SDLK_7:code=KBD_7;break;
	case SDLK_8:code=KBD_8;break;
	case SDLK_9:code=KBD_9;break;
	case SDLK_0:code=KBD_0;break;

	case SDLK_q:code=KBD_q;break;
	case SDLK_w:code=KBD_w;break;
	case SDLK_e:code=KBD_e;break;
	case SDLK_r:code=KBD_r;break;
	case SDLK_t:code=KBD_t;break;
	case SDLK_y:code=KBD_y;break;
	case SDLK_u:code=KBD_u;break;
	case SDLK_i:code=KBD_i;break;
	case SDLK_o:code=KBD_o;break;
	case SDLK_p:code=KBD_p;break;

	case SDLK_a:code=KBD_a;break;
	case SDLK_s:code=KBD_s;break;
	case SDLK_d:code=KBD_d;break;
	case SDLK_f:code=KBD_f;break;
	case SDLK_g:code=KBD_g;break;
	case SDLK_h:code=KBD_h;break;
	case SDLK_j:code=KBD_j;break;
	case SDLK_k:code=KBD_k;break;
	case SDLK_l:code=KBD_l;break;

	case SDLK_z:code=KBD_z;break;
	case SDLK_x:code=KBD_x;break;
	case SDLK_c:code=KBD_c;break;
	case SDLK_v:code=KBD_v;break;
	case SDLK_b:code=KBD_b;break;
	case SDLK_n:code=KBD_n;break;
	case SDLK_m:code=KBD_m;break;


	case SDLK_F1:code=KBD_f1;break;
	case SDLK_F2:code=KBD_f2;break;
	case SDLK_F3:code=KBD_f3;break;
	case SDLK_F4:code=KBD_f4;break;
	case SDLK_F5:code=KBD_f5;break;
	case SDLK_F6:code=KBD_f6;break;
	case SDLK_F7:code=KBD_f7;break;
	case SDLK_F8:code=KBD_f8;break;
	case SDLK_F9:code=KBD_f9;break;
	case SDLK_F10:code=KBD_f10;break;
	case SDLK_F11:code=KBD_f11;break;
	case SDLK_F12:code=KBD_f12;break;

	case SDLK_ESCAPE:code=KBD_esc;break;
	case SDLK_TAB:code=KBD_tab;break;
	case SDLK_BACKSPACE:code=KBD_backspace;break;
	case SDLK_RETURN:code=KBD_enter;break;
	case SDLK_SPACE:code=KBD_space;break;

	case SDLK_LALT:code=KBD_leftalt;break;
	case SDLK_RALT:code=KBD_rightalt;break;
	case SDLK_LCTRL:code=KBD_leftctrl;break;
	case SDLK_RCTRL:code=KBD_rightctrl;break;
	case SDLK_LSHIFT:code=KBD_leftshift;break;
	case SDLK_RSHIFT:code=KBD_rightshift;break;

	case SDLK_CAPSLOCK:code=KBD_capslock;break;
	case SDLK_SCROLLOCK:code=KBD_scrolllock;break;
	case SDLK_NUMLOCK:code=KBD_numlock;break;
	
	case SDLK_BACKQUOTE:code=KBD_grave;break;
	case SDLK_MINUS:code=KBD_minus;break;
	case SDLK_EQUALS:code=KBD_equals;break;
	case SDLK_BACKSLASH:code=KBD_backslash;break;
	case SDLK_LEFTBRACKET:code=KBD_leftbracket;break;
	case SDLK_RIGHTBRACKET:code=KBD_rightbracket;break;

	case SDLK_SEMICOLON:code=KBD_semicolon;break;
	case SDLK_QUOTE:code=KBD_quote;break;
	case SDLK_PERIOD:code=KBD_period;break;
	case SDLK_COMMA:code=KBD_comma;break;
	case SDLK_SLASH:code=KBD_slash;break;

	case SDLK_INSERT:code=KBD_insert;break;
	case SDLK_HOME:code=KBD_home;break;
	case SDLK_PAGEUP:code=KBD_pageup;break;
	case SDLK_DELETE:code=KBD_delete;break;
	case SDLK_END:code=KBD_end;break;
	case SDLK_PAGEDOWN:code=KBD_pagedown;break;
	case SDLK_LEFT:code=KBD_left;break;
	case SDLK_UP:code=KBD_up;break;
	case SDLK_DOWN:code=KBD_down;break;
	case SDLK_RIGHT:code=KBD_right;break;

	case SDLK_KP1:code=KBD_kp1;break;
	case SDLK_KP2:code=KBD_kp2;break;
	case SDLK_KP3:code=KBD_kp3;break;
	case SDLK_KP4:code=KBD_kp4;break;
	case SDLK_KP5:code=KBD_kp5;break;
	case SDLK_KP6:code=KBD_kp6;break;
	case SDLK_KP7:code=KBD_kp7;break;
	case SDLK_KP8:code=KBD_kp8;break;
	case SDLK_KP9:code=KBD_kp9;break;
	case SDLK_KP0:code=KBD_kp0;break;

	case SDLK_KP_DIVIDE:code=KBD_kpslash;break;
	case SDLK_KP_MULTIPLY:code=KBD_kpmultiply;break;
	case SDLK_KP_MINUS:code=KBD_kpminus;break;
	case SDLK_KP_PLUS:code=KBD_kpplus;break;
	case SDLK_KP_ENTER:code=KBD_kpenter;break;
	case SDLK_KP_PERIOD:code=KBD_kpperiod;break;

	/* Special Keys */
	default:
		code=KBD_1;
		LOG(LOG_KEYBOARD,LOG_ERROR)("Unhandled SDL keysym %d",key->keysym.sym);
		break;
	}
	/* Check the modifiers */
	Bitu mod=
		((key->keysym.mod & KMOD_CTRL) ? KBD_MOD_CTRL : 0) |
		((key->keysym.mod & KMOD_ALT) ? KBD_MOD_ALT : 0) |
		((key->keysym.mod & KMOD_SHIFT) ? KBD_MOD_SHIFT : 0);
	Bitu ascii=key->keysym.unicode<128 ? key->keysym.unicode : 0;
#ifdef MACOSX
	// HACK: Fix backspace on Mac OS X 
	// REMOVE ME oneday
	if (code==KBD_backspace)
		ascii=8;
#endif
	KEYBOARD_AddKey(code,ascii,mod,(key->state==SDL_PRESSED));
}

static void HandleMouseMotion(SDL_MouseMotionEvent * motion) {
	if (sdl.mouse.locked) {
		Mouse_CursorMoved((float)motion->xrel*sdl.mouse.sensitivity/100,(float)motion->yrel*sdl.mouse.sensitivity/100);
	} else {
//		Mouse_CursorSet((float)motion->x/(float)sdl.width,(float)motion->y/(float)sdl.height);
	}
}

static void HandleMouseButton(SDL_MouseButtonEvent * button) {
	switch (button->state) {
	case SDL_PRESSED:
		if (sdl.mouse.requestlock && !sdl.mouse.locked) {
			CaptureMouse();
			// Dont pass klick to mouse handler
			break;
		}
		switch (button->button) {
		case SDL_BUTTON_LEFT:
			Mouse_ButtonPressed(0);
			break;
		case SDL_BUTTON_RIGHT:
			Mouse_ButtonPressed(1);
			break;
		case SDL_BUTTON_MIDDLE:
			Mouse_ButtonPressed(2);
			break;
		}
		break;
	case SDL_RELEASED:
		switch (button->button) {
		case SDL_BUTTON_LEFT:
			Mouse_ButtonReleased(0);
			break;
		case SDL_BUTTON_RIGHT:
			Mouse_ButtonReleased(1);
			break;
		case SDL_BUTTON_MIDDLE:
			Mouse_ButtonReleased(2);
			break;
		}
		break;
	}
}

static void HandleJoystickAxis(SDL_JoyAxisEvent * jaxis) {
	switch (jaxis->axis) {
	case 0:
		JOYSTICK_Move_X(0,(float)(jaxis->value/32768.0));
		break;
	case 1:
		JOYSTICK_Move_Y(0,(float)(jaxis->value/32768.0));
		break;
	}
}

static void HandleJoystickButton(SDL_JoyButtonEvent * jbutton) {
	bool state;
	state=jbutton->type==SDL_JOYBUTTONDOWN;
	if (jbutton->button<2) {
		JOYSTICK_Button(0,jbutton->button,state);
	}
}


static void HandleVideoResize(SDL_ResizeEvent * resize) {





}

static Bit8u laltstate = SDL_KEYUP;

void GFX_Events() {
	
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	    switch (event.type) {
		case SDL_ACTIVEEVENT:
			if (event.active.state & SDL_APPINPUTFOCUS) {
				if (!event.active.gain && sdl.mouse.locked) {
					CaptureMouse();	
				}
			}
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			// ignore event lalt+tab
			if (event.key.keysym.sym==SDLK_LALT) laltstate = event.key.type;
			if ((event.key.keysym.sym==SDLK_TAB) && (laltstate==SDL_KEYDOWN)) break;
			HandleKey(&event.key);
			break;
		case SDL_MOUSEMOTION:
			HandleMouseMotion(&event.motion);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			HandleMouseButton(&event.button);
			break;
		case SDL_JOYAXISMOTION:
			HandleJoystickAxis(&event.jaxis);
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			HandleJoystickButton(&event.jbutton);
			break;
		case SDL_VIDEORESIZE:
			HandleVideoResize(&event.resize);
			break;
		case SDL_QUIT:
			throw(0);
			break;
		}
    }
}

void GFX_ShowMsg(char * format,...) {
	char buf[512];
	va_list msg;
	va_start(msg,format);
	vsprintf(buf,format,msg);
        strcat(buf,"\n");
	va_end(msg);
	printf(buf);       
};

int main(int argc, char* argv[]) {
	try {
		CommandLine com_line(argc,argv);
		Config myconf(&com_line);
		control=&myconf;

		/* Can't disable the console with debugger enabled */
#if defined(WIN32) && !(C_DEBUG)
		if (control->cmdline->FindExist("-noconsole")) {
			FreeConsole();
			/* Redirect standard input and standard output */
			freopen(STDOUT_FILE, "w", stdout);
			freopen(STDERR_FILE, "w", stderr);
			setvbuf(stdout, NULL, _IOLBF, BUFSIZ);	/* Line buffered */
			setbuf(stderr, NULL);					/* No buffering */
		} else {
			if (AllocConsole()) {
				fclose(stdin);
				fclose(stdout);
				fclose(stderr);
				freopen("CONIN$","w",stdin);
				freopen("CONOUT$","w",stdout);
				freopen("CONOUT$","w",stderr);
			}
		}
#endif  //defined(WIN32) && !(C_DEBUG)

#if C_DEBUG
		DEBUG_SetupConsole();
#endif

		if ( SDL_Init( SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_CDROM
#ifndef DISABLE_JOYSTICK
		|SDL_INIT_JOYSTICK
	
#endif
		) < 0 ) E_Exit("Can't init SDL %s",SDL_GetError());
		Section_prop * sdl_sec=control->AddSection_prop("sdl",&GUI_StartUp);
		sdl_sec->Add_bool("fullscreen",false);
		sdl_sec->Add_bool("autolock",true);
		sdl_sec->Add_int("sensitivity",100);
		sdl_sec->Add_bool("waitonerror",true);
		/* Init all the dosbox subsystems */
		
		MSG_Add("SDL_CONFIGFILE_HELP",
			"fullscreen -- Start dosbox directly in fullscreen.\n"
			"autolock -- Mouse will automatically lock, if you click on the screen.\n"
			"sensitiviy -- Mouse sensitivity.\n"
			"waitonerror -- Wait before closing the console if dosbox has an error.\n"
		);


		DOSBOX_Init();
		std::string config_file;
		if (control->cmdline->FindString("-conf",config_file,true)) {
			
		} else {
			config_file="dosbox.conf";
		}
		/* Parse the config file */
		control->ParseConfigFile(config_file.c_str());
#if (ENVIRON_LINKED)
		control->ParseEnv(environ);
#endif
		/* Init all the sections */
		control->Init();
		/* Some extra SDL Functions */
#ifndef DISABLE_JOYSTICK
		if (SDL_NumJoysticks()>0) {
			SDL_JoystickEventState(SDL_ENABLE);
			sdl.joy=SDL_JoystickOpen(0);
			LOG_MSG("Using joystick %s with %d axes and %d buttons",SDL_JoystickName(0),SDL_JoystickNumAxes(sdl.joy),SDL_JoystickNumButtons(sdl.joy));
			JOYSTICK_Enable(0,true);
		}
#endif	
		if (control->cmdline->FindExist("-fullscreen") || sdl_sec->Get_bool("fullscreen")) {
			SwitchFullScreen();
		}
		/* Start up main machine */
		control->StartUp();
		/* Shutdown everything */
	} catch (char * error) {
        if (sdl.full_screen) SwitchFullScreen();
	    if (sdl.mouse.locked) CaptureMouse();
		LOG_MSG("Exit to error: %s",error);
		if(sdl.wait_on_error) {
			//TODO Maybe look for some way to show message in linux?
#if (C_DEBUG)
			LOG_MSG("Press enter to continue",error);
			fgetc(stdin);
#elif defined(WIN32)
			Sleep(5000);
#endif 
		}

	}
	catch (int){ 
        if (sdl.full_screen) SwitchFullScreen();
	    if (sdl.mouse.locked) CaptureMouse();
	}
    return 0;
};
