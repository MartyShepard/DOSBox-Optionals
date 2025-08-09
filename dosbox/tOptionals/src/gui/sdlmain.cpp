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
 *  You should have received a copy of the GNU General Public License along along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#ifdef WIN32
	#include <signal.h>
	#include <process.h>
	#include "windows.h"
	#include <winuser.h>
	#include "Shellapi.h"	// for shellexecute
	#include "shell.h"		// for shellexecute

	#if defined(_MSC_VER)
	#include "SDL2/include/SDL_mixer.h"
	#include "SDL2/include/SDL_syswm.h"
	#else
	#include "SDL2/SDL_mixer.h"
	#include "SDL2/SDL_syswm.h"
	#endif
#endif

#include "cross.h"
#if defined(_MSC_VER)
	#include "SDL2/include/SDL.h"
#else
	#include "SDL2/SDL.h"
#endif
#include "dosbox.h"
#include "video.h"
#include "ide.h"
#include "isa.h"
#include "mouse.h"
#include "pic.h"
#include "timer.h"
#include "setup.h"
#include "support.h"
#include "debug.h"
#include "mapper.h"
#include "vga.h"
#include "keyboard.h"
#include "cpu.h"
#include "cross.h"
#include "control.h"
#include "joystick.h"
#include "bios_disk.h"
#include "hardware.h"
#include "version_optionals.h"


#define MAPPERFILE "KeyMap.map"
//#define MAPPERFILE "MAPPER-SDL2-" VERSION ".map"
//#define DISABLE_JOYSTICK

// Comment out out to use the Standard Windows Enviroment, Marty
// #define USERS_APPDATA

#if C_OPENGL
	#define GLEW_STATIC
	#if defined(_MSC_VER)
		#include <GL/glew.h>
	#else
		#include <GL/glew.h>
	#endif
	#define C_OPENGL_USE_SHADERS 1
	
	#ifdef C_OPENGL_USE_SHADERS
	#define GL_GLEXT_PROTOTYPES
	#define GL_SGIX_fragment_lighting
#endif
	#if defined(_MSC_VER)
		#include "SDL2/Include/SDL_opengl.h"
		#include "SDL2/Include/SDL_version.h"
	#else
		#include "SDL_opengl.h"
		#include "SDL_version.h"
	#endif
#endif //C_OPENGL

#include "../hardware/voodoo_emu.h"
#if !(ENVIRON_INCLUDED)
	extern char** environ;
#endif

#ifdef WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#if defined(_MSC_VER)
	#else
	#include <windows.h>
	#endif
	#if C_DDRAW
		#include <ddraw.h>
		struct private_hwdata {
			LPDIRECTDRAWSURFACE3 dd_surface;
			LPDIRECTDRAWSURFACE3 dd_writebuf;
		};
	#endif

	#define STDOUT_FILE	TEXT("dosbox_log.txt")
	#define STDERR_FILE	TEXT("dosbox_err.txt")
	
	#define STDOUTDATA_FILE	TEXT(".\\data\\dosbox_log.txt")
	#define STDERRDATA_FILE	TEXT(".\\data\\dosbox_err.txt")
	
	#define DEFAULT_CONFIG_FILE "/dosbox.conf"
	#elif defined(MACOSX)
	#define DEFAULT_CONFIG_FILE "/Library/Preferences/DOSBox Preferences"
	#else /*linux freebsd*/
	#define DEFAULT_CONFIG_FILE "/.dosboxrc"
#endif

#if C_SET_PRIORITY
	#include <sys/resource.h>
	#define PRIO_TOTAL (PRIO_MAX-PRIO_MIN)
#endif

#ifdef OS2
	#define INCL_DOS
	#define INCL_WIN
	#include <os2.h>
#endif

bool autoclocked;
	
enum SCREEN_TYPES	{
	SCREEN_SURFACE,
	SCREEN_TEXTURE,
	SCREEN_OPENGL
};

enum PRIORITY_LEVELS {
	PRIORITY_LEVEL_PAUSE,
	PRIORITY_LEVEL_LOWEST,
	PRIORITY_LEVEL_LOWER,
	PRIORITY_LEVEL_NORMAL,
	PRIORITY_LEVEL_HIGHER,
	PRIORITY_LEVEL_HIGHEST,
	PRIORITY_LEVEL_REALTIME
};


struct SDL_Block {
	
	bool inited;
	bool active;							//If this isn't set don't draw
	bool updating;
	bool update_display_contents;
	bool update_window;
	bool automaticheight;
	bool bVoodooScreenRequest;
	bool hSystemMenu;
	bool wait_on_error;	
	
	int window_desired_width;
	int window_desired_height;
	int windowstaskbaradjust;	
	
	struct {
		Bit32u width;
		Bit32u height;
		Bitu flags;
		double scalex,scaley;
		GFX_CallBack_t callback;
	} draw;
	
	struct {
		struct {
			Bit16u width, height;
			bool fixed;
			bool display_res;
		} full;
		
		struct {
			Bit16u width, height;
		} window;
		
		Bit8u bpp;
		Bit32u sdl2pixelFormat;
		Uint32 screenflag;
		bool fullscreen;
		bool lazy_fullscreen;
		bool lazy_fullscreen_req;
		/*bool vsync;*/
		Bitu vsync;
		bool borderless;
		SCREEN_TYPES type;
		SCREEN_TYPES want_type;
	} desktop;
	
#if C_OPENGL

	struct {
		SDL_GLContext context;
		void * framebuf;
		Bitu pitch;
		GLuint texture;
		GLint max_texsize;
		bool bilinear;

		std::string vertex_shader_src;
		std::string fragment_shader_src;
		GLuint program_object;
		GLuint vao;
		GLuint vertex_vbo;
		GLuint texture_vbo;
		GLuint ubo;
		GLuint vertex_shader;
		GLuint fragment_shader;

		GLfloat vertex_data[12];
		GLfloat texture_data[8];
	} opengl;

#endif	// C_OPENGL

	struct {
		PRIORITY_LEVELS focus;
		PRIORITY_LEVELS nofocus;
	} priority;

	SDL_Rect clip;
	SDL_Surface * surface;
	SDL_Window * window;
	SDL_Renderer * renderer;

	const char * rendererDriver;
	
	int displayNumber;
	
	struct {
		SDL_Texture * texture;
		SDL_PixelFormat * pixelFormat;
	} texture;
	
	SDL_cond *cond;
	struct {
		bool autolock;
		bool autoenable;
		bool requestlock;
		bool locked;
		bool middlemouselock;
		int xsensitivity;
		int ysensitivity;
	} mouse;
	SDL_Rect updateRects[1024];
	Bitu num_joysticks;
#if defined (WIN32)
	bool using_windib;
	// Time when sdl regains focus (alt-tab) in windowed mode
	Bit32u focus_ticks;
#endif

	bool SystemKeyDisable;
	bool SystemKeysLocked;	
		
	// state of alt-keys for certain special handlings
	SDL_EventType laltstate;
	SDL_EventType raltstate;
	
	SDL_EventType lwindows;
	SDL_EventType rwindows;	
	SDL_EventType rwinmenu;		
};

static SDL_Block sdl;
/* Added from DOSBox-X /////////////////////////////////////////////////////*/
#if defined(C_SDL2)
	#if defined(WIN32)
		HWND GetHWND()
		{
			SDL_SysWMinfo wmi;
			SDL_VERSION(&wmi.version);
			if (sdl.window == NULL)
				return nullptr;
			if (!SDL_GetWindowWMInfo(sdl.window, &wmi))
				return nullptr;
			return wmi.info.win.window;
		}

		HWND GetSurfaceHWND()
		{
			return GetHWND();
		}
	#endif
#endif

/* Added from DOSBox-X /////////////////////////////////////////////////////*/
#if C_OPENGL
	const std::string vertex_shader_default_src =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 textureCoord;\n"
		"\n"
		"out vec2 texCoord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = position;\n"
		"	texCoord = textureCoord;\n"
		"}\n";

	const std::string fragment_shader_default_src =
		"#version 330 core\n"
		"\n"
		"in vec2 texCoord;\n"
		"uniform sampler2D decal;\n"
		"\n"
		"out vec4 color;"
		"\n"
		"void main()\n"
		"{\n"
		"	color = texture(decal, texCoord);\n"
		"}\n";

		const GLuint POSITION_LOCATION 	= 0;
		const GLuint TEXTURE_LOCATION 	= 1;
#endif

static int SDL_Init_Wrapper(void)
{
	int result = ( SDL_Init( SDL_INIT_AUDIO|
							 SDL_INIT_VIDEO|
							 SDL_INIT_TIMER|
							 SDL_INIT_NOPARACHUTE));
	return result;
}

static void SDL_Quit_Wrapper(void)
{
	SDL_Quit();
}

/* Multimonitor Setup Varibales */
	int 	nCurrentDisplay;
	bool 	bVoodooInUse;
	bool 	bVoodooOpen;
	
	int 	bOpenGLStartupInfo;
	
	ExtVoodooMaschine extVoodoo;

	extern const char* RunningProgram;
	extern const char* GFX_Type;
	
	extern bool CPU_CycleAutoAdjust;
	extern bool CPU_FastForward;

	extern char sDriveLabel[256];

	extern void MAPPER_RunEvent(Bitu /*val*/);

	extern Bit32s MixerVolDownUpCDA_L;
	extern Bit32s MixerVolDownUpCDA_R;

/*	Globals for keyboard initialisation */
	bool startup_state_numlock	= true;
	bool startup_state_capslock	= false;

	const char* strPriority;

	bool useSoundGus		= false;
	bool useSoundSB			= false;
	bool useSoundAdlib		= false;
	bool useSoundMT32		= false;
	bool useSoundDisney		= false;
	bool useSoundTandy		= false;
	bool useSoundSSI2k1		= false;
	bool useSoundPS1		= false;

	bool isVirtualModus		= false;
	bool MediaLabelChanged	= false;

	bool SDLInfoStringShowed= false;

	/*
		Init the HWND Value for the DOSbox Icon (Left top) 
	*/
	HWND DosboxHwndIcon = NULL;
	
/* ----------------------------------------------------------------------------------------------------- */

static void PrefsEditDirect(HWND hwnd, std::string prgEdit) {
		std::string path,file, sFirstNotFound;
		FILE* f;
		Cross::GetPlatformConfigName(file);
		path = sCurrentWorkingPath() + "\\";
		path += file;
		
		f = fopen(path.c_str(),"r");
		if (!f)
		{			
			sFirstNotFound = path;
			path = sCurrentWorkingPath() + "\\DATA\\";
			path += file;
			
			f = fopen(path.c_str(),"r");
			if(!f)
			{
				std::string sMSG = std::string("Can't open for Editing\n") + sFirstNotFound + "\nor\n" + path;
				::MessageBox(hwnd, ( sMSG.c_str() ),"Huh.. Error", MB_OK|MB_ICONWARNING|MB_SYSTEMMODAL);							
			}			
		}
		
		if (f)
		{
			if (f) { fclose(f); }
			
			ShellExecute(0, "open", prgEdit.c_str(), path.c_str(), NULL, SW_SHOWNORMAL);
		}		
}
/* Icon Menu ------------------------------------------------------------------------------------------- */
#include "IconMenu\icon_menu.h"
	void Set_Window_HMenu(void){					
		/*
			Do the Proc Install once a Programm Start
		*/
		if (DosboxHwndIcon == NULL){
			SDL_SysWMinfo wminfo;
			SDL_VERSION(&wminfo.version);
			
			if (SDL_GetWindowWMInfo(sdl.window, &wminfo) == 1)
			{
				
				/*	Getting the current HWND				*/
				HWND DosboxHwndIcon = wminfo.info.win.window;
				
				/*	Execute LRESULT CALLBACK				*/			
				#include "IconMenu\icon_menu_appends.h"	
				
				/*	Install the Procdure					*/			
				SysMenuExtendWndProcInstall(DosboxHwndIcon);
			}		
		}
	};

/* ----------------------------------------------------------------------------------------------------- */

/****** Block Key ***************************************************************************************************/
#if defined(_MSC_VER)
	#include <windows.h>
#else
#include <windows.h>
#endif
//link against user32.lib when compiling in MSVC
#ifdef _MSC_VER
	#pragma comment(lib, "User32.lib")
#endif

class DisableKeys {
public:
    DisableKeys():mHKeyboardHook(NULL) {}
    //to avoid leaking the hook procedure handle
    ~DisableKeys(){ Unblock(); }
    //hook callback function (called on every system-wide key press)
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode,WPARAM wParam, LPARAM lParam) {
		
		SDL_SysWMinfo system_info;
		SDL_VERSION(&system_info.version);
		SDL_GetWindowWMInfo(sdl.window,&system_info);
		
        if(nCode == HC_ACTION) {
			
			HWND DOSBoxHWND = system_info.info.win.window;
			/* LOG_MSG("SDL: DOSBoxHWND %d",DOSBoxHWND); */
			
			 if ( GetFocus() == DOSBoxHWND ) {
				 
				 /*
					Nur im Windows 95/98 Modus
				*/
				 
				 KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT*)lParam;
				 /* LOG_MSG("SDL: Disable Keys %d (0x%x)(File: %s, Line %d)",p->vkCode,p->vkCode,__FILE__,__LINE__ ); */
				
				if (p->vkCode == VK_TAB && p->flags & LLKHF_ALTDOWN)
				{
					return 1;
				}
				 
				else if (p->vkCode == VK_ESCAPE && p->flags & LLKHF_ALTDOWN)
				{
					return 1;
				}
				
				else if (p->vkCode == VK_SPACE && p->flags & LLKHF_ALTDOWN)
				{
					return 1;
				}
				
				else if (p->vkCode == VK_PRINT && p->flags & LLKHF_ALTDOWN)
				{
					return 1;
				}				
					

				bool CtrlIsDown = GetAsyncKeyState (VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
				if (p->vkCode == VK_ESCAPE && CtrlIsDown)
				{
					return 1;
				}				
				
				 switch (p->vkCode)
				 {
					 case VK_LWIN: 			// Left Windows key (Tigger Start menu)	
						KEYBOARD_AddKey(KBD_lwindows, true);
						KEYBOARD_AddKey(KBD_lwindows, false);					
						return 1;
					 case VK_RWIN:			 // Right Windows key (Triggers Start menu)
						KEYBOARD_AddKey(KBD_lwindows, true);
						KEYBOARD_AddKey(KBD_lwindows, false);
						return 1;
					 //case VK_APPS:   		///myabe ....
					 case VK_VOLUME_MUTE:
						KEYBOARD_AddKey(KBD_audiomute, true);
						KEYBOARD_AddKey(KBD_audiomute, false);
						return 1;
					 case VK_VOLUME_DOWN:
						KEYBOARD_AddKey(KBD_volumedown, true);
						KEYBOARD_AddKey(KBD_volumedown, false);
						return 1;						
					 case VK_VOLUME_UP:		
						KEYBOARD_AddKey(KBD_volumeup, true);
						KEYBOARD_AddKey(KBD_volumeup, false);
						return 1;
					 case VK_PRINT:
						KEYBOARD_AddKey(KBD_printscreen, true);
						KEYBOARD_AddKey(KBD_printscreen, false);					
						return 1;
					 default:
						 LOG_MSG("[%d] Key Pressed: ", p->vkCode);
					}
				 
				 
			 }			
        }
        //this is not a message we are interested in
		//LOG_MSG("SDL: CallNextHookEx (File: %s, Line %d)",__FILE__,__LINE__ );
        return CallNextHookEx(NULL, //ignored paramater
                              nCode,
                              wParam,
                              lParam);
    }
    void Block(){
/* 		LOG_MSG("SDL: System Keys [Blocked]\n"
		        "     (File: %s, Line %d)\n",__FILE__,__LINE__ ); */
		sdl.SystemKeysLocked = true;		
        mHKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, //low-level keyboard hool
                                          &LowLevelKeyboardProc, //callback
                                          GetModuleHandle(NULL), 
                                          0);
										  
        // flush out and handle pending keyboard I/O
        {
            MSG msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }										  
    }
    void Unblock(){
/* 		LOG_MSG("SDL: System Keys [Unblocked]\n"
				"     (File: %s, Line %d)\n",__FILE__,__LINE__ ); */
		sdl.SystemKeysLocked = false;		
        if(mHKeyboardHook) UnhookWindowsHookEx(mHKeyboardHook);
        mHKeyboardHook = NULL;
    }

private:
    HHOOK mHKeyboardHook;
};
DisableKeys disableKeys;

/****** Block Key ***************************************************************************************************/


