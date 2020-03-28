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

#define Revision 4335
#define YearBuild 2020

 /* Build Automatic */
 
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define RCVersion2 "0.74.3." STR(Revision)

#define DOSBOXREVISION "r" STR(Revision) " (Optionals)"
#define DOSBOXSVERSION "r" STR(Revision)

#define Comment "DOSBox Fork: Optionals, a DOS & Win9X Emulator"
#define FileDescription "DOSBox r" STR(Revision) " (Optionals)"

#define DOSBOXFEATSGFX "3DFX CGA-Extension "
#define DOSBOXFEATSSND "MT32 SSI2001 M.A.M.E.-Sound Nuked-OPL3(v1.8)\n                   (dr)MP3 Decode (dr)FLAC Decode OGG-Vorbis Ogg-OPUS"

extern const char *gDosboxDay;
extern const char *gDosboxMonth;
extern const char *gDosboxYear;
extern const char *gDosboxBuildSVN;		// e.g. "SVN"
extern const char *gDosboxRevision;		// e.g. "r40xx"
extern const char *gDosboxBuildDate;    // e.g. "2002-01-01"
extern const char *gDosboxFullVersion;  // e.g. "DOSBox SVN r4036  (2002-01-01)"
extern const char *gDOSBoxFeatures;		// e.g. "32Bit (DynamicX68) (Fpu Core X68)"
extern const char *gDosboxCopyright;    // e.g. "Copyright 2002-2018 DOSBox Team"
extern const char *gDosboxTeamText;     // The Deafult Copyright Text

