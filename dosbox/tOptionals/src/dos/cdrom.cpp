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


// ******************************************************
// SDL CDROM 
// ******************************************************

#if defined(_MSC_VER)
#include <SDL2\include\SDL_version.h>
#else
#include "SDL_version.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dosbox.h"
#if defined(_MSC_VER)
#include <SDL2\include\SDL.h>
#else
#include <SDL.h>
#endif
#include "support.h"
#include "cdrom.h"

int CDROM_GetMountType(char* path, int forceCD) {
// 0 - physical CDROM
// 1 - Iso file
// 2 - subdirectory
	// 1. Smells like a real cdrom 
	// if ((strlen(path)<=3) && (path[2]=='\\') && (strchr(path,'\\')==strrchr(path,'\\')) && 	(GetDriveType(path)==DRIVE_CDROM)) return 0;

	char buffer[512];
	strcpy(buffer,path);
#if defined (WIN32) || defined(OS2)
	upcase(buffer);
#endif

	// Detect ISO
	struct stat file_stat;
	if ((stat(path, &file_stat) == 0) && (file_stat.st_mode & S_IFREG)) return 1; 
	return 2;
}

// ******************************************************
// Fake CDROM
// ******************************************************

bool CDROM_Interface_Fake :: GetAudioTracks(int& stTrack, int& end, TMSF& leadOut) {
	stTrack = end = 1;
	leadOut.min	= 60;
	leadOut.sec = leadOut.fr = 0;
	return true;
}

bool CDROM_Interface_Fake :: GetAudioTrackInfo(int track, TMSF& start, unsigned char& attr) {
	if (track>1) return false;
	start.min = start.fr = 0;
	start.sec = 2;
	attr	  = 0x60; // data / permitted
	return true;
}

bool CDROM_Interface_Fake :: GetAudioSub(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos){
	attr	= 0;
	track	= index = 1;
	relPos.min = relPos.fr = 0; relPos.sec = 2;
	absPos.min = absPos.fr = 0; absPos.sec = 2;
	return true;
}

bool CDROM_Interface_Fake :: GetAudioStatus(bool& playing, bool& pause) {
	playing = pause = false;
	return true;
}

bool CDROM_Interface_Fake :: GetMediaTrayStatus(bool& mediaPresent, bool& mediaChanged, bool& trayOpen) {
	mediaPresent = true;
	mediaChanged = false;
	trayOpen     = false;
	return true;
}
bool CDROM_Interface_Fake::ReadSectorsHost(void *buffer, bool raw, unsigned long sector, unsigned long num)
{
	return false;/*TODO*/
};