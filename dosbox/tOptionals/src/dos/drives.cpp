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

#if defined(_MSC_VER)
#include <SDL2\include\SDL.h>
#else
#include <SDL.h>
#endif

#include "dosbox.h"
#include "dos_system.h"
#include "drives.h"
#include "mapper.h"
#include "support.h"
#include "control.h"
#include "ide.h"

#include "programs.h" //WriteOut
#include "..\gui\version_optionals.h"
#include <bios_disk.h>

char sDriveNotify[4096];
char sDriveLabel[4096];
char sDskNameA[DOS_NAMELENGTH_ASCII];
char sDskNameB[DOS_NAMELENGTH_ASCII];
char sCDRName1[DOS_NAMELENGTH_ASCII];
char sCDRName2[DOS_NAMELENGTH_ASCII];

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
int  DriveManager::CurrentDisk(int drive)
{
	return driveInfos[drive].currentDisk;
}
int  DriveManager::DiskCounts(int drive) {
	return driveInfos[drive].disks.size();
}
void DriveManager::AppendDisk(int drive, DOS_Drive* disk) {
	driveInfos[drive].disks.push_back(disk);
}
void DriveManager::ResizeDisk(int drive, DOS_Drive* disk,int num) {
	driveInfos[drive].disks.resize(num, disk);
}
void DriveManager::InsertDisk(int drive, DOS_Drive* disk, int num) {
	driveInfos[drive].disks.assign(num, disk);
}
void DriveManager::ClearDisks(int drive) {
	driveInfos[drive].disks.clear();
}
std::string Get_FDLabel(Bit8u Laufwerk)
{
	char const* Buffer;
	char	    Memory[32];
	int			x = 0;

	if (strlen(Drives[Laufwerk]->GetInfo()) != 0)
	{
		Buffer = Drives[Laufwerk]->GetLabel();

		if (strlen(Buffer) == 0) return "KeinName";/*BlankFloppy*/
		else
		{ 
			for (int c = 0; c < strlen(Buffer); c++)
			{
				if ((c == 8) && (Buffer[8] == '.'))
						continue;

				Memory[x] = Buffer[c];
				x++;

				if (c == strlen(Buffer)-1)
					Memory[x] = '\0';
			}
			Buffer = Memory;

			#if defined(C_DEBUG)
				LOG(LOG_IMAGE, LOG_NORMAL)("[%d] Disk Label = '%s' [Lenght: %d]\n", __LINE__, Buffer, x);
			#endif

			return Buffer;
		}		
	}
	return "";
}

void Notify_SetLabel()
{
	memset(&sDriveLabel[0], 0, sizeof(sDriveLabel));
	strcpy(sDriveLabel, "");

	if (strlen(sDskNameA) != 0)
	{
		strcat(sDriveLabel, "[ ");
		strcat(sDriveLabel, sDskNameA);
		strcat(sDriveLabel, " ]");
	}

	if (strlen(sDskNameB) != 0)
	{
		strcat(sDriveLabel, "[ ");
		strcat(sDriveLabel, sDskNameB);
		strcat(sDriveLabel, " ]");
	}

	if (strlen(sCDRName1) != 0)
	{
		strcat(sDriveLabel, "[ ");
		strcat(sDriveLabel, sCDRName1);
		strcat(sDriveLabel, " ]");
	}

	if (strlen(sCDRName2) != 0)
	{
		strcat(sDriveLabel, "[ ");
		strcat(sDriveLabel, sCDRName2);
		strcat(sDriveLabel, " ]");
	}
}

