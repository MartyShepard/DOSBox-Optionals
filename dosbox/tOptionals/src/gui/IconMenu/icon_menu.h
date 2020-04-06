/* Dosbox
 *
 * Dosbox is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define ICNMNU 0x1110 						/* Arbitrary, needs to be < 0xF000 */

#define ICNMNU_FULLSCREEN  	(ICNMNU +1  )
#define ICNMNU_WIN640X480  	(ICNMNU +2  )
#define ICNMNU_WIN800X600  	(ICNMNU +3  )
#define ICNMNU_WIN1024X768 	(ICNMNU +4  )
#define ICNMNU_WIN1152X864 	(ICNMNU +5  )
#define ICNMNU_WIN1176X664 	(ICNMNU +6  )
#define ICNMNU_WIN1280X720 	(ICNMNU +7  )
#define ICNMNU_WIN1280X768 	(ICNMNU +8  )
#define ICNMNU_WIN1280X800 	(ICNMNU +9  )
#define ICNMNU_WIN1280X960 	(ICNMNU +10 )
#define ICNMNU_WIN1280X1024 (ICNMNU +11 )
#define ICNMNU_WIN1360X768  (ICNMNU +12 )
#define ICNMNU_WIN1366X768  (ICNMNU +13 )
#define ICNMNU_WIN1440X900  (ICNMNU +14 )
#define ICNMNU_WIN1600X900  (ICNMNU +15 )
#define ICNMNU_WIN1600X1024 (ICNMNU +16 )
#define ICNMNU_WIN1600X1200 (ICNMNU +17 )
#define ICNMNU_WIN1680X1050 (ICNMNU +18 )
#define ICNMNU_WIN1768X992  (ICNMNU +19 )
#define ICNMNU_WIN1920X1080 (ICNMNU +20 )
#define ICNMNU_WIN1920X1200 (ICNMNU +21 )
#define ICNMNU_WIN1920X1440 (ICNMNU +22 )
#define ICNMNU_WIN2048X1536 (ICNMNU +23 )
#define ICNMNU_WIN2560X1440 (ICNMNU +24 )
#define ICNMNU_WIN2560X1600 (ICNMNU +25 )
#define ICNMNU_WIN3840X2160 (ICNMNU +26 )
#define ICNMNU_WIN7680X4320 (ICNMNU +27 )

#define ICNMNU_FUL640X480  	(ICNMNU +28 )
#define ICNMNU_FUL800X600  	(ICNMNU +29 )
#define ICNMNU_FUL1024X768 	(ICNMNU +30 )
#define ICNMNU_FUL1152X864 	(ICNMNU +31 )
#define ICNMNU_FUL1176X664 	(ICNMNU +32 )
#define ICNMNU_FUL1280X720 	(ICNMNU +33 )
#define ICNMNU_FUL1280X768 	(ICNMNU +34 )
#define ICNMNU_FUL1280X800 	(ICNMNU +35 )
#define ICNMNU_FUL1280X960 	(ICNMNU +36 )
#define ICNMNU_FUL1280X1024 (ICNMNU +37 )
#define ICNMNU_FUL1360X768  (ICNMNU +38 )
#define ICNMNU_FUL1366X768  (ICNMNU +39 )
#define ICNMNU_FUL1440X900  (ICNMNU +40 )
#define ICNMNU_FUL1600X900  (ICNMNU +41 )
#define ICNMNU_FUL1600X1024 (ICNMNU +42 )
#define ICNMNU_FUL1600X1200 (ICNMNU +43 )
#define ICNMNU_FUL1680X1050 (ICNMNU +44 )
#define ICNMNU_FUL1768X992  (ICNMNU +45 )
#define ICNMNU_FUL1920X1080 (ICNMNU +46 )
#define ICNMNU_FUL1920X1200 (ICNMNU +47 )
#define ICNMNU_FUL1920X1440 (ICNMNU +48 )
#define ICNMNU_FUL2048X1536 (ICNMNU +49 )
#define ICNMNU_FUL2560X1440 (ICNMNU +50 )
#define ICNMNU_FUL2560X1600 (ICNMNU +51 )
#define ICNMNU_FUL3840X2160 (ICNMNU +52 )
#define ICNMNU_FUL7680X4320 (ICNMNU +53 )

#define ICNMNU_VD_FILT_NONE (ICNMNU +54 )
#define ICNMNU_VD_FLT_POINT (ICNMNU +55 )
#define ICNMNU_VD_FLT_BILINEAR (ICNMNU +56 )
#define ICNMNU_VD_FLT_TRILINER (ICNMNU +57 )
#define ICNMNU_VD_FLT_ANISOTRC (ICNMNU +58 )
#define ICNMNU_VD_FLT_TESTMODE (ICNMNU +59 )