void GFX_SetTitle(Bit32s cycles,int frameskip,bool paused){
	char strWinTitle  [128] = {0};
	char title        [256] = {0};
	char strDriveLabel[256] = {0};

	
	static Bit32s internal_cycles	=0;
	static Bit32s internal_frameskip=0;
	
	extern Bit32s MixerVolDownUpKeysL;
	extern Bit32s MixerVolDownUpKeysR;
	
	extern bool ticksLocked;

	bool showFrameSkip = false;										// Kann Später noch in die Config
		
			 if(strcmp(GFX_Type, (const char*)"hercules"	) == 0){ strcpy(strWinTitle,"Hercules"			);}
		else if(strcmp(GFX_Type, (const char*)"cga"		 	) == 0){ strcpy(strWinTitle,"CGA"				);}
		else if(strcmp(GFX_Type, (const char*)"cga_mono"	) == 0){ strcpy(strWinTitle,"CGA Mono"			);}		
		else if(strcmp(GFX_Type, (const char*)"tandy"	 	) == 0){ strcpy(strWinTitle,"Tandy"				);}
		else if(strcmp(GFX_Type, (const char*)"pcjr"		) == 0){ strcpy(strWinTitle,"PCJr"				);}
		else if(strcmp(GFX_Type, (const char*)"ega"		 	) == 0){ strcpy(strWinTitle,"EGA"				);}
		else if(strcmp(GFX_Type, (const char*)"svga_s3"	 	) == 0){ strcpy(strWinTitle,"S3 Trio"			);}
		else if(strcmp(GFX_Type, (const char*)"vesa_nolfb"	) == 0){ strcpy(strWinTitle,"S3 Trio - NoLFB"	);}
		else if(strcmp(GFX_Type, (const char*)"vesa_oldvbe"	) == 0){ strcpy(strWinTitle,"S3 Trio - Old VBE"	);}
		else if(strcmp(GFX_Type, (const char*)"svga_et4000"	) == 0){ strcpy(strWinTitle,"Tseng ET4000"		);}
		else if(strcmp(GFX_Type, (const char*)"svga_et3000"	) == 0){ strcpy(strWinTitle,"Tseng ET3000"		);}
		else if(strcmp(GFX_Type, (const char*)"svga_paradise") == 0){ strcpy(strWinTitle,"Paradise"			);}
		else if(strcmp(GFX_Type, (const char*)"vgaonly"		) == 0){ strcpy(strWinTitle,"VGA"				);}
		else if(strcmp(GFX_Type, (const char*)"vga"			) == 0){ strcat(strWinTitle,"VGA"				);}
		else if(strcmp(GFX_Type, (const char*)"svga"		) == 0){ strcat(strWinTitle,"S3 Trio"			);}
			
		if (strlen(sDriveLabel) != 0){
			sprintf(strDriveLabel,"%s, ",sDriveLabel);
		}

		
		if(cycles 	 != -1){ internal_cycles 	= cycles	;}
		if(frameskip != -1){ internal_frameskip = frameskip	;}
	
	
	if (ticksLocked) {		
		sprintf(title,"DOSBox %s %s, Cpu Cycles: Max Unlocked, [ Volume %2d%  (Ctrl+Alt+F9/10) ], [ %s ], %s (Program: %4s)",
				VERSION,DOSBOXREVISION,MixerVolDownUpKeysR,strPriority,strDriveLabel,RunningProgram);			
		
	} else if(CPU_CycleAutoAdjust && internal_cycles <= 99) {	
	
		sprintf(title,"DOSBox %s %s, CPU Cycles: %3d Percent, %s,[ Volume %2d%  (Ctrl+Alt+F9/10) ], [ %s ], %s (Program: %4s)",
				VERSION,DOSBOXREVISION,internal_cycles,strWinTitle,MixerVolDownUpKeysR,strPriority,strDriveLabel,RunningProgram);
	} else if(CPU_CycleAutoAdjust && internal_cycles >= 100) {	
	
		sprintf(title,"DOSBox %s %s, CPU Cycles: %3d Max, %s, [ Volume %2d%  (Ctrl+Alt+F9/10) ], [ %s ], %s (Program: %4s)",
				VERSION,DOSBOXREVISION,internal_cycles,strWinTitle,MixerVolDownUpKeysR,strPriority,strDriveLabel,RunningProgram);	
	} else {
			
		strcpy(strWinTitle,"DOSBox %s %s, CPU Cycles: %3d");
		
		switch(internal_cycles){
			case 341:	strcat(strWinTitle, " (CPU 8088/4.77mhz"		);	break;
			case 460:	strcat(strWinTitle, " (CPU 8088/7.16mhz"		);	break;
			case 618:	strcat(strWinTitle, " (CPU 8088/9.54mhz"		);	break;	
			case 1778:	strcat(strWinTitle, " (CPU 286/10mhz"			);	break;
			case 2616:	strcat(strWinTitle, " (CPU 286/12mhz"			);	break;
			case 3000:	strcat(strWinTitle, " (Default Cycles"			);	break;	
			case 3360:	strcat(strWinTitle, " (CPU 286/16mhz"			);	break;	
			case 4440:	strcat(strWinTitle, " (CPU 286/20mhz"			);	break;
			case 5240:	strcat(strWinTitle, " (CPU 286/25mhz"			);	break;		
			case 7785:	strcat(strWinTitle, " (CPU 386DX/25mhz"			);	break;
			case 9349:	strcat(strWinTitle, " (CPU 386DX/33mhz"			);	break;
			case 9384:	strcat(strWinTitle, " (CPU 386DX/40mhz"			);	break;	
			case 9870:	strcat(strWinTitle, " (CPU 486SX/25mhz"			);	break;
			case 13350:	strcat(strWinTitle, " (CPU 486DX/33mhz"			);	break;	
			case 13461:	strcat(strWinTitle, " (CPU 486SX/33mhz"			);	break;
			case 16100:	strcat(strWinTitle, " (CPU 486SX/40mhz"			);	break;	
			case 20100:	strcat(strWinTitle, " (CPU 486DX/50mhz"			);	break;
			case 27182:	strcat(strWinTitle, " (CPU 486DX-2/66mhz"		);	break;
			case 32501:	strcat(strWinTitle, " (CPU 486DX-2/80mhz"		);	break;
			case 40042:	strcat(strWinTitle, " (CPU 486DX-2/100mhz"		);	break;
			case 50821:	strcat(strWinTitle, " (CPU 486DX-4/100mhz"		);	break;
			case 60174:	strcat(strWinTitle, " (CPU 486DX-4/120mhz"		);	break;	
			case 51330:	strcat(strWinTitle, " (CPU Pentium/60 [~66mhz]"	);	break;
			case 69159:	strcat(strWinTitle, " (CPU Pentium/75 [~90mhz]"	);	break;
			case 77500:	strcat(strWinTitle, " (CPU Pentium/100mhz"		);	break;					
			default:	strcat(strWinTitle, " (Not Adjusted"			);	break;
		}
	
		
		if(strcmp(GFX_Type, (const char*)"hercules"			) == 0){ strcat(strWinTitle,"/ Hercules)"					);}
		else if(strcmp(GFX_Type, (const char*)"cga"			) == 0){ strcat(strWinTitle,"/ CGA)"						);}
		else if(strcmp(GFX_Type, (const char*)"cga_mono"	) == 0){ strcat(strWinTitle,"/ CGA Mono)"					);}
		else if(strcmp(GFX_Type, (const char*)"tandy"		) == 0){ strcat(strWinTitle,"/ Tandy)"						);}
		else if(strcmp(GFX_Type, (const char*)"pcjr"		) == 0){ strcat(strWinTitle,"/ PCJr)"						);}
		else if(strcmp(GFX_Type, (const char*)"ega"			) == 0){ strcat(strWinTitle,"/ EGA)"						);}
		else if(strcmp(GFX_Type, (const char*)"svga_s3"		) == 0){ strcat(strWinTitle,"/ S3 Trio)"					);}
		else if(strcmp(GFX_Type, (const char*)"vesa_nolfb"	) == 0){ strcat(strWinTitle,"/ S3 Trio - Vesa 2.0 Patch)"	);}
		else if(strcmp(GFX_Type, (const char*)"vesa_oldvbe"	) == 0){ strcat(strWinTitle,"/ S3 Trio - Old VBE)"			);}
		else if(strcmp(GFX_Type, (const char*)"svga_et4000"	) == 0){ strcat(strWinTitle,"/ Tseng ET4000)"				);}
		else if(strcmp(GFX_Type, (const char*)"svga_et3000"	) == 0){ strcat(strWinTitle,"/ Tseng ET3000)"				);}
		else if(strcmp(GFX_Type, (const char*)"svga_paradise")== 0){ strcat(strWinTitle,"/ Paradise)"					);}
		else if(strcmp(GFX_Type, (const char*)"vgaonly"		) == 0){ strcat(strWinTitle,"/ VGA)"						);}
		else if(strcmp(GFX_Type, (const char*)"vga"			) == 0){ strcat(strWinTitle,"/ VGA)"						);}
		else if(strcmp(GFX_Type, (const char*)"svga"		) == 0){ strcat(strWinTitle,"/ S3 Trio)"					);}
			
		strcat(strWinTitle,", [ Volume %2d% (Ctrl+Alt+F9/10) ], [ %s ], %s (Program: %4s)");		
																																						 
		sprintf(title			,
				strWinTitle		,
				VERSION			,
				DOSBOXREVISION	,
				internal_cycles	,MixerVolDownUpKeysR,strPriority,strDriveLabel,RunningProgram);		
	}
		
	if(paused){
		strcat(title," PAUSED");
	}
	

	if(sdl.mouse.locked || mouselocked ){
		if (sdl.mouse.middlemouselock == true ){
			strcat(title,"  (Ctrl+F10/MMB Release Mouse)");
		} else {
			strcat(title,"  (Ctrl+F10 Release Mouse)");
		}
	}
	
	if(CPU_FastForward){
		strcat(title," (Fast Forward On)");
	}	
		
	SDL_SetWindowTitle(sdl.window,title); // VERSION is gone...
}

static unsigned char logo[32*32*4]= {
#include "dosbox_logo.h"
};


static void GFX_SetIcon() {
#if !defined(MACOSX)
	/* Set Icon (must be done before any sdl_setvideomode call) */
	/* But don't set it on OS X, as we use a nicer external icon there. */
	/* Made into a separate call, so it can be called again when we restart the graphics output on win32 */
#ifdef WORDS_BIGENDIAN
	SDL_Surface* logos= SDL_CreateRGBSurfaceFrom((void*)logo,32,32,32,128,0xff000000,0x00ff0000,0x0000ff00,0);
#else
	SDL_Surface* logos= SDL_CreateRGBSurfaceFrom((void*)logo,32,32,32,128,0x000000ff,0x0000ff00,0x00ff0000,0);
#endif // WORDS_BIGENDIAN

#if defined (WIN32)
    		

#endif

	SDL_SetWindowIcon(sdl.window, logos);
#endif // !defined(MACOSX)
}

static void FloppyARequest(bool pressed) {	
	if (pressed)	
		MenuBrowseFDImage('A', 0);
	else
		return;	
}
static void FloppyBRequest(bool pressed) {	
	if (pressed)	
		MenuBrowseFDImage('B', 1);
	else
		return;	
}
static void CDRomERequest(bool pressed) {	
	if (pressed)	
		MenuBrowseCDImage('E',0);
	else
		return;	
}
static void CDRomFRequest(bool pressed) {	
	if (pressed)	
		MenuBrowseCDImage('F', 1);
	else
		return;	
}

static void KillSwitch(bool pressed) {
	
	if (sdl.SystemKeysLocked == true)
		disableKeys.Unblock();
	
	if (!pressed)
		return;
	throw 1;
}

static void SetPriority(PRIORITY_LEVELS level) {

	#if C_SET_PRIORITY
	// Do nothing if priorties are not the same and not root, else the highest
	// priority can not be set as users can only lower priority (not restore it)

		if((sdl.priority.focus != sdl.priority.nofocus ) &&
			(getuid()!=0) ) return;

	#endif
		switch (level) {
	#ifdef WIN32
		case PRIORITY_LEVEL_PAUSE:	// if DOSBox is paused, assume idle priority
		case PRIORITY_LEVEL_LOWEST:
			SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
			strPriority = "Prio: Lowest";
			break;
		case PRIORITY_LEVEL_LOWER:
			SetPriorityClass(GetCurrentProcess(),BELOW_NORMAL_PRIORITY_CLASS);
			strPriority = "Prio: Low";
			break;
		case PRIORITY_LEVEL_NORMAL:
			SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
			strPriority = "Prio: Normal";
			break;
		case PRIORITY_LEVEL_HIGHER:
			SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);
			strPriority = "Prio: High";
			break;
		case PRIORITY_LEVEL_HIGHEST:
			SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
			strPriority = "Prio: Highest";
			break;
		case PRIORITY_LEVEL_REALTIME:
			SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
			strPriority = "Prio: Realtime";
			break;		
	#elif C_SET_PRIORITY
	/* Linux use group as dosbox has mulitple threads under linux */
		case PRIORITY_LEVEL_PAUSE:	// if DOSBox is paused, assume idle priority
		case PRIORITY_LEVEL_LOWEST:
			setpriority (PRIO_PGRP, 0,PRIO_MAX);
			break;
		case PRIORITY_LEVEL_LOWER:
			setpriority (PRIO_PGRP, 0,PRIO_MAX-(PRIO_TOTAL/3));
			break;
		case PRIORITY_LEVEL_NORMAL:
			setpriority (PRIO_PGRP, 0,PRIO_MAX-(PRIO_TOTAL/2));
			break;
		case PRIORITY_LEVEL_HIGHER:
			setpriority (PRIO_PGRP, 0,PRIO_MAX-((3*PRIO_TOTAL)/5) );
			break;
		case PRIORITY_LEVEL_HIGHEST:
			setpriority (PRIO_PGRP, 0,PRIO_MAX-((3*PRIO_TOTAL)/4) );
			break;
	#endif
		default:
			break;
		}
		GFX_SetTitle(-1,-1,false);
}

static void PauseDOSBox(bool pressed) {
	if (!pressed)
		return;
	SDL_Keymod inkeymod = SDL_GetModState();
		
	GFX_SetTitle(-1,-1,true);
	bool paused = true;
	SDL_Delay(500);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// flush event queue.
	}
	/* NOTE: This is one of the few places where we use SDL key codes
	with SDL 2.0, rather than scan codes. Is that the correct behavior? */
	while (paused) {
		SDL_WaitEvent(&event);    // since we're not polling, cpu usage drops to 0.
		switch (event.type) {

			case SDL_QUIT: KillSwitch(true); break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
					// We may need to re-create a texture and more
					GFX_ResetVoodoo();
					LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);							
				}
				break;
			case SDL_KEYDOWN:   // Must use Pause/Break Key to resume.
			case SDL_KEYUP:
			if ( event.key.keysym.sym == SDLK_PAUSE ) {
				SDL_Keymod outkeymod = SDL_GetModState();
				if (inkeymod != outkeymod) {
					KEYBOARD_ClrBuffer();
					MAPPER_LosingFocus();
					//Not perfect if the pressed alt key is switched, but then we have to 
					//insert the keys into the mapper or create/rewrite the event and push it.
					//Which is tricky due to possible use of scancodes.
					//keystate&KMOD_NUM)
				}
				paused = false;
				GFX_SetTitle(-1,-1,false);
				SetPriority(sdl.priority.focus);
				CPU_Disable_SkipAutoAdjust();				
				break;
			}
#if defined (MACOSX)
			if (event.key.keysym.sym == SDLK_q &&
			    (event.key.keysym.mod == KMOD_RGUI ||
			     event.key.keysym.mod == KMOD_LGUI)
			   ) {
				/* On macs, all aps exit when pressing cmd-q */
				KillSwitch(true);
				break;
			}
#endif
		}
	}
}

/* Reset the screen with current values in the sdl structure */
Bitu GFX_GetBestMode(Bitu flags) {
	/* For simplicity, with SDL 2.0 we accept
	the desktop's color depth only for now */
	switch (sdl.desktop.want_type) {
	
		case SCREEN_SURFACE:
			//We only accept 32bit output from the scalers here
			//Can't handle true color inputs
			if (flags & GFX_RGBONLY || !(flags&GFX_CAN_32)) goto check_surface;
				flags|=GFX_SCALING;
				flags&=~(GFX_CAN_8|GFX_CAN_15|GFX_CAN_16);
			break;	
		
		case SCREEN_TEXTURE:
	
check_surface:
			flags &= ~GFX_LOVE_8;		//Disable love for 8bpp modes
			/* If we can't get our favorite mode check for another working one */
			switch (sdl.desktop.bpp)
			{
			case 8:
				if (flags & GFX_CAN_8) flags&=~(GFX_CAN_15|GFX_CAN_16|GFX_CAN_32);
				break;
			case 15:
				if (flags & GFX_CAN_15) flags&=~(GFX_CAN_8|GFX_CAN_16|GFX_CAN_32);
				break;
			case 16:
				if (flags & GFX_CAN_16) flags&=~(GFX_CAN_8|GFX_CAN_15|GFX_CAN_32);
				break;
			case 24:
			case 32:
				if (flags & GFX_CAN_32) flags&=~(GFX_CAN_8|GFX_CAN_15|GFX_CAN_16);
				break;
			}
			if (sdl.desktop.want_type == SCREEN_TEXTURE)
				flags |= GFX_SCALING; // We want texture...
			else	// Or we want/FORCE surface (e.g. for most scalers)
				flags |= GFX_CAN_RANDOM;
			break;
	
	#if C_OPENGL
		case SCREEN_OPENGL:		
		//We only accept 32bit output from the scalers here
		if (!(flags&GFX_CAN_32)) goto check_surface;
			flags|=GFX_SCALING;
			flags&=~(GFX_CAN_8|GFX_CAN_15|GFX_CAN_16);
			break;
	#endif
		default:
			goto check_surface;
			break;
		}
	return flags;
}


void GFX_ResetScreen(void) {
	GFX_Stop();
	if (sdl.draw.callback)
		(sdl.draw.callback)( GFX_CallBackReset );
	GFX_Start();		
	CPU_Reset_AutoAdjust();
	GFX_SetTitle(-1,-1,false);	
}

void GFX_ResetVoodooDisplayMove(void){
	if ( bVoodooOpen == true){	
		voodoo_leave();								
		voodoo_activate();		
	}	
}

void GFX_ResetVoodoo(void){
	if ( bVoodooOpen == true){	
		voodoo_leave();		
		GFX_ResetScreen();							
		voodoo_activate();		
	} else {
		GFX_ResetScreen();
	}
	GFX_CaptureMouse();
	
}
void GFX_ForceFullscreenExit(void) {
	if (sdl.desktop.lazy_fullscreen) {
//		sdl.desktop.lazy_fullscreen_req=true;
		LOG_MSG("SDL : Invalid Screen  Change \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);
	} else {
		sdl.desktop.fullscreen=false;
		GFX_ResetScreen();
		LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);		
	}
}

void GFX_UpdateResolution(int w, int h, bool windowed){
		
	if (windowed == true){
				
		if ( (sdl.desktop.window.width != w) || (sdl.desktop.window.height != h) ) 
		{		
	
			if ( w == 0 && h == 0 )
			{
				SDL_DisplayMode displayMode;
				SDL_GetDesktopDisplayMode(nCurrentDisplay, &displayMode);		
				w = displayMode.w;
				h = displayMode.h;	
			}	
			extVoodoo.bForceWindowUpdate= true;		 	
			sdl.desktop.window.width 	= w;
			sdl.desktop.window.height 	= h;	
			extVoodoo.pciW				= w;		
			extVoodoo.pciH				= h;
			GFX_ResetVoodoo();
			control->SaveConfig_ResX(w,h,windowed);
		}
		
	}else{	
	
		if ( (sdl.desktop.full.width) != w || (sdl.desktop.full.height != h) )
		{		
			if ( w == 0 && h == 0 )
			{
				SDL_DisplayMode displayMode;
				SDL_GetDesktopDisplayMode(nCurrentDisplay, &displayMode);		
				w = displayMode.w;
				h = displayMode.h;	
			}
			extVoodoo.bForceFullSnUpdate= true;	
			sdl.desktop.full.width		= w;
			sdl.desktop.full.height		= h;	
			extVoodoo.pciFSW			= w;		
			extVoodoo.pciFSH			= h;
			control->SaveConfig_ResX(w,h,windowed);			
		}		
	}		
}
					
