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
 

	HMENU hSubFllRes = CreatePopupMenu();
	HMENU hSubWinRes = CreatePopupMenu();
	HMENU hvoodooflt = CreatePopupMenu();
	HMENU hfinternet = CreatePopupMenu();	
	
	
	HMENU hSysMenu 	 = 
	
	::GetSystemMenu(DosboxHwndIcon, FALSE);
		
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_FULLSCREEN		, ("&FullScreen\tAlt+Enter"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	

	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hSubWinRes	, ("Resolution: Window"));	
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

	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hvoodooflt	, ("Voodoo: Filter"));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FILT_NONE	, ("None"			));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_POINT	, ("Point"			));		
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_BILINEAR, ("Bilinear"		));			
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_TRILINER, ("Trilinear"		));			
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_ANISOTRC, ("Anisotropic"	));				
		::AppendMenu(hvoodooflt	, MF_STRING			, ICNMNU_VD_FLT_TESTMODE, ("Debug Testmode"	));			
		
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	 	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_RESTART		, ("&Maschine: Restart\tCtrl+Alt+Home"));	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	 	
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_BINDMAPPER		, ("&Key & Joystick Mapper\tCtrl+F1"));	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");	
	
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_DISKCACHE		, ("&Cache: Swap Next Floppy\tCtrl+F4"));		
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_CDROMSWAP		, ("&Cache: Swap Next Cd-Rom\tCtrl+F3"));		
	
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");		
	::AppendMenu(hSysMenu	, MF_STRING|MF_POPUP	,(UINT_PTR)hfinternet	, ("In the Internet "));	
		::AppendMenu(hfinternet	, MF_STRING			, ICNMNU_WWWDOSBOX		, ("DOSBox Official" ));
		::AppendMenu(hfinternet	, MF_SEPARATOR			, 0, "");		
		::AppendMenu(hfinternet	, MF_STRING			, ICNMNU_WWWVOGONS		, ("Vogons Forum" 	 ));
		::AppendMenu(hfinternet	, MF_STRING			, ICNMNU_WWWGITHUB		, ("DOSBox Optionals"));
	::AppendMenu(hSysMenu	, MF_SEPARATOR			, 0, "");
	::AppendMenu(hSysMenu	, MF_STRING				, ICNMNU_ABOUTOPTLS		, ("&... About ..."));	
	