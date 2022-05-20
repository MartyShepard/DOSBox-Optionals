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

#include <mixer.h>
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

#define ICNMNU_SND_VOL_CD100 (ICNMNU +162 )
#define ICNMNU_SND_VOL_CD090 (ICNMNU +163 )
#define ICNMNU_SND_VOL_CD080 (ICNMNU +164 )
#define ICNMNU_SND_VOL_CD070 (ICNMNU +165 )
#define ICNMNU_SND_VOL_CD060 (ICNMNU +166 )
#define ICNMNU_SND_VOL_CD050 (ICNMNU +167 )
#define ICNMNU_SND_VOL_CD040 (ICNMNU +168 )
#define ICNMNU_SND_VOL_CD030 (ICNMNU +169 )
#define ICNMNU_SND_VOL_CD020 (ICNMNU +170 )
#define ICNMNU_SND_VOL_CD010 (ICNMNU +171 )
#define ICNMNU_SND_VOL_CD000 (ICNMNU +172 )

#define ICNMNU_SND_VOL_SB100 (ICNMNU +173 )
#define ICNMNU_SND_VOL_SB090 (ICNMNU +174 )
#define ICNMNU_SND_VOL_SB080 (ICNMNU +175 )
#define ICNMNU_SND_VOL_SB070 (ICNMNU +176 )
#define ICNMNU_SND_VOL_SB060 (ICNMNU +177 )
#define ICNMNU_SND_VOL_SB050 (ICNMNU +178 )
#define ICNMNU_SND_VOL_SB040 (ICNMNU +179 )
#define ICNMNU_SND_VOL_SB030 (ICNMNU +180 )
#define ICNMNU_SND_VOL_SB020 (ICNMNU +181 )
#define ICNMNU_SND_VOL_SB010 (ICNMNU +182 )
#define ICNMNU_SND_VOL_SB000 (ICNMNU +183 )

#define ICNMNU_SND_VOL_GS100 (ICNMNU +184 )
#define ICNMNU_SND_VOL_GS090 (ICNMNU +185 )
#define ICNMNU_SND_VOL_GS080 (ICNMNU +186 )
#define ICNMNU_SND_VOL_GS070 (ICNMNU +187 )
#define ICNMNU_SND_VOL_GS060 (ICNMNU +188 )
#define ICNMNU_SND_VOL_GS050 (ICNMNU +189 )
#define ICNMNU_SND_VOL_GS040 (ICNMNU +190 )
#define ICNMNU_SND_VOL_GS030 (ICNMNU +191 )
#define ICNMNU_SND_VOL_GS020 (ICNMNU +192 )
#define ICNMNU_SND_VOL_GS010 (ICNMNU +193 )
#define ICNMNU_SND_VOL_GS000 (ICNMNU +194 )

#define ICNMNU_SND_VOL_FM100 (ICNMNU +195 )
#define ICNMNU_SND_VOL_FM090 (ICNMNU +196 )
#define ICNMNU_SND_VOL_FM080 (ICNMNU +197 )
#define ICNMNU_SND_VOL_FM070 (ICNMNU +198 )
#define ICNMNU_SND_VOL_FM060 (ICNMNU +199 )
#define ICNMNU_SND_VOL_FM050 (ICNMNU +200 )
#define ICNMNU_SND_VOL_FM040 (ICNMNU +201 )
#define ICNMNU_SND_VOL_FM030 (ICNMNU +202 )
#define ICNMNU_SND_VOL_FM020 (ICNMNU +203 )
#define ICNMNU_SND_VOL_FM010 (ICNMNU +204 )
#define ICNMNU_SND_VOL_FM000 (ICNMNU +205 )

#define ICNMNU_SND_VOL_MT100 (ICNMNU +206 )
#define ICNMNU_SND_VOL_MT090 (ICNMNU +207 )
#define ICNMNU_SND_VOL_MT080 (ICNMNU +208 )
#define ICNMNU_SND_VOL_MT070 (ICNMNU +209 )
#define ICNMNU_SND_VOL_MT060 (ICNMNU +210 )
#define ICNMNU_SND_VOL_MT050 (ICNMNU +211 )
#define ICNMNU_SND_VOL_MT040 (ICNMNU +212 )
#define ICNMNU_SND_VOL_MT030 (ICNMNU +213 )
#define ICNMNU_SND_VOL_MT020 (ICNMNU +214 )
#define ICNMNU_SND_VOL_MT010 (ICNMNU +215 )
#define ICNMNU_SND_VOL_MT000 (ICNMNU +216 )

#define ICNMNU_SND_VOL_DS100 (ICNMNU +217 )
#define ICNMNU_SND_VOL_DS090 (ICNMNU +218 )
#define ICNMNU_SND_VOL_DS080 (ICNMNU +219 )
#define ICNMNU_SND_VOL_DS070 (ICNMNU +220 )
#define ICNMNU_SND_VOL_DS060 (ICNMNU +221 )
#define ICNMNU_SND_VOL_DS050 (ICNMNU +222 )
#define ICNMNU_SND_VOL_DS040 (ICNMNU +223 )
#define ICNMNU_SND_VOL_DS030 (ICNMNU +224 )
#define ICNMNU_SND_VOL_DS020 (ICNMNU +225 )
#define ICNMNU_SND_VOL_DS010 (ICNMNU +226 )
#define ICNMNU_SND_VOL_DS000 (ICNMNU +227 )

#define ICNMNU_SND_VOL_TD100 (ICNMNU +228 )
#define ICNMNU_SND_VOL_TD090 (ICNMNU +229 )
#define ICNMNU_SND_VOL_TD080 (ICNMNU +230 )
#define ICNMNU_SND_VOL_TD070 (ICNMNU +231 )
#define ICNMNU_SND_VOL_TD060 (ICNMNU +232 )
#define ICNMNU_SND_VOL_TD050 (ICNMNU +233 )
#define ICNMNU_SND_VOL_TD040 (ICNMNU +234 )
#define ICNMNU_SND_VOL_TD030 (ICNMNU +235 )
#define ICNMNU_SND_VOL_TD020 (ICNMNU +236 )
#define ICNMNU_SND_VOL_TD010 (ICNMNU +237 )
#define ICNMNU_SND_VOL_TD000 (ICNMNU +238 )