static int int_log2 (int val) {
    int log = 0;
    while ((val >>= 1) != 0)
	log++;
    return log;
}





/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
void Center_SDL_SetWindowMode(int pciW, int pciH){
		RECT r; int wTBH = 0; int DesktopIndex = SDL_GetWindowDisplayIndex(sdl.window);	int posX; int posY;
		
		SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);

		SDL_DisplayMode displayMode;
		
		SDL_GetDesktopDisplayMode(DesktopIndex, &displayMode);		

		//LOG_MSG("VOODOO: Mode: 01: DesW:%d DeskH:%d CurrW:%d CurrH:%d",displayMode.w,displayMode.h,pciW,pciH);	
		/*==========================================================================*/
		if ( pciW < displayMode.w && pciH < displayMode.h ){	
			//LOG_MSG("SDL : Mode: 01: Index:%d, DesW:%d DeskH:%d CurrW:%d CurrH:%d, Bottom:%d",DesktopIndex, displayMode.w,displayMode.h,pciW,pciH,r.bottom);	
			wTBH = 0;
			if (DesktopIndex == 0)	{
				if (displayMode.h != r.bottom){
					wTBH = sdl.windowstaskbaradjust; 
				}				
				wTBH += (displayMode.h - r.bottom )/2;								
			}			
			posX          = (displayMode.w - pciW) /2;
			posY         = ((displayMode.h - wTBH) - pciH) /2;
			SDL_SetWindowPosition(sdl.window,posX , posY);				
			return;
		}
		/*==========================================================================*/		
		if ( pciW > displayMode.w && pciH > displayMode.h ){							
			posX          = (displayMode.w - pciW) /2;
			posY         = (displayMode.h - pciH) /2;
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}
		/*==========================================================================*/
		if ( pciW < displayMode.w && pciH > displayMode.h ){							
			posX          = (displayMode.w - pciW) /2;
			posY         = (displayMode.h - pciH) /2;		
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}	
		/*==========================================================================*/
		if ( pciW > displayMode.w &&  pciH < displayMode.h ){	
			wTBH = 0;		
			if (DesktopIndex == 0)	{		
				if (displayMode.h != r.bottom){
					wTBH = sdl.windowstaskbaradjust; 
				}				
				wTBH += (displayMode.h - r.bottom )/2;	
			}
			posX          = (displayMode.w - pciW) /2;
			posY         = ((displayMode.h - wTBH) - pciH) /2;		
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}			
		/*==========================================================================*/
		if ( pciW == displayMode.w &&  pciH == displayMode.h ){	
			posX          = (displayMode.w - pciW) /2;
			posY         = (displayMode.h - pciH) /2;		
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}	
		/*==========================================================================*/
		if ( pciW == displayMode.w &&  pciH > displayMode.h ){	
			posX          = (displayMode.w - pciW) /2;
			posY         = (displayMode.h - pciH) /2;		
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}	
		/*==========================================================================*/
		if ( pciW == displayMode.w &&  pciH < displayMode.h ){	
			wTBH = 0;		
			if (DesktopIndex == 0)	{		
				if (displayMode.h != r.bottom){
					wTBH = sdl.windowstaskbaradjust; 
				}				
				wTBH += (displayMode.h - r.bottom )/2;	
			}
			posX          = (displayMode.w - pciW) /2;
			posY         = ((displayMode.h - wTBH) - pciH) /2;			
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}			
		/*==========================================================================*/
		if ( pciW > displayMode.w &&  pciH == displayMode.h ){	
			posX          = (displayMode.w - pciW) /2;
			posY         = (displayMode.h - pciH) /2;		
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}
		/*==========================================================================*/
		if ( pciW > displayMode.w &&  pciH == displayMode.h ){	
			posX          = (displayMode.w - pciW) /2;
			posY         = (displayMode.h - pciH) /2;			
			SDL_SetWindowPosition(sdl.window,posX , posY);			
			return;
		}		
	

	
}



static SDL_Window * GFX_SetSDLWindowMode(Bit16u width, Bit16u height, bool fullscreen, SCREEN_TYPES screenType) {
				
	static SCREEN_TYPES lastType = SCREEN_SURFACE;
	
		
	if (sdl.renderer) {
		SDL_DestroyRenderer(sdl.renderer);
		sdl.renderer=0;
	}
	if (sdl.texture.pixelFormat) {
		SDL_FreeFormat(sdl.texture.pixelFormat);
		sdl.texture.pixelFormat = 0;
	}
	if (sdl.texture.texture) {
		SDL_DestroyTexture(sdl.texture.texture);
		sdl.texture.texture=0;
	}
#if C_OPENGL
	if (sdl.opengl.context) {
		SDL_GL_DeleteContext(sdl.opengl.context);
		sdl.opengl.context=0;
	}

	if (sdl.opengl.vao) {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &sdl.opengl.vao);
		sdl.opengl.vao = 0;
	}

	if (sdl.opengl.vertex_vbo) {
		glDeleteBuffers(1, &sdl.opengl.vertex_vbo);
		sdl.opengl.vertex_vbo = 0;
	}

	if (sdl.opengl.texture_vbo) {
		glDeleteBuffers(1, &sdl.opengl.texture_vbo);
		sdl.opengl.texture_vbo = 0;
	}

	if (sdl.opengl.ubo) {
		glDeleteBuffers(1, &sdl.opengl.ubo);
		sdl.opengl.ubo = 0;
	}

	if (sdl.opengl.program_object) {
		glUseProgram(0);
		glDeleteProgram(sdl.opengl.program_object);
		sdl.opengl.program_object = 0;
	}
			
#endif
	sdl.window_desired_width = width;
	sdl.window_desired_height = height;
	int currWidth, currHeight;
				
	if (sdl.window) {
		if (!sdl.update_window) {			
			SDL_GetWindowSize(sdl.window, &currWidth, &currHeight);
			sdl.update_display_contents = ((width == currWidth) && (height == currHeight));
			return sdl.window;
		}
	}
	/* If we change screen type, recreate the window. Furthermore, if
	 * it is our very first time then we simply create a new window.
	 */
	if ( !sdl.window || (lastType != screenType) )
	{
		lastType = screenType;
		
		if (sdl.window)
		{
			SDL_DestroyWindow(sdl.window);
		}				
			
		sdl.window = SDL_CreateWindow("",
		                 SDL_WINDOWPOS_CENTERED_DISPLAY( sdl.displayNumber ),
		                 SDL_WINDOWPOS_CENTERED_DISPLAY( sdl.displayNumber ),
		                 width, height,
		                 (fullscreen ? (sdl.desktop.full.display_res ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN) : 0)
		                 |SDL_RENDERER_ACCELERATED| ((screenType == SCREEN_OPENGL) ? SDL_WINDOW_OPENGL : 0) | SDL_WINDOW_HIDDEN| (sdl.desktop.borderless ? SDL_WINDOW_BORDERLESS : 0));		
		
		nCurrentDisplay = sdl.displayNumber;
					
		if (sdl.window){								
			GFX_SetTitle(-1,-1,false); //refresh title.
			
			if (sdl.displayNumber == 0){
				Center_SDL_SetWindowMode(width, height);			
			} else {
						
				SDL_SetWindowPosition(sdl.window,
				SDL_WINDOWPOS_CENTERED_DISPLAY( sdl.displayNumber ) , SDL_WINDOWPOS_CENTERED_DISPLAY( sdl.displayNumber ));
				
			}	

			SDL_GetWindowSize(sdl.window, &currWidth, &currHeight);
			sdl.update_display_contents = ((width == currWidth) && (height == currHeight));								
			
			
			return sdl.window;
		}
	}
	
	/* Fullscreen mode switching has its limits, and is also problematic on
	 * some window managers. For now, the following may work up to some
	 * level. On X11, SDL_VIDEO_X11_LEGACY_FULLSCREEN=1 can also help,
	 * although it has its own issues.
	 * Suggestion: Use the desktop res if possible, with output=surface
	 * if one is not interested in scaling.
	 * On Android, desktop res is the only way.
	 */
	if (fullscreen) {
		
		if ( sdl.desktop.screenflag == SDL_WINDOW_FULLSCREEN ){
				bool success = false;
		
				const int mode_count= SDL_GetNumDisplayModes( nCurrentDisplay );
				for( int m= 0; m < mode_count; m++ )
				{
					SDL_DisplayMode displayMode;
					const int result= SDL_GetDisplayMode( 0, m, &displayMode );
					if( result < 0 )
						continue;
					if( !( SDL_BITSPERPIXEL( displayMode.format ) == 24 || SDL_BITSPERPIXEL( displayMode.format ) == 32 ) )
						continue;

					if( displayMode.w == int(width) && displayMode.h == int(height) && displayMode.refresh_rate == int(60) )
					{
						const int result= SDL_SetWindowDisplayMode( sdl.window, &displayMode );
						if( result == 0 ){
							success = true;
							SDL_SetWindowFullscreen(sdl.window,sdl.desktop.screenflag);
							return sdl.window;
						}
					}
				}
				
				if	(!success)	{				
					LOG_MSG("SDL : ERROR: Couln't set Current Resolution on:");
					LOG_MSG("SDL : Monitor  Nr=%d", sdl.displayNumber);				
					LOG_MSG("SDL : Resolution =%dx%d", width, height);				
					LOG_MSG("SDL : Using Default Mode 640x480");						
					
					SDL_DisplayMode displayMode;
					displayMode.w = 640;
					displayMode.h = 480;
					displayMode.refresh_rate = 60;
					displayMode.format       = 32;
					SDL_SetWindowDisplayMode(sdl.window, &displayMode);
					SDL_SetWindowFullscreen(sdl.window, sdl.desktop.screenflag);
				}				
				LOG_MSG("SDL : =========================================\n");				
				return sdl.window;
		}else{
					
				SDL_DisplayMode displayMode;
				SDL_GetWindowDisplayMode(sdl.window, &displayMode);
				displayMode.w = width;
				displayMode.h = height;
				SDL_SetWindowDisplayMode(sdl.window, &displayMode);	
				SDL_SetWindowFullscreen(sdl.window, SDL_WINDOW_FULLSCREEN_DESKTOP);				
		}
	} else {
				
		SDL_SetWindowFullscreen(sdl.window, 0);
		SDL_SetWindowSize(sdl.window, width, height);
					
	}
	// Maybe some requested fullscreen resolution is unsupported?
	SDL_GetWindowSize(sdl.window, &currWidth, &currHeight);
	sdl.update_display_contents = ((width == currWidth) && (height == currHeight));
	return sdl.window;
}

// Used for the mapper UI and more: Creates a fullscreen window with desktop res
// on Android, and a non-fullscreen window with the input dimensions otherwise.
SDL_Window * GFX_SetSDLSurfaceWindow(Bit16u width, Bit16u height) {
	return GFX_SetSDLWindowMode(width, height, sdl.desktop.fullscreen, SCREEN_SURFACE);
}

// Returns the rectangle in the current window to be used for scaling a
// sub-window with the given dimensions, like the mapper UI.
SDL_Rect GFX_GetSDLSurfaceSubwindowDims(Bit16u width, Bit16u height) {
	SDL_Rect rect;
	rect.x=rect.y=0;
	rect.w=width;
	rect.h=height;
	return rect;
}

// Currently used for an initial test here
static SDL_Window * GFX_SetSDLOpenGLWindow(Bit16u width, Bit16u height) {
	return GFX_SetSDLWindowMode(width, height, sdl.desktop.fullscreen, SCREEN_OPENGL);
}

// Different functions, similar function bodies (SDL 1.2 vs 2.0)

static SDL_Window * GFX_SetupWindowScaled(SCREEN_TYPES screenType)
{
	// LOG_MSG("SDL : SCREEN_TEXTURE = %d",screenType);
	Bit16u fixedWidth;
	Bit16u fixedHeight;

	if (sdl.desktop.fullscreen) {
		fixedWidth = sdl.desktop.full.fixed ? sdl.desktop.full.width : 0;
		fixedHeight = sdl.desktop.full.fixed ? sdl.desktop.full.height : 0;
	} else {

		fixedWidth = sdl.desktop.window.width;
		fixedHeight = sdl.desktop.window.height;
	}
	 LOG_MSG("SDL : fixedWidth = %d",sdl.desktop.window.width);
	 LOG_MSG("SDL : fixedHeight = %d",sdl.desktop.window.height);	//1024
	
	
	if (fixedWidth && fixedHeight) {
		
		 LOG_MSG("SDL : sdl.draw.width = %d",sdl.draw.width);
		 LOG_MSG("SDL : sdl.draw.height = %d",sdl.draw.height);		
		 LOG_MSG("SDL : sdl.draw.scalex = %d",sdl.draw.scalex);
		 LOG_MSG("SDL : sdl.draw.scaley = %d",sdl.draw.scaley);
		
		double ratio_w=(double)fixedWidth/(sdl.draw.width*sdl.draw.scalex);
		double ratio_h=(double)fixedHeight/(sdl.draw.height*sdl.draw.scaley);
		if ( ratio_w < ratio_h) {
			sdl.clip.w=fixedWidth;
			
		if (sdl.automaticheight){
			sdl.clip.h=(Bit16u)(sdl.draw.height*sdl.draw.scaley*ratio_w + 0.1); //possible rounding issues				
		} else {
			sdl.clip.h=fixedHeight;
		}
			
		} else {
			/*
			 * The 0.4 is there to correct for rounding issues.
			 * (partly caused by the rounding issues fix in RENDER_SetSize)
			 */
			sdl.clip.w=(Bit16u)(sdl.draw.width*sdl.draw.scalex*ratio_h + 0.4);
			sdl.clip.h=(Bit16u)fixedHeight;
		}
		if (sdl.desktop.fullscreen) {
			sdl.window = GFX_SetSDLWindowMode(fixedWidth, fixedHeight, sdl.desktop.fullscreen, screenType);
						
		} else {

			sdl.window = GFX_SetSDLWindowMode(sdl.clip.w, sdl.clip.h, sdl.desktop.fullscreen, screenType);
			
			if (sdl.displayNumber==0){ 
				Center_SDL_SetWindowMode(sdl.clip.w, sdl.clip.h);
			} else {
						
				SDL_SetWindowPosition(sdl.window,
				SDL_WINDOWPOS_CENTERED_DISPLAY( sdl.displayNumber ) , SDL_WINDOWPOS_CENTERED_DISPLAY( sdl.displayNumber ));				
			}
		}		
		if (sdl.window && SDL_GetWindowFlags(sdl.window) & SDL_WINDOW_FULLSCREEN) {
			int windowWidth;
			SDL_GetWindowSize(sdl.window, &windowWidth, NULL);
			
		 LOG_MSG("SDL : sdl.clip.x = %d",sdl.clip.x);
		 LOG_MSG("SDL : sdl.clip.y = %d",sdl.clip.y);
		
			sdl.clip.x=(Sint16)((windowWidth-sdl.clip.w)/2);
			sdl.clip.y=(Sint16)((fixedHeight-sdl.clip.h)/2);
		} else {
			sdl.clip.x = 0;
			sdl.clip.y = 0;
		}
		return sdl.window;
	} else {
		sdl.clip.x=0;sdl.clip.y=0;
		sdl.clip.w=(Bit16u)(sdl.draw.width*sdl.draw.scalex);
		sdl.clip.h=(Bit16u)(sdl.draw.height*sdl.draw.scaley);
		sdl.window = GFX_SetSDLWindowMode(sdl.clip.w, sdl.clip.h, sdl.desktop.fullscreen, screenType);

		return sdl.window;
	}
}

#if C_OPENGL
/* Create a GLSL shader object, load the shader source, and compile the shader.
 */
GLuint GFX_LoadGLShader(GLenum type, const char *shaderSrc) {

	// Create the shader object
	GLuint shader = 0;
	if (!(shader = glCreateShader(type))) {
		LOG_MSG("SDL : %s", SDL_GetError());
		return 0;
	}

	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);

	// Compile the shader
	glCompileShader(shader);

	// Check the compile status
	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			std::vector<GLchar> infoLog(infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, &infoLog[0]);
			std::stringstream ss;
			for (std::vector<GLchar>::iterator it = infoLog.begin(); it != infoLog.end(); ++it)
			{
				ss << *it;
			}
			LOG_MSG("SDL : OpenGL, Error compiling program: %s", ss.rdbuf()->str().c_str());
		}

		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
#endif

#if C_OPENGL
GLfloat get_texture_x(GLfloat vertex_x, GLfloat video_x, GLfloat texture_x) {
	return (vertex_x + 1.0) / 2.0 * video_x / texture_x;
}


GLfloat get_texture_y(GLfloat vertex_y, GLfloat video_y, GLfloat texture_y) {
	return (1.0 - vertex_y) / 2.0 * video_y / texture_y;
}
#endif