#define ICNMNU_RESTART 		(ICNMNU +60 )
#define ICNMNU_DISKCACHE	(ICNMNU +61 )
#define ICNMNU_CDROMSWAP 	(ICNMNU +62 )
#define ICNMNU_BINDMAPPER 	(ICNMNU +63 )
#define ICNMNU_WWWDOSBOX	(ICNMNU +64 )
#define ICNMNU_WWWVOGONS	(ICNMNU +65 )
#define ICNMNU_WWWGITHUB	(ICNMNU +66 )
#define ICNMNU_ABOUTOPTLS	(ICNMNU +67 )
#define ICNMNU_INTERNALUI	(ICNMNU +68 )
#define ICNMNU_PREFSEDIT	(ICNMNU +69 )
#define ICNMNU_CAPSHOTSW 	(ICNMNU +70 )
#define ICNMNU_CAPSHOTHWBMP (ICNMNU +71 )
#define ICNMNU_CAPSHOTHWPNG (ICNMNU +72 )
#define ICNMNU_FULDESKTOP 	(ICNMNU +73 )
#define ICNMNU_WINDESKTOP 	(ICNMNU +74 )

static WNDPROC oldproc = NULL;
static LRESULT CALLBACK SysMenuExtendWndProc(HWND hwnd, UINT uiMsg, WPARAM wparam, LPARAM lparam)
{
	/*
		static DWORD dwTickKeep = 0;
		if ((GetTickCount()-dwTickKeep)>300)
		{   dwTickKeep = GetTickCount();
			Beep(2000, 100);
		}	
	*/

    switch (uiMsg)
    {
		case WM_NCHITTEST:
                return DefWindowProc(hwnd, uiMsg, wparam, lparam);		
		case WM_SYSCOMMAND:
		{
			switch (wparam)
			{
			
				case ICNMNU:		
				{}
				case ICNMNU_FULLSCREEN	:	{ GFX_SwitchFullScreen();				}return 0;
				case ICNMNU_WIN640X480	:	{ GFX_UpdateResolution(640, 480, true);	}return 0;
				case ICNMNU_WIN800X600	:	{ GFX_UpdateResolution(800, 600, true);	}return 0;
				case ICNMNU_WIN1024X768 :	{ GFX_UpdateResolution(1024,768, true);	}return 0;
				case ICNMNU_WIN1152X864 :	{ GFX_UpdateResolution(1152,864, true);	}return 0;
				case ICNMNU_WIN1176X664 :	{ GFX_UpdateResolution(1176,664, true);	}return 0;
				case ICNMNU_WIN1280X720 :	{ GFX_UpdateResolution(1280,720, true);	}return 0;	
				case ICNMNU_WIN1280X768 :	{ GFX_UpdateResolution(1280,768, true);	}return 0;
				case ICNMNU_WIN1280X800 :	{ GFX_UpdateResolution(1280,800, true);	}return 0;
				case ICNMNU_WIN1280X960 :	{ GFX_UpdateResolution(1280,960, true);	}return 0;
				case ICNMNU_WIN1280X1024:	{ GFX_UpdateResolution(1280,1024,true);	}return 0;
				case ICNMNU_WIN1360X768 :	{ GFX_UpdateResolution(1360,768 ,true);	}return 0;
				case ICNMNU_WIN1366X768 :	{ GFX_UpdateResolution(1366,768 ,true);	}return 0;
				case ICNMNU_WIN1440X900 :	{ GFX_UpdateResolution(1440,900 ,true);	}return 0;	
				case ICNMNU_WIN1600X900 :	{ GFX_UpdateResolution(1600,900 ,true);	}return 0;
				case ICNMNU_WIN1600X1024:	{ GFX_UpdateResolution(1600,1024,true);	}return 0;
				case ICNMNU_WIN1600X1200:	{ GFX_UpdateResolution(1600,1200,true);	}return 0;
				case ICNMNU_WIN1680X1050:	{ GFX_UpdateResolution(1680,1050,true);	}return 0;
				case ICNMNU_WIN1768X992 :	{ GFX_UpdateResolution(1768,992, true);	}return 0;
				case ICNMNU_WIN1920X1080:	{ GFX_UpdateResolution(1920,1080,true);	}return 0;
				case ICNMNU_WIN1920X1200:	{ GFX_UpdateResolution(1920,1200,true);	}return 0;	
				case ICNMNU_WIN1920X1440:	{ GFX_UpdateResolution(1920,1440,true);	}return 0;
				case ICNMNU_WIN2048X1536:	{ GFX_UpdateResolution(2048,1536,true);	}return 0;
				case ICNMNU_WIN2560X1440:	{ GFX_UpdateResolution(2560,1440,true);	}return 0;
				case ICNMNU_WIN2560X1600:	{ GFX_UpdateResolution(2560,1600,true);	}return 0;
				case ICNMNU_WIN3840X2160:	{ GFX_UpdateResolution(3840,2160,true);	}return 0;
				case ICNMNU_WIN7680X4320:	{ GFX_UpdateResolution(7680,4320,true);	}return 0;	
				case ICNMNU_WINDESKTOP	:	{ GFX_UpdateResolution(0,0		,true);	}return 0;					
				case ICNMNU_FUL640X480	:	{ GFX_UpdateResolution(640, 480, false);}return 0;
				case ICNMNU_FUL800X600	:	{ GFX_UpdateResolution(800, 600, false);}return 0;
				case ICNMNU_FUL1024X768 :	{ GFX_UpdateResolution(1024,768, false);}return 0;
				case ICNMNU_FUL1152X864 :	{ GFX_UpdateResolution(1152,864, false);}return 0;
				case ICNMNU_FUL1176X664 :	{ GFX_UpdateResolution(1176,664, false);}return 0;
				case ICNMNU_FUL1280X720 :	{ GFX_UpdateResolution(1280,720, false);}return 0;	
				case ICNMNU_FUL1280X768 :	{ GFX_UpdateResolution(1280,768, false);}return 0;
				case ICNMNU_FUL1280X800 :	{ GFX_UpdateResolution(1280,800, false);}return 0;
				case ICNMNU_FUL1280X960 :	{ GFX_UpdateResolution(1280,960, false);}return 0;
				case ICNMNU_FUL1280X1024:	{ GFX_UpdateResolution(1280,1024,false);}return 0;
				case ICNMNU_FUL1360X768 :	{ GFX_UpdateResolution(1360,768 ,false);}return 0;
				case ICNMNU_FUL1366X768 :	{ GFX_UpdateResolution(1366,768 ,false);}return 0;
				case ICNMNU_FUL1440X900 :	{ GFX_UpdateResolution(1440,900 ,false);}return 0;	
				case ICNMNU_FUL1600X900 :	{ GFX_UpdateResolution(1600,900 ,false);}return 0;
				case ICNMNU_FUL1600X1024:	{ GFX_UpdateResolution(1600,1024,false);}return 0;
				case ICNMNU_FUL1600X1200:	{ GFX_UpdateResolution(1600,1200,false);}return 0;
				case ICNMNU_FUL1680X1050:	{ GFX_UpdateResolution(1680,1050,false);}return 0;
				case ICNMNU_FUL1768X992 :	{ GFX_UpdateResolution(1768,992, false);}return 0;
				case ICNMNU_FUL1920X1080:	{ GFX_UpdateResolution(1920,1080,false);}return 0;
				case ICNMNU_FUL1920X1200:	{ GFX_UpdateResolution(1920,1200,false);}return 0;	
				case ICNMNU_FUL1920X1440:	{ GFX_UpdateResolution(1920,1440,false);}return 0;
				case ICNMNU_FUL2048X1536:	{ GFX_UpdateResolution(2048,1536,false);}return 0;
				case ICNMNU_FUL2560X1440:	{ GFX_UpdateResolution(2560,1440,false);}return 0;
				case ICNMNU_FUL2560X1600:	{ GFX_UpdateResolution(2560,1600,false);}return 0;
				case ICNMNU_FUL3840X2160:	{ GFX_UpdateResolution(3840,2160,false);}return 0;
				case ICNMNU_FUL7680X4320:	{ GFX_UpdateResolution(7680,4320,false);}return 0;
				case ICNMNU_FULDESKTOP	:	{ GFX_UpdateResolution(0,0		,false);}return 0;				
				case ICNMNU_VD_FILT_NONE:	{ extVoodoo.GL_filtering = 0; GFX_ResetVoodoo();}return 0;
				case ICNMNU_VD_FLT_POINT:	{ extVoodoo.GL_filtering = 1; GFX_ResetVoodoo();}return 0;
				case ICNMNU_VD_FLT_BILINEAR:{ extVoodoo.GL_filtering = 2; GFX_ResetVoodoo();}return 0;
				case ICNMNU_VD_FLT_TRILINER:{ extVoodoo.GL_filtering = 3; GFX_ResetVoodoo();}return 0;
				case ICNMNU_VD_FLT_ANISOTRC:{ extVoodoo.GL_filtering = 4; GFX_ResetVoodoo();}return 0;
				case ICNMNU_VD_FLT_TESTMODE:{ extVoodoo.GL_filtering = 5; GFX_ResetVoodoo();}return 0;
				case ICNMNU_RESTART		   :{ Restart(true);						}return 0;
				case ICNMNU_DISKCACHE	   :{ swapInNextDisk(true, true, false);	}return 0;			
				case ICNMNU_CDROMSWAP	   :{ swapInNextDisk(true, false, true);	}return 0;
				//case ICNMNU_INTERNALUI	:{ UI_Run(true);						}return 0;	
				case ICNMNU_CAPSHOTSW  :
					{ 
						CAPTURE_ScreenShotEvent(true); 								
					}return 0;		
				case ICNMNU_CAPSHOTHWBMP:
					{ 		
						CaptureOGLScreenShot(0, 0, sdl.desktop.window.width, sdl.desktop.window.height, ".bmp");
						
					}return 0;	
				case ICNMNU_CAPSHOTHWPNG:
					{ 		
						CaptureOGLScreenShot(0, 0, sdl.desktop.window.width, sdl.desktop.window.height, ".png");
						
					}return 0;					
				case ICNMNU_PREFSEDIT	   :{ PrefsEditDirect(hwnd, "notepad.exe");	}return 0;				
				case ICNMNU_WWWDOSBOX	   :
					{ 
						ShellExecute(0, "open", "http://dosbox.com", "", NULL, SW_SHOWNORMAL);
					}return 0;
					
				case ICNMNU_WWWVOGONS	   :
					{ 
						ShellExecute(0, "open", "https://www.vogons.org/viewforum.php?f=53", "", NULL, SW_SHOWNORMAL);
					}return 0;				
					
				case ICNMNU_WWWGITHUB	   :
					{ 
						ShellExecute(0, "open", "https://github.com/MartyShepard/DOSBox-Optionals", "", NULL, SW_SHOWNORMAL);
					}return 0;
					
				case ICNMNU_ABOUTOPTLS:
					{
						::MessageBox(hwnd, (
							"\tDOSBox " VERSION " " DOSBOXREVISION " Build on (" __DATE__ " " __TIME__ ")\r\r"
							"\tCopyright 2002-2020 DOSBox Team\r\r"
							"\tOriginal DOSBox is written by the DOSBox Team. See Authors File.\n"
							"\tThis is a DOSBox Fork from the Original DOSBox 0.74 " VERSION " " DOSBOXSVERSION "\n\n"
							"\tOptionals features added by Marty Shepard\n\n"							
							"\tDOSBox comes with ABSOLUTELY NO WARRANTY. This is free\n"
							"\tsoftware and you are welcome to redistribute,  it under\n"
							"\tcertain conditions. If you paid for it, you were screwed.\n\n"							
							"\tGreetings, credits & thanks:\n"
							"\tDOSBox Team, dungan, NY00123, tauro, bloodbat, Yesterplay80\n"
							"\tnukeykt, VileRancour, D_Skywalk, Vasyl Tsvirkunov, Moe, kekko\n"
							"\tTaeWoong Yoo, krcroft\n\n"
							"\t\tGreetings to the CGBoard & Vogons Board\r\n\n"
							"\t\tDon't forget the good old days and relive\n"
							"\t\tthe best old games under DOS & Windows9x"
							), 
							"About DOSBox", MB_OK);
					}
					return 0;
				case ICNMNU_BINDMAPPER:
					{			
						/*
							Mapper Start own Window. Dont lose and
							Assign after Mapper Exit a new HWND Value. > Crash
						*/
						HWND hOld = hwnd;										
						MAPPER_RunEvent(0);
						
						HWND hwnd = hOld;
						Set_Window_HMenu();
					}			
					return 0;	
			};					
		};
		default:
		return oldproc ? CallWindowProc(oldproc, hwnd, uiMsg, wparam, lparam) : DefWindowProc(hwnd, uiMsg, wparam, lparam);
    }
}

static void SysMenuExtendWndProcInstall(HWND hwndSubject)
{
    //if (prevWndProc) __debugbreak();
    oldproc = (WNDPROC)::GetWindowLongPtr(hwndSubject, GWLP_WNDPROC);
	WNDPROC oldproc  = (WNDPROC)SetWindowLongPtr(hwndSubject, GWLP_WNDPROC, (LONG_PTR)&SysMenuExtendWndProc);
    
}