#define ICNMNU_SND_VOL_IV100 (ICNMNU +239 )
#define ICNMNU_SND_VOL_IV090 (ICNMNU +240 )
#define ICNMNU_SND_VOL_IV080 (ICNMNU +241 )
#define ICNMNU_SND_VOL_IV070 (ICNMNU +242 )
#define ICNMNU_SND_VOL_IV060 (ICNMNU +243 )
#define ICNMNU_SND_VOL_IV050 (ICNMNU +244 )
#define ICNMNU_SND_VOL_IV040 (ICNMNU +245 )
#define ICNMNU_SND_VOL_IV030 (ICNMNU +246 )
#define ICNMNU_SND_VOL_IV020 (ICNMNU +247 )
#define ICNMNU_SND_VOL_IV010 (ICNMNU +248 )
#define ICNMNU_SND_VOL_IV000 (ICNMNU +249 )

#define ICNMNU_SND_VOL_PS1100 (ICNMNU +250 )
#define ICNMNU_SND_VOL_PS1090 (ICNMNU +251 )
#define ICNMNU_SND_VOL_PS1080 (ICNMNU +252 )
#define ICNMNU_SND_VOL_PS1070 (ICNMNU +253 )
#define ICNMNU_SND_VOL_PS1060 (ICNMNU +254 )
#define ICNMNU_SND_VOL_PS1050 (ICNMNU +255 )
#define ICNMNU_SND_VOL_PS1040 (ICNMNU +256 )
#define ICNMNU_SND_VOL_PS1030 (ICNMNU +257 )
#define ICNMNU_SND_VOL_PS1020 (ICNMNU +258 )
#define ICNMNU_SND_VOL_PS1010 (ICNMNU +259 )
#define ICNMNU_SND_VOL_PS1000 (ICNMNU +260 )

#define ICNMNU_SND_VOL_PSD100 (ICNMNU +261 )
#define ICNMNU_SND_VOL_PSD090 (ICNMNU +262 )
#define ICNMNU_SND_VOL_PSD080 (ICNMNU +263 )
#define ICNMNU_SND_VOL_PSD070 (ICNMNU +264 )
#define ICNMNU_SND_VOL_PSD060 (ICNMNU +265 )
#define ICNMNU_SND_VOL_PSD050 (ICNMNU +266 )
#define ICNMNU_SND_VOL_PSD040 (ICNMNU +267 )
#define ICNMNU_SND_VOL_PSD030 (ICNMNU +268 )
#define ICNMNU_SND_VOL_PSD020 (ICNMNU +269 )
#define ICNMNU_SND_VOL_PSD010 (ICNMNU +270 )
#define ICNMNU_SND_VOL_PSD000 (ICNMNU +271 )

#define ICNMNU_SND_VOL_SPK100 (ICNMNU +272 )
#define ICNMNU_SND_VOL_SPK090 (ICNMNU +273 )
#define ICNMNU_SND_VOL_SPK080 (ICNMNU +274 )
#define ICNMNU_SND_VOL_SPK070 (ICNMNU +275 )
#define ICNMNU_SND_VOL_SPK060 (ICNMNU +276 )
#define ICNMNU_SND_VOL_SPK050 (ICNMNU +277 )
#define ICNMNU_SND_VOL_SPK040 (ICNMNU +278 )
#define ICNMNU_SND_VOL_SPK030 (ICNMNU +279 )
#define ICNMNU_SND_VOL_SPK020 (ICNMNU +280 )
#define ICNMNU_SND_VOL_SPK010 (ICNMNU +281 )
#define ICNMNU_SND_VOL_SPK000 (ICNMNU +282 )

#define ICNMNU_JOY_INV_AXIS01 (ICNMNU +283 )
#define ICNMNU_JOY_INV_AXIS02 (ICNMNU +284 )
#define ICNMNU_JOY_INV_AXIS03 (ICNMNU +285 )

#define ICNMNU_JOY_INV_AUTOFR (ICNMNU +286 )
#define ICNMNU_JOY_INV_BUWRAP (ICNMNU +287 )
#define ICNMNU_JOY_INV_CIRCIN (ICNMNU +288 )
#define ICNMNU_JOY_INV_TIMEED (ICNMNU +289 )
#define ICNMNU_JOY_INV_SWAP34 (ICNMNU +290 )

#define ICNMNU_FXCYCLES_01000 (ICNMNU +291 )
#define ICNMNU_FXCYCLES_02000 (ICNMNU +292 )
#define ICNMNU_FXCYCLES_03000 (ICNMNU +293 )
#define ICNMNU_FXCYCLES_04000 (ICNMNU +294 )
#define ICNMNU_FXCYCLES_05000 (ICNMNU +295 )
#define ICNMNU_FXCYCLES_06000 (ICNMNU +296 )
#define ICNMNU_FXCYCLES_07000 (ICNMNU +297 )
#define ICNMNU_FXCYCLES_08000 (ICNMNU +298 )
#define ICNMNU_FXCYCLES_09000 (ICNMNU +299 )
#define ICNMNU_FXCYCLES_10000 (ICNMNU +300 )
#define ICNMNU_FXCYCLES_11000 (ICNMNU +301 )
#define ICNMNU_FXCYCLES_12000 (ICNMNU +302 )
#define ICNMNU_FXCYCLES_13000 (ICNMNU +303 )
#define ICNMNU_FXCYCLES_14000 (ICNMNU +304 )
#define ICNMNU_FXCYCLES_15000 (ICNMNU +305 )
#define ICNMNU_FXCYCLES_16000 (ICNMNU +306 )
#define ICNMNU_FXCYCLES_17000 (ICNMNU +307 )
#define ICNMNU_FXCYCLES_18000 (ICNMNU +308 )
#define ICNMNU_FXCYCLES_19000 (ICNMNU +309 )
#define ICNMNU_FXCYCLES_20000 (ICNMNU +310 )
#define ICNMNU_FXCYCLES_00000 (ICNMNU +311 )

#define ICNMNU_WIN1440X1080  (ICNMNU +312 )
#define ICNMNU_FUL1440X1080  (ICNMNU +313 )