Bitu GFX_SetSize(Bitu width,Bitu height,Bitu flags,double scalex,double scaley,GFX_CallBack_t callback) {

	/* Try to set the Current Desktop Index on Multi Monitor Systems*/

	//sdl.displayNumber = SDL_GetWindowDisplayIndex(sdl.window);
	//nCurrentDisplay   = sdl.displayNumber;
	
	Section_prop *section = static_cast<Section_prop *>(control->GetSection("render"));	
	bool rDebug = section->Get_bool("debug");
	
	if (sdl.updating)
		GFX_EndUpdate( 0 );

	sdl.draw.width=width;
	sdl.draw.height=height;
	sdl.draw.callback=callback;
	sdl.draw.scalex=scalex;
	sdl.draw.scaley=scaley;

	Bitu retFlags = 0;
	switch (sdl.desktop.want_type) {
	case SCREEN_SURFACE:	
dosurface:
		sdl.desktop.type=SCREEN_SURFACE;
		sdl.clip.w=width;
		sdl.clip.h=height;
		if (sdl.desktop.fullscreen) {
			if (sdl.desktop.full.fixed) {
				sdl.clip.x=(Sint16)((sdl.desktop.full.width-width)/2);
				sdl.clip.y=(Sint16)((sdl.desktop.full.height-height)/2);
				sdl.window = GFX_SetSDLWindowMode(sdl.desktop.full.width,
				                                  sdl.desktop.full.height,
				                                  sdl.desktop.fullscreen, sdl.desktop.type);
				
				if (sdl.window == NULL)
					E_Exit("Could not set fullscreen video mode %ix%i-%i: %s",sdl.desktop.full.width,sdl.desktop.full.height,sdl.desktop.bpp,SDL_GetError());
			} else {
				sdl.clip.x=0;sdl.clip.y=0;
				sdl.window = GFX_SetSDLWindowMode(width, height,
				                                  sdl.desktop.fullscreen, sdl.desktop.type);
				if (sdl.window == NULL)
					E_Exit("Could not set fullscreen video mode %ix%i-%i: %s",(int)width,(int)height,sdl.desktop.bpp,SDL_GetError());
			}
		} else {
			sdl.clip.x=0;sdl.clip.y=0;
			sdl.window = GFX_SetSDLWindowMode(width, height,
			                                  sdl.desktop.fullscreen, sdl.desktop.type);
			if (sdl.window == NULL)
				E_Exit("Could not set windowed video mode %ix%i-%i: %s",(int)width,(int)height,sdl.desktop.bpp,SDL_GetError());
		}
		sdl.surface = SDL_GetWindowSurface(sdl.window);
		if (sdl.surface == NULL)
				E_Exit("Could not retrieve window surface: %s",SDL_GetError());
		switch (sdl.surface->format->BitsPerPixel) {
			case 8:
				retFlags = GFX_CAN_8;
				break;
			case 15:
				retFlags = GFX_CAN_15;
				break;
			case 16:
				retFlags = GFX_CAN_16;
				break;
			case 32:
				retFlags = GFX_CAN_32;
				break;
		}
		/* Fix a glitch with aspect=true occuring when
		changing between modes with different dimensions */
		SDL_FillRect(sdl.surface, NULL, SDL_MapRGB(sdl.surface->format, 0, 0, 0));
		SDL_UpdateWindowSurface(sdl.window);
		SDL_ShowWindow(sdl.window);	
		break;
	case SCREEN_TEXTURE:
	{
		if (!GFX_SetupWindowScaled(sdl.desktop.want_type)) {
			LOG_MSG("SDL : Can't set video mode, falling back to surface");
			goto dosurface;
		}

		if (strcmp(sdl.rendererDriver, "auto"))
			SDL_SetHint(SDL_HINT_RENDER_DRIVER, sdl.rendererDriver);
			sdl.renderer = SDL_CreateRenderer(sdl.window, nCurrentDisplay,
		                                  SDL_RENDERER_ACCELERATED |
		                                  (sdl.desktop.vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
		if (!sdl.renderer) {
			LOG_MSG("SDL : %s\n", SDL_GetError());
			LOG_MSG("SDL : Can't create renderer, falling back to surface");
			goto dosurface;
		}
		/* SDL_PIXELFORMAT_ARGB8888 is possible with most
		rendering drivers, "opengles" being a notable exception */
		sdl.texture.texture = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888,
		                                        SDL_TEXTUREACCESS_STREAMING, width, height);
		/* SDL_PIXELFORMAT_ABGR8888 (not RGB) is the
		only supported format for the "opengles" driver */
		if (!sdl.texture.texture) {
			if (flags & GFX_RGBONLY) goto dosurface;
			sdl.texture.texture = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ABGR8888,
			                                        SDL_TEXTUREACCESS_STREAMING, width, height);
		}
		if (!sdl.texture.texture) {
			SDL_DestroyRenderer(sdl.renderer);
			sdl.renderer = NULL;
			LOG_MSG("SDL : Can't create texture, falling back to surface");
			goto dosurface;
		}
		SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		sdl.desktop.type=SCREEN_TEXTURE;
		Uint32 pixelFormat;
		SDL_QueryTexture(sdl.texture.texture, &pixelFormat, NULL, NULL, NULL);
		sdl.texture.pixelFormat = SDL_AllocFormat(pixelFormat);
		switch (SDL_BITSPERPIXEL(pixelFormat)) {
			case 8:
				retFlags = GFX_CAN_8;
				break;
			case 15:
				retFlags = GFX_CAN_15;
				break;
			case 16:
				retFlags = GFX_CAN_16;
				break;
			case 24: /* SDL_BYTESPERPIXEL is probably 4, though. */
			case 32:
				retFlags = GFX_CAN_32;
				break;
		}
		retFlags |= GFX_SCALING;
		SDL_RendererInfo rendererInfo;
		SDL_GetRendererInfo(sdl.renderer, &rendererInfo);
		LOG_MSG("SDL : Using driver \"%s\" for renderer", rendererInfo.name);
		if (rendererInfo.flags & SDL_RENDERER_ACCELERATED)
			retFlags |= GFX_HARDWARE;
		
		SDL_ShowWindow(sdl.window);			
		break;
	}
#if C_OPENGL
	case SCREEN_OPENGL:
	{
		if (flags & GFX_CAN_8){
			if (rDebug == true){
			LOG_MSG("SDL : OpenGL, Using 8Bit");
			}
		}
		if (flags & GFX_CAN_15){
			if (rDebug == true){			
			LOG_MSG("SDL : OpenGL, Using 15Bit");
			}
		}		
		if (flags & GFX_CAN_16){
			if (rDebug == true){			
			LOG_MSG("SDL : OpenGL, Using 16Bit");
			}
		}		
		if (flags & GFX_CAN_32){
			if (rDebug == true){			
			LOG_MSG("SDL : OpenGL, Using 32Bit");			
			}
		}		
		if (flags & GFX_RGBONLY){
			if (rDebug == true){		
			LOG_MSG("SDL : OpenGL, Using RGB Only");
			}
		}		
		
		if (!(flags&GFX_CAN_32)) {
			//We only accept 32bit output from the scalers here
			//goto dosurface; // BGRA otherwise
		}
		int texsize=2 << int_log2(width > height ? width : height);
		if (texsize>sdl.opengl.max_texsize) {
			LOG_MSG("SDL : OpenGL, No support for texturesize of %d, falling back to surface",texsize);
			goto dosurface;
		}
		SDL_GL_ResetAttributes();
		//SDL_GL_LoadLibrary(NULL); // Default OpenGL is fine.
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

		/*
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		*/
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		/* Sync to VBlank if desired */
		/* SDL_GL_SetSwapInterval(sdl.desktop.vsync ? 1 : 0);*/
		SDL_GL_SetSwapInterval(sdl.desktop.vsync);
		GFX_SetupWindowScaled(sdl.desktop.want_type);


		/* We may simply use SDL_BYTESPERPIXEL
		here rather than SDL_BITSPERPIXEL   */
		if (!sdl.window || SDL_BYTESPERPIXEL(SDL_GetWindowPixelFormat(sdl.window))<2) {
			LOG_MSG("SDL : OpenGL, Can't open drawing window, are you running in 16bpp(or higher) mode?");
			goto dosurface;
		}
		sdl.opengl.context = SDL_GL_CreateContext(sdl.window);
		if (NULL == sdl.opengl.context) {
			LOG_MSG("SDL : OpenGL, Can't create OpenGL context, falling back to surface");
			goto dosurface;
		}

		int major_version = 0;
		if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_version) != 0) {
			LOG_MSG("SDL : %s", SDL_GetError());
		}
		int minor_version = 0;
		if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_version) != 0) {
			LOG_MSG("SDL : %s", SDL_GetError());
		}
		// if (major_version < 3 || minor_version < 3) {
			// LOG_MSG("SDL : OpenGL, Can't create OpenGL 3.3 context, falling back to surface.");
			// goto dosurface;
		// }

		
		SDL_GL_MakeCurrent(sdl.window, sdl.opengl.context);



		sdl.opengl.framebuf=calloc(1, texsize*texsize*4);		//32 bit color


		sdl.opengl.pitch=width*4;
		int windowHeight;
		SDL_GetWindowSize(sdl.window, NULL, &windowHeight);

		GLuint fragmentShader = 0;
		GLuint vertexShader = GFX_LoadGLShader(GL_VERTEX_SHADER, sdl.opengl.vertex_shader_src.c_str());
		if (vertexShader) {
			fragmentShader = GFX_LoadGLShader(GL_FRAGMENT_SHADER, sdl.opengl.fragment_shader_src.c_str());
			if (!fragmentShader) {
				glDeleteShader(vertexShader);
				LOG_MSG("SDL : OpenGL, Can't compile fragment shader, falling back to stock.");
				vertexShader = GFX_LoadGLShader(GL_VERTEX_SHADER, vertex_shader_default_src.c_str());
				fragmentShader = GFX_LoadGLShader(GL_FRAGMENT_SHADER, fragment_shader_default_src.c_str());
			}
		} else {
			LOG_MSG("SDL : OpenGL, Can't compile vertex shader, falling back to stock.");
			vertexShader = GFX_LoadGLShader(GL_VERTEX_SHADER, vertex_shader_default_src.c_str());
			fragmentShader = GFX_LoadGLShader(GL_FRAGMENT_SHADER, fragment_shader_default_src.c_str());
		}

		sdl.opengl.program_object = glCreateProgram();
		if (!sdl.opengl.program_object) {
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
			goto dosurface;
		}
		glAttachShader (sdl.opengl.program_object, vertexShader);
		glAttachShader (sdl.opengl.program_object, fragmentShader);
		// Link the program
		glLinkProgram (sdl.opengl.program_object);
		// Even if we *are* successful, we may delete the shader objects
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Check the link status
		GLint isProgramLinked;
		glGetProgramiv (sdl.opengl.program_object, GL_LINK_STATUS, &isProgramLinked);

		if (!isProgramLinked)  {
			GLint infoLen = 0;

			glGetProgramiv(sdl.opengl.program_object, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1) {
				std::vector<GLchar> infoLog(infoLen);
				glGetProgramInfoLog(sdl.opengl.program_object, infoLen, NULL, &infoLog[0]);
				std::stringstream ss;
				for (std::vector<GLchar>::iterator it = infoLog.begin(); it != infoLog.end(); ++it)
				{
					ss << *it;
				}
				LOG_MSG("SDL : OpenGL, Error linking program: %s", ss.rdbuf()->str().c_str());
			}

			glDeleteProgram(sdl.opengl.program_object);
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Fall back to stock shaders.
			vertexShader = GFX_LoadGLShader(GL_VERTEX_SHADER, vertex_shader_default_src.c_str());
			fragmentShader = GFX_LoadGLShader(GL_FRAGMENT_SHADER, fragment_shader_default_src.c_str());
			sdl.opengl.program_object = glCreateProgram();
			glAttachShader (sdl.opengl.program_object, vertexShader);
			glAttachShader (sdl.opengl.program_object, fragmentShader);
			glLinkProgram (sdl.opengl.program_object);
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
		}

		 LOG_MSG("=============================================");		
		 LOG_MSG("sdl.clip.x: %d",sdl.clip.x);
		 LOG_MSG("windowHeight: %d",windowHeight);		
		 LOG_MSG("sdl.clip.y: %d",sdl.clip.y);
		 LOG_MSG("sdl.clip.h: %d",sdl.clip.h);
		 LOG_MSG("sdl.clip.w: %d",sdl.clip.w);	
		 LOG_MSG("windowHeight: %d",windowHeight-(sdl.clip.y+sdl.clip.h),sdl.clip.w,sdl.clip.h);
		 LOG_MSG("=============================================");		
		
		glViewport(sdl.clip.x,windowHeight-(sdl.clip.y+sdl.clip.h),sdl.clip.w,sdl.clip.h);


		glDeleteTextures(1,&sdl.opengl.texture);
 		glGenTextures(1,&sdl.opengl.texture);
		glBindTexture(GL_TEXTURE_2D,sdl.opengl.texture);
		// No borders
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		if (!sdl.opengl.bilinear || ( (sdl.clip.h % height) == 0 && (sdl.clip.w % width) == 0) ) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texsize, texsize, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (const GLvoid*)sdl.opengl.framebuf);
		
		// #if defined (MACOSX) || defined(LINUX)		
		// Bit8u* emptytex = new Bit8u[texsize * texsize * 4];
		// memset((void*) emptytex, 0, texsize * texsize * 4);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texsize, texsize, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (const GLvoid*)emptytex);
		// delete [] emptytex;
		//#endif

		glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
		glShadeModel (GL_FLAT);
		glBindTexture(GL_TEXTURE_2D, sdl.opengl.texture);

		// Time to take advantage of the shader now
		glUseProgram(sdl.opengl.program_object);
		if (rDebug == true){
			LOG_MSG("SDL : Game resolution: %dx%d", width, height);
		}

		float uniform_block[6];
		uniform_block[0] = width;
		uniform_block[1] = height;
		uniform_block[2] = texsize;
		uniform_block[3] = texsize;
		uniform_block[4] = sdl.clip.w;
		uniform_block[5] = sdl.clip.h;

		// Pack the uniforms block
		glGenBuffers(1, &sdl.opengl.ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, sdl.opengl.ubo);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, sdl.opengl.ubo);
		glBufferData(GL_UNIFORM_BUFFER, 24, uniform_block, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenVertexArrays(1, &sdl.opengl.vao);
		glBindVertexArray(sdl.opengl.vao);

		// Vertex coordinates

		// upper left
		sdl.opengl.vertex_data[0] = -1.0f;
		sdl.opengl.vertex_data[1] = 1.0f;
		sdl.opengl.vertex_data[2] = 0.0f;

		// lower left
		sdl.opengl.vertex_data[3] = -1.0f;
		sdl.opengl.vertex_data[4] = -1.0f;
		sdl.opengl.vertex_data[5] = 0.0f;

		// upper right
		sdl.opengl.vertex_data[6] = 1.0f;
		sdl.opengl.vertex_data[7] = 1.0f;
		sdl.opengl.vertex_data[8] = 0.0f;

		// lower right
		sdl.opengl.vertex_data[9] = 1.0f;
		sdl.opengl.vertex_data[10] = -1.0f;
		sdl.opengl.vertex_data[11] = 0.0f;

		glGenBuffers(1, &sdl.opengl.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, sdl.opengl.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sdl.opengl.vertex_data), sdl.opengl.vertex_data, GL_STATIC_DRAW);

		glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(POSITION_LOCATION);

		// upper left
		sdl.opengl.texture_data[0] = get_texture_x(-1.0f, width, texsize);
		sdl.opengl.texture_data[1] = get_texture_y(1.0f, height, texsize);

		// lower left
		sdl.opengl.texture_data[2] = get_texture_x(-1.0f, width, texsize);
		sdl.opengl.texture_data[3] = get_texture_y(-1.0f, height, texsize);

		// upper right
		sdl.opengl.texture_data[4] = get_texture_x(1.0f, width, texsize);
		sdl.opengl.texture_data[5] = get_texture_y(1.0f, height, texsize);

		// lower right
		sdl.opengl.texture_data[6] = get_texture_x(1.0f, width, texsize);
		sdl.opengl.texture_data[7] = get_texture_y(-1.0f, height, texsize);

		glGenBuffers(1, &sdl.opengl.texture_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, sdl.opengl.texture_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sdl.opengl.texture_data), sdl.opengl.texture_data, GL_STATIC_DRAW);
		glVertexAttribPointer(TEXTURE_LOCATION, 2, GL_FLOAT, GL_TRUE, 2 * sizeof (GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(TEXTURE_LOCATION);

		sdl.desktop.type=SCREEN_OPENGL;
		retFlags = GFX_CAN_32 | GFX_SCALING;
		retFlags |= GFX_HARDWARE;
						
		if (SDLInfoStringShowed == false);
		{
		LOG_MSG("SDL : OpenGL\n"
		        "      Vendor  : %s\n"
				"      Renderer: %s\n"
				"      Version : %s\n"
				"      Shader  : %s\n"
			    "      VSync   : %d\n\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), SDL_GL_GetSwapInterval());
		SDLInfoStringShowed = true;
		}
		
		SDL_ShowWindow(sdl.window);			
	
	break;
		}//OPENGL
#endif	//C_OPENGL
	default:
		goto dosurface;
		break;
	}//CASE

	// System Menu ---------------------------------------------------------------------------------------------
	if ( sdl.desktop.borderless == false) {
	
		if (!sdl.desktop.fullscreen){
		
			if (sdl.hSystemMenu == false){
				Set_Window_HMenu();
			}
					
			sdl.hSystemMenu = true;		
		}
	}

	// System Menu ---------------------------------------------------------------------------------------------
	
	if (retFlags)
		GFX_Start();
	if (!sdl.mouse.autoenable) SDL_ShowCursor(sdl.mouse.autolock?SDL_DISABLE:SDL_ENABLE);
	return retFlags;
};

void GFX_GetCaptureMouse(void){}

void GFX_CaptureMouse(void) {
	sdl.mouse.locked=!sdl.mouse.locked;
	
	if (sdl.mouse.locked)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);		
		SDL_ShowCursor(SDL_DISABLE);

		if (sdl.desktop.fullscreen)
		{
			SDL_SetWindowGrab(sdl.window, SDL_TRUE);
		}
	
		if ((sdl.SystemKeyDisable == true) || ((isVirtualModus == true) && (sdl.desktop.fullscreen == true)))
			disableKeys.Block();
		
	} else {

		SDL_SetRelativeMouseMode(SDL_FALSE);
		if (!sdl.desktop.fullscreen)
		{
			SDL_SetWindowGrab(sdl.window, SDL_FALSE);
		}
		
		if (sdl.mouse.autoenable || !sdl.mouse.autolock)
		{
			SDL_ShowCursor(SDL_ENABLE);
			
			if ((sdl.SystemKeyDisable == true) || ((isVirtualModus == true) && (!sdl.desktop.fullscreen == true)))
				disableKeys.Unblock();
		}
	}
    mouselocked=sdl.mouse.locked;		
	GFX_SetTitle(-1,-1,false);	
	
	//LOG_MSG("SDL GFX_CaptureMouse Mouse %d",mouselocked?true:false);	
}

void GFX_CaptureMouse_Mousecap_on(void) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_ShowCursor(SDL_DISABLE);
		//if (sdl.desktop.fullscreen){
			SDL_SetWindowGrab(sdl.window, SDL_TRUE);
		//}
	mouselocked=true;
	sdl.mouse.locked=mouselocked;		
	GFX_SetTitle(-1,-1,false);	
	
	
	//LOG_MSG("SDL GFX_CaptureMouse_Mousecap_on Mouse %d",mouselocked?true:false);	
}

void GFX_UpdateSDLCaptureState(void) {
	if (sdl.mouse.locked) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_SetRelativeMouseMode(SDL_FALSE);
		if (sdl.mouse.autoenable || !sdl.mouse.autolock) SDL_ShowCursor(SDL_ENABLE);
	}
	CPU_Reset_AutoAdjust();
	GFX_SetTitle(-1,-1,false);
}


bool mouselocked; //Global variable for mapper

static void CaptureMouse(bool pressed) {
	if (!pressed){
		return;
	}
	GFX_CaptureMouse();
}

