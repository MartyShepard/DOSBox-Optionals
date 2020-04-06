/*
 *  Copyright (C) 2002-2019  The DOSBox Team
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

#include "SDL.h"
#include "dosbox.h"
#include "dos_system.h"
#include "drives.h"
#include "mapper.h"
#include "support.h"

#include "programs.h" //WriteOut
#include "..\gui\version.h"

char sDriveNotify[4096];
char sDriveLabel[4096];
char sDskNameA[DOS_NAMELENGTH_ASCII];
char sDskNameB[DOS_NAMELENGTH_ASCII];
char sDskNameZ[DOS_NAMELENGTH_ASCII];


extern void GFX_SetTitle(Bit32s cycles,int frameskip,bool paused);

bool WildFileCmp(const char * file, const char * wild) 
{
	char file_name[9];
	char file_ext[4];
	char wild_name[9];
	char wild_ext[4];
	const char * find_ext;
	Bitu r;

	strcpy(file_name,"        ");
	strcpy(file_ext,"   ");
	strcpy(wild_name,"        ");
	strcpy(wild_ext,"   ");

	find_ext=strrchr(file,'.');
	if (find_ext) {
		Bitu size=(Bitu)(find_ext-file);
		if (size>8) size=8;
		memcpy(file_name,file,size);
		find_ext++;
		memcpy(file_ext,find_ext,(strlen(find_ext)>3) ? 3 : strlen(find_ext)); 
	} else {
		memcpy(file_name,file,(strlen(file) > 8) ? 8 : strlen(file));
	}
	upcase(file_name);upcase(file_ext);
	find_ext=strrchr(wild,'.');
	if (find_ext) {
		Bitu size=(Bitu)(find_ext-wild);
		if (size>8) size=8;
		memcpy(wild_name,wild,size);
		find_ext++;
		memcpy(wild_ext,find_ext,(strlen(find_ext)>3) ? 3 : strlen(find_ext));
	} else {
		memcpy(wild_name,wild,(strlen(wild) > 8) ? 8 : strlen(wild));
	}
	upcase(wild_name);upcase(wild_ext);
	/* Names are right do some checking */
	r=0;
	while (r<8) {
		if (wild_name[r]=='*') goto checkext;
		if (wild_name[r]!='?' && wild_name[r]!=file_name[r]) return false;
		r++;
	}
checkext:
    r=0;
	while (r<3) {
		if (wild_ext[r]=='*') return true;
		if (wild_ext[r]!='?' && wild_ext[r]!=file_ext[r]) return false;
		r++;
	}
	return true;
}

void Set_Label(char const * const input, char * const output, bool cdrom) {
	Bitu togo     = 8;
	Bitu vnamePos = 0;
	Bitu labelPos = 0;
	bool point    = false;

	//spacepadding the filenamepart to include spaces after the terminating zero is more closely to the specs. (not doing this now)
	// HELLO\0' '' '

	while (togo > 0) {
		if (input[vnamePos]==0) break;
		if (!point && (input[vnamePos]=='.')) {	togo=4; point=true; }

		//another mscdex quirk. Label is not always uppercase. (Daggerfall)
		output[labelPos] = (cdrom?input[vnamePos]:toupper(input[vnamePos]));

		labelPos++; vnamePos++;
		togo--;
		if ((togo==0) && !point) {
			if (input[vnamePos]=='.') vnamePos++;
			output[labelPos]='.'; labelPos++; point=true; togo=3;
		}
	};
	output[labelPos]=0;

	//Remove trailing dot. except when on cdrom and filename is exactly 8 (9 including the dot) letters. MSCDEX feature/bug (fifa96 cdrom detection)
	if((labelPos > 0) && (output[labelPos-1] == '.') && !(cdrom && labelPos ==9))
		output[labelPos-1] = 0;
}



DOS_Drive::DOS_Drive() {
	curdir[0]=0;
	info[0]=0;
}

char * DOS_Drive::GetInfo(void) {
	return info;
}

// static members variables
int DriveManager::currentDrive;
DriveManager::DriveInfo DriveManager::driveInfos[26];

void DriveManager::AppendDisk(int drive, DOS_Drive* disk) {
	driveInfos[drive].disks.push_back(disk);
}

