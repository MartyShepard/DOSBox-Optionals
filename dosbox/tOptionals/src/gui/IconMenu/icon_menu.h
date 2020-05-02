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

#define ICNMNU_CCYCAUTO 		 (ICNMNU +75 )
#define ICNMNU_CCYCi8088_716 	 (ICNMNU +76 )
#define ICNMNU_CCYCi8088_954 	 (ICNMNU +77 )
#define ICNMNU_CCYCi286_10 	 (ICNMNU +78 )
#define ICNMNU_CCYCi286_12	 (ICNMNU +79 )
#define ICNMNU_CCYCi286_16 	 (ICNMNU +80 )
#define ICNMNU_CCYCi286_20 	 (ICNMNU +81 )
#define ICNMNU_CCYCi286_25 	 (ICNMNU +82 )
#define ICNMNU_CCYCi386dx_25 	 (ICNMNU +83 )
#define ICNMNU_CCYCi386dx_33 	 (ICNMNU +84 )
#define ICNMNU_CCYCi386dx_40 	 (ICNMNU +85 )
#define ICNMNU_CCYCi486sx_25 	 (ICNMNU +86 )
#define ICNMNU_CCYCi486dx_33 	 (ICNMNU +87 )
#define ICNMNU_CCYCi486sx_33 	 (ICNMNU +88 )
#define ICNMNU_CCYCi486sx_40 	 (ICNMNU +89 )
#define ICNMNU_CCYCi486dx_50 	 (ICNMNU +90 )
#define ICNMNU_CCYCi486dx2_66  (ICNMNU +91 )
#define ICNMNU_CCYCi486sx2_80  (ICNMNU +92 )
#define ICNMNU_CCYCi486dx2_100 (ICNMNU +93 )
#define ICNMNU_CCYCi486dx4_100 (ICNMNU +94 )
#define ICNMNU_CCYCi486dx4_120 (ICNMNU +95 )
#define ICNMNU_CCYCp60 		 (ICNMNU +96 )
#define ICNMNU_CCYCp75 		 (ICNMNU +97 )
#define ICNMNU_CCYCp100 		 (ICNMNU +98 )
#define ICNMNU_CCYCMAX100		 (ICNMNU +99 )
#define ICNMNU_CCYCMAX150		 (ICNMNU +100 )
#define ICNMNU_CCYCMAX200		 (ICNMNU +101 )
#define ICNMNU_CCYCMAX250		 (ICNMNU +102 )

#define ICNMNU_CTYPE_AUTO (ICNMNU +103 )
#define ICNMNU_CTYPE_386  (ICNMNU +104 )
#define ICNMNU_CTYPE_386P (ICNMNU +105 )
#define ICNMNU_CTYPE_386S (ICNMNU +106 )
#define ICNMNU_CTYPE_486S (ICNMNU +107 )
#define ICNMNU_CTYPE_PENS (ICNMNU +108 )
#define ICNMNU_CTYPE_PPRS (ICNMNU +109 )

#define ICNMNU_CCORE_AUTO (ICNMNU +110 )
#define ICNMNU_CCORE_DYNA (ICNMNU +111 )
#define ICNMNU_CCORE_NORM (ICNMNU +112 )
#define ICNMNU_CCORE_SIMP (ICNMNU +113 )

#define ICNMNU_SBTYPE_SB1	(ICNMNU +114 )
#define ICNMNU_SBTYPE_SB2	(ICNMNU +115 )
#define ICNMNU_SBTYPE_PRO1	(ICNMNU +116 )
#define ICNMNU_SBTYPE_PRO2	(ICNMNU +117 )
#define ICNMNU_SBTYPE_SB16	(ICNMNU +118 )
#define ICNMNU_SBTYPE_SB16V	(ICNMNU +119 )
#define ICNMNU_SBTYPE_GB 	(ICNMNU +120 )
#define ICNMNU_SBTYPE_ESS	(ICNMNU +121 )
#define ICNMNU_SBTYPE_NONE	(ICNMNU +122 )