#if defined (WIN32)
STICKYKEYS stick_keys = {sizeof(STICKYKEYS), 0};
void sticky_keys(bool restore){
	static bool inited = false;
	if (!inited){
		inited = true;
		SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &stick_keys, 0);
	}
	if (restore) {
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &stick_keys, 0);
		return;
	}
	//Get current sticky keys layout:
	STICKYKEYS s = {sizeof(STICKYKEYS), 0};
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &s, 0);
	if ( !(s.dwFlags & SKF_STICKYKEYSON)) { //Not on already
		s.dwFlags &= ~SKF_HOTKEYACTIVE;
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &s, 0);
	}
}
#endif

void GFX_SwitchFullScreen(void) {
	if (!sdl.desktop.fullscreen){
		autoclocked = SDL_GetWindowGrab(sdl.window);
	}
	
	sdl.desktop.fullscreen=!sdl.desktop.fullscreen;
	
	if (sdl.desktop.fullscreen)
	{		
		if (!sdl.mouse.locked)
		{
			GFX_CaptureMouse();	
			sticky_keys(false); //disable sticky keys in fullscreen mode			
		}
	} else {
		if (autoclocked == 1)
		{
			GFX_CaptureMouse();			
		} 
		sticky_keys(true); //restore sticky keys to default state in windowed mode.
	}
	
		GFX_ResetVoodoo();
		LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);
		
		
}

static void SwitchFullScreen(bool pressed) {
	if (!pressed){
		return;
	}		
	 
	if (sdl.desktop.lazy_fullscreen) {
		sdl.desktop.lazy_fullscreen_req=true;
		LOG(LOG_MISC,LOG_WARN)("SDL : GFX LF, fullscreen switching not supported \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);
	} else {	
	
	
		LOG(LOG_MISC,LOG_WARN)("SDL : Screen Switch (Is Voodoo Activ %d)",bVoodooOpen?true:false);	
				
		GFX_SwitchFullScreen();
					
	}		
}

int GFX_GetSDLVideo(void) {
	
	 int wDATA = SDL_GetWindowID(sdl.window);
	 return wDATA;
}

void GFX_SwitchLazyFullscreen(bool lazy) {
	//LOG_MSG("SDL GFX_SwitchLazyFullscreen %d",lazy?true:false);
	sdl.desktop.lazy_fullscreen=lazy;
	sdl.desktop.lazy_fullscreen_req=false;
}

void GFX_SwitchFullscreenNoReset(void) {
	sdl.desktop.fullscreen=!sdl.desktop.fullscreen;
	LOG_MSG("SDL GFX_SwitchFullscreenNoReset %d",sdl.desktop.fullscreen?true:false);	
}

bool GFX_LazyFullscreenRequested(void) {
	//LOG_MSG("SDL GFX_LazyFullscreenRequested");
	if (sdl.desktop.lazy_fullscreen)
	{
		return sdl.desktop.lazy_fullscreen_req;
	}
	return false;
}

void GFX_RestoreMode(void) {
	

	GFX_SetSize(sdl.draw.width,sdl.draw.height,sdl.draw.flags,sdl.draw.scalex,sdl.draw.scaley,sdl.draw.callback);
	GFX_UpdateSDLCaptureState();
}


bool GFX_StartUpdate(Bit8u * & pixels,Bitu & pitch) {
	if (!sdl.update_display_contents)
		return false;
	if (!sdl.active || sdl.updating)
		return false;
	switch (sdl.desktop.type) {
	case SCREEN_SURFACE:
		{
			pixels=(Bit8u *)sdl.surface->pixels;
			pixels+=sdl.clip.y*sdl.surface->pitch;
			pixels+=sdl.clip.x*sdl.surface->format->BytesPerPixel;
			pitch=sdl.surface->pitch;
		}
		sdl.updating=true;
		return true;
	case SCREEN_TEXTURE:
	{
		void * texPixels;
		int texPitch;
		if (SDL_LockTexture(sdl.texture.texture, NULL, &texPixels, &texPitch) < 0)
			return false;
		pixels = (Bit8u *)texPixels;
		pitch = texPitch;
		sdl.updating=true;
		return true;
	}
#if C_OPENGL
	case SCREEN_OPENGL:
	    pixels=(Bit8u *)sdl.opengl.framebuf;

		pitch=sdl.opengl.pitch;
		sdl.updating=true;
		return true;
#endif
	default:
		break;
	}
	return false;
}


void GFX_EndUpdate( const Bit16u *changedLines ) {
	if (!sdl.update_display_contents)
		return;
	if (!sdl.updating)
		return;
	sdl.updating=false;
	switch (sdl.desktop.type)
	{
		case SCREEN_SURFACE:
			if (changedLines)
			{
				Bitu y			= 0;
				Bitu index 		= 0;
				Bitu rectCount 	= 0;
				
				while (y < sdl.draw.height)
				{
					if ( !(index & 1) )
					{
						y += changedLines[index];
						
					} else {
						SDL_Rect *rect 	= &sdl.updateRects[rectCount++];
						rect->x 		= sdl.clip.x;
						rect->y 		= sdl.clip.y + y;
						rect->w 		= (Bit16u)sdl.draw.width;
						rect->h 		= changedLines[index];
						
						y += changedLines[index];
					}
					index++;
				}
			
				if (rectCount)
					SDL_UpdateWindowSurfaceRects( sdl.window, sdl.updateRects, rectCount );
			}
		break;
		
		case SCREEN_TEXTURE:
			SDL_UnlockTexture(sdl.texture.texture);
			SDL_RenderClear(sdl.renderer);
			SDL_RenderCopy(sdl.renderer, sdl.texture.texture, NULL, &sdl.clip);
			SDL_RenderPresent(sdl.renderer);
		break;
		
	case SCREEN_OPENGL:
	
		/* 
			Clear drawing area. Some drivers (on Linux) have more than 2 buffers and
			the screen might be dirty because of other programs.
		*/
			
			glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);	
		
			if (changedLines) {
				Bitu y 		= 0;
				Bitu index 	= 0;
			
				glBindTexture(GL_TEXTURE_2D, sdl.opengl.texture);
				
				while (y < sdl.draw.height)
				{
					if (!(index & 1))
					{
						y += changedLines[index];
				
					} else {
					
						Bit8u *pixels = (Bit8u *)sdl.opengl.framebuf + y * sdl.opengl.pitch;
						Bitu height = changedLines[index];
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, y,
						sdl.draw.width, height, GL_BGRA_EXT,
						GL_UNSIGNED_INT_8_8_8_8_REV, pixels );
						y += height;
					}
					index++;
				}
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				SDL_GL_SwapWindow(sdl.window);
			}
		break;

		default:
		break;
	}
}


Bitu GFX_GetRGB(Bit8u red,Bit8u green,Bit8u blue) {
	switch (sdl.desktop.type) {
	case SCREEN_SURFACE:
		return SDL_MapRGB(sdl.surface->format,red,green,blue);
	case SCREEN_TEXTURE:
		return SDL_MapRGB(sdl.texture.pixelFormat,red,green,blue);
	case SCREEN_OPENGL:
		return ((blue << 0) | (green << 8) | (red << 16)) | (255 << 24);
	}
	return 0;
}

void GFX_TearDown(void) {
	if (sdl.updating)
		GFX_EndUpdate( 0 );

	if (sdl.surface) {
		SDL_FreeSurface(sdl.surface);
		sdl.surface=0;
	}
}

void GFX_Stop() {
	if (sdl.updating)
		GFX_EndUpdate( 0 );
	sdl.active=false;
}

void GFX_Start() {
	sdl.active=true;		
}

/* NOTE: The following appears to do its job on Android only *before*
 * a screen rotation occurs. After that, the old dimensions are retrieved.
 * For the updated dimensions we should listen to a window resize event.
 */
void GFX_ObtainDisplayDimensions() {
	SDL_Rect displayDimensions;
	SDL_GetDisplayBounds(sdl.displayNumber, &displayDimensions);
	sdl.desktop.full.width = displayDimensions.w;
	sdl.desktop.full.height = displayDimensions.h;

}

/* Manually update display dimensions in case of a window resize,
 * IF there is the need for that ("yes" on Android, "no" otherwise).
 * Used for the mapper UI on Android.
 * Reason is the usage of GFX_GetSDLSurfaceSubwindowDims, as well as a
 * mere notification of the fact that the window's dimensions are modified.
 */
void GFX_UpdateDisplayDimensions(int width, int height) {
	if (sdl.desktop.full.display_res && sdl.desktop.fullscreen) {
		/* Note: We should not use GFX_ObtainDisplayDimensions
		(SDL_GetDisplayBounds) on Android after a screen rotation:
		The older values from application startup are returned. */
		sdl.desktop.full.width = width;
		sdl.desktop.full.height = height;
	}
}

static void GUI_ShutDown(Section * /*sec*/) {
	GFX_Stop();
	
	if (sdl.draw.callback)
	   (sdl.draw.callback)( GFX_CallBackStop );
	
	if (sdl.mouse.locked)
		GFX_CaptureMouse();
	
	/* The following line code cause a crasdh at Exit in Fullscreen Mode
	   if (sdl.desktop.fullscreen) GFX_SwitchFullScreen();	
	*/
	LOG_MSG("SDL : Exit -- Successfully");
}




extern Bit8u int10_font_14[256 * 14];
static void OutputString(Bitu x,Bitu y,const char * text,Bit32u color,Bit32u color2,SDL_Surface * output_surface) {
	Bit32u * draw=(Bit32u*)(((Bit8u *)output_surface->pixels)+((y)*output_surface->pitch))+x;
	while (*text) {
		Bit8u * font=&int10_font_14[(*text)*14];
		Bitu i,j;
		Bit32u * draw_line=draw;
		for (i=0;i<14;i++) {
			Bit8u map=*font++;
			for (j=0;j<8;j++) {
				if (map & 0x80) *((Bit32u*)(draw_line+j))=color; else *((Bit32u*)(draw_line+j))=color2;
				map<<=1;
			}
			draw_line+=output_surface->pitch/4;
		}
		text++;
		draw+=8;
	}
}

#include "dosbox_splash.h"

extern void Restart(bool pressed);
extern void UI_Run(bool);

static void GUI_StartUp(Section * sec) {	


	/*
		Voodoo Struct Inits
	*/
	extVoodoo.GL_filtering	= 999;
	extVoodoo.GL_shade		= 999;
	extVoodoo.gl_wrap_s		= 999;
	extVoodoo.gl_wrap_t		= 999;
	extVoodoo.RGB_Format	= 999;
	extVoodoo.RGB_Type		= 999;
	extVoodoo.GLDark		= 999;
	extVoodoo.GLScan		= 999;
	
	
	sec->AddDestroyFunction(&GUI_ShutDown);
	Section_prop * section=static_cast<Section_prop *>(sec);
	sdl.active=false;
	sdl.updating=false;
	sdl.update_window=true;
	sdl.update_display_contents=true;
	GFX_SetIcon();

	sdl.desktop.lazy_fullscreen=false;
	sdl.desktop.lazy_fullscreen_req=false;

	sdl.desktop.fullscreen=section->Get_bool("fullscreen");

	/* Change the Fullscreenmode (Normal <-> Desktop Exklusiv)
	   Set Default to  SDL_WINDOW_FULLSCREEN_DESKTOP
	*/
	
	sdl.desktop.screenflag = SDL_WINDOW_FULLSCREEN_DESKTOP;
	std::string fsflag=section->Get_string("fullscreenmode");	
	if      (fsflag == "standard")  { sdl.desktop.screenflag = SDL_WINDOW_FULLSCREEN;  }
	else if (fsflag == "exklusiv")  { sdl.desktop.screenflag = SDL_WINDOW_FULLSCREEN_DESKTOP;  }
	
	sdl.wait_on_error=section->Get_bool("waitonerror");
	sdl.desktop.borderless=section->Get_bool("windowborderless");	
	
	Prop_multival* p=section->Get_multival("priority");
	std::string focus = p->GetSection()->Get_string("active");
	std::string notfocus = p->GetSection()->Get_string("inactive");

	if      (focus == "lowest")  { sdl.priority.focus = PRIORITY_LEVEL_LOWEST;  }
	else if (focus == "lower")   { sdl.priority.focus = PRIORITY_LEVEL_LOWER;   }
	else if (focus == "normal")  { sdl.priority.focus = PRIORITY_LEVEL_NORMAL;  }
	else if (focus == "higher")  { sdl.priority.focus = PRIORITY_LEVEL_HIGHER;  }
	else if (focus == "highest") { sdl.priority.focus = PRIORITY_LEVEL_HIGHEST; }
	else if (focus == "realtime"){ sdl.priority.focus = PRIORITY_LEVEL_REALTIME; }

	if      (notfocus == "lowest")  { sdl.priority.nofocus=PRIORITY_LEVEL_LOWEST;  }
	else if (notfocus == "lower")   { sdl.priority.nofocus=PRIORITY_LEVEL_LOWER;   }
	else if (notfocus == "normal")  { sdl.priority.nofocus=PRIORITY_LEVEL_NORMAL;  }
	else if (notfocus == "higher")  { sdl.priority.nofocus=PRIORITY_LEVEL_HIGHER;  }
	else if (notfocus == "highest") { sdl.priority.nofocus=PRIORITY_LEVEL_HIGHEST; }
	else if (notfocus == "realtime"){ sdl.priority.nofocus=PRIORITY_LEVEL_REALTIME;}	
	else if (notfocus == "pause")   {
		/* we only check for pause here, because it makes no sense
		 * for DOSBox to be paused while it has focus
		 */
		sdl.priority.nofocus=PRIORITY_LEVEL_PAUSE;
		
			
	}

	sdl.displayNumber=section->Get_int("display");
	LOG_MSG("SDL : Display Found: 0->%d", SDL_GetNumVideoDisplays()-1);		
	
	if ( (sdl.displayNumber < 0) || (sdl.displayNumber >= SDL_GetNumVideoDisplays()) ) {		
		LOG_MSG("SDL : Display  (%d): Out of Range, Using 0",sdl.displayNumber);
		sdl.displayNumber = 0;
	} else {
		LOG_MSG("SDL : Display Use  : %d",sdl.displayNumber);	
	}
	nCurrentDisplay = sdl.displayNumber;
	
	SetPriority(sdl.priority.focus); //Assume focus on startup
	sdl.mouse.locked=false;
	mouselocked=false; //Global for mapper
	sdl.mouse.requestlock=false;
	sdl.desktop.full.fixed=false;
	const char* fullresolution=section->Get_string("fullresolution");
	sdl.desktop.full.width  = 0;
	sdl.desktop.full.height = 0;
	if(fullresolution && *fullresolution) {
		char res[100];
		safe_strncpy( res, fullresolution, sizeof( res ));
		fullresolution = lowcase (res);//so x and X are allowed
		if (strcmp(fullresolution,"original")) {
			sdl.desktop.full.fixed = true;
			//if (strcmp(fullresolution,"desktop")) { //desktop = 0x0
				char* height = const_cast<char*>(strchr(fullresolution,'x'));
				if (height && * height) {
					*height = 0;
					sdl.desktop.full.height = (Bit16u)atoi(height+1);
					sdl.desktop.full.width  = (Bit16u)atoi(res);					
				}
			/*}else{
				SDL_DisplayMode displayMode;
				SDL_GetDesktopDisplayMode(nCurrentDisplay, &displayMode);				
				sdl.desktop.full.height = displayMode.h;
				sdl.desktop.full.width  = displayMode.w;					
			}*/
		}
		// On SDL_WINDOW_FULLSCREEN_DESKTOP Force to 0x0
		if ( sdl.desktop.screenflag == SDL_WINDOW_FULLSCREEN_DESKTOP ){
			 sdl.desktop.full.height = 0;
			 sdl.desktop.full.width  = 0;
		}		
	}
	
	sdl.desktop.window.width  = 0;
	sdl.desktop.window.height = 0;
	const char* windowresolution=section->Get_string("windowresolution");
	if(windowresolution && *windowresolution) {
		char res[100];
		safe_strncpy( res,windowresolution, sizeof( res ));
		windowresolution = lowcase (res);//so x and X are allowed
		if(strcmp(windowresolution,"original")) {
			if (strcmp(windowresolution,"desktop")) { //desktop = 0x0
				char* height = const_cast<char*>(strchr(windowresolution,'x'));
				if(height && *height) {
					*height = 0;
					sdl.desktop.window.height = (Bit16u)atoi(height+1);
					sdl.desktop.window.width  = (Bit16u)atoi(res);
				}
			}else{
				SDL_DisplayMode displayMode;
				SDL_GetDesktopDisplayMode(nCurrentDisplay, &displayMode);				
				sdl.desktop.window.height = displayMode.h;
				sdl.desktop.window.width  = displayMode.w;				
			}			
		}
	}
	
	LOG_MSG("SDL : Display Full : %dx%d",sdl.desktop.full.width, sdl.desktop.full.height);
	LOG_MSG("SDL : Display Win  : %dx%d",sdl.desktop.window.width, sdl.desktop.window.height);	

	sdl.automaticheight=section->Get_bool("AutomaticHeight");
	sdl.windowstaskbaradjust=section->Get_int("WindowsTaskbarAdjust");
	//sdl.desktop.vsync=section->Get_bool("vsync");
	sdl.desktop.vsync = section->Get_int("vsync");
		
	sdl.desktop.full.display_res = sdl.desktop.full.fixed && (!sdl.desktop.full.width || !sdl.desktop.full.height);
	if (sdl.desktop.full.display_res) {
		GFX_ObtainDisplayDimensions();
	}


	sdl.mouse.autoenable=section->Get_bool("autolock");
	if (!sdl.mouse.autoenable){
		SDL_ShowCursor(SDL_DISABLE);
		sdl.mouse.autolock=false;
	} else {
		SDL_ShowCursor(SDL_ENABLE);
		sdl.mouse.autolock=true;		
	}
	
	sdl.mouse.middlemouselock=section->Get_bool("mmbutton");
	
	Prop_multival* p3 = section->Get_multival("sensitivity");
	sdl.mouse.xsensitivity = p3->GetSection()->Get_int("xsens");
	sdl.mouse.ysensitivity = p3->GetSection()->Get_int("ysens");
	
	/* Setup Mouse correctly if fullscreen */
	if(sdl.desktop.fullscreen) GFX_CaptureMouse();
	
	sdl.SystemKeyDisable=section->Get_bool("DisableSystemKeys");
	
	std::string output=section->Get_string("output");
	if (output == "surface") {
		sdl.desktop.want_type=SCREEN_SURFACE;		
	} else if (output == "texture") {
		sdl.desktop.want_type=SCREEN_TEXTURE;
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");			
	} else if (output == "texturenb") {
		sdl.desktop.want_type=SCREEN_TEXTURE;
		// Currently the default, but... oh well
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	} else if (output == "texturebest") {
		sdl.desktop.want_type=SCREEN_TEXTURE;
		// Currently the default, but... oh well
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");		
#if C_OPENGL
	} else if (output == "opengl") {
		sdl.desktop.want_type=SCREEN_OPENGL;
		sdl.opengl.bilinear=true;
	} else if (output == "openglnb") {
		sdl.desktop.want_type=SCREEN_OPENGL;
		sdl.opengl.bilinear=false;

#endif
	} else {
		LOG_MSG("SDL : Unsupported output device %s, switching back to surface",output.c_str());
		sdl.desktop.want_type=SCREEN_SURFACE;//SHOULDN'T BE POSSIBLE anymore
	}
	sdl.texture.texture=0;
	sdl.texture.pixelFormat=0;
	sdl.window=0;
	sdl.renderer=0;
	sdl.rendererDriver 			= section->Get_string("texture.renderer");
	

#if C_OPENGL


	if(sdl.desktop.want_type==SCREEN_OPENGL) { /* OPENGL is requested */
		
		
		if (!GFX_SetSDLOpenGLWindow(sdl.desktop.window.width,sdl.desktop.window.height)) {
			LOG_MSG("SDL : Could not create OpenGL window, switching back to surface");
			sdl.desktop.want_type=SCREEN_SURFACE;
		} else {
			

			sdl.opengl.context = SDL_GL_CreateContext(sdl.window);
			if (sdl.opengl.context == 0) {
				LOG_MSG("SDL : Could not create OpenGL context, switching back to surface");
				sdl.desktop.want_type=SCREEN_SURFACE;
			} else {				
			}
		}

			if (sdl.desktop.fullscreen == true){			
				SDL_ShowWindow(sdl.window);	
			} else {
				SDL_SetWindowBordered(sdl.window,SDL_FALSE);
			}
			
		
		//SDL_SetWindowPosition(sdl.window,640, 0);			
		sdl.opengl.program_object=0;
		sdl.opengl.vao = 0;
		sdl.opengl.vertex_vbo = 0;
		sdl.opengl.ubo = 0;
		sdl.opengl.vertex_shader = 0;
		sdl.opengl.fragment_shader = 0;
		sdl.opengl.vertex_shader_src = vertex_shader_default_src;
		sdl.opengl.fragment_shader_src = fragment_shader_default_src;
		std::string shader_filename=section->Get_string("gl.shader");
		if (!shader_filename.empty()) {
		
			std::string config_path;
			config_path = section->Get_string("gl.shpath");
			std::string vertex_shader_path = config_path + CROSS_FILESPLIT + shader_filename + ".vert";
			std::ifstream vertex_fstream(vertex_shader_path.c_str());
			std::stringstream ss;
			if (vertex_fstream.is_open()) {
				ss << vertex_fstream.rdbuf();
				sdl.opengl.vertex_shader_src = ss.str();
			} else {
				LOG_MSG("SDL : Unable to open: %s", vertex_shader_path.c_str());
			}

			ss.str("");
			ss.clear();

			std::string fragment_shader_path = config_path + CROSS_FILESPLIT + shader_filename + ".frag";
			std::ifstream fragment_fstream(fragment_shader_path.c_str());
			if (fragment_fstream.is_open()) {
				ss << fragment_fstream.rdbuf();
				sdl.opengl.fragment_shader_src = ss.str();
			} else {
				LOG_MSG("SDL : Unable to open: %s", fragment_shader_path.c_str());
			}
		} 		
		sdl.opengl.texture=0;
		glGetIntegerv (GL_MAX_TEXTURE_SIZE, &sdl.opengl.max_texsize);
		glewExperimental = GL_TRUE;
		glewInit();		
	} /* OPENGL is requested end */
	
#endif	//OPENGL

	if (sdl.desktop.fullscreen == false){
		if (!sdl.desktop.want_type == SCREEN_SURFACE){	
				SDL_SetWindowBordered(sdl.window,SDL_FALSE);						
			}			
	}
	

#if C_OPENGL

	if(sdl.desktop.want_type==SCREEN_OPENGL) { /* OPENGL is requested */	
	/* Get some Event handlers */
			if ( sdl.desktop.borderless == false) {
				 SDL_SetWindowBordered(sdl.window,SDL_TRUE);
			}
			//SDL_RestoreWindow(sdl.window);
		

			
	} /* OPENGL is requested end */	
	
	
		#endif	//OPENGL
		
	int  dosbox_shutdown = 1;
	dosbox_shutdown = section->Get_int("Dosbox Shutdown Key");
	switch ( dosbox_shutdown ){
		case 0:
			//MAPPER_AddHandler(KillSwitch,MK_f9,MMOD1,"shutdown","ShutDown");
			//LOG_MSG("SDL : Shutdown DOSBox with - CTRL-F9 (Default)\n");
			//break;
		case 1:
			MAPPER_AddHandler(KillSwitch,MK_WHDA,MMOD1|MMOD2,"shutdown","ShutDown");
			LOG_MSG("SDL : Shutdown DOSBox with - CTRL + ALT + Keypad (*)\n");
			break;	
		case 2:
			MAPPER_AddHandler(KillSwitch,MK_WHDB,MMOD1|MMOD2,"shutdown","ShutDown");
			LOG_MSG("SDL : Shutdown DOSBox with - CTRL + ALT + Keypad (-)\n");		
			break;	
		case 3:
			MAPPER_AddHandler(KillSwitch,MK_WHDC,MMOD1|MMOD2,"shutdown","ShutDown");
			LOG_MSG("SDL : Shutdown DOSBox with - CTRL + ALT + Keypad (+)\n");	
			break;				
		case 4:
			MAPPER_AddHandler(KillSwitch,MK_WHDD,MMOD1|MMOD2,"shutdown","ShutDown");
			LOG_MSG("SDL : Shutdown DOSBox with - CTRL + ALT + Keypad (/)\n");				
			break;				
		default:
			MAPPER_AddHandler(KillSwitch,MK_WHDA,MMOD1|MMOD2,"shutdown","ShutDown");
			break;										
			
		
		
	}

	//MAPPER_AddHandler(CaptureMouse,MK_f10,MMOD1,"capmouse","Cap Mouse");
	MAPPER_AddHandler(SwitchFullScreen,MK_return,MMOD2,"fullscr","Fullscreen");
	MAPPER_AddHandler(Restart,MK_home,MMOD1|MMOD2,"restart","Restart");
	
	MAPPER_AddHandler(FloppyARequest,MK_f9,MMOD1|MMOD2,"FlopAOpen","Open-FloppyA");
	MAPPER_AddHandler(FloppyBRequest,MK_f10,MMOD1|MMOD2,"FlopBOpen","Open-FloppyB");	
	MAPPER_AddHandler(CDRomERequest,MK_f11,MMOD1|MMOD2,"CDEOpen","Open-CDRom-E");
	MAPPER_AddHandler(CDRomFRequest,MK_f12,MMOD1|MMOD2,"CDFOPEN","Open-CDRom-F");
	
#if defined(C_DEBUG)
	/* Pause binds with activate-debugger */
#else
	MAPPER_AddHandler(&PauseDOSBox, MK_pause, MMOD2, "pause", "Pause DBox");
#endif
	/* Get Keyboard state of numlock and capslock */
	SDL_Keymod keystate = SDL_GetModState();
	if(keystate&KMOD_NUM) startup_state_numlock = true;
	if(keystate&KMOD_CAPS) startup_state_capslock = true;
	

}