void DriveManager::NotifyWaitLabel(Bit8u Laufwerk, int Pos, int Max) {
	/*
		On Initialize Show the Label
	*/
	switch (Laufwerk)
	{
		case 0:
		{
			memset(&sDskNameA[0], 0, sizeof(sDskNameA));
			strcpy(sDskNameA, "");
			sprintf(sDskNameA, "%c:\\..Load..", 'A');
		}
		break;
		case 1:
		{
			memset(&sDskNameB[0], 0, sizeof(sDskNameB));
			strcpy(sDskNameB, "");
			sprintf(sDskNameB, "%c:\\..Load..", 'B');
		}
		break;
	default:
		{
			if (Laufwerk > 2)
			{
				int CDRomDrives = 0;
				for (unsigned int idrive = 2; idrive < DOS_DRIVES; idrive++)
				{
					if (dynamic_cast<const isoDrive*>(Drives[idrive]) == NULL)
					{
						continue;
					}

					++CDRomDrives;
					if (idrive == Laufwerk)
						break;
				}

				/*
					Nicht Lauferk C:\
				*/

				if (CDRomDrives == 1)
				{
					memset(&sCDRName1[0], 0, sizeof(sCDRName1));
					strcpy(sCDRName1, "");
					sprintf(sCDRName1, "%c:\\..Load..", Laufwerk + 'A');
				}
				else if (CDRomDrives == 2)
				{
					memset(&sCDRName2[0], 0, sizeof(sCDRName2));
					strcpy(sCDRName2, "");
					sprintf(sCDRName2, "%c:\\..Load..", Laufwerk + 'A');
				}

				#if defined(C_DEBUG)
					LOG(LOG_IMAGE, LOG_NORMAL)("[%d] LAUFWERK NUMMER='%d/%d'", __LINE__, Laufwerk, CDRomDrives);
				#endif
			}
		}
	}

	Notify_SetLabel();

	sprintf(sDriveNotify, "\tMount Drive %s (%d of %d is now Active)", sDriveLabel, Pos + 1, Max);

	/* Schreibe in die Log   */
	LOG_MSG(sDriveNotify);

	/* Aktualisiere das Window Title */
	GFX_SetTitle(-1, -1, false);
}
void DriveManager::NotifyShowLabel(Bit8u Laufwerk, int Pos, int Max ){
	/*
		On Initialize Show the Label
	*/		
	
	switch(Laufwerk)
	{
		case 0:
		{	
			memset(&sDskNameA[0], 0, sizeof(sDskNameA));
			strcpy (sDskNameA, "");
			sprintf(sDskNameA, "%c:\\%s", 'A', Get_FDLabel(Laufwerk).c_str());
		}
		break;
		case 1:
		{
			memset(&sDskNameB[0], 0, sizeof(sDskNameB));
			strcpy (sDskNameB, "");
			sprintf(sDskNameB, "%c:\\%s", 'B', Get_FDLabel(Laufwerk).c_str());
		}
		break;
		default:
		{

			if (Laufwerk > 2)
			{
				int CDRomDrives = 0;
				for (unsigned int idrive = 2; idrive < DOS_DRIVES; idrive++)
				{
					if (dynamic_cast<const isoDrive*>(Drives[idrive]) == NULL)
					{
						continue;
					}

					++CDRomDrives;
					if (idrive == Laufwerk)
						break;
				}

				/*
					Nicht Lauferk C:\
				*/

				if (CDRomDrives == 1)
				{
					memset(&sCDRName1[0], 0, sizeof(sCDRName1));
					strcpy(sCDRName1, "");
					sprintf(sCDRName1, "%c:\\%s", Laufwerk + 'A', Get_FDLabel(Laufwerk).c_str());
				}
				else if (CDRomDrives == 2)
				{
					memset(&sCDRName2[0], 0, sizeof(sCDRName2));
					strcpy(sCDRName2, "");
					sprintf(sCDRName2, "%c:\\%s", Laufwerk + 'A', Get_FDLabel(Laufwerk).c_str());
				}

				#if defined(C_DEBUG)
					LOG(LOG_IMAGE, LOG_NORMAL)("[%d] LAUFWERK NUMMER='%d/%d'", __LINE__, Laufwerk, CDRomDrives);
				#endif					
			}
		}
	}
					
	Notify_SetLabel();

	sprintf(sDriveNotify,"\tMount Drive %s (%d of %d is now Active)", sDriveLabel, Pos+1, Max);

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
		if (driveInfo.disks.size() > 0){
			disk->Activate();
			NotifyShowLabel(drive,currentDisk, driveInfo.disks.size() );
		}
	}
}
/*
std::string DriveManager::GetFilePath(int drive, DOS_Drive* disk, int num) {
	driveInfos[drive].disks[num];
	return disk->info;
}
*/
std::string DriveManager::GetFilePath(int drive, DOS_Drive* disk, int num)
{
    DOS_Drive* selected = driveInfos[drive].disks[num];
    return selected ? selected->info : (disk ? disk->info : "");
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

INLINE std::string Replace(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

std::vector<std::string*>DriveManager::Get_CDROM_Paths(std::vector<std::string>& Stacks, int drive)
{
	std::vector<std::string*> result;  // ← Hier den Rückgabe-Vektor anlegen

	int MaxDisk = (int)driveInfos[drive].disks.size();
	int Current = (int)driveInfos[drive].currentDisk;

	std::string sFindTxt = "isoDrive ";
	std::string sReplace = "";
	std::string FilePath;

	for (int i = 0; i < MaxDisk; i++)
	{
		FilePath = Replace(driveInfos[drive].disks[i]->info, sFindTxt, sReplace);
		Stacks.push_back(FilePath);               // ← Hier push_back den String (nicht c_str!)
		result.push_back(new std::string(FilePath));  // ← Oder new-String, wenn du Pointer brauchst
	}
	return result;  // ← Jetzt fehlt das return nicht mehr!

}

std::vector<std::string*>DriveManager::Get_FLOPPY_Paths(std::vector<std::string>& Stacks, int drive)
{
	std::vector<std::string*> result;  // ← Hier den Rückgabe-Vektor anlegen

	int MaxDisk = (int)driveInfos[drive].disks.size();
	int Current = (int)driveInfos[drive].currentDisk;

	std::string sFindTxt = "fatDrive ";
	std::string sReplace = "";
	std::string FilePath;

	for (int i = 0; i < MaxDisk; i++)
	{
		FilePath = Replace(driveInfos[drive].disks[i]->info, sFindTxt, sReplace);
    Stacks.push_back(FilePath);               // ← Hier push_back den String (nicht c_str!)
    result.push_back(new std::string(FilePath));  // ← Oder new-String, wenn du Pointer brauchst
    #if defined(C_DEBUG)
			LOG(LOG_IMAGE, LOG_NORMAL)("[%d] PushBack: %s", __LINE__, FilePath.c_str());
    #endif
	}
	return result;  // ← Jetzt fehlt das return nicht mehr!
}



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

			NotifyShowLabel(drive,currentDisk, numDisks );
		}
	}

}
void DriveManager::CycleImage(int drive, bool notify, int LastDisk) {
	int numDisks = (int)driveInfos[drive].disks.size();
	int n = 1;
	if (isVirtualModus)
		n = -1;

	if (numDisks > n) {
		// cycle disk
		int currentDisk = driveInfos[drive].currentDisk;
		DOS_Drive* oldDisk = driveInfos[drive].disks[currentDisk];
		currentDisk = LastDisk;// (currentDisk + LastDisk) % numDisks;
		DOS_Drive* newDisk = driveInfos[drive].disks[currentDisk];
		driveInfos[drive].currentDisk = currentDisk;

		// copy working directory, acquire system resources and finally switch to next drive		
		strcpy(newDisk->curdir, oldDisk->curdir);
		newDisk->Activate();
		Drives[drive] = newDisk;
		//LOG_MSG("%s", Drives[drive]->GetLabel());
		if (notify == true) {

			NotifyShowLabel(drive, currentDisk, numDisks);
		}
		Drives[drive]->EmptyCache();
		Drives[drive]->MediaChange();
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

void DriveManager::ChangeDisk(int drive, DOS_Drive* disk)
{
	DriveInfo& driveInfo = driveInfos[drive];
	
	if (Drives[drive] == NULL || disk == NULL || !driveInfo.disks.size())
		return;

	isoDrive* cdrom = dynamic_cast<isoDrive*>(Drives[drive]);
	
	signed char index	= -1;
	bool slave			= false;
	
	if (cdrom)
		IDE_CDROM_Detach_Ret(index, slave, drive);

	strcpy(disk->curdir, driveInfo.disks[driveInfo.currentDisk]->curdir);
	disk->Activate();
	disk->UpdateDPB(currentDrive);

	if (cdrom && isVirtualModus)
		cdrom->loadImage();

	driveInfo.disks[driveInfo.currentDisk] = disk;

	Drives[drive] = disk;
	
	if (cdrom && index > -1)
		IDE_CDROM_Attach(index, slave, drive);

	Drives[drive]->EmptyCache();
	Drives[drive]->MediaChange();
	
	if (cdrom && !isVirtualModus)
	{
		IDE_CDROM_Detach_Ret(index, slave, drive);
		if (index > -1) IDE_CDROM_Attach(index, slave, drive);
	}

}

int  DriveManager::UnmountDrive(int drive) {
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