#define ICNMNU_SBMIXER	    (ICNMNU +123 )
#define ICNMNU_DSPAInit	    (ICNMNU +124 )
#define ICNMNU_GoldPlay	    (ICNMNU +125 )
#define ICNMNU_GoldPlayF    (ICNMNU +126 )
#define ICNMNU_GoldPlayS    (ICNMNU +127 )
#define ICNMNU_StereoCTRL   (ICNMNU +128 )
#define ICNMNU_IOPrtAlias   (ICNMNU +129 )
#define ICNMNU_IDirectDAC   (ICNMNU +130 )
#define ICNMNU_DSPReturn7F  (ICNMNU +131 )

#define ICNMNU_320x240x8  (ICNMNU +132 )
#define ICNMNU_400x300x8  (ICNMNU +133 )
#define ICNMNU_640x480x15 (ICNMNU +134 )
#define ICNMNU_640x480x16 (ICNMNU +135 )
#define ICNMNU_32BitModes (ICNMNU +136 )

#define ICNMNU_ScreenFixFrnt (ICNMNU +137 )
#define ICNMNU_ScreenFixBack (ICNMNU +138 )
#define ICNMNU_LogRegisterNr (ICNMNU +139 )
#define ICNMNU_QuadsDraw_use (ICNMNU +140 )

#define ICNMNU_GUS_ENBALE  (ICNMNU +141 )
#define ICNMNU_GUS_TYPECL  (ICNMNU +142 )
#define ICNMNU_GUS_TYPE37  (ICNMNU +143 )
#define ICNMNU_GUS_TYPEMX  (ICNMNU +144 )
#define ICNMNU_GUS_TYPEIN  (ICNMNU +145 )
#define ICNMNU_GUS_FIXRATE (ICNMNU +146 )
#define ICNMNU_GUS_AUTOAMP (ICNMNU +147 )
#define ICNMNU_GUS_IRQFORC (ICNMNU +148 )
#define ICNMNU_GUS_IRQUNMS (ICNMNU +149 )
#define ICNMNU_GUS_IRQUNDM (ICNMNU +150 )
#define ICNMNU_GUS_POLLING (ICNMNU +151 )
#define ICNMNU_GUS_CLEAIRQ (ICNMNU +152 )
#define ICNMNU_GUS_FDETECT (ICNMNU +153 )
#define ICNMNU_GUS_R89SILT (ICNMNU +154 )
#define ICNMNU_GUS_MONOMIX (ICNMNU +155 )

#define ICNMNU_SBTYPE_S400 (ICNMNU +156 )

#define ICNMNU_EnableASP   (ICNMNU +157 )
#define ICNMNU_SRLimits    (ICNMNU +158 )
#define ICNMNU_UnMaskIRQ   (ICNMNU +159 )
#define ICNMNU_ProStereo   (ICNMNU +160 )

#define ICNMNU_EnableSpeaker  (ICNMNU +161 )