void Mouse_AutoLock(bool enable) {
	sdl.mouse.autolock=enable;
	if (sdl.mouse.autoenable) sdl.mouse.requestlock=enable;
	else {
		SDL_ShowCursor(enable?SDL_DISABLE:SDL_ENABLE);
		sdl.mouse.requestlock=false;
	}
}

static void HandleMouseMotion(SDL_MouseMotionEvent * motion) {
	if (sdl.mouse.locked || !sdl.mouse.autoenable)
		Mouse_CursorMoved((float)motion->xrel*sdl.mouse.xsensitivity/100.0f,
						  (float)motion->yrel*sdl.mouse.ysensitivity/100.0f,
						  (float)(motion->x-sdl.clip.x)/(sdl.clip.w-1)*sdl.mouse.xsensitivity/100.0f,
						  (float)(motion->y-sdl.clip.y)/(sdl.clip.h-1)*sdl.mouse.ysensitivity/100.0f,
						  sdl.mouse.locked);
}

static void HandleMouseButton(SDL_MouseButtonEvent * button) {
	switch (button->state) {
		case SDL_PRESSED:
			if (sdl.mouse.requestlock && !sdl.mouse.locked) {
				GFX_CaptureMouse();
				// Don't pass click to mouse handler
				break;
			}
			if (!sdl.mouse.autoenable && sdl.mouse.autolock && button-> button == SDL_BUTTON_MIDDLE) {
				GFX_CaptureMouse();
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
					if (sdl.mouse.middlemouselock == true){
						GFX_CaptureMouse();
					}
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

void GFX_LosingFocus(void) {
	sdl.laltstate=SDL_KEYUP;
	sdl.raltstate=SDL_KEYUP;
	MAPPER_LosingFocus();
	
	if (sdl.SystemKeysLocked == true)	
		disableKeys.Unblock();
}

void GFX_HandleVideoResize(int width, int height) {
	/* Maybe a screen rotation has just occurred, so we simply resize.
	There may be a different cause for a forced resized, though.    */
	if (sdl.desktop.full.display_res && sdl.desktop.fullscreen) {
		/* Note: We should not use GFX_ObtainDisplayDimensions
		(SDL_GetDisplayBounds) on Android after a screen rotation:
		The older values from application startup are returned. */
		sdl.desktop.full.width  = width;
		sdl.desktop.full.height = height;
	}
	/* Even if the new window's dimensions are actually the desired ones
	 * we may still need to re-obtain a new window surface or do
	 * a different thing. So we basically call GFX_SetSize, but without
	 * touching the window itself (or else we may end in an infinite loop).
	 *
	 * Furthermore, if the new dimensions are *not* the desired ones, we
	 * don't fight it. Rather than attempting to resize it back, we simply
	 * keep the window as-is and disable screen updates. This is done
	 * in SDL_SetSDLWindowSurface by setting sdl.update_display_contents
	 * to false.
	 */
	sdl.update_window = false;		
	
	GFX_ResetVoodoo();
	LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);			
	sdl.update_window = true;
	
}

bool GFX_IsFullscreen(void) {
	return sdl.desktop.fullscreen;
}

#if defined(MACOSX)
	#define DB_POLLSKIP 3
#else
	//Not used yet, see comment below
	#define DB_POLLSKIP 1
#endif

#if defined(LINUX)
#define SDL_XORG_FIX 1
#else
#define SDL_XORG_FIX 0
#endif


void GFX_Events() {
	//Don't poll too often. This can be heavy on the OS, especially Macs.
	//In idle mode 3000-4000 polls are done per second without this check.
	//Macs, with this code,  max 250 polls per second. (non-macs unused default max 500)
	//Currently not implemented for all platforms, given the ALT-TAB stuff for WIN32.
#if defined (MACOSX)
	static int last_check = 0;
	int current_check = GetTicks();
	if (current_check - last_check <=  DB_POLLSKIP) return;
	last_check = current_check;
#endif
		
	SDL_Event event;
#if defined (REDUCE_JOYSTICK_POLLING)
	static int poll_delay = 0;
	int time = GetTicks();
	if (time - poll_delay > 20) {
		poll_delay = time;
		if (sdl.num_joysticks > 0) SDL_JoystickUpdate();
		MAPPER_UpdateJoysticks();
	}
#endif
	

	while (SDL_PollEvent(&event)) {

#if SDL_XORG_FIX
		// Special code for broken SDL with Xorg 1.20.1, where pairs of inputfocus gain and loss events are generated
		// when locking the mouse in windowed mode.
		// This also seems to give wrong key up events in fullscreen mode.
		// sdl-1.2 has a fix in hg, but this shouldn't interfere with that fix.
		if (event.type == SDL_ACTIVEEVENT && event.active.state == SDL_APPINPUTFOCUS && event.active.gain == 0) {
			SDL_Event test; //Check if the next event would undo this one.
			if (SDL_PeepEvents(&test, 1, SDL_PEEKEVENT, SDL_ACTIVEEVENTMASK) == 1 && test.active.state == SDL_APPINPUTFOCUS && test.active.gain == 1) {
				// Skip both events.
				SDL_PeepEvents(&test, 1, SDL_GETEVENT, SDL_ACTIVEEVENTMASK);
				// Look for KEY UP events and check their validity.
				while (SDL_PeepEvents(&test, 1, SDL_PEEKEVENT, SDL_KEYUPMASK) == 1) {
					const Uint8 * kstate = SDL_GetKeyState(NULL);
					if (kstate[test.key.keysym.sym] != SDL_PRESSED) break;
					SDL_PeepEvents(&test, 1, SDL_GETEVENT, SDL_KEYUPMASK);						
				}
				continue;
			}
		}
#endif
		
		
		//blocker.unblock();
		if (event.type == SDL_QUIT) {			
			throw(0);
			break;	
		}			
		switch (event.type) {		
			case SDL_WINDOWEVENT:
			{					
				switch (event.window.event) {
					
					case SDL_WINDOWEVENT_MOVED:
						{							
							LOG_MSG("SDL : Use now Display Index %d",SDL_GetWindowDisplayIndex(sdl.window));
							sdl.displayNumber = SDL_GetWindowDisplayIndex(sdl.window);
							nCurrentDisplay   = sdl.displayNumber;		
							GFX_ResetVoodooDisplayMove();						
							continue;
						}
					case SDL_WINDOWEVENT_RESTORED:
					{
						/* We may need to re-create a texture
						 * and more on Android. Another case:
						 * Update surface while using X11.
						 */		
				 
						GFX_ResetVoodoo();
						LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);
						
						continue;
					}	
					case SDL_WINDOWEVENT_RESIZED:
					{						
						GFX_HandleVideoResize(event.window.data1, event.window.data2);
						continue;
					}	
					case SDL_WINDOWEVENT_EXPOSED:
					{						
						if (sdl.draw.callback) sdl.draw.callback( GFX_CallBackRedraw );											
						continue;
					}	
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					{
						if (sdl.desktop.fullscreen && !sdl.mouse.locked){
							GFX_CaptureMouse();
						}
						
						SetPriority(sdl.priority.focus);
						CPU_Disable_SkipAutoAdjust();	
						break;
						
						
					}	
					case SDL_WINDOWEVENT_FOCUS_LOST:
					{							
						if (sdl.mouse.locked) {
							if (sdl.desktop.fullscreen) {							
								VGA_KillDrawing();
								GFX_ForceFullscreenExit();
							}
							//GFX_CaptureMouse();
						}
						SetPriority(sdl.priority.nofocus);
						GFX_LosingFocus();
						CPU_Enable_SkipAutoAdjust();
						break;
					}
					default:
					{}
				}

				/* Non-focus priority is set to pause; check to see if we've lost window or input focus
				 * i.e. has the window been minimised or made inactive?
				 */
				
					if (sdl.priority.nofocus == PRIORITY_LEVEL_PAUSE) {
									
					if ((event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) ||
						(event.window.event == SDL_WINDOWEVENT_MINIMIZED)) {
							
							/* Window has lost focus, pause the emulator.
							 * This is similar to what PauseDOSBox() does, but the exit criteria is different.
							 * Instead of waiting for the user to hit Alt-Break, we wait for the window to
							 * regain window or input focus.
							 */
								bool paused = true;							 
								SDL_Event ev;

								GFX_SetTitle(-1,-1,true);
								KEYBOARD_ClrBuffer();														
								SDL_Delay(12);
								while (SDL_PollEvent(&ev)) {
									// flush event queue.
								}																					

							while (paused) {
								// WaitEvent waits for an event rather than polling, so CPU usage drops to zero
								SDL_WaitEvent(&ev);

								switch (ev.type)
								{
									case SDL_QUIT:
									{
										throw(0);
										break;			 		// a bit redundant at linux at least as the active events gets before the quit event.
									}
									
									case SDL_WINDOWEVENT:     	// wait until we get window focus back
									{
										if ((ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST)  ||
											(ev.window.event == SDL_WINDOWEVENT_MINIMIZED)   ||
											(ev.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)||
											(ev.window.event == SDL_WINDOWEVENT_RESTORED)    ||
											(ev.window.event == SDL_WINDOWEVENT_EXPOSED)){
												
												// We've got focus back, so unpause and break out of the loop
												if ((ev.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) ||
													(ev.window.event == SDL_WINDOWEVENT_RESTORED)     ||
													(ev.window.event == SDL_WINDOWEVENT_EXPOSED)){
													paused = false;
													GFX_SetTitle(-1,-1,false);
													SetPriority(sdl.priority.focus);
													CPU_Disable_SkipAutoAdjust();													
													//blocker.block();													
												}

												/* Now poke a "release ALT" command into the keyboard buffer
												 * we have to do this, otherwise ALT will 'stick' and cause
												 * problems with the app running in the DOSBox.
												 */
												KEYBOARD_AddKey(KBD_leftalt, false);
												KEYBOARD_AddKey(KBD_rightalt, false);		
												if (ev.window.event == SDL_WINDOWEVENT_RESTORED) {
													
													// We may need to re-create a texture and more
													GFX_ResetVoodoo();
													LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);														LOG(LOG_MISC,LOG_WARN)("SDL : Update Screen \n(File %s:, Line: %d)\n\n",__FILE__,__LINE__);												
												}
										}
										break;
									}
								}
							}
						}
					}						
				break;
			
			}
			
/* 									KEYBOARD_AddKey(KBD_lwindows,true);
						KEYBOARD_AddKey(KBD_lwindows,false);break;
						KEYBOARD_AddKey(KBD_rwindows,true);
						KEYBOARD_AddKey(KBD_rwindows,false); break;	 */
			case SDL_MOUSEMOTION:
				HandleMouseMotion(&event.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				HandleMouseButton(&event.button);
				break;
			case SDL_KEYDOWN:;				
			case SDL_KEYUP:
						
				// ignore event alt+tab
				if (event.key.keysym.sym==SDLK_LALT){
					sdl.laltstate = (SDL_EventType)event.key.type;
				}
				if (event.key.keysym.sym==SDLK_RALT){
					sdl.raltstate = (SDL_EventType)event.key.type;
				}				
				if (((event.key.keysym.sym==SDLK_TAB)) && ((sdl.laltstate==SDL_KEYDOWN) || (sdl.raltstate==SDL_KEYDOWN))){	
					break;
				}
				// This can happen as well.
				if (((event.key.keysym.sym == SDLK_TAB )) && (event.key.keysym.mod & KMOD_ALT)){
					break;
				}
				// ignore tab events that arrive just after regaining focus. (likely the result of alt-tab)
				if ((event.key.keysym.sym == SDLK_TAB) && (GetTicks() - sdl.focus_ticks < 2)){
					break;
				}
				if (event.key.keysym.sym == SDLK_F4 && event.key.keysym.mod == SDLK_LALT){						
					KillSwitch(true);
					break;	
				}

/* 				if (event.key.keysym.sym==102){
					LOG_MSG("TEST %d",event.key.keysym.sym);
					sdl.lwindows = (SDL_EventType)event.key.type;					
				}

				if ( (event.key.keysym.sym==102) && (sdl.lwindows==SDL_KEYDOWN) ){	
					break;
				} */
			
			default:
				void MAPPER_CheckEvent(SDL_Event * event);
				MAPPER_CheckEvent(&event);
			}
	}	
}


// Menu System -- COMMANDS ----------------------------------------------------------------------------------
#if defined(WIN32)

// Menu System -- COMMANDS ----------------------------------------------------------------------------------
#endif



#if defined (WIN32)
static BOOL WINAPI ConsoleEventHandler(DWORD event) {
	switch (event) {
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
		raise(SIGTERM);
		return TRUE;
	case CTRL_C_EVENT:
	default: //pass to the next handler
		return FALSE;
	}
}
#endif


/* static variable to show wether there is not a valid stdout.
 * Fixes some bugs when -noconsole is used in a read only directory */
static bool no_stdout = false;
void GFX_ShowMsg(char const* format,...) {
	char buf[512];
	va_list msg;
	va_start(msg,format);
	vsnprintf(buf,sizeof(buf),format,msg);
	va_end(msg);

	buf[sizeof(buf) - 1] = '\0';
	if (!no_stdout) puts(buf); //Else buf is parsed again. (puts adds end of line)
}


void Config_Add_SDL() {
	Section_prop * sdl_sec=control->AddSection_prop("sdl",&GUI_StartUp);
	sdl_sec->AddInitFunction(&MAPPER_StartUp);
	Prop_bool* Pbool;
	Prop_string* Pstring;
	Prop_int* Pint;
	Prop_float* Pfloat;
	Prop_multival* Pmulti;

	Pbool = sdl_sec->Add_bool("fullscreen",Property::Changeable::Always,false);
	Pbool->Set_help(  "================================================================================================\n"
                      "Start dosbox directly in fullscreen. (Press ALT-Enter to go back)");	
					  
	const char* ScrOutflag[] = {"standard","exklusiv",0 };					  		
	Pstring = sdl_sec->Add_string("fullscreenmode",Property::Changeable::Always,"exklusiv");	
	Pstring->Set_help(  "================================================================================================\n"
                      "Which Mode do you using with ALT-Enter for FullScreen.\n"
					  "Standard: Normal Fullscreen\n"
					  "Exklusiv: Desktop Fullscreen");
	
	Pint = sdl_sec->Add_int("display",Property::Changeable::Always,0);
	Pint->SetMinMax(0,10);
	Pint->Set_help(   "================================================================================================\n"
	                  "For Multi Monitor Systems. Set The Display Number.");
					  
	Pint = sdl_sec->Add_int("vsync",Property::Changeable::Always,0);
	Pint->SetMinMax(-1, 1);
	Pint->Set_help(  "================================================================================================\n"
	                  "Sync to Vblank IF supported by the output device and renderer (if relevant). It can reduce\n"
	                  "screen flickering, but it can also result in a slow DOSBox.");

	Pstring = sdl_sec->Add_string("fullresolution",Property::Changeable::Always,"desktop");
	Pstring->Set_help("================================================================================================\n"
	                  "What resolution to use for fullscreen: original, desktop or a fixed size (e.g. 1024x768).\n"
	                  "Using your monitor's native resolution with aspect=true might give the best results. If you end\n"
			          "up with small window on a large screen, try an output different from surface.");

	Pstring = sdl_sec->Add_string("windowresolution",Property::Changeable::Always,"1280x960");	
	Pstring->Set_help("================================================================================================\n"
	                  "Scale the window to this size IF the output device supports hardware scaling.\n"
	                  "(output=surface does not!)");

	Pbool = sdl_sec->Add_bool("windowborderless",Property::Changeable::Always, false);
	Pbool->Set_help(  "================================================================================================\n"
	                  "Remove the border in Window mode");

					  
	const char* outputs[] = {"surface", "texture", "texturenb", "texturebest", "opengl", "openglnb", 0 };
	
	Pstring = sdl_sec->Add_string("output",Property::Changeable::Always,"opengl");
	Pstring->Set_help("================================================================================================\n"
	                  "What video system to use for output.\n"
					  "Info DOSBox SDL1  -  DOSBox SDL2\n"
					  "     Surface  is now Surface\n"
					  "     OpenGL   is now Texture\n"
					  "     OpenGLNB is now TextureNB\n"					  
					  "NB means No Bilinear Filtering. Dont't use OpenGL/NB with Voodoo. OpenGL and OpenGLNB can now\n"
					  "take advantage of external shaders.");
	Pstring->Set_values(outputs);

	Pstring = sdl_sec->Add_string("gl.shpath",Property::Changeable::Always,".\\DATA\\SHADERS");	
	Pstring->Set_help("================================================================================================\n"
	                  "Set Shader Path");	
	Pstring = sdl_sec->Add_string("gl.shader",Property::Changeable::Always,"crt-lottes_mod");	
	Pstring->Set_help("================================================================================================\n"
	                  "What set of GLSL shaders to use with an OpenGL output. Keep empty if this is not desired. Note\n"
	                  "that in case it is used, the respective shader files must be found in the \"shaders\" subdirectory\n"
	                  "relatively to where the default DOSBox configuration file is stored or change the path in \n"
	                  "\"gl.shpath\" For shader file naming convention, suppose that you have a pair of shader files ready:\n"
	                  "mysample.vert and mysample.frag. Then shader=mysample should be set.");


	const char* renderers[] = { "auto", "direct3d",	"direct3d11", "opengl", "opengles", "opengles2", "software", "vulkan",0 };
	
	Pstring = sdl_sec->Add_string("texture.renderer",Property::Changeable::Always,"opengl");		
	Pstring->Set_help("================================================================================================\n"
	                  "Choose a renderer driver if output=texture or output=texturenb. Use output=auto for an automatic\n"
	                  "choice.");
	Pstring->Set_values(renderers);

	Pbool = sdl_sec->Add_bool("UseAspectHeight",Property::Changeable::Always,true);	
	Pbool->Set_help(  "================================================================================================\n"
	                  "This will set the Screen height in combination with Aspect Ratio. Example: if you set 1280x1024,\n"
					  "this will shrink do the Aspect Scale 1280x960. If this set to false, Aspect Ratio has no Effect\n"
					  "to the Screen. I use most 1280x1024 and i dont like the Screen shrink");	
	
	Pbool = sdl_sec->Add_bool("autolock",Property::Changeable::Always,true);
	Pbool->Set_help(  "================================================================================================\n"
	                  "Mouse will automatically lock, if you click on the screen. (Press CTRL-F10 to unlock)");

	Pbool = sdl_sec->Add_bool("mmbutton",Property::Changeable::Always,true);
	Pbool->Set_help(  "================================================================================================\n"
	                  "If true, Release/Lock with Middle Mouse Button if you click on the screen.");
					  
	Pmulti = sdl_sec->Add_multi("sensitivity",Property::Changeable::Always, ",");
	Pmulti->SetValue("100");
	Pmulti->Set_help(   "================================================================================================\n"
	                  "Mouse sensitivity. The optional second parameter specifies vertical sensitivity (e.g. 100,-50).");

	Pfloat = sdl_sec->Add_float("mousedelay", Property::Changeable::OnlyAtStart,5.0);
	Pfloat->SetMinMax(0.001,1000.0);
	Pfloat->Set_help(   "================================================================================================\n"
	                  "Mouse Delay. Set Internal Pic Delay Parameter. in Emulated Windows 9x it need to configure 30.0 or\n"
					  "or higher. If you hear Crackling Sounds at mousemoves set this value higher. Default 5.0");
					  
	Pbool = sdl_sec->Add_bool("DisableSystemKeys",Property::Changeable::Always,false);
	Pbool->Set_help(   "================================================================================================\n"
					   "Lock System Keys in Windows 95/98 Mode (ALT-Tab, ALT-Esc, CTRL-Esc, Win-Keys) if Mouse is Captured.");

	Pint = Pmulti->GetSection()->Add_int("xsens",Property::Changeable::Always,100);
	Pint->SetMinMax(-1000,1000);
	Pint = Pmulti->GetSection()->Add_int("ysens",Property::Changeable::Always,100);
	Pint->SetMinMax(-1000,1000);
					  
	Pbool = sdl_sec->Add_bool("waitonerror",Property::Changeable::Always, true);
	Pbool->Set_help(  "================================================================================================\n"
	                  "Wait before closing the console if dosbox has an error.");

	Pmulti = sdl_sec->Add_multi("priority", Property::Changeable::Always, ",");
	Pmulti->SetValue( "higher,normal");		
	Pmulti->Set_help( "================================================================================================\n"
	                  "Priority levels for dosbox. Second entry behind the comma is for when dosbox is not focused/\n"
	                  "minimized. pause is only valid for the second entry.");

	const char* actt[] = { "lowest", "lower", "normal", "higher", "highest", "realtime", "pause", 0};
	Pstring = Pmulti->GetSection()->Add_string("active",Property::Changeable::Always,"higher");
	Pstring->Set_values(actt);

	const char* inactt[] = { "lowest", "lower", "normal", "higher", "highest","realtime", "pause", 0};
	Pstring = Pmulti->GetSection()->Add_string("inactive",Property::Changeable::Always,"normal");
	Pstring->Set_values(inactt);

	Pstring = sdl_sec->Add_path("mapperfile",Property::Changeable::Always,"\\" MAPPERFILE);
	Pstring->Set_help( "================================================================================================\n"
	                   "File used to load/save the key/event mappings from. Resetmapper only works with the default value.");		
		
	Pint = sdl_sec->Add_int("Dosbox Shutdown Key",Property::Changeable::Always,0);
	Pint->SetMinMax(0,4);	
	Pint->Set_help(    "================================================================================================\n"
	                   "Change the DOSBox Shutdown Key:\n"
				       "0: CTRL-F9  (default)\n"
				       "1: CTRL+ALT+(Keypad *)\n"
				       "2: CTRL+ALT+(Keypad -)\n"
				       "3: CTRL+ALT+(Keypad +)\n"
				       "4: CTRL+ALT+(Keypad /)\n"
					   "If you saved the mapper file and you change then Quit Key in the Config. The Old Values are in\n"
					   "mapper file.");	

	Pbool = sdl_sec->Add_bool("VoodooUseOwnWindowRes",Property::Changeable::Always,false);
	Pbool->Set_help(  "================================================================================================\n"
	                  "Voodoo use the same Window Resolution how Dosbox 'windowresolution'. If set to true you can change\n"
					   "in the section 'pci' a other Resolution");
					   
	Pbool = sdl_sec->Add_bool("VoodooUseOwnFullScRes",Property::Changeable::Always,false);
	Pbool->Set_help(  "================================================================================================\n"
	                  "Voodoo use the same Full Resolution how Dosbox 'fullresolution'. If set to true you can change\n"
					   "in the section 'pci' a other Resolution");
					   
	Pbool = sdl_sec->Add_bool("VoodooDesktopFullScrn",Property::Changeable::Always,true);
	Pbool->Set_help(  "================================================================================================\n"
	                  "Voodoo Fullscreen Only. If true  - The 3DFX Voodoo Screen use the Fullscreen Desktop Mode.\n"
					  "                        if false - The 3DFX Voodoo Screen use the normal Fullscreen.");					   
					   
	Pint = sdl_sec->Add_int("WindowsTaskbarAdjust",Property::Changeable::Always,-5);	
	Pint->SetMinMax(-256,256);	
	Pint->Set_help(    "================================================================================================\n"
	                   "Window Only: Fine Adjust the height of the centered window. Use this if your Centered Window\n"
					   "under and behind the Windows Taskbar is.");					   

	Pbool = sdl_sec->Add_bool("DisableIntroStartup",Property::Changeable::Always,false);
	Pbool->Set_help(  "================================================================================================\n"
	                  "This disabled the Intro Startup");					   
}
/*--------------------------------------------------------------------------------------------------------------------------*/
static void show_warning(char const * const message) {
		bool textonly = true;
	#ifdef WIN32
		textonly = false;
		if ( !sdl.inited && SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) textonly = true;
		sdl.inited = true;
	#endif
		printf("%s",message);
		if(textonly) return;
		if (!sdl.window)
			if (!GFX_SetSDLSurfaceWindow(640,400)) return;
		sdl.surface = SDL_GetWindowSurface(sdl.window);
		if(!sdl.surface) return;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		Bit32u rmask = 0xff000000;
		Bit32u gmask = 0x00ff0000;
		Bit32u bmask = 0x0000ff00;
	#else
		Bit32u rmask = 0x000000ff;
		Bit32u gmask = 0x0000ff00;
		Bit32u bmask = 0x00ff0000;
	#endif
		SDL_Surface* splash_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 400, 32, rmask, gmask, bmask, 0);
		if (!splash_surf) return;

		int x = 120,y = 20;
		std::string m(message),m2;
		std::string::size_type a,b,c,d;

		while(m.size()) { //Max 50 characters. break on space before or on a newline
			c = m.find('\n');
			d = m.rfind(' ',50);
			if(c>d) a=b=d; else a=b=c;
			if( a != std::string::npos) b++;
			m2 = m.substr(0,a); m.erase(0,b);
			OutputString(x,y,m2.c_str(),0xffffffff,0,splash_surf);
			y += 20;
		}

		SDL_BlitSurface(splash_surf, NULL, sdl.surface, NULL);
		SDL_UpdateWindowSurface(sdl.window);
		SDL_Delay(12000);
}
/*--------------------------------------------------------------------------------------------------------------------------*/
static void launcheditor() {
		std::string path,file;
		Cross::CreatePlatformConfigDir(path);
		Cross::GetPlatformConfigName(file);
		
		path += file;
		FILE* f = fopen(path.c_str(),"r");
		if(!f && !control->PrintConfig(path.c_str())) {
			printf("tried creating %s. but failed.\n",path.c_str());
			exit(1);
		}
		if(f) fclose(f);
	/*	if(edit.empty()) {
			printf("no editor specified.\n");
			exit(1);
		}*/
		std::string edit;
		while(control->cmdline->FindString("-editconf",edit,true)) //Loop until one succeeds
			execlp(edit.c_str(),edit.c_str(),path.c_str(),(char*) 0);
		//if you get here the launching failed!
		printf("can't find editor(s) specified at the command line.\n");
		exit(1);
}