#define ICNMNU_Cache_Delete			 (ICNMNU +314 )
#define ICNMNU_High_Ratio			 (ICNMNU +315 )
#define ICNMNU_GLShade_NONE			 (ICNMNU +316 )
#define ICNMNU_GLShade_FLAT			 (ICNMNU +317 )
#define ICNMNU_GLShade_SMOOTH		 (ICNMNU +318 )
#define ICNMNU_TEXTURE_WRAPS_REPEAT  (ICNMNU +319 )
#define ICNMNU_TEXTURE_WRAPS_REPEATM (ICNMNU +320 )
#define ICNMNU_TEXTURE_WRAPS_CLAMPTE (ICNMNU +321 )
#define ICNMNU_TEXTURE_WRAPS_CLAMPTB (ICNMNU +322 )
#define ICNMNU_TEXTURE_WRAPR_REPEAT  (ICNMNU +323 )
#define ICNMNU_TEXTURE_WRAPR_REPEATM (ICNMNU +324 )
#define ICNMNU_TEXTURE_WRAPR_CLAMPTE (ICNMNU +325 )
#define ICNMNU_TEXTURE_WRAPR_CLAMPTB (ICNMNU +326 )
#define ICNMNU_VOODOO_ASPECT		 (ICNMNU +327 )
#define ICNMNU_FBZCOLORPATHFIX		 (ICNMNU +328 )
#define ICNMNU_GLP_SOOTH_FLAG		 (ICNMNU +329 )
#define ICNMNU_GLL_SOOTH_FLAG		 (ICNMNU +330 )
#define ICNMNU_GLBLENDFC_FLAG		 (ICNMNU +331 )
#define ICNMNU_GLPERSOCORFLAG		 (ICNMNU +332 )
#define ICNMNU_GL_MIPMAP_FLAG		 (ICNMNU +333 )
#define ICNMNU_GL_FOG_FLAG			 (ICNMNU +334 )
#define ICNMNU_COMAPTIBLE_FLAG		 (ICNMNU +335 )
	
#define ICNMNU_GL_BYTE								(ICNMNU +336 )
#define ICNMNU_GL_UNSIGNED_BYTE						(ICNMNU +337 )
#define ICNMNU_GL_SHORT								(ICNMNU +338 )
#define ICNMNU_GL_UNSIGNED_SHORT					(ICNMNU +339 )
#define ICNMNU_GL_INT								(ICNMNU +340 )
#define ICNMNU_GL_UNSIGNED_INT						(ICNMNU +341 )
#define ICNMNU_GL_FLOAT								(ICNMNU +342 )
#define ICNMNU_GL_2_BYTES							(ICNMNU +343 )
#define ICNMNU_GL_3_BYTES							(ICNMNU +344 )
#define ICNMNU_GL_4_BYTES							(ICNMNU +345 )
#define ICNMNU_GL_DOUBLE							(ICNMNU +346 )
#define ICNMNU_GL_HALF_FLOAT						(ICNMNU +347 )
#define ICNMNU_GL_UNSIGNED_BYTE_332					(ICNMNU +348 )
#define ICNMNU_GL_UNSIGNED_SHORT_4444				(ICNMNU +349 )
#define ICNMNU_GL_UNSIGNED_SHORT_5551				(ICNMNU +350 )
#define ICNMNU_GL_UNSIGNED_INT_8888					(ICNMNU +351 )
#define ICNMNU_GL_UNSIGNED_INT_1010102				(ICNMNU +352 )
#define ICNMNU_GL_UNSIGNED_BYTE_233_REV				(ICNMNU +353 )
#define ICNMNU_GL_UNSIGNED_SHORT_565				(ICNMNU +354 )
#define ICNMNU_GL_UNSIGNED_SHORT_565_REV			(ICNMNU +355 )
#define ICNMNU_GL_UNSIGNED_SHORT_4444_REV			(ICNMNU +356 )
#define ICNMNU_GL_UNSIGNED_SHORT_1555_REV			(ICNMNU +357 )
#define ICNMNU_GL_UNSIGNED_INT_8888_REV				(ICNMNU +358 )
#define ICNMNU_GL_UNSIGNED_INT_2101010_REV			(ICNMNU +359 )
#define ICNMNU_GL_UNSIGNED_INT_248					(ICNMNU +360 )
#define ICNMNU_GL_UNSIGNED_INT_10F11F11FREV			(ICNMNU +361 )
#define ICNMNU_GL_UNSIGNED_INT_5999REV				(ICNMNU +362 )
#define ICNMNU_GL_FLOAT_32_UNSIGNED_INT_24_8_REV	(ICNMNU +363 )

#define ICNMNU_GL_RGB4		(ICNMNU +364 )
#define ICNMNU_GL_RGB5		(ICNMNU +365 )
#define ICNMNU_GL_RGB8		(ICNMNU +366 )
#define ICNMNU_GL_RGB10		(ICNMNU +367 )
#define ICNMNU_GL_RGB12		(ICNMNU +368 )
#define ICNMNU_GL_RGB16		(ICNMNU +369 )
#define ICNMNU_GL_RGBA2		(ICNMNU +370 )
#define ICNMNU_GL_RGBA4		(ICNMNU +371 )
#define ICNMNU_GL_RGB5_A1	(ICNMNU +372 )
#define ICNMNU_GL_RGBA8		(ICNMNU +373 )
#define ICNMNU_GL_RGB10_A2	(ICNMNU +374 )
#define ICNMNU_GL_RGBA12	(ICNMNU +375 )
#define ICNMNU_GL_RGBA16	(ICNMNU +376 )
#define ICNMNU_GL_RED		(ICNMNU +377 )
#define ICNMNU_GL_GREEN		(ICNMNU +378 )
#define ICNMNU_GL_BLUE		(ICNMNU +379 )
#define ICNMNU_GL_RGB		(ICNMNU +380 )
#define ICNMNU_GL_BGR		(ICNMNU +381 )
#define ICNMNU_GL_RGBA		(ICNMNU +382 )
#define ICNMNU_GL_BGRA		(ICNMNU +383 )

#define ICNMNU_GL_SCAN0		(ICNMNU +384 )
#define ICNMNU_GL_SCAN1		(ICNMNU +385 )
#define ICNMNU_GL_SCAN2		(ICNMNU +386 )