void DriveManager__NotifyShowLabel(DOS_Drive* Floppy, int Laufwerk, int Pos, int Max ){
				
	/*
		On Initialize Show the Label
	*/		
	Drives[Laufwerk] = Floppy;
			
	switch(Laufwerk)
	{
		case 0:
		{
			strcpy(sDskNameA, "");					
			if ( strlen( Drives[0]->GetInfo() ) != 0 ){
				LOG_MSG(">>>>>>>>>>>> %s",Drives[0]->GetInfo() );
				if ( strlen( Drives[0]->GetLabel() ) == 0 ){
					sprintf(sDskNameA, "%c:Blank",'A');
					
				} else { sprintf(sDskNameA, "%c:%s", 'A', Drives[0]->GetLabel());}						 
			}				
		}
		break;
		case 1:
		{
			strcpy(sDskNameB, "");					
			if ( strlen( Drives[1]->GetInfo() ) != 0 ){
				
				if ( strlen( Drives[1]->GetLabel() ) == 0 ){
					sprintf(sDskNameB, "%c:Blank",'B');
					
				} else {sprintf(sDskNameB, "%c:%s",'B', Drives[1]->GetLabel());}						 
			}				
		}
		break;
		default:
		{
			if (Laufwerk != 2){
				/*
					Nicht Lauferk C:\
				*/
				strcpy(sDskNameZ, "");					
				if ( strlen( Drives[Laufwerk]->GetInfo() ) != 0 ){
					
					if ( strlen( Drives[Laufwerk]->GetLabel() ) == 0 ){
						sprintf(sDskNameZ, "%c:Blank",'A'+Laufwerk);
						
					} else {sprintf(sDskNameZ, "%c:%s",'A'+Laufwerk, Drives[Laufwerk]->GetLabel());}						 
				}				
			}					
		}
	}
					
	if (( strlen( sDskNameA ) != 0 ) &&  ( strlen( sDskNameB ) != 0 ) && ( strlen( sDskNameZ ) != 0 )) {				
			sprintf(sDriveLabel, "[ %s / %s / %s ]",sDskNameA, sDskNameB, sDskNameZ);										
	}
	
	else if (( strlen( sDskNameA ) != 0 ) &&  ( strlen( sDskNameB ) == 0 ) && ( strlen( sDskNameZ ) != 0 )) {				
			sprintf(sDriveLabel, "[ %s / %s ]",sDskNameA, sDskNameZ);								
	}	
	
	else if (( strlen( sDskNameA ) == 0 ) &&  ( strlen( sDskNameB ) != 0 ) && ( strlen( sDskNameZ ) != 0 )) {				
			sprintf(sDriveLabel, "[ %s / %s ]",sDskNameB, sDskNameZ);								
	}
	
	else if (( strlen( sDskNameA ) != 0 ) &&  ( strlen( sDskNameB ) != 0 ) && ( strlen( sDskNameZ ) == 0 )) {				
			sprintf(sDriveLabel, "[ %s / %s ]",sDskNameA, sDskNameB);								
	}
	
	else if (( strlen( sDskNameA ) == 0 ) &&  ( strlen( sDskNameB ) == 0 ) && ( strlen( sDskNameZ ) != 0 )) {				
			sprintf(sDriveLabel, "[ %s ]",sDskNameZ);							
	}
	
	else if (( strlen( sDskNameA ) == 0 ) &&  ( strlen( sDskNameB ) != 0 ) && ( strlen( sDskNameZ ) == 0 )) {				
			sprintf(sDriveLabel, "[ %s ]",sDskNameB);							
	}	
	else if (( strlen( sDskNameA ) != 0 ) &&  ( strlen( sDskNameB ) == 0 ) && ( strlen( sDskNameZ ) == 0 )) {				
			sprintf(sDriveLabel, "[ %s ]",sDskNameA);								
	}		
	sprintf(sDriveNotify,"Mount Drive %s (%d of %d is now Active)", sDriveLabel, Pos+1, Max);

	/* Schreibe in die Log   */
		LOG_MSG(sDriveNotify);			
		
	/* Aktualisiere das Window Title */
		GFX_SetTitle(-1,-1,false);
	
}

void DriveManager::InitializeDrive(int drive) {

	currentDrive = drive;
	DriveInfo& driveInfo = driveInfos[currentDrive];
	if (driveInfo.disks.size() > 0) {
		driveInfo.currentDisk = 0;
		DOS_Drive* disk = driveInfo.disks[driveInfo.currentDisk];
		int currentDisk = driveInfos[drive].currentDisk;
		Drives[currentDrive] = disk;
		
		/*
			On Initialize Show the Label
		*/			
		if (driveInfo.disks.size() > 1){
			disk->Activate();
			DriveManager__NotifyShowLabel(disk, drive,currentDisk, driveInfo.disks.size() );
		}
	}
}