/*--------------------------------------------------------------------------------------------------------------------------*/
#if defined(C_DEBUG)
	extern void DEBUG_ShutDown(Section * /*sec*/);
#endif
/*--------------------------------------------------------------------------------------------------------------------------*/

void restart_program(std::vector<std::string> & parameters) {
		char** newargs = new char* [parameters.size() + 1];
		// parameter 0 is the executable path
		// contents of the vector follow
		// last one is NULL
		for(Bitu i = 0; i < parameters.size(); i++) newargs[i] = (char*)parameters[i].c_str();
		newargs[parameters.size()] = NULL;
		SDL_CloseAudio();
		SDL_Delay(50);
		SDL_Quit_Wrapper();
	#if defined(C_DEBUG)
		// shutdown curses
		DEBUG_ShutDown(NULL);
	#endif

		if(execvp(newargs[0], newargs) == -1) {
	#ifdef WIN32
			if(newargs[0][0] == '\"') {
				//everything specifies quotes around it if it contains a space, however my system disagrees
				std::string edit = parameters[0];
				edit.erase(0,1);edit.erase(edit.length() - 1,1);
				//However keep the first argument of the passed argv (newargs) with quotes, as else repeated restarts go wrong.
				if(execvp(edit.c_str(), newargs) == -1) E_Exit("Restarting failed");
			}
	#endif
			E_Exit("Restarting failed");
		}
		delete [] newargs;
}
/*--------------------------------------------------------------------------------------------------------------------------*/
void Restart(bool pressed) { // mapper handler
	restart_program(control->startup_params);
}
/*--------------------------------------------------------------------------------------------------------------------------*/

static void launchcaptures(std::string const& edit) {
	

		std::string path,file;
		
		//char current[MAX_PATH+1];
		//GetCurrentDirectory(sizeof(current), current);
		
		Section* t = control->GetSection("dosbox");
		if(t) file = t->GetPropValue("captures");
		if(!t || file == NO_SUCH_PROPERTY) {
			printf("Config system messed up.\n");
			exit(1);
		}
		//Cross::CreatePlatformConfigDir(path);
		//path = strncat(current, "\\",strlen(current+1)); 
		path += file;
		Cross::CreateDir(path);
		struct stat cstat;
		if(stat(path.c_str(),&cstat) || (cstat.st_mode & S_IFDIR) == 0) {
			printf("%s doesn't exists or isn't a directory.\n",path.c_str());
			exit(1);
		}
	/*	if(edit.empty()) {
			printf("no editor specified.\n");
			exit(1);
		}*/

		execlp(edit.c_str(),edit.c_str(),path.c_str(),(char*) 0);
		//if you get here the launching failed!
		printf("can't find filemanager %s\n",edit.c_str());
		exit(1);
}
/*--------------------------------------------------------------------------------------------------------------------------*/

	static void printconfiglocation() {
		std::string path,file;
		Cross::CreatePlatformConfigDir(path);
		Cross::GetPlatformConfigName(file);
		path += file;

		FILE* f = fopen(path.c_str(),"r");
		if(!f && !control->PrintConfig(path.c_str())) {
			printf("tried creating %s. but failed",path.c_str());
			exit(1);
		}
		if(f) fclose(f);
		printf("%s\n",path.c_str());
		exit(0);
}
/*--------------------------------------------------------------------------------------------------------------------------*/