static HMENU hSysMenu	= NULL;
static HMENU hSubFllRes = NULL;
static HMENU hSubWinRes = NULL;
static HMENU hvoodooflt = NULL;
static HMENU hfinternet = NULL;
static HMENU hcpucycles = NULL;	
static HMENU hcputypes  = NULL;	
static HMENU hcpucores  = NULL;	
static HMENU hsbTypen	= NULL;
static HMENU hsbOptionen= NULL;
static HMENU hdbOptionen= NULL;
static HMENU hgsTypen	= NULL;
static HMENU hgsOptionen= NULL;
		
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

	//CheckMenuItem(hsbtypes, 15, 0x00000008L);//(control->CHECKMARK("sblaster", "mixer")? MF_CHECKED: MF_UNCHECKED) );
	
    switch (uiMsg)
    {
		case WM_NCHITTEST:
                return DefWindowProc(hwnd, uiMsg, wparam, lparam);		
		case WM_INITMENU:
		{
			MENUITEMINFO menuItem = {0};
			menuItem.cbSize = sizeof(MENUITEMINFO);
			menuItem.fMask = MIIM_STATE;	

			menuItem.fState = control->CHECKMARK("dosbox", "S3Screen_Fix_320x240x8");			
			SetMenuItemInfo(hdbOptionen, ICNMNU_320x240x8, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("dosbox", "S3Screen_Fix_400x300x8");			
			SetMenuItemInfo(hdbOptionen, ICNMNU_400x300x8, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("dosbox", "S3Screen_Fix_640x480x15");			
			SetMenuItemInfo(hdbOptionen, ICNMNU_640x480x15, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("dosbox", "S3Screen_Fix_640x480x16");			
			SetMenuItemInfo(hdbOptionen, ICNMNU_640x480x16, FALSE, &menuItem);		
			
			menuItem.fState = control->CHECKMARK("dosbox", "VesaVbe1.2_32Bit_Modes");			
			SetMenuItemInfo(hdbOptionen, ICNMNU_32BitModes, FALSE, &menuItem);
					
			menuItem.fState = control->CHECKMARK("pci", "LFB_ScreenFixFrnt");
			SetMenuItemInfo(hvoodooflt, ICNMNU_ScreenFixFrnt, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "LFB_ScreenFixBack");
			SetMenuItemInfo(hvoodooflt, ICNMNU_ScreenFixBack, FALSE, &menuItem);			
			
			menuItem.fState = control->CHECKMARK("pci", "LFB_LogRegisterNr");
			SetMenuItemInfo(hvoodooflt, ICNMNU_LogRegisterNr, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("pci", "gl_QuadsDraw_use");
			SetMenuItemInfo(hvoodooflt, ICNMNU_QuadsDraw_use, FALSE, &menuItem);	
						
			menuItem.fState = control->CHECKMARK("sblaster", "sbmixer");			
			SetMenuItemInfo(hsbOptionen, ICNMNU_SBMIXER, FALSE, &menuItem);
							
			menuItem.fState = control->CHECKMARK("sblaster", "DSP.ForceAutoInit");			
			SetMenuItemInfo(hsbOptionen, ICNMNU_DSPAInit, FALSE, &menuItem);
	
			menuItem.fState = control->CHECKMARK("sblaster", "GoldPlay");			
			SetMenuItemInfo(hsbOptionen, ICNMNU_GoldPlay, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("sblaster", "GoldPlay.Force");
			SetMenuItemInfo(hsbOptionen, ICNMNU_GoldPlayF, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("sblaster", "GoldPlay.Stereo");
			SetMenuItemInfo(hsbOptionen, ICNMNU_GoldPlayS, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("sblaster", "SBPro.StereoCtrl");
			SetMenuItemInfo(hsbOptionen, ICNMNU_StereoCTRL, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("sblaster", "I/O.PortAliasing");
			SetMenuItemInfo(hsbOptionen, ICNMNU_IOPrtAlias, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("sblaster", "Enable.BrokenCode");
			SetMenuItemInfo(hsbOptionen, ICNMNU_IDirectDAC, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("sblaster", "DSP.ForceReturn");
			SetMenuItemInfo(hsbOptionen, ICNMNU_DSPReturn7F, FALSE, &menuItem);			
	
			menuItem.fState = control->CHECKMARK("sblaster", "Enable.ASP/CSP");
			SetMenuItemInfo(hsbOptionen, ICNMNU_EnableASP, FALSE, &menuItem);	
			
			menuItem.fState = control->CHECKMARK("sblaster", "LimitSampleRates");
			SetMenuItemInfo(hsbOptionen, ICNMNU_SRLimits, FALSE, &menuItem);			

			menuItem.fState = control->CHECKMARK("sblaster", "SBHack-IRQ.UnMask");
			SetMenuItemInfo(hsbOptionen, ICNMNU_UnMaskIRQ, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("sblaster", "SBPro.StereoBitSet");
			SetMenuItemInfo(hsbOptionen, ICNMNU_ProStereo, FALSE, &menuItem);	

			menuItem.fState = control->CHECKMARK("sblaster", "Activate.Speaker");
			SetMenuItemInfo(hsbOptionen, ICNMNU_EnableSpeaker, FALSE, &menuItem);			
			
			menuItem.fState = control->CHECKMARK("gus", "gus");
			SetMenuItemInfo(hgsTypen, ICNMNU_GUS_ENBALE, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("gus", "fixrate");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_FIXRATE, FALSE, &menuItem);			

			menuItem.fState = control->CHECKMARK("gus", "AutoAmp");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_AUTOAMP, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("gus", "ForceMIRQ");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_IRQFORC, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("gus", "UnMaskIRQ");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_IRQUNMS, FALSE, &menuItem);	
			
			menuItem.fState = control->CHECKMARK("gus", "UnMaskDMA");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_IRQUNDM, FALSE, &menuItem);	

			menuItem.fState = control->CHECKMARK("gus", "EnCPolling");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_POLLING, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("gus", "ClearTcIRQ");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_CLEAIRQ, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("gus", "ForceDetect");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_FDETECT, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("gus", "R89Silent");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_R89SILT, FALSE, &menuItem);
			
			menuItem.fState = control->CHECKMARK("gus", "MonoMixed");
			SetMenuItemInfo(hgsOptionen, ICNMNU_GUS_MONOMIX, FALSE, &menuItem);
			
		}
		case WM_SYSCOMMAND:
		{					
			switch (wparam)
			{
				case ICNMNU:		
				{}	
			
				case ICNMNU_ScreenFixFrnt  :{control->ChangeOnTheFly("pci",  "LFB_ScreenFixFrnt","BOOL"	);}return 0;
				case ICNMNU_ScreenFixBack  :{control->ChangeOnTheFly("pci",  "LFB_ScreenFixBack","BOOL"	);}return 0;
				case ICNMNU_LogRegisterNr  :{control->ChangeOnTheFly("pci",  "LFB_LogRegisterNr","BOOL"	);}return 0;
				case ICNMNU_QuadsDraw_use  :{control->ChangeOnTheFly("pci",  "gl_QuadsDraw_use", "BOOL"	);}return 0;

				case ICNMNU_320x240x8      :{control->ChangeOnTheFly("dosbox",  "S3Screen_Fix_320x240x8",  "BOOL"	);}return 0;
				case ICNMNU_400x300x8      :{control->ChangeOnTheFly("dosbox",  "S3Screen_Fix_400x300x8",  "BOOL"	);}return 0;
				case ICNMNU_640x480x15     :{control->ChangeOnTheFly("dosbox",  "S3Screen_Fix_640x480x15", "BOOL"	);}return 0;
				case ICNMNU_640x480x16     :{control->ChangeOnTheFly("dosbox",  "S3Screen_Fix_640x480x16", "BOOL"	);}return 0;
				case ICNMNU_32BitModes     :{control->ChangeOnTheFly("dosbox",  "VesaVbe1.2_32Bit_Modes",  "BOOL"	);}return 0;
				
				case ICNMNU_SBMIXER	       :{control->ChangeOnTheFly("sblaster",  "sbmixer",    "BOOL"	);}return 0;
				case ICNMNU_DSPAInit       :{control->ChangeOnTheFly("sblaster",  "DSP.ForceAutoInit",   "BOOL"	);}return 0;
				case ICNMNU_GoldPlay       :{control->ChangeOnTheFly("sblaster",  "GoldPlay",   "BOOL"	);}return 0;
				case ICNMNU_GoldPlayF	   :{control->ChangeOnTheFly("sblaster",  "GoldPlay.Force",    "BOOL"	);}return 0;
				case ICNMNU_GoldPlayS      :{control->ChangeOnTheFly("sblaster",  "GoldPlay.Stereo",   "BOOL"	);}return 0;
				case ICNMNU_StereoCTRL     :{control->ChangeOnTheFly("sblaster",  "SBPro.StereoCtrl", "BOOL"	);}return 0;
				case ICNMNU_IOPrtAlias	   :{control->ChangeOnTheFly("sblaster",  "I/O.PortAliasing", "BOOL"	);}return 0;
				case ICNMNU_IDirectDAC     :{control->ChangeOnTheFly("sblaster",  "Enable.BrokenCode", "BOOL"	);}return 0;
				case ICNMNU_DSPReturn7F    :{control->ChangeOnTheFly("sblaster",  "DSP.ForceReturn","BOOL"	);}return 0;	
				case ICNMNU_EnableASP	   :{control->ChangeOnTheFly("sblaster",  "Enable.ASP/CSP",  "BOOL"	);}return 0;
				case ICNMNU_SRLimits	   :{control->ChangeOnTheFly("sblaster",  "LimitSampleRates",   "BOOL"	);}return 0;				
				case ICNMNU_UnMaskIRQ	   :{control->ChangeOnTheFly("sblaster",  "SBHack-IRQ.UnMask",	"BOOL"	);}return 0;
				case ICNMNU_ProStereo	   :{control->ChangeOnTheFly("sblaster",  "SBPro.StereoBitSet",	"BOOL"	);}return 0;
				case ICNMNU_EnableSpeaker  :{control->ChangeOnTheFly("sblaster",  "Activate.Speaker","BOOL");}return 0;				
				
				case ICNMNU_SBTYPE_SB1 	   :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "sb1" 		);}return 0;
				case ICNMNU_SBTYPE_SB2     :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "sb2" 		);}return 0;
				case ICNMNU_SBTYPE_PRO1    :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "sbpro1" 		);}return 0;
				case ICNMNU_SBTYPE_PRO2    :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "sbpro2" 		);}return 0;
				case ICNMNU_SBTYPE_SB16    :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "sb16" 		);}return 0;
				case ICNMNU_SBTYPE_SB16V   :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "sb16vibra"	);}return 0;
				case ICNMNU_SBTYPE_GB      :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "gb" 			);}return 0;
				case ICNMNU_SBTYPE_ESS     :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "ess688" 		);}return 0;
				case ICNMNU_SBTYPE_NONE    :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "none" 		);}return 0;				
				case ICNMNU_SBTYPE_S400    :{ control->ChangeOnTheFly("sblaster",  "sbtype",  "reveal_sc400");}return 0;				
								
				case ICNMNU_GUS_ENBALE 	   :{ control->ChangeOnTheFly("gus",  "gus",	  	"BOOL" 		);}return 0;
				case ICNMNU_GUS_TYPECL     :{ control->ChangeOnTheFly("gus",  "gustype",  	"classic" 	);}return 0;
				case ICNMNU_GUS_TYPE37     :{ control->ChangeOnTheFly("gus",  "gustype",  	"classic37" );}return 0;
				case ICNMNU_GUS_TYPEMX     :{ control->ChangeOnTheFly("gus",  "gustype",  	"max" 		);}return 0;
				case ICNMNU_GUS_TYPEIN     :{ control->ChangeOnTheFly("gus",  "gustype",  	"interwave" );}return 0;
				case ICNMNU_GUS_FIXRATE    :{ control->ChangeOnTheFly("gus",  "fixrate",  	"BOOL"		);}return 0;
				case ICNMNU_GUS_AUTOAMP    :{ control->ChangeOnTheFly("gus",  "AutoAmp",  	"BOOL" 		);}return 0;
				case ICNMNU_GUS_IRQFORC    :{ control->ChangeOnTheFly("gus",  "ForceMIRQ",	"BOOL" 		);}return 0;
				case ICNMNU_GUS_IRQUNMS    :{ control->ChangeOnTheFly("gus",  "UnMaskIRQ",	"BOOL" 		);}return 0;								
				case ICNMNU_GUS_IRQUNDM    :{ control->ChangeOnTheFly("gus",  "UnMaskDMA",	"BOOL" 		);}return 0;
				case ICNMNU_GUS_POLLING    :{ control->ChangeOnTheFly("gus",  "EnCPolling",	"BOOL" 		);}return 0;
				case ICNMNU_GUS_CLEAIRQ    :{ control->ChangeOnTheFly("gus",  "ClearTcIRQ", "BOOL" 		);}return 0;
				case ICNMNU_GUS_FDETECT    :{ control->ChangeOnTheFly("gus",  "ForceDetect","BOOL" 		);}return 0;
				case ICNMNU_GUS_R89SILT    :{ control->ChangeOnTheFly("gus",  "R89Silent",  "BOOL" 		);}return 0;
				case ICNMNU_GUS_MONOMIX    :{ control->ChangeOnTheFly("gus",  "MonoMixed",  "BOOL"		);}return 0;
		
				case ICNMNU_CCYCAUTO 	   :{ control->ChangeOnTheFly("cpu",  "cycles",  "auto" 		);}return 0;
				case ICNMNU_CCYCi8088_716  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i8088_716" 	);}return 0;
				case ICNMNU_CCYCi8088_954  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i8088_954" 	);}return 0;
				case ICNMNU_CCYCi286_10    :{ control->ChangeOnTheFly("cpu",  "cycles",  "i286_10" 		);}return 0;
				case ICNMNU_CCYCi286_12    :{ control->ChangeOnTheFly("cpu",  "cycles",  "i286_12" 		);}return 0;
				case ICNMNU_CCYCi286_16    :{ control->ChangeOnTheFly("cpu",  "cycles",  "i286_16" 		);}return 0;
				case ICNMNU_CCYCi286_20    :{ control->ChangeOnTheFly("cpu",  "cycles",  "i286_20" 		);}return 0;
				case ICNMNU_CCYCi286_25    :{ control->ChangeOnTheFly("cpu",  "cycles",  "i286_25" 		);}return 0;
				case ICNMNU_CCYCi386dx_25  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i386dx_25" 	);}return 0;
				case ICNMNU_CCYCi386dx_33  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i386dx_33" 	);}return 0;
				case ICNMNU_CCYCi386dx_40  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i386dx_40" 	);}return 0;
				case ICNMNU_CCYCi486sx_25  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486sx_25" 	);}return 0;
				case ICNMNU_CCYCi486dx_33  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486dx_33" 	);}return 0;
				case ICNMNU_CCYCi486sx_33  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486sx_33" 	);}return 0;
				case ICNMNU_CCYCi486sx_40  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486sx_40" 	);}return 0;
				case ICNMNU_CCYCi486dx_50  :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486dx_50" 	);}return 0;				
				case ICNMNU_CCYCi486dx2_66 :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486dx2_66" 	);}return 0;
				case ICNMNU_CCYCi486sx2_80 :{ control->ChangeOnTheFly("cpu",  "cycles",  "i486sx2_80" 	);}return 0;
				case ICNMNU_CCYCi486dx2_100:{ control->ChangeOnTheFly("cpu",  "cycles",  "i486dx2_100" 	);}return 0;
				case ICNMNU_CCYCi486dx4_100:{ control->ChangeOnTheFly("cpu",  "cycles",  "i486dx4_100" 	);}return 0;
				case ICNMNU_CCYCi486dx4_120:{ control->ChangeOnTheFly("cpu",  "cycles",  "i486dx4_120" 	);}return 0;
				case ICNMNU_CCYCp60		   :{ control->ChangeOnTheFly("cpu",  "cycles",  "p60" 			);}return 0;
				case ICNMNU_CCYCp75		   :{ control->ChangeOnTheFly("cpu",  "cycles",  "p75" 			);}return 0;
				case ICNMNU_CCYCp100	   :{ control->ChangeOnTheFly("cpu",  "cycles",  "p75" 			);}return 0;
				case ICNMNU_CCYCMAX100	   :{ control->ChangeOnTheFly("cpu",  "cycles",  "max" 			);}return 0;
				case ICNMNU_CCYCMAX150	   :{ control->ChangeOnTheFly("cpu",  "cycles",  "max %150" 	);}return 0;
				case ICNMNU_CCYCMAX200	   :{ control->ChangeOnTheFly("cpu",  "cycles",  "max %200" 	);}return 0;
				case ICNMNU_CCYCMAX250	   :{ control->ChangeOnTheFly("cpu",  "cycles",  "max %250" 	);}return 0;									

				case ICNMNU_CCORE_AUTO	   :{ control->ChangeOnTheFly("cpu",  "core", "auto" 	);}return 0;
				case ICNMNU_CCORE_DYNA	   :{ control->ChangeOnTheFly("cpu",  "core", "dynamic" );}return 0;
				case ICNMNU_CCORE_NORM	   :{ control->ChangeOnTheFly("cpu",  "core", "normal" 	);}return 0;
				case ICNMNU_CCORE_SIMP	   :{ control->ChangeOnTheFly("cpu",  "core", "simple" 	);}return 0;
			
				case ICNMNU_CTYPE_AUTO	   :{ control->ChangeOnTheFly("cpu",  "cputype", "auto" 			);}return 0;
				case ICNMNU_CTYPE_386	   :{ control->ChangeOnTheFly("cpu",  "cputype", "386" 				);}return 0;
				case ICNMNU_CTYPE_386P	   :{ control->ChangeOnTheFly("cpu",  "cputype", "386_prefetch" 	);}return 0;
				case ICNMNU_CTYPE_386S	   :{ control->ChangeOnTheFly("cpu",  "cputype", "386_slow" 		);}return 0;
				case ICNMNU_CTYPE_486S	   :{ control->ChangeOnTheFly("cpu",  "cputype", "486_slow" 		);}return 0;
				case ICNMNU_CTYPE_PENS	   :{ control->ChangeOnTheFly("cpu",  "cputype", "pentium_slow"		);}return 0;
				case ICNMNU_CTYPE_PPRS	   :{ control->ChangeOnTheFly("cpu",  "cputype", "pentiumpro_slow"	);}return 0;
				
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