/*
void DriveManager::CycleDrive(bool pressed) {
	if (!pressed) return;
		
	// do one round through all drives or stop at the next drive with multiple disks
	int oldDrive = currentDrive;
	do {
		currentDrive = (currentDrive + 1) % DOS_DRIVES;
		int numDisks = driveInfos[currentDrive].disks.size();
		if (numDisks > 1) break;
	} while (currentDrive != oldDrive);
}

void DriveManager::CycleDisk(bool pressed) {
	if (!pressed) return;
	
	int numDisks = driveInfos[currentDrive].disks.size();
	if (numDisks > 1) {
		// cycle disk
		int currentDisk = driveInfos[currentDrive].currentDisk;
		DOS_Drive* oldDisk = driveInfos[currentDrive].disks[currentDisk];
		currentDisk = (currentDisk + 1) % numDisks;		
		DOS_Drive* newDisk = driveInfos[currentDrive].disks[currentDisk];
		driveInfos[currentDrive].currentDisk = currentDisk;
		
		// copy working directory, acquire system resources and finally switch to next drive		
		strcpy(newDisk->curdir, oldDisk->curdir);
		newDisk->Activate();
		Drives[currentDrive] = newDisk;
	}
}
*/

void DriveManager::CycleDisks(int drive, bool notify) {
	int numDisks = (int)driveInfos[drive].disks.size();
	if (numDisks > 1) {
		// cycle disk
		int currentDisk = driveInfos[drive].currentDisk;
		DOS_Drive* oldDisk = driveInfos[drive].disks[currentDisk];
		currentDisk = (currentDisk + 1) % numDisks;		
		DOS_Drive* newDisk = driveInfos[drive].disks[currentDisk];
		driveInfos[drive].currentDisk = currentDisk;
		
		// copy working directory, acquire system resources and finally switch to next drive		
		strcpy(newDisk->curdir, oldDisk->curdir);
		newDisk->Activate();
		Drives[drive] = newDisk;
		//LOG_MSG("%s", Drives[drive]->GetLabel());
		if (notify){

			DriveManager__NotifyShowLabel(newDisk, drive,currentDisk, numDisks );
		}
	}

}

void DriveManager::CycleAllDisks(void) {
	/*
	for (int idrive=0; idrive<DOS_DRIVES; idrive++)CycleDisks(idrive, true);
	*/
	for (int idrive=0; idrive<2; idrive++) CycleDisks(idrive, true); 			/* Cycle all DISKS meaning A: and B: */
}

void DriveManager::CycleAllCDs(void) {
	for (int idrive=2; idrive<DOS_DRIVES; idrive++) CycleDisks(idrive, true); 	/* Cycle all CDs in C: D: ... Z: */
}
 
int DriveManager::UnmountDrive(int drive) {
	int result = 0;
	// unmanaged drive
	if (driveInfos[drive].disks.size() == 0) {
		result = Drives[drive]->UnMount();
	} else {
		// managed drive
		int currentDisk = driveInfos[drive].currentDisk;
		result = driveInfos[drive].disks[currentDisk]->UnMount();
		// only delete on success, current disk set to NULL because of UnMount
		if (result == 0) {
			driveInfos[drive].disks[currentDisk] = NULL;
			for (int i = 0; i < (int)driveInfos[drive].disks.size(); i++) {
				delete driveInfos[drive].disks[i];
			}
			driveInfos[drive].disks.clear();
		}
	}
	
	return result;
}

void DriveManager::Init(Section* /* sec */) {
	
	// setup driveInfos structure
	currentDrive = 0;
	for(int i = 0; i < DOS_DRIVES; i++) {
		driveInfos[i].currentDisk = 0;
	}
	
//	MAPPER_AddHandler(&CycleDisk, MK_f3, MMOD1, "cycledisk", "Cycle Disk");
//	MAPPER_AddHandler(&CycleDrive, MK_f3, MMOD2, "cycledrive", "Cycle Drv");
}

void DRIVES_Init(Section* sec) {
	DriveManager::Init(sec);
}