static void eraseconfigfile() {
		FILE* f = fopen("dosbox.conf","r");
		if(f) {
			fclose(f);
			show_warning("Warning: dosbox.conf exists in current working directory.\nThis will override the configuration file at runtime.\n");
		}
		std::string path,file;
		Cross::GetPlatformConfigDir(path);
		Cross::GetPlatformConfigName(file);
		path += file;
		f = fopen(path.c_str(),"r");
		if(!f) exit(0);
		fclose(f);
		unlink(path.c_str());
		exit(0);
}
/*--------------------------------------------------------------------------------------------------------------------------*/

static void erasemapperfile() {
		FILE* g = fopen("dosbox.conf","r");
		if(g) {
			fclose(g);
			show_warning("Warning: dosbox.conf exists in current working directory.\nKeymapping might not be properly reset.\n"
						 "Please reset configuration as well and delete the dosbox.conf.\n");
		}

		std::string path,file=MAPPERFILE;
		Cross::GetPlatformConfigDir(path);
		path += file;
		FILE* f = fopen(path.c_str(),"r");
		if(!f) exit(0);
		fclose(f);
		unlink(path.c_str());
		exit(0);
}
/*--------------------------------------------------------------------------------------------------------------------------*/

std::string current_working_path()
{
	char current[MAX_PATH+1];
	GetCurrentDirectory(sizeof(current), current);
	return current;
}
/*--------------------------------------------------------------------------------------------------------------------------*/


/* 
	Have to remember where i ripped this code sometime ago.

*/
	#ifndef min
		#define min(a,b) ((a)<(b)?(a):(b))
	#endif
	#ifndef max
		#define max(a,b) ((a)>(b)?(a):(b))
	#endif
/*--------------------------------------------------------------------------------------------------------------------------*/

bool dirExists(const std::string& dirName_in)
{
	  DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	  if (ftyp == INVALID_FILE_ATTRIBUTES);
		return false;  //something is wrong with your path!

	  if (ftyp & FILE_ATTRIBUTE_DIRECTORY);
		return true;   // this is a directory!

	  return false;    // this is not a directory!
}
/*--------------------------------------------------------------------------------------------------------------------------*/

static void OpenConsole( HANDLE hConsole, SHORT xSize, SHORT ySize, SHORT yTop ) {   
		CONSOLE_SCREEN_BUFFER_INFO csbi; // Hold Current Console Buffer Info 
		bool bSuccess;   
		SMALL_RECT srWindowRect;         // Hold the New Console Size 
		COORD coordScreen;    
		DWORD fdwMode;
		
		bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
		
		// Get the Largest Size we can size the Console Window to 
		coordScreen = GetLargestConsoleWindowSize( hConsole );
		
		// Define the New Console Window Size and Scroll Position 
		srWindowRect.Right  = (SHORT)(min(xSize, coordScreen.X) - 1);
		srWindowRect.Bottom = (SHORT)(min(ySize, coordScreen.Y) - 1);
		srWindowRect.Left   = srWindowRect.Top = (SHORT)0;
			
		// Define the New Console Buffer Size    
		coordScreen.X = xSize;
		
		// If the Current Buffer *is* the Size we want, Don't do anything!	
		
		if (control->cmdline->FindExist("-version") || control->cmdline->FindExist("--version") ) {
			coordScreen.Y = ySize;	
		}else {
			coordScreen.Y = SHRT_MAX - (ySize - 1);		
		}	

		

		// If the Current Buffer is Larger than what we want, Resize the 
		// Console Window First, then the Buffer 
		if( (DWORD)csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
		{
			bSuccess = SetConsoleWindowInfo( hConsole, TRUE, &srWindowRect );
			bSuccess = SetConsoleScreenBufferSize( hConsole, coordScreen );

		}
		
		// If the Current Buffer is Smaller than what we want, Resize the 
		// Buffer First, then the Console Window 
		if( (DWORD)csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize )
		{
			bSuccess = SetConsoleScreenBufferSize( hConsole, coordScreen );
			bSuccess = SetConsoleWindowInfo( hConsole, TRUE, &srWindowRect );
		}
		

		// If the Current Buffer *is* the Size we want, Don't do anything!	
		if (control->cmdline->FindExist("-version") || control->cmdline->FindExist("--version") ) {
			
		}else {
			
		}
					fclose(stdin);
					fclose(stdout);
					fclose(stderr);
					freopen("CONIN$","r",stdin);
					freopen("CONOUT$","w",stdout);
					freopen("CONOUT$","w",stderr);

		SetConsoleTitle("DOSBox: Status Window");		
		
		fdwMode = ENABLE_WINDOW_INPUT    |
				  ENABLE_MOUSE_INPUT     |
				  ENABLE_QUICK_EDIT_MODE |
				  ENABLE_ECHO_INPUT      |
				  ENABLE_EXTENDED_FLAGS; 
		SetConsoleMode(hConsole, fdwMode);	
		return;
}
/*--------------------------------------------------------------------------------------------------------------------------*/

void Disable_OS_Scaling() {
#if defined (WIN32)
	typedef BOOL (*function_set_dpi_pointer)();
	function_set_dpi_pointer function_set_dpi;
	function_set_dpi = (function_set_dpi_pointer) GetProcAddress(LoadLibrary("user32.dll"), "SetProcessDPIAware");
	if (function_set_dpi) {
		function_set_dpi();
	}
#endif
}

//extern void UI_Init(void);
int main(int argc, char* argv[]) {
	try {
		Disable_OS_Scaling(); //Do this early on, maybe override it through some parameter.
		
		CommandLine com_line(argc,argv);
		Config myconf(&com_line);
		control=&myconf;
		/* Init the configuration system and add default values */
		Config_Add_SDL();
		DOSBOX_Init();
		
#if defined(WIN32) && !defined(C_DEBUG)
		/* Can't disable the console with debugger enabled */
		if (control->cmdline->FindExist("-console")) {
			if (AllocConsole())
			{	
			
				OpenConsole(GetStdHandle(STD_OUTPUT_HANDLE),70,50,0);
			}

		} else {
			FreeConsole();
												
			std::string config_path;
			config_path = current_working_path() + "\\DATA\\";	
			
			if (dirExists(config_path)){
				/* Redirect standard input and standard output */
				if(freopen(STDOUT_FILE, "w", stdout) == NULL){
					// No stdout so don't write messages
					no_stdout = true; 
				}			
				freopen(STDERR_FILE, "w", stderr);
			}else{
				
				/* Redirect standard input and standard output */
				if(freopen(STDOUTDATA_FILE, "w", stdout) == NULL){
					// No stdout so don't write messages
					no_stdout = true; 
				}			
				freopen(STDERRDATA_FILE, "w", stderr);				
			}
			setvbuf(stdout, NULL, _IOLBF, BUFSIZ);	/* Line buffered */
			setbuf(stderr, NULL);		    		/* No buffering */
		}
#endif
	
		std::string editor;
		if (control->cmdline->FindString("-editconf", editor, false)) launcheditor();
		if (control->cmdline->FindString("-opencaptures", editor, true)) launchcaptures(editor);

		if (control->cmdline->FindExist("-eraseconf")) eraseconfigfile();
		if (control->cmdline->FindExist("-resetconf")) eraseconfigfile();
		if (control->cmdline->FindExist("-erasemapper")) erasemapperfile();
		if (control->cmdline->FindExist("-resetmapper")) erasemapperfile();
		if (control->cmdline->FindExist("-printconf")) printconfiglocation();

		if (control->cmdline->FindExist("-version") || control->cmdline->FindExist("--version")) {
			if (AllocConsole())
			{
				OpenConsole(GetStdHandle(STD_OUTPUT_HANDLE), 80, 40, 0);
				SetConsoleTitle("DOSBox: Status Window - Version");
				printf("\n");
				printf("%s\n\n", gDosboxFullVersion);
				printf("Features Compiled: %s\n\n", gDOSBoxFeatures);
				printf("\n%s\n\n", gDosboxCopyright);
				printf("%s\n", gDosboxTeamText);
				/*Can take SDL_Delay. Don't Close Immedeatly the Window, Wait for User too*/
				SDL_Delay(10000000);
			}
			exit(0);
		}

		if (control->cmdline->FindExist("-help") || control->cmdline->FindExist("--help")) {
			if (AllocConsole())
			{
				OpenConsole(GetStdHandle(STD_OUTPUT_HANDLE), 105, 48, 0);
				SetConsoleTitle("DOSBox: Help Window");
				printf("\n");
				printf("%s\n\n", gDosboxFullVersion);
				printf("  -help, --help                     Show this screen\n\n");
				printf("  -console                          Show logging console\n");
				printf("  -version, --version               Display version information\n");
				printf("  -fullscreen                       Start in fullscreen mode\n\n");
				printf("  -conf <configfile>                Start the specific (multiple) configuration file(s).\n");
				printf("                                    eq: \"dosbox.exe -conf config1.conf -conf config2.conf\"\n\n");
				printf("  -editconf <editor>                Edit the config file with the specific editor\n");
				printf("  -printconf                        Print config file location\n");
				printf("  -eraseconf (or -resetconf)        Erase loaded config file (or user config file and exit)\n");
				printf("  -erasemapper (or -resetmapper)    Erase loaded mapper file (or user mapper file and exit)\n");
				printf("  -opencaptures <param>             Launch captures\n");
				printf("  -startmapper                      Start the Key/Joystick mapper editor\n");
				printf("  -socket <socketnum>               Specify the socket number for null-modem emulation\n");
				printf("  -lang <message file>              Use specific message file instead of language= setting\n");
				printf("  -securemode                       Enable secure mode (no drive mounting etc)\n\n");
				printf("  -c <command string>               Execute this command in addition to AUTOEXEC.BAT.\n");
				printf("                                    eq: dosbox.exe -c \"mount c .\\Game\\\" -c \"c:\" -c \"bond.exe\"\n\n");
				printf("  -exit                             Exit after executing AUTOEXEC.BAT\n");
				printf("  -machine <machinetype>            Use Specific machine type. choices are:\n");
				printf("                                    hercules\n");
				printf("                                    amstrad\n");
				printf("                                    tandy\n");
				printf("                                    pcjr\n");
				printf("                                    cga,        cga_mono,    ega,          vga,         vgaonly\n");
				printf("                                    svga,       svga_s3,     svga_et3000,  svga_et4000, svga_paradise\n");
				printf("                                    vesa_nolfb, vesa_oldvbe, vesa_no24bpp, vesa_nolfb_no24bpp\n\n");
				printf("  -scaler <type>                    Use Specific scaler render. choices are:\n");
				printf("                                    none       , supereagle\n");
				printf("                                    normal2x   , normal3x\n");
				printf("                                    advmame2x  , advmame3x\n");
				printf("                                    advinterp2x, advinterp3x\n");
				printf("                                    hq2x       , hq3x\n");
				printf("                                    2xsai      , super2xsai\n");
				printf("                                    tv2x       , tv3x,\n");
				printf("                                    rgb2x      , rgb3x\n");
				printf("                                    scan2x     , scan3x.\n");
				printf("  -forcescaler                      force usage of the specified scaler even if it might not fit.");
				/*Can take SDL_Delay. Don't Close Immedeatly the Window, Wait for User too*/
				SDL_Delay(10000000);
			}
			exit(0);
		}

#if defined(C_DEBUG)
		DEBUG_SetupConsole();
#endif

#if defined(WIN32)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleEventHandler,TRUE);
#endif

#ifdef OS2
        PPIB pib;
        PTIB tib;
        DosGetInfoBlocks(&tib, &pib);
        if (pib->pib_ultype == 2) pib->pib_ultype = 3;
        setbuf(stdout, NULL);
        setbuf(stderr, NULL);
#endif

	/* Display Welcometext in the console */
	LOG_MSG("%s\n",gDosboxFullVersion);
	LOG_MSG("Copyright 2002-%s DOSBox Team, published under GNU GPL.",gDosboxYear);
	LOG_MSG("\n\n");

#if defined(WIN32)
	SDL_setenv("SDL_AUDIODRIVER", "directsound", 1);
#endif
	
	/* Init SDL */
	if ( SDL_Init_Wrapper() < 0 )
		E_Exit("Can't init SDL %s",SDL_GetError());
	sdl.inited = true;

#ifndef DISABLE_JOYSTICK
	//Initialise Joystick separately. This way we can warn when it fails instead
	//of exiting the application
	if( SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 ) LOG_MSG("SDL : Failed to init joystick support");
#endif

	sdl.laltstate = SDL_KEYUP;
	sdl.raltstate = SDL_KEYUP;
	sdl.num_joysticks=SDL_NumJoysticks();

	/* Parse configuration files */
	std::string config_file, config_path, config_combined;
	
#if defined (USERS_APPDATA)	&& defined(WIN32)	
	Cross::GetPlatformConfigDir(config_path);
#else
	config_path = current_working_path() + "\\";
#endif		
	
	//First parse -userconf
	if(control->cmdline->FindExist("-userconf",true)){
		config_file.clear();
		
#if defined (USERS_APPDATA)	&& defined(WIN32)	
		Cross::GetPlatformConfigDir(config_path);		
#else
		config_path = current_working_path() + "\\";
#endif	
		Cross::GetPlatformConfigName(config_file);
		config_combined = config_path + config_file;
		control->ParseConfigFile(config_combined.c_str());
		if(!control->configfiles.size()) {
			//Try to create the userlevel configfile.
			config_file.clear();
#if defined (USERS_APPDATA)	&& defined(WIN32)	
		Cross::GetPlatformConfigDir(config_path);		
#else
		config_path = current_working_path() + "\\";
#endif
			Cross::GetPlatformConfigName(config_file);
			config_combined = config_path + config_file;
			if(control->PrintConfig(config_combined.c_str())) {
				LOG_MSG("CONFIG: Generating default configuration.\n"
						"        Writing it to %s\n",config_combined.c_str());
				//Load them as well. Makes relative paths much easier
				control->ParseConfigFile(config_combined.c_str());
			}
		}
	}

	//Second parse -conf switches
	while(control->cmdline->FindString("-conf",config_file,true)) {
		if (!control->ParseConfigFile(config_file.c_str())) {
			// try to load it from the user directory
			if (!control->ParseConfigFile((config_path + config_file).c_str())) {
				LOG_MSG("CONFIG: Can't open specified config file:\n"
						"        %s\n",config_file.c_str());
			}
		}
	}
	// if none found => parse localdir conf
	if(!control->configfiles.size()){
		
#if defined (USERS_APPDATA)	&& defined(WIN32)			
#else
		// Data Path
		config_path = current_working_path() + "\\";
		if (!control->ParseConfigFile((config_path + "DATA\\dosbox.conf").c_str())) {
			
			LOG_MSG("CONFIG: Can't open default config file:\n"
					"        %s\n",(config_path + "DATA\\dosbox.conf").c_str());				
		}
#endif		
		
	} else if(!control->configfiles.size()){
		
#if defined (USERS_APPDATA)	&& defined(WIN32)			
		control->ParseConfigFile("dosbox.conf");
#else
		// Dosbox Root Path
		config_path = current_working_path() + "\\";
		if (!control->ParseConfigFile((config_path + "dosbox.conf").c_str())) {	
			
			LOG_MSG("CONFIG: Can't open default config file:\n"
				"		 %s\n",(config_path + "dosbox.conf").c_str());				
		}	
#endif	
	
	} else if(!control->configfiles.size()){
		
#if defined (USERS_APPDATA)	&& defined(WIN32)			
#else
	// App data Path 
	Cross::GetPlatformConfigDir(config_path);
	control->ParseConfigFile((config_path + "dosbox.conf").c_str());		
#endif		
	} 


	// if none found => parse userlevel conf
	if(!control->configfiles.size()) {
		config_file.clear();
		Cross::GetPlatformConfigName(config_file);
		
#if defined (USERS_APPDATA)	&& defined(WIN32)	
		Cross::GetPlatformConfigDir(config_path);
#else
		control->ParseConfigFile((config_path + config_file).c_str());	
#endif	

	}

	if(!control->configfiles.size()) {
		//Try to create the userlevel configfile.
		config_file.clear();
#if defined (USERS_APPDATA)	&& defined(WIN32)	
		Cross::GetPlatformConfigDir(config_path);
#else
		config_path = current_working_path() + "\\";
		control->ParseConfigFile((config_path + config_file).c_str());	
#endif	

		Cross::GetPlatformConfigName(config_file);
		config_combined = config_path + config_file;
		if(control->PrintConfig(config_combined.c_str())) {
			LOG_MSG("CONFIG: Generating default configuration.\n"
			        "        Writing to: %s\n",config_combined.c_str());
			//Load them as well. Makes relative paths much easier
			control->ParseConfigFile(config_combined.c_str());
		} else {
			LOG_MSG("CONFIG: Using default settings. Create a configfile to change them");
		}
	}


#if (ENVIRON_LINKED)
		control->ParseEnv(environ);
#endif
//		UI_Init();
//		if (control->cmdline->FindExist("-startui")) UI_Run(false);
		/* Init all the sections */
	
		control->Init();
		/* Some extra SDL Functions */
		Section_prop * sdl_sec=static_cast<Section_prop *>(control->GetSection("sdl"));

		if (control->cmdline->FindExist("-fullscreen") || sdl_sec->Get_bool("fullscreen")) {
			if(!sdl.desktop.fullscreen) { //only switch if not already in fullscreen
				GFX_SwitchFullScreen();
			}
		}

		/* Init the keyMapper */
		MAPPER_Init();
		if (control->cmdline->FindExist("-startmapper")) MAPPER_RunInternal();
		/* Start up main machine */
		control->StartUp();
		/* Shutdown everything */
	} catch (char * error) {
#if defined (WIN32)
		sticky_keys(true);
#endif
		GFX_ShowMsg("Exit to error: %s",error);
		fflush(NULL);
		if(sdl.wait_on_error) {
			//TODO Maybe look for some way to show message in linux?
#if defined(C_DEBUG)
			GFX_ShowMsg("Press enter to continue");
			fflush(NULL);
			fgetc(stdin);
#elif defined(WIN32)
			Sleep(5000);
#endif
		}

	}
	catch (int){
		; //nothing, pressed killswitch
	}
	catch(...){
		; // Unknown error, let's just exit.
	}
#if defined (WIN32)
	sticky_keys(true); //Might not be needed if the shutdown function switches to windowed mode, but it doesn't hurt
#endif
	//Force visible mouse to end user. Somehow this sometimes doesn't happen
	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_ShowCursor(SDL_ENABLE);

	SDL_Quit_Wrapper();//Let's hope sdl will quit as well when it catches an exception
	return 0;
}

void GFX_GetSize(int &width, int &height, bool &fullscreen) {
	width = sdl.draw.width;
	height = sdl.draw.height;
	fullscreen = sdl.desktop.fullscreen;
}


