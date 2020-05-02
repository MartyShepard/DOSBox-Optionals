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

	hSysMenu   = 
	
	::GetSystemMenu(DosboxHwndIcon, FALSE);
	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FULLSCREEN		, ("&FullScreen\tAlt+Enter"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	 	
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_BINDMAPPER		, ("&Key && Joystick Mapper\tCtrl+F1"));	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_PREFSEDIT		, ("Edit Preferences Direct"));		
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_DISKCACHE		, ("&Cache: Swap Next Floppy\tCtrl+F4"));		
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CDROMSWAP		, ("&Cache: Swap Next Cd-Rom\tCtrl+F3"));	
	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");		
	//::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_INTERNALUI		, ("Internal Preferences"));	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CAPSHOTSW		, ("&Capture: Screenshot [SW]\tCtrl+F5"));	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CAPSHOTHWBMP	, ("&Capture: Screenshot [OpenGL] (BMP)"));
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CAPSHOTHWPNG	, ("&Capture: Screenshot [OpenGL] (PNG)"));	
	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	
	
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
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1360X768	, ("\t1360x768"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1366X768	, ("\t1366x768"		));	
		::AppendMenu(hSubWinRes	, MF_STRING			, ICNMNU_WIN1440X900	, ("\t1440x900"		));	
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
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1360X768	, ("\t1360x768"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1366X768	, ("\t1366x768"		));	
		::AppendMenu(hSubFllRes	, MF_STRING			, ICNMNU_FUL1440X900	, ("\t1440x900"		));	
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

	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hdbOptionen	, ("Resolution Fixes"));		
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_320x240x8		, ("320x240x4 <-> 320x200x8"));	
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_400x300x8		, ("400x300x8 <-> 320x480x8"));	
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_640x480x16		, ("640x480x16 <-> 320x480x8"));
		::AppendMenu(hdbOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_640x480x15		, ("Stretch 640x480x15"));
		::AppendMenu(hdbOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hdbOptionen, MF_STRING			, ICNMNU_32BitModes		, ("Use 32Bit Modes"));	

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
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_LogRegisterNr	, ("LFB LOG Register Nr."  ));
		::AppendMenu(hvoodooflt	, MF_SEPARATOR			, 0, "");			
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_QuadsDraw_use	, ("OpenGL QuadsDraw"	   ));	
					
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hcpucores	, ("CPU: Cores"));		
		::AppendMenu(hcpucores	, MF_STRING				,ICNMNU_CCORE_AUTO		, ("Automatic"));
		::AppendMenu(hcpucores	, MF_SEPARATOR			, 0, "");
		::AppendMenu(hcpucores	, MF_STRING				,ICNMNU_CCORE_SIMP		, ("Simple"));		
		::AppendMenu(hcpucores	, MF_STRING				,ICNMNU_CCORE_NORM		, ("Normal"));
		::AppendMenu(hcpucores	, MF_STRING				,ICNMNU_CCORE_DYNA		, ("Dynamic"));
		
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hcputypes	, ("CPU: Types"));		
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_AUTO		, ("Automatic"));
		::AppendMenu(hcputypes	, MF_SEPARATOR			, 0, "");
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_386	, ("386"));	
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_386P	, ("386 Prefetch"));
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_386S	, ("386 Slow"));
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_486S	, ("486 Slow"));
		::AppendMenu(hcputypes	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_PENS	, ("Pentium"));
		::AppendMenu(hcputypes	, MF_STRING				,ICNMNU_CTYPE_PPRS	, ("Pentium Pro"));		
		
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hcpucycles	, ("CPU: Cycles"));		
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCAUTO		, ("Automatic"));	
		::AppendMenu(hcpucycles	, MF_SEPARATOR			, 0, "");	
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi8088_716	, ("8088 (7,16mhz)"));	
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi8088_954	, ("8088 (9,54mhz)"));
		::AppendMenu(hcpucycles	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi286_10		, ("268    (10mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi286_12		, ("268    (12mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi286_16		, ("268    (16mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi286_20		, ("268    (20mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi286_25		, ("268    (25mhz)"));
		::AppendMenu(hcpucycles	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi386dx_25	, ("368DX  (25mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi386dx_33	, ("368DX  (33mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi386dx_40	, ("368DX  (40mhz)"));
		::AppendMenu(hcpucycles	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486sx_25	, ("468SX  (25mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486dx_33	, ("468DX  (33mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486sx_33	, ("468SX  (33mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486sx_40	, ("468SX  (40mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486dx_50	, ("468DX  (50mhz)"));		
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486dx2_66	, ("468DX/2(66mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486sx2_80	, ("468DX/2(80mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486dx2_100	, ("468DX/2(100mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCi486dx4_100	, ("468DX/4(100mhz)"));
		::AppendMenu(hcpucycles	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCp60			, ("Pentium(60mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCp75			, ("Pentium(75mhz)"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCp100		, ("Pentium(100mhz)"));	
		::AppendMenu(hcpucycles	, MF_SEPARATOR			, 0, "");
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCMAX100		, ("Max"));			
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCMAX150		, ("Max 150%"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCMAX200		, ("Max 200%"));
		::AppendMenu(hcpucycles	, MF_STRING				,ICNMNU_CCYCMAX250		, ("Max 250%"));
		
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");			
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hsbTypen			, ("SB Types"));		
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_SB1		, ("Type: SB1"));		
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_SB2		, ("Type: SB2"));
		::AppendMenu(hsbTypen	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_PRO1		, ("Type: SBPro1"));
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_PRO2		, ("Type: SBPro2"));
		::AppendMenu(hsbTypen	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_SB16		, ("Type: SB16"));
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_SB16V	, ("Type: SB16 VibraC"));
		::AppendMenu(hsbTypen	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_GB		, ("Type: Gameblaster"));
		::AppendMenu(hsbTypen	, MF_SEPARATOR			, 0, "");
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_ESS		, ("Type: ESS 688"));
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_S400		, ("Type: Reveal SC400"));
		::AppendMenu(hsbTypen	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hsbTypen	, MF_STRING				,ICNMNU_SBTYPE_NONE		, ("Type: None"));		

	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hsbOptionen			, ("SB Options"));		
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_SBMIXER			, ("SB Mixer"));	
		::AppendMenu(hsbOptionen, MF_SEPARATOR		, 0, "");			
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_EnableASP		, ("SB16 Advanced Chip"));
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_StereoCTRL		, ("SBPro Stereo Control"));
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_ProStereo		, ("SBPro Stereo (PreSet)"));		
		::AppendMenu(hsbOptionen, MF_SEPARATOR		, 0, "");			
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_GoldPlay		, ("Goldplay Emulation"));		
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_GoldPlayF		, ("Goldplay Force"));			
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_GoldPlayS		, ("Goldplay Stereo"));			
		::AppendMenu(hsbOptionen, MF_SEPARATOR		, 0, "");			
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_EnableSpeaker	, ("Enable Speaker (VM)"));	
		::AppendMenu(hsbOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_DSPAInit		, ("Force DSP AutoInit"));
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_DSPReturn7F		, ("DSP Return 7f/ff"));		
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_IOPrtAlias		, ("IO Port Aliasing"));
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_IDirectDAC		, ("Instant Direct DAC"));
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_UnMaskIRQ		, ("PIC Umnask IRQ"));
		::AppendMenu(hsbOptionen, MF_SEPARATOR		, 0, "");			
		::AppendMenu(hsbOptionen, MF_STRING			,ICNMNU_SRLimits		, ("Sample Rate Limits"));		
			
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hgsTypen		, ("GS Types"));
		::AppendMenu(hgsTypen	, MF_STRING			,ICNMNU_GUS_ENBALE		, ("Enable Ultrasound"));
		::AppendMenu(hgsTypen, MF_SEPARATOR			, 0, "");
		::AppendMenu(hgsTypen	, MF_STRING			,ICNMNU_GUS_TYPECL		, ("Type: Classic (Pre)"));		
		::AppendMenu(hgsTypen	, MF_STRING			,ICNMNU_GUS_TYPE37		, ("Type: Classic v3.74"));		
		::AppendMenu(hgsTypen	, MF_STRING			,ICNMNU_GUS_TYPEMX		, ("Type: UltraSound MAX"));			
		::AppendMenu(hgsTypen	, MF_STRING			,ICNMNU_GUS_TYPEIN		, ("Type: GUS Interweave"));
		
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hgsOptionen	, ("GS Options"));
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_AUTOAMP		, ("Automatic AMP"));
		::AppendMenu(hgsOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_FDETECT		, ("Force Detect"));
		::AppendMenu(hgsOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_R89SILT		, ("Mute Register 89"));
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_MONOMIX		, ("Mono Mixed"));		
		::AppendMenu(hgsOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_FIXRATE		, ("Fixed Render Rate"));		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_IRQFORC		, ("Force Master IRQ"));
		::AppendMenu(hgsOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_IRQUNMS		, ("Unmask IRQ"));		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_IRQUNDM		, ("Unmask DMA"));		
		::AppendMenu(hgsOptionen, MF_SEPARATOR		, 0, "");		
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_POLLING		, ("DMA Control Polling"));
		::AppendMenu(hgsOptionen, MF_STRING			,ICNMNU_GUS_CLEAIRQ		, ("DMA Clear Ter.Count"));
				
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
	
