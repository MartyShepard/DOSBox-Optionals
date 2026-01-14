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

 /*
		Value 			hex				Meaning

		MF_BITMAP		0x00000004L		Uses a bitmap as the menu item. The lpNewItem parameter contains
										a handle to the bitmap.

		MF_CHECKED 		0x00000008L		Places a check mark next to the menu item. If the application
										provides check-mark bitmaps (see SetMenuItemBitmaps, this flag
										displays the check-mark bitmap next to the menu item.

		MF_DISABLED		0x00000002L		Disables the menu item so that it cannot be selected, but the flag
										does not gray it.

		MF_ENABLED		0x00000000L		Enables the menu item so that it can be selected, and restores it
										from its grayed state.

		MF_GRAYED		0x00000001L		Disables the menu item and grays it so that it cannot be selected.

		MF_MENUBARBREAK	0x00000020L		Functions the same as the MF_MENUBREAK flag for a menu bar. For a
										drop-down menu, submenu, or shortcut menu, the new column is separated
										from the old column by a vertical line.

		MF_MENUBREAK	0x00000040L     Places the item on a new line (for a menu bar) or in a new column
										(for a drop-down menu, submenu, or shortcut menu) without separating columns.

		MF_OWNERDRAW	0x00000100L		Specifies that the item is an owner-drawn item. Before the menu is
										displayed for the first time, the window that owns the menu receives
										a WM_MEASUREITEM message to retrieve the width and height of the menu
										item. The WM_DRAWITEM message is then sent to the window procedure of
										the owner window whenever the appearance of the menu item must be updated.
										
		MF_POPUP		0x00000010L		Specifies that the menu item opens a drop-down menu or submenu. The
										uIDNewItem parameter specifies a handle to the drop-down menu or submenu.
										This flag is used to add a menu name to a menu bar, or a menu item that
										opens a submenu to a drop-down menu, submenu, or shortcut menu.

		MF_SEPARATOR	0x00000800L		Draws a horizontal dividing line. This flag is used only in a drop-down
										menu, submenu, or shortcut menu. The line cannot be grayed, disabled, or
										highlighted. The lpNewItem and uIDNewItem parameters are ignored.

		MF_STRING		0x00000000L		Specifies that the menu item is a text string; the lpNewItem parameter
										is a pointer to the string.

		MF_UNCHECKED	0x00000000L		Does not place a check mark next to the item (default). If the application
										supplies check-mark bitmaps (see SetMenuItemBitmaps), this flag displays
										the clear bitmap next to the menu item. 
	
		https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-appendmenua
 */
 

	hSubFllRes = CreatePopupMenu();
	hSubWinRes = CreatePopupMenu();
	hvoodooflt = CreatePopupMenu();
	hfinternet = CreatePopupMenu();
	hcpucycles = CreatePopupMenu();	
	hcputypes  = CreatePopupMenu();	
	hcpucores  = CreatePopupMenu();	
	hsbTypen   = CreatePopupMenu();
	hsbOptionen= CreatePopupMenu();
	hdbOptionen= CreatePopupMenu();
	hgsTypen   = CreatePopupMenu();
	hgsOptionen= CreatePopupMenu();
	hSoundOpCDA= CreatePopupMenu();
	hSoundOpSBL= CreatePopupMenu();
	hSoundOpGUS= CreatePopupMenu();
	hSoundOpFM = CreatePopupMenu();
	hSoundOpMT = CreatePopupMenu();
	hSoundOpDS = CreatePopupMenu();
	hSoundOpTD = CreatePopupMenu();
	hSoundOpINV= CreatePopupMenu();
	hSoundOpPS1= CreatePopupMenu();
	hSoundOpPSD= CreatePopupMenu();
	hSoundOpSPK= CreatePopupMenu();
	hJoystickOp= CreatePopupMenu();
	hVoodooSub = CreatePopupMenu();
	hVoodooTex = CreatePopupMenu();
	hVoodooRGB = CreatePopupMenu();

	hSysMenu   = 
	
	::GetSystemMenu(DosboxHwndIcon, FALSE);
	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FULLSCREEN		, ("&FullScreen\tAlt+Enter"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	 	
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_BINDMAPPER		, ("&Key && Joystick Mapper\tCtrl+F1"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_PREFSEDIT		, ("Edit Preferences Direct"));		
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FOPN_FLOPPYA	, ("&Open Floppy A: Image\tCtrl+Alt+F9"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FOPN_FLOPPYB	, ("&Open Floppy B: Image\tCtrl+Alt+F10"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_DISKCACHE		, ("&Cache: Swap Next Floppy\tCtrl+F9"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FOPN_CDROM		, ("&Open CDROM Device 1: Image\tCtrl+Alt+F11"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FOPN_CDROM2	, ("&Open CDROM Device 2: Image\tCtrl+Alt+F12"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CDROMSWAP		, ("&Cache: Swap Next Cd-Rom\tCtrl+F11"));	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");		
	//::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_INTERNALUI		, ("Internal Preferences"));	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CAPSHOTSW		, ("&Capture: Screenshot [SW]\tCtrl+F5"));	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CAPSHOTHWBMP	, ("&Capture: Screenshot [OpenGL] (BMP)"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CAPSHOTHWPNG	, ("&Capture: Screenshot [OpenGL] (PNG)"));	
	
	::AppendMenu(hSysMenu, MF_SEPARATOR, 0, "");
	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hcpucores, ("CPU: Cores"));
	::AppendMenu(hcpucores, MF_STRING, ICNMNU_CCORE_AUTO, ("Automatic"));
	::AppendMenu(hcpucores, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucores, MF_STRING, ICNMNU_CCORE_SIMP, ("Simple"));
	::AppendMenu(hcpucores, MF_STRING, ICNMNU_CCORE_NORM, ("Normal"));
	::AppendMenu(hcpucores, MF_STRING, ICNMNU_CCORE_DYNA, ("Dynamic"));

	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hcputypes, ("CPU: Types"));
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_AUTO, ("Automatic"));
	::AppendMenu(hcputypes, MF_SEPARATOR, 0, "");
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_386, ("386"));
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_386P, ("386 Prefetch"));
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_386S, ("386 Slow"));
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_486S, ("486 Slow"));
	::AppendMenu(hcputypes, MF_SEPARATOR, 0, "");
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_PENS, ("Pentium"));
	::AppendMenu(hcputypes, MF_STRING, ICNMNU_CTYPE_PPRS, ("Pentium Pro"));

	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hcpucycles, ("CPU: Cycles"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCAUTO, ("Automatic"));
	::AppendMenu(hcpucycles, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi8088_716, ("8088 (7,16mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi8088_954, ("8088 (9,54mhz)"));
	::AppendMenu(hcpucycles, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi286_10, ("268    (10mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi286_12, ("268    (12mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi286_16, ("268    (16mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi286_20, ("268    (20mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi286_25, ("268    (25mhz)"));
	::AppendMenu(hcpucycles, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi386dx_25, ("368DX  (25mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi386dx_33, ("368DX  (33mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi386dx_40, ("368DX  (40mhz)"));
	::AppendMenu(hcpucycles, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486sx_25, ("468SX  (25mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486dx_33, ("468DX  (33mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486sx_33, ("468SX  (33mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486sx_40, ("468SX  (40mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486dx_50, ("468DX  (50mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486dx2_66, ("468DX/2(66mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486sx2_80, ("468DX/2(80mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486dx2_100, ("468DX/2(100mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCi486dx4_100, ("468DX/4(100mhz)"));
	::AppendMenu(hcpucycles, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCp60, ("Pentium(60mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCp75, ("Pentium(75mhz)"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCp100, ("Pentium(100mhz)"));
	::AppendMenu(hcpucycles, MF_SEPARATOR, 0, "");
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCMAX100, ("Max"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCMAX150, ("Max 150%"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCMAX200, ("Max 200%"));
	::AppendMenu(hcpucycles, MF_STRING, ICNMNU_CCYCMAX250, ("Max 250%"));

	::AppendMenu(hSysMenu, MF_SEPARATOR, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hSubWinRes	, ("Resolution: Window"));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WINDESKTOP		, ("\tDesktop"		));		
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN640X480		, ("\t 640x480"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN800X600		, ("\t 800x600"		));			
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1024X768	, ("\t1024x768"		));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1152X864	, ("\t1152x864"		));			
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1176X664	, ("\t1176x664"		));		
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1280X720	, ("\t1280x720"		));		
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1280X768	, ("\t1280x768"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1280X800	, ("\t1280x800"		));			
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1280X960	, ("\t1280x960"		));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1280X1024	, ("\t1280x1024"	));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1280X1080	, ("\t1280x1080"	));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1360X768	, ("\t1360x768"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1366X768	, ("\t1366x768"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1440X900	, ("\t1440x900"		));	
		::AppendMenu(hSubWinRes , MF_STRING			, ICNMNU_WIN1440X1080	, ("\t1440x1080"	));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1600X900	, ("\t1600x900"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1600X1024	, ("\t1600x1024"	));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1600X1200	, ("\t1600x1200"	));			
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1680X1050	, ("\t1680x1050"	));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1768X992	, ("\t1768x992"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1920X1080	, ("\t1920x1080"	));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1920X1200	, ("\t1920x1200"	));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1920X1440	, ("\t1920x1440"	));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN2048X1536	, ("\t2048x1536"	));
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN2560X1440	, ("\t2560x1440"	));		
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN2560X1600	, ("\t2560x1600"	));			
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN3840X2160	, ("\t3840x2160"	));		
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN7680X4320	, ("\t7680x4320"	));		

	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hSubFllRes	, ("Resolution: FullScreen"));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FULDESKTOP		, ("\tDesktop"		));		
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL640X480		, ("\t 640x480"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL800X600		, ("\t 800x600"		));			
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1024X768	, ("\t1024x768"		));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1152X864	, ("\t1152x864"		));			
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1176X664	, ("\t1176x664"		));		
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1280X720	, ("\t1280x720"		));		
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1280X768	, ("\t1280x768"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1280X800	, ("\t1280x800"		));			
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1280X960	, ("\t1280x960"		));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1280X1024	, ("\t1280x1024"	));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_FUL1280X1080	, ("\t1280x1080"	));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1360X768	, ("\t1360x768"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1366X768	, ("\t1366x768"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1440X900	, ("\t1440x900"		));
		::AppendMenu(hSubFllRes , MF_STRING			, ICNMNU_FUL1440X1080	, ("\t1440x1080"	));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1600X900	, ("\t1600x900"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1600X1024	, ("\t1600x1024"	));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1600X1200	, ("\t1600x1200"	));			
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1680X1050	, ("\t1680x1050"	));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1768X992	, ("\t1768x992"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1920X1080	, ("\t1920x1080"	));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1920X1200	, ("\t1920x1200"	));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1920X1440	, ("\t1920x1440"	));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL2048X1536	, ("\t2048x1536"	));
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL2560X1440	, ("\t2560x1440"	));		
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL2560X1600	, ("\t2560x1600"	));			
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL3840X2160	, ("\t3840x2160"	));		
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL7680X4320	, ("\t7680x4320"	));			

	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hdbOptionen	, ("Resolution Fixes"));		
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_320x240x8		, ("320x240x4 <-> 320x200x8"));	
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_400x300x8		, ("400x300x8 <-> 320x480x8"));	
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_640x480x16		, ("640x480x16 <-> 320x480x8"));
		::AppendMenu(hdbOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_640x480x15		, ("Stretch 640x480x15"));
		::AppendMenu(hdbOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_32BitModes		, ("Use 32Bit Modes"));	
		::AppendMenu(hdbOptionen, MF_SEPARATOR, 0, "");
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_S3_OVERCLCKA	, ("S3 Clock: 14.318kHz"));
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_S3_OVERCLCKB	, ("S3 Clock: 28.636kHz"));
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_S3_OVERCLCKC	, ("S3 Clock: 57.272kHz"));
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_S3_OVERCLCKD	, ("S3 Clock: 85.908kHz"));
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_S3_OVERCLCKE	, ("S3 Clock: 114.544kHz"));

	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hvoodooflt	, ("Voodoo: Options"	));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FILT_NONE	, ("Filter: None"		));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_POINT	, ("Filter: Point"		));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_BILINEAR, ("Filter: Bilinear"	));			
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_TRILINER, ("Filter: Trilinear"	));			
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_ANISOTRC, ("Filter: Anisotropic"));				
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_TESTMODE, ("Filter: Testmode"	));

		::AppendMenu(hvoodooflt	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_ScreenFixFrnt	, ("LFB Screen Fix (Front)"));	
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_ScreenFixBack	, ("LFB Screen Fix (Back)" ));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_LogRegisterNr	, ("LFB LOG Register Number"));

		::AppendMenu(hvoodooflt	, MF_SEPARATOR			, 0, "");			
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_QuadsDraw_use	, ("OpenGL QuadsDraw"	   ));	
		::AppendMenu(hvoodooflt,  MF_STRING			, ICNMNU_Cache_Delete	, ("Delete Cache Textures" ));
		::AppendMenu(hvoodooflt,  MF_STRING			, ICNMNU_High_Ratio		, ("High Ratio Cycles"	   ));

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING			, ICNMNU_GLShade_NONE, ("GL Shade: None"));
		::AppendMenu(hvoodooflt, MF_STRING			, ICNMNU_GLShade_FLAT, ("GL Shade: Flat"));
		::AppendMenu(hvoodooflt, MF_STRING			, ICNMNU_GLShade_SMOOTH, ("GL Shade: Smooth"));

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING			, ICNMNU_GL_SCAN0, ("Draw: Scanmode 0"));
		::AppendMenu(hvoodooflt, MF_STRING			, ICNMNU_GL_SCAN1, ("Draw: Scanmode 1"));
		::AppendMenu(hvoodooflt, MF_STRING			, ICNMNU_GL_SCAN2, ("Draw: Scanmode 2"));

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING | MF_POPUP, (UINT_PTR)hVoodooRGB, ("Make Darker"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK000, ("Disabled"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK005, ("- 005"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK010, ("- 010"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK015, ("- 015"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK020, ("- 020"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK025, ("- 025"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK030, ("- 030"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK035, ("- 035"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK040, ("- 040"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK045, ("- 045"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK050, ("- 050"));
		/*
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK055, ("- 055"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK060, ("- 060"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK065, ("- 065"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK070, ("- 070"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK075, ("- 075"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK080, ("- 080"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK085, ("- 085"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK090, ("- 090"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK095, ("- 095"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK100, ("- 100"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK105, ("- 105"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK110, ("- 110"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK115, ("- 115"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK120, ("- 120"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK125, ("- 125"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK130, ("- 130"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK135, ("- 135"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK140, ("- 140"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK145, ("- 145"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK150, ("- 150"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK155, ("- 155"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK160, ("- 160"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK165, ("- 165"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK170, ("- 170"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK175, ("- 175"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK180, ("- 180"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK185, ("- 185"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK190, ("- 190"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK195, ("- 195"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK200, ("- 200"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK205, ("- 205"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK210, ("- 210"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK215, ("- 215"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK220, ("- 220"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK225, ("- 225"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK230, ("- 230"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK235, ("- 235"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK240, ("- 240"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK245, ("- 245"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK250, ("- 250"));
		::AppendMenu(hVoodooRGB, MF_STRING, ICNMNU_GL_DARK254, ("- 255"));
		*/

		#if defined(C_DEBUG)
		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING | MF_POPUP, (UINT_PTR)hVoodooSub, ("RGB Options"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_BYTE,							  ("GL BYTE"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_BYTE,				  ("GL UNSIGNED BYTE"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_SHORT,						  ("GL_SHORT"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT,				  ("GL BYTE"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_INT,							  ("GL INT"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT,					  ("GL UNSIGNED_INT"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_FLOAT,						  ("GL FLOAT"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_2_BYTES,						  ("GL 2 BYTES"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_3_BYTES,						  ("GL 3 BYTES"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_4_BYTES,						  ("GL 4 BYTES"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_DOUBLE,						  ("GL DOUBLE"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_HALF_FLOAT,					  ("GL HALF FLOAT"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_BYTE_332,			  ("GL UNSIGNED BYTE 3.3.2"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT_4444,			  ("GL UNSIGNED SHORT 4.4.4.4"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT_5551,			  ("GL UNSIGNED SHORT 5.5.5.1"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_8888,			  ("GL UNSIGNED INT 8.8.8.8"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_1010102,			  ("GL UNSIGNED INT 10.10.10.2"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_BYTE_233_REV,		  ("GL UNSIGNED BYTE 2.3.3 REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT_565,			  ("GL UNSIGNED SHORT 5.6.5"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT_565_REV,		  ("GL UNSIGNED SHORT 5.6.5 REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT_4444_REV,		  ("GL UNSIGNED SHORT 4.4.4.4 REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_SHORT_1555_REV,		  ("GL UNSIGNED SHORT 1.5.5.5 REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_8888_REV,		  ("GL UNSIGNED INT 8.8.8.8 REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_2101010_REV,		  ("GL UNSIGNED INT 2.10.10.10 REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_248,				  ("GL UNSIGNED INT 2.4.8"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_10F11F11FREV,	  ("GL UNSIGNED INT 10F.11F.11F.REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_UNSIGNED_INT_5999REV,			  ("GL UNSIGNED INT 5.9.9.9.REV"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_FLOAT_32_UNSIGNED_INT_24_8_REV, ("GL FLOAT 32 UNSIGNED INT 24.8 REV"));

		::AppendMenu(hVoodooSub, MF_SEPARATOR, 0, "");
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB4,		("RGB 4"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB5,		("RGB 5"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB8,		("RGB 8"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB10,	("RGB 10"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB12,	("RGB 12"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB16,	("RGB 16"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGBA2,	("RGB A2"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGBA4,	("RGB A4"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB5_A1,  ("RGB 5 A1"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGBA8,	("RGB A8"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB10_A2, ("RGB 10 A2"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGBA12,	("RGB A12"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGBA16,	("RGB A16"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RED,		("RGB RED"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_GREEN,	("RGB GREEN"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_BLUE,		("RGB BLUE"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGB,		("RGB"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_BGR,		("BGR"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_RGBA,		("RGBA"));
		::AppendMenu(hVoodooSub, MF_STRING, ICNMNU_GL_BGRA,		("BGRA"));
		#endif

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING | MF_POPUP, (UINT_PTR)hVoodooTex, ("Wrap Options"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPS_REPEAT, ("Texture Wrap S: Repeat"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPS_REPEATM, ("Texture Wrap S: Repeat Mirrored"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPS_CLAMPTE, ("Texture Wrap S: Clamp To Edge"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPS_CLAMPTB, ("Texture Wrap S: Clamp To Border"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPR_REPEAT,  ("Texture Wrap R: Repeat"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPR_REPEATM, ("Texture Wrap R: Repeat Mirrored"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPR_CLAMPTE, ("Texture Wrap R: Clamp To Edge"));
		::AppendMenu(hVoodooTex, MF_STRING, ICNMNU_TEXTURE_WRAPR_CLAMPTB, ("Texture Wrap R: Clamp To Border"));

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_VOODOO_ASPECT, ("Voodoo Aspect"));

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FBZCOLORPATHFIX, ("Shader FBZ ColorPath Fix"));

		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GLBLENDFC_FLAG  , ("GL: Blending"));
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GL_FOG_FLAG     , ("GL: Fog Hint"));
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GL_SCISSOR_FLAG , ("GL: Scissor Flag"));
		::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");

		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GLP_SOOTH_FLAG, ("Antialiasing for Points"));
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GLL_SOOTH_FLAG, ("Antialiasing for Lines"));
		/*::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_COMAPTIBLE_FLAG,("Compatible Flag"));*/
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GL_MIPMAP_FLAG, ("Generate MipMap Hint"));
		::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_GLPERSOCORFLAG, ("Perspective Correction Hint"));



		
		if (bVoodooUseHighRatio) {
			::AppendMenu(hvoodooflt, MF_SEPARATOR, 0, "");
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_01000, ("Add Cycles Speed:  1000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_02000, ("Add Cycles Speed:  2000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_03000, ("Add Cycles Speed:  3000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_04000, ("Add Cycles Speed:  4000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_05000, ("Add Cycles Speed:  5000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_06000, ("Add Cycles Speed:  6000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_07000, ("Add Cycles Speed:  7000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_08000, ("Add Cycles Speed:  8000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_09000, ("Add Cycles Speed:  9000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_10000, ("Add Cycles Speed: 10000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_11000, ("Add Cycles Speed: 11000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_12000, ("Add Cycles Speed: 12000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_13000, ("Add Cycles Speed: 13000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_14000, ("Add Cycles Speed: 14000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_15000, ("Add Cycles Speed: 15000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_16000, ("Add Cycles Speed: 16000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_17000, ("Add Cycles Speed: 17000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_18000, ("Add Cycles Speed: 18000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_19000, ("Add Cycles Speed: 19000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_20000, ("Add Cycles Speed: 20000"));
			::AppendMenu(hvoodooflt, MF_STRING, ICNMNU_FXCYCLES_00000, ("Disable Cycles Speed"));
		}
	::AppendMenu(hSysMenu, MF_SEPARATOR, 0, "");


		if (useSoundSB) {
			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpSBL, ("Volume: Soundblaster"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpSBL, MF_STRING, ICNMNU_SND_VOL_SB000, ("Set Volume: Mute"));

			::AppendMenu(hSysMenu	, MF_STRING | MF_POPUP, (UINT_PTR)hsbTypen, ("SB Sound Typen"));
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_SB1	, ("Type: SB 1"));
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_SB2	, ("Type: SB 2"));
			::AppendMenu(hsbTypen	, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_PRO1	, ("Type: SBPro 1"));
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_PRO2	, ("Type: SBPro 2"));
			::AppendMenu(hsbTypen	, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_SB16	, ("Type: SB16"));
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_SB16V, ("Type: SB16 VibraC"));
			::AppendMenu(hsbTypen	, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_GB	, ("Type: Gameblaster"));
			::AppendMenu(hsbTypen	, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_ESS	, ("Type: ESS 688"));
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_S400	, ("Type: Reveal SC400"));
			::AppendMenu(hsbTypen	, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbTypen	, MF_STRING, ICNMNU_SBTYPE_NONE	, ("Type: None"));

			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hsbOptionen, ("SB Sound Options"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_SBMIXER		, ("SB Mixer"));
			::AppendMenu(hsbOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_EnableASP	, ("SB16 Advanced Chip"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_StereoCTRL	, ("SBPro Stereo Control"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_ProStereo	, ("SBPro Stereo (PreSet)"));
			::AppendMenu(hsbOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_GoldPlay	, ("Goldplay Emulation"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_GoldPlayF	, ("Goldplay Force"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_GoldPlayS	, ("Goldplay Stereo"));
			::AppendMenu(hsbOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_EnableSpeaker, ("Enable Speaker (VM)"));
			::AppendMenu(hsbOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_DSPAInit	, ("Force DSP AutoInit"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_DSPReturn7F	, ("DSP Return 7f/ff"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_IOPrtAlias	, ("IO Port Aliasing"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_IDirectDAC	, ("Instant Direct DAC"));
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_UnMaskIRQ	, ("PIC Umnask IRQ"));
			::AppendMenu(hsbOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hsbOptionen, MF_STRING, ICNMNU_SRLimits	, ("Sample Rate Limits"));
			::AppendMenu(hSysMenu, MF_SEPARATOR, 0, "");
		}
		if (useSoundGus) {
			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpGUS, ("Volume: Gravis Ultrasound"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpGUS, MF_STRING, ICNMNU_SND_VOL_GS000, ("Set Volume: Mute"));

			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hgsTypen, ("GUS Sound Board"));
			::AppendMenu(hgsTypen, MF_STRING, ICNMNU_GUS_ENBALE, ("Enable Ultrasound"));
			::AppendMenu(hgsTypen, MF_SEPARATOR, 0, "");
			::AppendMenu(hgsTypen, MF_STRING, ICNMNU_GUS_TYPECL, ("Type: Classic (Pre)"));
			::AppendMenu(hgsTypen, MF_STRING, ICNMNU_GUS_TYPE37, ("Type: Classic v3.74"));
			::AppendMenu(hgsTypen, MF_STRING, ICNMNU_GUS_TYPEMX, ("Type: UltraSound MAX"));
			::AppendMenu(hgsTypen, MF_STRING, ICNMNU_GUS_TYPEIN, ("Type: GUS Interweave"));

			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hgsOptionen, ("GUS Options"));
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_AUTOAMP, ("Automatic AMP"));
			::AppendMenu(hgsOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_FDETECT, ("Force Detect"));
			::AppendMenu(hgsOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_R89SILT, ("Mute Register 89"));
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_MONOMIX, ("Mono Mixed"));
			::AppendMenu(hgsOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_FIXRATE, ("Fixed Render Rate"));
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_IRQFORC, ("Force Master IRQ"));
			::AppendMenu(hgsOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_IRQUNMS, ("Unmask IRQ"));
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_IRQUNDM, ("Unmask DMA"));
			::AppendMenu(hgsOptionen, MF_SEPARATOR, 0, "");
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_POLLING, ("DMA Control Polling"));
			::AppendMenu(hgsOptionen, MF_STRING, ICNMNU_GUS_CLEAIRQ, ("DMA Clear Ter.Count"));
			::AppendMenu(hSysMenu, MF_SEPARATOR, 0, "");
		}
	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpCDA, ("Volume: CDRom Audio"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD100, ("Set Volume: 100%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD090, ("Set Volume: 090%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD080, ("Set Volume: 080%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD070, ("Set Volume: 070%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD060, ("Set Volume: 060%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD050, ("Set Volume: 050%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD040, ("Set Volume: 040%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD030, ("Set Volume: 030%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD020, ("Set Volume: 020%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD010, ("Set Volume: 010%"));
		::AppendMenu(hSoundOpCDA, MF_STRING, ICNMNU_SND_VOL_CD000, ("Set Volume: Mute"));
	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpFM, ("Volume: Adlib (FM)"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM100, ("Set Volume: 100%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM090, ("Set Volume: 090%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM080, ("Set Volume: 080%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM070, ("Set Volume: 070%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM060, ("Set Volume: 060%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM050, ("Set Volume: 050%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM040, ("Set Volume: 040%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM030, ("Set Volume: 030%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM020, ("Set Volume: 020%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM010, ("Set Volume: 010%"));
		::AppendMenu(hSoundOpFM, MF_STRING, ICNMNU_SND_VOL_FM000, ("Set Volume: Mute"));

	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpMT, ("Volume: Roland MT32"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT100, ("Set Volume: 100%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT090, ("Set Volume: 090%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT080, ("Set Volume: 080%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT070, ("Set Volume: 070%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT060, ("Set Volume: 060%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT050, ("Set Volume: 050%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT040, ("Set Volume: 040%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT030, ("Set Volume: 030%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT020, ("Set Volume: 020%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT010, ("Set Volume: 010%"));
		::AppendMenu(hSoundOpMT, MF_STRING, ICNMNU_SND_VOL_MT000, ("Set Volume: Mute"));

		if (useSoundDisney) {
			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpDS, ("Volume: Disney Sound/ Covox"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpDS, MF_STRING, ICNMNU_SND_VOL_DS000, ("Set Volume: Mute"));
		}
		if (useSoundTandy) {
			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpTD, ("Volume: Tandy 1000/2000"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpTD, MF_STRING, ICNMNU_SND_VOL_TD000, ("Set Volume: Mute"));
		}
		if (useSoundSSI2k1) {
			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpINV, ("Volume: Innova SSI2001 (SID)"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpINV, MF_STRING, ICNMNU_SND_VOL_IV000, ("Set Volume: Mute"));
		}
		if (useSoundPS1) {
			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpPS1, ("Volume: IBM PS/1 Audio"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpPS1, MF_STRING, ICNMNU_SND_VOL_PS1000, ("Set Volume: Mute"));

			::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpPSD, ("Volume: IBM PS/1 DAC"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD100, ("Set Volume: 100%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD090, ("Set Volume: 090%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD080, ("Set Volume: 080%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD070, ("Set Volume: 070%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD060, ("Set Volume: 060%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD050, ("Set Volume: 050%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD040, ("Set Volume: 040%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD030, ("Set Volume: 030%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD020, ("Set Volume: 020%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD010, ("Set Volume: 010%"));
			::AppendMenu(hSoundOpPSD, MF_STRING, ICNMNU_SND_VOL_PSD000, ("Set Volume: Mute"));
		}
	::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSoundOpSPK, ("Volume: PC Speaker"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK100, ("Set Volume: 100%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK090, ("Set Volume: 090%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK080, ("Set Volume: 080%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK070, ("Set Volume: 070%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK060, ("Set Volume: 060%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK050, ("Set Volume: 050%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK040, ("Set Volume: 040%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK030, ("Set Volume: 030%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK020, ("Set Volume: 020%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK010, ("Set Volume: 010%"));
		::AppendMenu(hSoundOpSPK, MF_STRING, ICNMNU_SND_VOL_SPK000, ("Set Volume: Mute"));

	::AppendMenu(hSysMenu, MF_SEPARATOR, 0, "");
		::AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hJoystickOp, ("Joystick: Settings"));
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_AUTOFR, ("Autofire"));
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_BUWRAP, ("Button Wrapping"));
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_CIRCIN, ("Circular Input to Square"));

		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_TIMEED, ("Axis Timed Intervals"));
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_SWAP34, ("Swap 3th and 4Th Axis"));
		::AppendMenu(hJoystickOp, MF_SEPARATOR, 0, "");
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_AXIS01, ("Invert Joystick Axis 1"));
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_AXIS02, ("Invert Joystick Axis 2"));
		::AppendMenu(hJoystickOp, MF_STRING, ICNMNU_JOY_INV_AXIS03, ("Invert Joystick Axis 3"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	 	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_RESTART		, ("&Maschine: Restart\tCtrl+Alt+Home"));		
	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");		
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hfinternet	, ("In the Internet "));	
		::AppendMenu(hfinternet	, MF_STRING			, ICNMNU_WWWDOSBOX		, ("DOSBox Official" ));
		::AppendMenu(hfinternet	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hfinternet	, MF_STRING			, ICNMNU_WWWVOGONS		, ("Vogons Forum" 	 ));
		::AppendMenu(hfinternet	, MF_STRING			, ICNMNU_WWWGITHUB		, ("DOSBox Optionals"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_ABOUTOPTLS		, ("&... About ..."));
	