#define ICNMNU_GL_DARK000	(ICNMNU +387 )
#define ICNMNU_GL_DARK005	(ICNMNU +388 )
#define ICNMNU_GL_DARK015	(ICNMNU +389 )
#define ICNMNU_GL_DARK020	(ICNMNU +390 )
#define ICNMNU_GL_DARK025	(ICNMNU +391 )
#define ICNMNU_GL_DARK030	(ICNMNU +392 )
#define ICNMNU_GL_DARK035	(ICNMNU +393 )
#define ICNMNU_GL_DARK040	(ICNMNU +394 )
#define ICNMNU_GL_DARK045	(ICNMNU +395 )
#define ICNMNU_GL_DARK050	(ICNMNU +396 )
#define ICNMNU_GL_DARK055	(ICNMNU +397 )
#define ICNMNU_GL_DARK060	(ICNMNU +398 )
#define ICNMNU_GL_DARK065	(ICNMNU +399 )
#define ICNMNU_GL_DARK070	(ICNMNU +400 )
#define ICNMNU_GL_DARK075	(ICNMNU +401 )
#define ICNMNU_GL_DARK080	(ICNMNU +402 )
#define ICNMNU_GL_DARK085	(ICNMNU +403 )
#define ICNMNU_GL_DARK090	(ICNMNU +404 )
#define ICNMNU_GL_DARK095	(ICNMNU +405 )
#define ICNMNU_GL_DARK100	(ICNMNU +406 )
#define ICNMNU_GL_DARK105	(ICNMNU +407 )
#define ICNMNU_GL_DARK115	(ICNMNU +408 )
#define ICNMNU_GL_DARK120	(ICNMNU +409 )
#define ICNMNU_GL_DARK125	(ICNMNU +410 )
#define ICNMNU_GL_DARK130	(ICNMNU +411 )
#define ICNMNU_GL_DARK135	(ICNMNU +412 )
#define ICNMNU_GL_DARK140	(ICNMNU +413 )
#define ICNMNU_GL_DARK145	(ICNMNU +414 )
#define ICNMNU_GL_DARK150	(ICNMNU +415 )
#define ICNMNU_GL_DARK155	(ICNMNU +416 )
#define ICNMNU_GL_DARK160	(ICNMNU +417 )
#define ICNMNU_GL_DARK165	(ICNMNU +418 )
#define ICNMNU_GL_DARK170	(ICNMNU +419 )
#define ICNMNU_GL_DARK175	(ICNMNU +420 )
#define ICNMNU_GL_DARK180	(ICNMNU +421 )
#define ICNMNU_GL_DARK185	(ICNMNU +422 )
#define ICNMNU_GL_DARK190	(ICNMNU +423 )
#define ICNMNU_GL_DARK195	(ICNMNU +424 )
#define ICNMNU_GL_DARK200	(ICNMNU +425 )
#define ICNMNU_GL_DARK205	(ICNMNU +426 )
#define ICNMNU_GL_DARK215	(ICNMNU +427 )
#define ICNMNU_GL_DARK220	(ICNMNU +428 )
#define ICNMNU_GL_DARK225	(ICNMNU +429 )
#define ICNMNU_GL_DARK230	(ICNMNU +430 )
#define ICNMNU_GL_DARK235	(ICNMNU +431 )
#define ICNMNU_GL_DARK240	(ICNMNU +432 )
#define ICNMNU_GL_DARK245	(ICNMNU +433 )
#define ICNMNU_GL_DARK250	(ICNMNU +434 )
#define ICNMNU_GL_DARK254	(ICNMNU +435 )
#define ICNMNU_GL_DARK110	(ICNMNU +436 )
#define ICNMNU_GL_DARK010	(ICNMNU +437 )
#define ICNMNU_GL_DARK210	(ICNMNU +438 )
#define ICNMNU_FOPN_CDROM	(ICNMNU +439 ) 
#define ICNMNU_FOPN_FLOPPYA	(ICNMNU +440 ) 
#define ICNMNU_FOPN_FLOPPYB	(ICNMNU +441 )

#define ICNMNU_S3_OVERCLCKA	(ICNMNU +442 )
#define ICNMNU_S3_OVERCLCKB	(ICNMNU +443 )
#define ICNMNU_S3_OVERCLCKC	(ICNMNU +444 )
#define ICNMNU_S3_OVERCLCKD	(ICNMNU +445 )
#define ICNMNU_S3_OVERCLCKE	(ICNMNU +446 )

#define ICNMNU_FOPN_CDROM2	(ICNMNU +447 ) 

