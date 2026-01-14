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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <stdio.h>
#include "..\..\config.h"
#include "version_optionals.h" 

/*
 * Version string and build date string. These can be used by anything that
 * wants to display this information to the user (e.g. about dialog).
 *
 * Note: it would be very nice if we could instead of (or in addition to) the
 * build date present a date which corresponds to the date our source files
 * were last changed. To understand the difference, imagine that a user
 * makes a checkout on January 1, then after a week compiles it
 * (e.g. after doing a 'make clean'). The build date then will say January 8
 * even though the files were last changed on January 1.
 *
 * Another problem is that __DATE__/__TIME__ depend on the local time zone.
 *
 * It's clear that such a "last changed" date would be much more useful to us
 * for feedback purposes. After all, when somebody files a bug report, we
 * don't care about the build date, we want to know which date their checkout
 * was made.
 *
 * So, how could we implement this? At least on unix systems, a special script
 * could do it. Basically, that script could parse the output of "svn info" or
 * "svnversion" to determine the revision of the checkout, and insert that
 * information somehow into the build process (e.g. by generating a tiny
 * header file, analog to internal_version.h, maybe called svn_rev.h or so.)
 *
 * Drawback: This only works on systems which can run suitable scripts as part
 * of the build process (so I guess Visual C++ would be out of the game here?
 * I don't know VC enough to be sure). And of course it must be robust enough
 * to properly work in exports (i.e. release tar balls etc.).
 */
 
/* The Old Version Info
 printf("\nDOSBox version %s %s, Copyright 2002-2018 DOSBox Team.\n\n",VERSION,DOSBOXREVISION);
 printf("\n");
 printf("DOSBox is written by the DOSBox Team (See AUTHORS file))\n");
 printf("DOSBox comes with ABSOLUTELY NO WARRANTY.  This is free software,\n");
 printf("and you are welcome to redistribute it under certain conditions;\n");
 printf("please read the COPYING file thoroughly before doing so.\n\n");
*/

const char *gDosboxDay         = __DATE__ + 4;
const char *gDosboxMonth       = __DATE__ + 0;
const char *gDosboxYear        = __DATE__ + 7;
const char *gDosboxBuildSVN    = VERSION;
const char *gDosboxRevision    = DOSBOXREVISION;
const char *gDosboxBuildDate   = __DATE__ " " __TIME__;
const char *gDosboxFullVersion = "DOSBox " VERSION " " DOSBOXREVISION " Build on (" __DATE__ " " __TIME__ ")";
const char *gDosboxCopyright   = "\tCopyright 2002-2020 DOSBox Team";
const char *gDosboxTeamText    = "\tDOSBox is written by the DOSBox Team (See AUTHORS file))\n\tDOSBox comes with ABSOLUTELY NO WARRANTY. This is free\n\tsoftware and you are welcome to redistribute,  it under\n\tcertain conditions.\n\tThis is a DOSBox Fork from the Original DOSBox 0.74 " VERSION " " DOSBOXSVERSION "\n\n\tGreetings, credits & thanks:\n\tDOSBox Team, dungan, NY00123, tauro, bloodbat, Yesterplay80\n\tnukeykt, VileRancour, D_Skywalk, Vasyl Tsvirkunov, Moe, kekko\n\tTaeWoong Yoo, krcroft\n\n\t\tGreetings to the CGBoard & Vogons Board";
const char *gDOSBoxSDLVersion  = DOSBOXSDLVERSION;
const char *gDOSBoxFeatures    = ""											
#if C_TARGETCPU == X86
	"32Bit "
#else
	"64Bit "
#endif	
	
#ifdef C_DYNAMIC_X86
	"DynamicX86 "
#endif

#ifdef C_FPU
	"FPUCore"
#endif	

#ifdef C_FPU_X86
	"X86 "
#endif

 DOSBOXSDLVERSION

#ifdef C_OPENGL
	" OpenGL3\n                   " DOSBOXFEATSGFX
#endif

#ifdef C_RETINAFIX
	"RetinaFix "
#endif	

#ifdef C_SSHOT
	"PNG "
#endif

#ifdef C_FLUIDSYNTH
	"Fluidsynth "
#endif

	"MT32 SSI2001\n                   M.A.M.E.-Sound Nuked-OPL3(v1.8)\n                   (dr)MP3 Decode (dr)FLAC Decode OGG-Vorbis\n                   Ogg-OPUS " 

#ifdef C_SDL_SOUND
	"SDLSound "
#endif

#ifdef HAVE_ALSA
	"ALSA "
#endif

#ifdef HAVE_LIBASOUND
	"LibSound "
#endif

#ifdef C_DIRECTSERIAL
	"DirectSerial "
#endif	

#ifdef C_IPX
	"IPX "
#endif

#ifdef C_MODEM
	"Modem "
#endif

#ifdef C_NE2000
	"NE2000 "
#endif

#ifdef C_HEAVY_DEBUG
	"Heavy "
#endif

#ifdef C_DEBUG
	"Debug Enabled"
#endif

;