#define ICNMNU_GL_SCISSOR_FLAG (ICNMNU +448 ) 

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
static HMENU hSoundOpCDA= NULL;
static HMENU hSoundOpSBL= NULL;
static HMENU hSoundOpGUS= NULL;
static HMENU hSoundOpFM = NULL;
static HMENU hSoundOpMT = NULL;
static HMENU hSoundOpDS = NULL;
static HMENU hSoundOpTD = NULL;
static HMENU hSoundOpINV= NULL;
static HMENU hSoundOpPS1= NULL;
static HMENU hSoundOpPSD= NULL;
static HMENU hSoundOpSPK= NULL;
static HMENU hJoystickOp= NULL;
static HMENU hVoodooSub  = NULL;
static HMENU hVoodooTex = NULL;
static HMENU hVoodooRGB = NULL;

		
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

			menuItem.fState = control->CHECKMARK("joystick", "invertyaxis3");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_AXIS03, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "invertyaxis2");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_AXIS02, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "invertyaxis1");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_AXIS01, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "autofire");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_AUTOFR, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "buttonwrap");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_BUWRAP, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "circularinput");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_CIRCIN, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "timed");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_TIMEED, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("joystick", "swap34");
			SetMenuItemInfo(hJoystickOp, ICNMNU_JOY_INV_SWAP34, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "Cache_Delete_Loop");
			SetMenuItemInfo(hvoodooflt, ICNMNU_Cache_Delete, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "High_Cycles_Ratio");
			SetMenuItemInfo(hvoodooflt, ICNMNU_High_Ratio, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "voodoo_Aspect");
			SetMenuItemInfo(hvoodooflt, ICNMNU_VOODOO_ASPECT, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "sh_FbzcpCca_Sw2");
			SetMenuItemInfo(hvoodooflt, ICNMNU_FBZCOLORPATHFIX, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "glP_Smoth_flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GLP_SOOTH_FLAG, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "glL_Smoth_flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GLL_SOOTH_FLAG, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "glBlendFc_flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GLBLENDFC_FLAG, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "glPersCor_flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GLPERSOCORFLAG, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "glGMipMap_flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GL_MIPMAP_FLAG, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "gl_GLFog__flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GL_FOG_FLAG, FALSE, &menuItem);

			//menuItem.fState = control->CHECKMARK("pci", "compatible_flag");
			//SetMenuItemInfo(hvoodooflt, ICNMNU_COMAPTIBLE_FLAG, FALSE, &menuItem);

			menuItem.fState = control->CHECKMARK("pci", "glScissor_flag");
			SetMenuItemInfo(hvoodooflt, ICNMNU_GL_SCISSOR_FLAG, FALSE, &menuItem);
			

		}
		case WM_SYSCOMMAND:
		{					
			switch (wparam)
			{
				case ICNMNU:		
				{}	

				case ICNMNU_JOY_INV_AXIS01: {control->ChangeOnTheFly("joystick", "invertyaxis1", "BOOL"); }return 0;
				case ICNMNU_JOY_INV_AXIS02: {control->ChangeOnTheFly("joystick", "invertyaxis2", "BOOL"); }return 0;
				case ICNMNU_JOY_INV_AXIS03: {control->ChangeOnTheFly("joystick", "invertyaxis3", "BOOL"); }return 0;

				case ICNMNU_JOY_INV_AUTOFR: {control->ChangeOnTheFly("joystick", "autofire", "BOOL"); }return 0;
				case ICNMNU_JOY_INV_BUWRAP: {control->ChangeOnTheFly("joystick", "buttonwrap", "BOOL"); }return 0;
				case ICNMNU_JOY_INV_CIRCIN: {control->ChangeOnTheFly("joystick", "circularinput", "BOOL"); }return 0;
				case ICNMNU_JOY_INV_TIMEED: {control->ChangeOnTheFly("joystick", "timed", "BOOL"); }return 0;
				case ICNMNU_JOY_INV_SWAP34: {control->ChangeOnTheFly("joystick", "swap34", "BOOL"); }return 0;

				case ICNMNU_SND_VOL_SPK100: { MIXER_UpdateVolCD(1.00f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK090: { MIXER_UpdateVolCD(0.90f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK080: { MIXER_UpdateVolCD(0.80f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK070: { MIXER_UpdateVolCD(0.70f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK060: { MIXER_UpdateVolCD(0.60f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK050: { MIXER_UpdateVolCD(0.50f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK040: { MIXER_UpdateVolCD(0.40f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK030: { MIXER_UpdateVolCD(0.30f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK020: { MIXER_UpdateVolCD(0.20f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK010: { MIXER_UpdateVolCD(0.10f, false, "SPKR"); }return 0;
				case ICNMNU_SND_VOL_SPK000: { MIXER_UpdateVolCD(0.00f, false, "SPKR"); }return 0;

				case ICNMNU_SND_VOL_PSD100: { MIXER_UpdateVolCD(1.00f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD090: { MIXER_UpdateVolCD(0.90f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD080: { MIXER_UpdateVolCD(0.80f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD070: { MIXER_UpdateVolCD(0.70f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD060: { MIXER_UpdateVolCD(0.60f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD050: { MIXER_UpdateVolCD(0.50f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD040: { MIXER_UpdateVolCD(0.40f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD030: { MIXER_UpdateVolCD(0.30f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD020: { MIXER_UpdateVolCD(0.20f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD010: { MIXER_UpdateVolCD(0.10f, false, "PS1 DAC"); }return 0;
				case ICNMNU_SND_VOL_PSD000: { MIXER_UpdateVolCD(0.00f, false, "PS1 DAC"); }return 0;

				case ICNMNU_SND_VOL_PS1100: { MIXER_UpdateVolCD(1.00f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1090: { MIXER_UpdateVolCD(0.90f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1080: { MIXER_UpdateVolCD(0.80f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1070: { MIXER_UpdateVolCD(0.70f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1060: { MIXER_UpdateVolCD(0.60f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1050: { MIXER_UpdateVolCD(0.50f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1040: { MIXER_UpdateVolCD(0.40f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1030: { MIXER_UpdateVolCD(0.30f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1020: { MIXER_UpdateVolCD(0.20f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1010: { MIXER_UpdateVolCD(0.10f, false, "PS1 SN76496"); }return 0;
				case ICNMNU_SND_VOL_PS1000: { MIXER_UpdateVolCD(0.00f, false, "PS1 SN76496"); }return 0;

				case ICNMNU_SND_VOL_IV100: { MIXER_UpdateVolCD(1.00f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV090: { MIXER_UpdateVolCD(0.90f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV080: { MIXER_UpdateVolCD(0.80f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV070: { MIXER_UpdateVolCD(0.70f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV060: { MIXER_UpdateVolCD(0.60f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV050: { MIXER_UpdateVolCD(0.50f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV040: { MIXER_UpdateVolCD(0.40f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV030: { MIXER_UpdateVolCD(0.30f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV020: { MIXER_UpdateVolCD(0.20f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV010: { MIXER_UpdateVolCD(0.10f, false, "INNOVA"); }return 0;
				case ICNMNU_SND_VOL_IV000: { MIXER_UpdateVolCD(0.00f, false, "INNOVA"); }return 0;

				case ICNMNU_SND_VOL_TD100: { MIXER_UpdateVolCD(1.00f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD090: { MIXER_UpdateVolCD(0.90f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD080: { MIXER_UpdateVolCD(0.80f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD070: { MIXER_UpdateVolCD(0.70f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD060: { MIXER_UpdateVolCD(0.60f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD050: { MIXER_UpdateVolCD(0.50f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD040: { MIXER_UpdateVolCD(0.40f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD030: { MIXER_UpdateVolCD(0.30f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD020: { MIXER_UpdateVolCD(0.20f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD010: { MIXER_UpdateVolCD(0.10f, false, "TANDY"); }return 0;
				case ICNMNU_SND_VOL_TD000: { MIXER_UpdateVolCD(0.00f, false, "TANDY"); }return 0;

				case ICNMNU_SND_VOL_DS100: { MIXER_UpdateVolCD(1.00f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS090: { MIXER_UpdateVolCD(0.90f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS080: { MIXER_UpdateVolCD(0.80f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS070: { MIXER_UpdateVolCD(0.70f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS060: { MIXER_UpdateVolCD(0.60f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS050: { MIXER_UpdateVolCD(0.50f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS040: { MIXER_UpdateVolCD(0.40f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS030: { MIXER_UpdateVolCD(0.30f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS020: { MIXER_UpdateVolCD(0.20f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS010: { MIXER_UpdateVolCD(0.10f, false, "DISNEY"); }return 0;
				case ICNMNU_SND_VOL_DS000: { MIXER_UpdateVolCD(0.00f, false, "DISNEY"); }return 0;

				case ICNMNU_SND_VOL_MT100: { MIXER_UpdateVolCD(1.00f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT090: { MIXER_UpdateVolCD(0.90f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT080: { MIXER_UpdateVolCD(0.80f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT070: { MIXER_UpdateVolCD(0.70f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT060: { MIXER_UpdateVolCD(0.60f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT050: { MIXER_UpdateVolCD(0.50f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT040: { MIXER_UpdateVolCD(0.40f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT030: { MIXER_UpdateVolCD(0.30f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT020: { MIXER_UpdateVolCD(0.20f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT010: { MIXER_UpdateVolCD(0.10f, false, "MT32"); }return 0;
				case ICNMNU_SND_VOL_MT000: { MIXER_UpdateVolCD(0.00f, false, "MT32"); }return 0;

				case ICNMNU_SND_VOL_FM100: { MIXER_UpdateVolCD(1.00f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM090: { MIXER_UpdateVolCD(0.90f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM080: { MIXER_UpdateVolCD(0.80f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM070: { MIXER_UpdateVolCD(0.70f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM060: { MIXER_UpdateVolCD(0.60f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM050: { MIXER_UpdateVolCD(0.50f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM040: { MIXER_UpdateVolCD(0.40f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM030: { MIXER_UpdateVolCD(0.30f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM020: { MIXER_UpdateVolCD(0.20f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM010: { MIXER_UpdateVolCD(0.10f, false, "FM"); }return 0;
				case ICNMNU_SND_VOL_FM000: { MIXER_UpdateVolCD(0.00f, false, "FM"); }return 0;

				case ICNMNU_SND_VOL_GS100: { MIXER_UpdateVolCD(1.00f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS090: { MIXER_UpdateVolCD(0.90f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS080: { MIXER_UpdateVolCD(0.80f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS070: { MIXER_UpdateVolCD(0.70f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS060: { MIXER_UpdateVolCD(0.60f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS050: { MIXER_UpdateVolCD(0.50f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS040: { MIXER_UpdateVolCD(0.40f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS030: { MIXER_UpdateVolCD(0.30f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS020: { MIXER_UpdateVolCD(0.20f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS010: { MIXER_UpdateVolCD(0.10f, false, "GUS"); }return 0;
				case ICNMNU_SND_VOL_GS000: { MIXER_UpdateVolCD(0.00f, false, "GUS"); }return 0;

				case ICNMNU_SND_VOL_SB100: { MIXER_UpdateVolCD(1.00f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB090: { MIXER_UpdateVolCD(0.90f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB080: { MIXER_UpdateVolCD(0.80f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB070: { MIXER_UpdateVolCD(0.70f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB060: { MIXER_UpdateVolCD(0.60f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB050: { MIXER_UpdateVolCD(0.50f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB040: { MIXER_UpdateVolCD(0.40f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB030: { MIXER_UpdateVolCD(0.30f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB020: { MIXER_UpdateVolCD(0.20f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB010: { MIXER_UpdateVolCD(0.10f, false, "SB"); }return 0;
				case ICNMNU_SND_VOL_SB000: { MIXER_UpdateVolCD(0.00f, false, "SB"); }return 0;

				case ICNMNU_SND_VOL_CD100: { MIXER_UpdateVolCD(1.00f, false, "CDAUDIO");}return 0;
				case ICNMNU_SND_VOL_CD090: { MIXER_UpdateVolCD(0.90f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD080: { MIXER_UpdateVolCD(0.80f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD070: { MIXER_UpdateVolCD(0.70f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD060: { MIXER_UpdateVolCD(0.60f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD050: { MIXER_UpdateVolCD(0.50f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD040: { MIXER_UpdateVolCD(0.40f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD030: { MIXER_UpdateVolCD(0.30f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD020: { MIXER_UpdateVolCD(0.20f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD010: { MIXER_UpdateVolCD(0.10f, false, "CDAUDIO"); }return 0;
				case ICNMNU_SND_VOL_CD000: { MIXER_UpdateVolCD(0.00f, false, "CDAUDIO"); }return 0;
				
				case ICNMNU_ScreenFixFrnt:	{extVoodoo.bLFBFixFrnt		= control->Menu_ChangeCheck("pci", "LFB_ScreenFixFrnt","BOOL"); FX_Menu(0) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_ScreenFixBack:	{extVoodoo.bLFBFixBack		= control->Menu_ChangeCheck("pci", "LFB_ScreenFixBack","BOOL"); FX_Menu(1) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_LogRegisterNr:	{extVoodoo.bLFBDebugLg		= control->Menu_ChangeCheck("pci", "LFB_LogRegisterNr","BOOL"); FX_Menu(2) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_QuadsDraw_use:	{extVoodoo.gl_QuadsDraw		= control->Menu_ChangeCheck("pci", "gl_QuadsDraw_use" ,"BOOL");	FX_Menu(3) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_Cache_Delete :	{extVoodoo.ChacheDelete		= control->Menu_ChangeCheck("pci", "Cache_Delete_Loop","BOOL"); FX_Menu(4) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_High_Ratio	 :	{extVoodoo.Use3DFXCycles	= control->Menu_ChangeCheck("pci", "High_Cycles_Ratio","BOOL"); FX_Menu(5) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_VOODOO_ASPECT:  {extVoodoo.voodoo_aspect	= control->Menu_ChangeCheck("pci", "voodoo_Aspect"	  ,"BOOL"); FX_Menu(6) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_FBZCOLORPATHFIX:{extVoodoo.sh_FbzcpCca_Sw2  = control->Menu_ChangeCheck("pci", "sh_FbzcpCca_Sw2"  ,"BOOL"); FX_Menu(7) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GLP_SOOTH_FLAG: {extVoodoo.glP_Smoth_flag	= control->Menu_ChangeCheck("pci", "glP_Smoth_flag"	  ,"BOOL"); FX_Menu(8) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GLL_SOOTH_FLAG: {extVoodoo.glL_Smoth_flag	= control->Menu_ChangeCheck("pci", "glL_Smoth_flag"	  ,"BOOL"); FX_Menu(9) ; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GLBLENDFC_FLAG: {extVoodoo.glBlendFc_flag	= control->Menu_ChangeCheck("pci", "glBlendFc_flag"	  ,"BOOL"); FX_Menu(10); GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GLPERSOCORFLAG: {extVoodoo.glPersCor_flag	= control->Menu_ChangeCheck("pci", "glPersCor_flag"	  ,"BOOL"); FX_Menu(12); GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_MIPMAP_FLAG: {extVoodoo.glGMipMap_flag	= control->Menu_ChangeCheck("pci", "glGMipMap_flag"	  ,"BOOL"); FX_Menu(11); GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_FOG_FLAG	  : {extVoodoo.gl_GLFog__flag   = control->Menu_ChangeCheck("pci", "gl_GLFog__flag"	  ,"BOOL"); FX_Menu(13); GFX_ResetVoodoo(); }return 0;
				//case ICNMNU_COMAPTIBLE_FLAG:{extVoodoo.compatibleFlag   = control->Menu_ChangeCheck("pci", "compatible_flag"  ,"BOOL"); FX_Menu(14); GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_SCISSOR_FLAG:{extVoodoo.glScissor_flag   = control->Menu_ChangeCheck("pci", "glScissor_flag"   ,"BOOL"); FX_Menu(15); GFX_ResetVoodoo(); }return 0;

				case ICNMNU_GLShade_NONE  : {extVoodoo.GL_shade = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GLShade_FLAT  : {extVoodoo.GL_shade = 1; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GLShade_SMOOTH: {extVoodoo.GL_shade = 2; GFX_ResetVoodoo(); }return 0;
				
				case ICNMNU_TEXTURE_WRAPS_REPEAT	: { extVoodoo.gl_wrap_s = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPS_REPEATM	: { extVoodoo.gl_wrap_s = 1; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPS_CLAMPTE	: { extVoodoo.gl_wrap_s = 2; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPS_CLAMPTB	: { extVoodoo.gl_wrap_s = 3; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPR_REPEAT	: { extVoodoo.gl_wrap_t = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPR_REPEATM	: { extVoodoo.gl_wrap_t = 1; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPR_CLAMPTE	: { extVoodoo.gl_wrap_t = 2; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_TEXTURE_WRAPR_CLAMPTB	: { extVoodoo.gl_wrap_t = 3; GFX_ResetVoodoo(); }return 0;

				case ICNMNU_GL_BYTE							: { extVoodoo.RGB_Type = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_BYTE				: { extVoodoo.RGB_Type = 1; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_SHORT						: { extVoodoo.RGB_Type = 2; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT				: { extVoodoo.RGB_Type = 3; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_INT							: { extVoodoo.RGB_Type = 4; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT					: { extVoodoo.RGB_Type = 5; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_FLOAT						: { extVoodoo.RGB_Type = 6; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_2_BYTES						: { extVoodoo.RGB_Type = 7; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_3_BYTES						: { extVoodoo.RGB_Type = 8; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_4_BYTES						: { extVoodoo.RGB_Type = 9; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DOUBLE						: { extVoodoo.RGB_Type = 10; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_HALF_FLOAT					: { extVoodoo.RGB_Type = 11; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_BYTE_332			: { extVoodoo.RGB_Type = 12; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT_4444			: { extVoodoo.RGB_Type = 13; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT_5551			: { extVoodoo.RGB_Type = 14; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_8888			: { extVoodoo.RGB_Type = 15; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_1010102			: { extVoodoo.RGB_Type = 16; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_BYTE_233_REV		: { extVoodoo.RGB_Type = 17; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT_565			: { extVoodoo.RGB_Type = 18; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT_565_REV		: { extVoodoo.RGB_Type = 19; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT_4444_REV		: { extVoodoo.RGB_Type = 20; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_SHORT_1555_REV		: { extVoodoo.RGB_Type = 21; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_8888_REV		: { extVoodoo.RGB_Type = 22; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_2101010_REV		: { extVoodoo.RGB_Type = 23; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_248				: { extVoodoo.RGB_Type = 24; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_10F11F11FREV	: { extVoodoo.RGB_Type = 25; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_UNSIGNED_INT_5999REV			: { extVoodoo.RGB_Type = 26; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_FLOAT_32_UNSIGNED_INT_24_8_REV:{ extVoodoo.RGB_Type = 27; GFX_ResetVoodoo(); }return 0;

				case ICNMNU_GL_RGB4		: { extVoodoo.RGB_Format = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB5		: { extVoodoo.RGB_Format = 1; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB8		: { extVoodoo.RGB_Format = 2; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB10	: { extVoodoo.RGB_Format = 3; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB12	: { extVoodoo.RGB_Format = 4; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB16	: { extVoodoo.RGB_Format = 5; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGBA2	: { extVoodoo.RGB_Format = 6; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGBA4	: { extVoodoo.RGB_Format = 7; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB5_A1	: { extVoodoo.RGB_Format = 8; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGBA8	: { extVoodoo.RGB_Format = 9; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB10_A2	: { extVoodoo.RGB_Format = 10; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGBA12	: { extVoodoo.RGB_Format = 11; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGBA16	: { extVoodoo.RGB_Format = 12; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RED		: { extVoodoo.RGB_Format = 13; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_GREEN	: { extVoodoo.RGB_Format = 14; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_BLUE		: { extVoodoo.RGB_Format = 15; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGB		: { extVoodoo.RGB_Format = 16; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_BGR		: { extVoodoo.RGB_Format = 17; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_RGBA		: { extVoodoo.RGB_Format = 18; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_BGRA		: { extVoodoo.RGB_Format = 19; GFX_ResetVoodoo(); }return 0;

				case ICNMNU_FXCYCLES_01000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 1000"); CPU_FX_Cycles = 1000; }return 0;
				case ICNMNU_FXCYCLES_02000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 2000"); CPU_FX_Cycles = 2000; }return 0;
				case ICNMNU_FXCYCLES_03000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 3000"); CPU_FX_Cycles = 3000; }return 0;
				case ICNMNU_FXCYCLES_04000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 4000"); CPU_FX_Cycles = 4000; }return 0;
				case ICNMNU_FXCYCLES_05000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 5000"); CPU_FX_Cycles = 5000; }return 0;
				case ICNMNU_FXCYCLES_06000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 6000"); CPU_FX_Cycles = 6000; }return 0;
				case ICNMNU_FXCYCLES_07000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 7000"); CPU_FX_Cycles = 7000; }return 0;
				case ICNMNU_FXCYCLES_08000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 8000"); CPU_FX_Cycles = 8000; }return 0;
				case ICNMNU_FXCYCLES_09000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 9000"); CPU_FX_Cycles = 9000; }return 0;
				case ICNMNU_FXCYCLES_10000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 10000"); CPU_FX_Cycles = 10000; }return 0;
				case ICNMNU_FXCYCLES_11000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 11000"); CPU_FX_Cycles = 11000; }return 0;
				case ICNMNU_FXCYCLES_12000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 12000"); CPU_FX_Cycles = 12000; }return 0;
				case ICNMNU_FXCYCLES_13000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 13000"); CPU_FX_Cycles = 13000; }return 0;
				case ICNMNU_FXCYCLES_14000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 14000"); CPU_FX_Cycles = 14000; }return 0;
				case ICNMNU_FXCYCLES_15000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 15000"); CPU_FX_Cycles = 15000; }return 0;
				case ICNMNU_FXCYCLES_16000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 16000"); CPU_FX_Cycles = 16000; }return 0;
				case ICNMNU_FXCYCLES_17000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 17000"); CPU_FX_Cycles = 17000; }return 0;
				case ICNMNU_FXCYCLES_18000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 18000"); CPU_FX_Cycles = 18000; }return 0;
				case ICNMNU_FXCYCLES_19000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 19000"); CPU_FX_Cycles = 19000; }return 0;
				case ICNMNU_FXCYCLES_20000: { control->ChangeOnTheFly("cpu", "cycles", "max fxsst 20000"); CPU_FX_Cycles = 20000; }return 0;
				case ICNMNU_FXCYCLES_00000: { control->ChangeOnTheFly("cpu", "cycles", "max"); CPU_FX_Cycles = 0; }return 0;

				case ICNMNU_GL_SCAN0: { extVoodoo.GLScan = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_SCAN1: { extVoodoo.GLScan = 1; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_SCAN2: { extVoodoo.GLScan = 2; GFX_ResetVoodoo(); }return 0;

				case ICNMNU_GL_DARK000: { extVoodoo.GLDark = 0; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK005: { extVoodoo.GLDark = 5; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK010: { extVoodoo.GLDark = 10; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK015: { extVoodoo.GLDark = 15; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK020: { extVoodoo.GLDark = 20; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK025: { extVoodoo.GLDark = 25; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK030: { extVoodoo.GLDark = 30; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK035: { extVoodoo.GLDark = 35; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK040: { extVoodoo.GLDark = 40; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK045: { extVoodoo.GLDark = 45; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK050: { extVoodoo.GLDark = 50; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK055: { extVoodoo.GLDark = 55; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK060: { extVoodoo.GLDark = 60; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK065: { extVoodoo.GLDark = 65; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK070: { extVoodoo.GLDark = 70; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK075: { extVoodoo.GLDark = 75; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK080: { extVoodoo.GLDark = 80; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK085: { extVoodoo.GLDark = 85; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK090: { extVoodoo.GLDark = 90; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK095: { extVoodoo.GLDark = 95; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK100: { extVoodoo.GLDark = 100; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK105: { extVoodoo.GLDark = 105; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK110: { extVoodoo.GLDark = 110; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK115: { extVoodoo.GLDark = 115; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK120: { extVoodoo.GLDark = 120; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK125: { extVoodoo.GLDark = 125; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK130: { extVoodoo.GLDark = 130; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK135: { extVoodoo.GLDark = 135; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK140: { extVoodoo.GLDark = 140; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK145: { extVoodoo.GLDark = 145; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK150: { extVoodoo.GLDark = 150; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK155: { extVoodoo.GLDark = 155; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK160: { extVoodoo.GLDark = 160; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK165: { extVoodoo.GLDark = 165; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK170: { extVoodoo.GLDark = 170; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK175: { extVoodoo.GLDark = 175; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK180: { extVoodoo.GLDark = 180; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK185: { extVoodoo.GLDark = 185; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK190: { extVoodoo.GLDark = 190; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK195: { extVoodoo.GLDark = 195; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK200: { extVoodoo.GLDark = 200; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK205: { extVoodoo.GLDark = 205; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK210: { extVoodoo.GLDark = 210; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK215: { extVoodoo.GLDark = 215; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK220: { extVoodoo.GLDark = 220; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK225: { extVoodoo.GLDark = 225; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK230: { extVoodoo.GLDark = 230; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK235: { extVoodoo.GLDark = 235; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK240: { extVoodoo.GLDark = 240; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK245: { extVoodoo.GLDark = 245; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK250: { extVoodoo.GLDark = 250; GFX_ResetVoodoo(); }return 0;
				case ICNMNU_GL_DARK254: { extVoodoo.GLDark = 255; GFX_ResetVoodoo(); }return 0;

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
				case ICNMNU_WIN1440X1080:   { GFX_UpdateResolution(1440,1080,true);	}return 0;
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
				case ICNMNU_FUL1440X1080:   { GFX_UpdateResolution(1440,1080,false);}return 0;
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
				case ICNMNU_S3_OVERCLCKA   :{ OverDriveOC = 0;						}return 0;
				case ICNMNU_S3_OVERCLCKB   :{ OverDriveOC = 1;						}return 0;
				case ICNMNU_S3_OVERCLCKC   :{ OverDriveOC = 2;						}return 0;
				case ICNMNU_S3_OVERCLCKD   :{ OverDriveOC = 3;						}return 0;
				case ICNMNU_S3_OVERCLCKE   :{ OverDriveOC = 4;						}return 0;
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

				case ICNMNU_FOPN_CDROM:
					{
						MenuBrowseCDImage('E',0);
					}
					return 0;
				case ICNMNU_FOPN_CDROM2:
					{
						MenuBrowseCDImage('F', 1);
					}
				return 0;
				case ICNMNU_FOPN_FLOPPYA:
					{
						MenuBrowseFDImage('A', 0);
					}
				return 0;
				case ICNMNU_FOPN_FLOPPYB:
					{
						MenuBrowseFDImage('B', 1);
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