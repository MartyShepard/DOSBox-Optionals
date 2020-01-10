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


#include "dosbox.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <time.h>
#include "programs.h"
#include "support.h"
#include "drives.h"
#include "cross.h"
#include "regs.h"
#include "callback.h"
#include "cdrom.h"
#include "dos_system.h"
#include "dos_inc.h"
#include "bios.h"
#include "bios_disk.h" 
#include "setup.h"
#include "control.h"
#include "inout.h"
#include "dma.h"
#include "mousedrv.h"
#include "ide.h"



#if defined(OS2)
#define INCL DOSFILEMGR
#define INCL_DOSERRORS
#include "os2.h"
#endif

#if defined(WIN32)
#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT)==S_IFDIR)
#endif
#endif

#if defined(C_DEBUG)
Bitu DEBUG_EnableDebugger(void);
#endif

class MOUSE : public Program {
public:
	void Run(void);
};

void MOUSE::Run(void) {
	if (cmd->FindExist("/?",false) || cmd->FindExist("/h",false)) {
		WriteOut(MSG_Get("PROGRAM_MOUSE_HELP"));
		return;
	}
	switch (Mouse_Drv) {
	case 0:
		if (cmd->FindExist("/u",false))
			WriteOut(MSG_Get("PROGRAM_MOUSE_NOINSTALLED"));
		else {
			Mouse_Drv = true;
			WriteOut(MSG_Get("PROGRAM_MOUSE_INSTALL"));
		}
		break;
	case 1:
		if (cmd->FindExist("/u",false)) {
			Mouse_Drv = false;
			WriteOut(MSG_Get("PROGRAM_MOUSE_UNINSTALL"));
		} else
			WriteOut(MSG_Get("PROGRAM_MOUSE_ERROR"));
		break;
	}
	return;
}

static void MOUSE_ProgramStart(Program * * make) {
	*make=new MOUSE;
}


static Bitu ZDRIVE_NUM = 25;

static const char* UnmountHelper(char umount) {
	int i_drive;
	if (umount < '0' || umount > 3+'0')
		i_drive = toupper(umount) - 'A';
	else
		i_drive = umount - '0';

	if (i_drive >= DOS_DRIVES || i_drive < 0)
		return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

	if (i_drive < MAX_DISK_IMAGES && Drives[i_drive] == NULL && imageDiskList[i_drive] == NULL)
		return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

	if (i_drive >= MAX_DISK_IMAGES && Drives[i_drive] == NULL)
		return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

	if (Drives[i_drive]) {
		/*
			isoDrive * cdrom = dynamic_cast<isoDrive*>(Drives[i_drive]);
		*/
						
		switch (DriveManager::UnmountDrive(i_drive)) {
			/*
			case 0: //success
				{
			*/
					/* 
						If the drive letter is also a CD-ROM drive attached to IDE, then let the IDE code know
					*/
			/*
				if (cdrom) IDE_CDROM_Detach(i_drive);

					Drives[i_drive] = NULL;
					if (i_drive == DOS_GetDefaultDrive())
						DOS_SetDrive(toupper('Z') - 'A');					
					return MSG_Get("PROGRAM_MOUNT_UMOUNT_SUCCESS");
				}
			*/			
			
			case 1: return MSG_Get("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL");
			case 2: return MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS");
		}
		Drives[i_drive] = 0;
		//mem_writeb(Real2Phys(dos.tables.mediaid)+i_drive*9,0);
		if (i_drive == DOS_GetDefaultDrive()) {
			DOS_SetDrive(ZDRIVE_NUM);
		}

	}

	if (i_drive < MAX_DISK_IMAGES && imageDiskList[i_drive]) {
		delete imageDiskList[i_drive];
		imageDiskList[i_drive] = NULL;
	}

	return MSG_Get("PROGRAM_MOUNT_UMOUNT_SUCCESS");
}


class MOUNT : public Program {
public:
	void Move_Z(char new_z) {
		char newz_drive = (char) toupper(new_z);
		int i_newz = newz_drive - 'A';
		if (i_newz >= 0 && i_newz < DOS_DRIVES-1 && !Drives[i_newz]) {
			ZDRIVE_NUM = i_newz;
			/* remap drives */
			Drives[i_newz] = Drives[25];
			Drives[25] = 0;
			if (!first_shell) return; //Should not be possible			
			/* Update environment */
			std::string line = "";
			char ppp[2] = {newz_drive,0};
			std::string tempenv = ppp; tempenv += ":\\";
			if (first_shell->GetEnvStr("PATH",line)){
				std::string::size_type idx = line.find('=');
				std::string value = line.substr(idx +1 , std::string::npos);
				while ( (idx = value.find("Z:\\")) != std::string::npos ||
					(idx = value.find("z:\\")) != std::string::npos  )
					value.replace(idx,3,tempenv);
				line = value;
			}
			if (!line.size()) line = tempenv;
			first_shell->SetEnv("PATH",line.c_str());
			tempenv += "COMMAND.COM";
			first_shell->SetEnv("COMSPEC",tempenv.c_str());

			/* Update batch file if running from Z: (very likely: autoexec) */
			if(first_shell->bf) {
				std::string &name = first_shell->bf->filename;
				if(name.length() >2 &&  name[0] == 'Z' && name[1] == ':') name[0] = newz_drive;
			}
			/* Change the active drive */
			if (DOS_GetDefaultDrive() == 25) DOS_SetDrive(i_newz);
		}
	}
	void ListMounts(void) {
		char name[DOS_NAMELENGTH_ASCII];Bit32u size;Bit16u date;Bit16u time;Bit8u attr;
		/* Command uses dta so set it to our internal dta */
		RealPt save_dta = dos.dta();
		dos.dta(dos.tables.tempdta);
		DOS_DTA dta(dos.dta());

		WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_1"));
		WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_FORMAT"),"Drive","Type","Label");

		for(int p = 0;p < 8;p++) WriteOut("----------");

		for (char d = 0; d < DOS_DRIVES; d++) {
			if (!Drives[d]) continue;

			char root[7] = {static_cast<char>('A'+d),':','\\','*','.','*',0};
			bool ret = DOS_FindFirst(root,DOS_ATTR_VOLUME);
			if (ret) {
				dta.GetResult(name,size,date,time,attr);
				DOS_FindNext(); //Mark entry as invalid
			} else name[0] = 0;

			/* Change 8.3 to 11.0 */
			char* dot = strchr(name,'.');
			if(dot && (dot - name == 8) ) { 
				name[8] = name[9];name[9] = name[10];name[10] = name[11];name[11] = 0;
			}

			root[1] = 0; //This way, the format string can be reused.
			WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_FORMAT"),root, Drives[d]->GetInfo(),name);		
		}
		dos.dta(save_dta);
	}

	void Run(void) {
		DOS_Drive * newdrive;char drive;
		std::string label;
		std::string umount;
		std::string newz;

		//Hack To allow long commandlines
		ChangeToLongCmd();
		/* Parse the command line */
		/* if the command line is empty show current mounts */
		if (!cmd->GetCount()) {
			ListMounts();
			return;
		}

		/* In secure mode don't allow people to change mount points. 
		 * Neither mount nor unmount */
		if(control->SecureMode()) {
			WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
			return;
		}
		bool path_relative_to_last_config = false;
		if (cmd->FindExist("-pr",true)) path_relative_to_last_config = true;

		/* Check for unmounting */
		if (cmd->FindString("-u",umount,false)) {
			WriteOut(UnmountHelper(umount[0]), toupper(umount[0]));
			return;
		}
		
		/* Check for moving Z: */
		/* Only allowing moving it once. It is merely a convenience added for the wine team */
		if (ZDRIVE_NUM == 25 && cmd->FindString("-z", newz,false)) {
				Move_Z(newz[0]);
			return;
		}
		/* Show list of cdroms */
		if (cmd->FindExist("-cd",false)) {
			WriteOut(MSG_Get("PROGRAM_MOUNT_PHYS_CDROMS_NOT_SUPPORTED"));
			return;
		}

		std::string type="dir";
		cmd->FindString("-t",type,true);
		bool iscdrom = (type =="cdrom"); //Used for mscdex bug cdrom label name emulation
		if (type=="floppy" || type=="dir" || type=="cdrom" || type =="overlay") {
			Bit16u sizes[4] ={0};
			Bit8u mediaid;
			std::string str_size = "";
			if (type=="floppy") {
				str_size="512,1,2880,2880";/* All space free */
				mediaid=0xF0;		/* Floppy 1.44 media */
			} else if (type=="dir" || type == "overlay") {
				// 512*32*32765==~500MB total size
				// 512*32*16000==~250MB total free size
				str_size="512,32,32765,16000";
				mediaid=0xF8;		/* Hard Disk */
			} else if (type=="cdrom") {
				str_size="2048,1,65535,0";
				mediaid=0xF8;		/* Hard Disk */
			} else {
				WriteOut(MSG_Get("PROGAM_MOUNT_ILL_TYPE"),type.c_str());
				return;
			}
			/* Parse the free space in mb's (kb's for floppies) */
			std::string mb_size;
			if(cmd->FindString("-freesize",mb_size,true)) {
				char teststr[1024];
				Bit16u freesize = static_cast<Bit16u>(atoi(mb_size.c_str()));
				if (type=="floppy") {
					// freesize in kb
					sprintf(teststr,"512,1,2880,%d",freesize*1024/(512*1));
				} else {
					Bit32u total_size_cyl=32765;
					Bit32u free_size_cyl=(Bit32u)freesize*1024*1024/(512*32);
					if (free_size_cyl>65534) free_size_cyl=65534;
					if (total_size_cyl<free_size_cyl) total_size_cyl=free_size_cyl+10;
					if (total_size_cyl>65534) total_size_cyl=65534;
					sprintf(teststr,"512,32,%d,%d",total_size_cyl,free_size_cyl);
				}
				str_size=teststr;
			}
		   
			cmd->FindString("-size",str_size,true);
			char number[21] = { 0 };const char * scan = str_size.c_str();
			Bitu index = 0;Bitu count = 0;
			/* Parse the str_size string */
			while (*scan && index < 20 && count < 4) {
				if (*scan==',') {
					number[index] = 0;
					sizes[count++] = atoi(number);
					index = 0;
				} else number[index++] = *scan;
				scan++;
			}
			if (count < 4) {
				number[index] = 0; //always goes correct as index is max 20 at this point.
				sizes[count] = atoi(number);
			}
		
			// get the drive letter
			cmd->FindCommand(1,temp_line);
			if ((temp_line.size() > 2) || ((temp_line.size()>1) && (temp_line[1]!=':'))) goto showusage;
			int i_drive = toupper(temp_line[0]);
			if (!isalpha(i_drive)) goto showusage;
			if ((i_drive - 'A') >= DOS_DRIVES || (i_drive-'A') < 0 ) goto showusage;
			drive = static_cast<char>(i_drive);

			if (!cmd->FindCommand(2,temp_line)) goto showusage;
			if (!temp_line.size()) goto showusage;
			if(path_relative_to_last_config && control->configfiles.size() && !Cross::IsPathAbsolute(temp_line)) {
				std::string lastconfigdir(control->configfiles[control->configfiles.size()-1]);
				std::string::size_type pos = lastconfigdir.rfind(CROSS_FILESPLIT);
				if(pos == std::string::npos) pos = 0; //No directory then erase string
				lastconfigdir.erase(pos);
				if (lastconfigdir.length())	temp_line = lastconfigdir + CROSS_FILESPLIT + temp_line;
			}
			struct stat test;
			//Win32 : strip tailing backslashes
			//os2: some special drive check
			//rest: substitute ~ for home
			bool failed = false;
#if defined (WIN32) || defined(OS2)
			/* Removing trailing backslash if not root dir so stat will succeed */
			if(temp_line.size() > 3 && temp_line[temp_line.size()-1]=='\\') temp_line.erase(temp_line.size()-1,1);
			if (stat(temp_line.c_str(),&test)) {
#endif
#if defined(WIN32)
// Nothing to do here.
#elif defined (OS2)
				if (temp_line.size() <= 2) // Seems to be a drive.
				{
					failed = true;
					HFILE cdrom_fd = 0;
					ULONG ulAction = 0;

					APIRET rc = DosOpen((unsigned char*)temp_line.c_str(), &cdrom_fd, &ulAction, 0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
						OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0L);
					DosClose(cdrom_fd);
					if (rc != NO_ERROR && rc != ERROR_NOT_READY)
					{
						failed = true;
					} else {
						failed = false;
					}
				}
			}
			if (failed) {
#else
			if (stat(temp_line.c_str(),&test)) {
				failed = true;
				Cross::ResolveHomedir(temp_line);
				//Try again after resolving ~
				if(!stat(temp_line.c_str(),&test)) failed = false;
			}
			if(failed) {
#endif
				WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_1"),temp_line.c_str());
				return;
			}
			/* Not a switch so a normal directory/file */
			if (!S_ISDIR(test.st_mode)) {
#ifdef OS2
				HFILE cdrom_fd = 0;
				ULONG ulAction = 0;

				APIRET rc = DosOpen((unsigned char*)temp_line.c_str(), &cdrom_fd, &ulAction, 0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
					OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0L);
				DosClose(cdrom_fd);
				if (rc != NO_ERROR && rc != ERROR_NOT_READY) {
				WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str());
				return;
			}
#else
				WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str());
				return;
#endif
			}

			if (temp_line[temp_line.size()-1]!=CROSS_FILESPLIT) temp_line+=CROSS_FILESPLIT;
			Bit8u bit8size=(Bit8u) sizes[1];
			if (type=="cdrom") {
				if (cmd->FindExist("-usecd",false)
				    || cmd->FindExist("-aspi",false)
				    || cmd->FindExist("-ioctl_dio",false)
				    || cmd->FindExist("-ioctl_dx",false)
#if defined (WIN32)
				    || cmd->FindExist("-ioctl_mci",false)
#endif
				    || cmd->FindExist("-noioctl",false)
				) {
					WriteOut(MSG_Get("PROGRAM_MOUNT_PHYS_CDROMS_NOT_SUPPORTED"));
					/* Just ignore, mount anyway */
				}
				int error = 0;
				newdrive  = new cdromDrive(drive,temp_line.c_str(),sizes[0],bit8size,sizes[2],0,mediaid,error);
				// Check Mscdex, if it worked out...
				switch (error) {
					case 0  :	WriteOut(MSG_Get("MSCDEX_SUCCESS"));				break;
					case 1  :	WriteOut(MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS"));	break;
					case 2  :	WriteOut(MSG_Get("MSCDEX_ERROR_NOT_SUPPORTED"));	break;
					case 3  :	WriteOut(MSG_Get("MSCDEX_ERROR_PATH"));				break;
					case 4  :	WriteOut(MSG_Get("MSCDEX_TOO_MANY_DRIVES"));		break;
					case 5  :	WriteOut(MSG_Get("MSCDEX_LIMITED_SUPPORT"));		break;
					default :	WriteOut(MSG_Get("MSCDEX_UNKNOWN_ERROR"));			break;
				};
				if (error && error!=5) {
					delete newdrive;
					return;
				}
			} else {
				/* Give a warning when mount c:\ or the / */
#if defined (WIN32) || defined(OS2)
				if( (temp_line == "c:\\") || (temp_line == "C:\\") || 
				    (temp_line == "c:/") || (temp_line == "C:/")    )	
					WriteOut(MSG_Get("PROGRAM_MOUNT_WARNING_WIN"));
#else
				if(temp_line == "/") WriteOut(MSG_Get("PROGRAM_MOUNT_WARNING_OTHER"));
#endif
				if(type == "overlay") {
					//Ensure that the base drive exists:
					if (!Drives[drive-'A']) { 
						WriteOut("No basedrive mounted yet!");
						return;
					}
					localDrive* ldp = dynamic_cast<localDrive*>(Drives[drive-'A']);
					cdromDrive* cdp = dynamic_cast<cdromDrive*>(Drives[drive-'A']);
					if (!ldp || cdp) {
						WriteOut("Basedrive not compatible");
						return;
					}
					std::string base = ldp->getBasedir();
					Bit8u o_error = 0;
					newdrive = new Overlay_Drive(base.c_str(),temp_line.c_str(),sizes[0],bit8size,sizes[2],sizes[3],mediaid,o_error);
					//Erase old drive on success
					if (newdrive) {
						if (o_error) { 
							if (o_error == 1) WriteOut("No mixing of relative and absolute paths. Overlay failed.");
							else if (o_error == 2) WriteOut("overlay directory can not be the same as underlying file system.");
							else WriteOut("Something went wrong");
							delete newdrive;
							return;
						}
						delete Drives[drive-'A'];
						Drives[drive-'A'] = 0;
					} else { 
						WriteOut("overlaydrive construction failed.");
						return;
					}
				} else {
					newdrive=new localDrive(temp_line.c_str(),sizes[0],bit8size,sizes[2],sizes[3],mediaid);
				}
			}
		} else {
			WriteOut(MSG_Get("PROGRAM_MOUNT_ILL_TYPE"),type.c_str());
			return;
		}
		if (Drives[drive-'A']) {
			WriteOut(MSG_Get("PROGRAM_MOUNT_ALREADY_MOUNTED"),drive,Drives[drive-'A']->GetInfo());
			delete newdrive;
			newdrive = 0;
			return;
		}
		if (!newdrive) E_Exit("DOS:Can't create drive");
		Drives[drive-'A']=newdrive;
		/* Set the correct media byte in the table */
		mem_writeb(Real2Phys(dos.tables.mediaid)+(drive-'A')*9,newdrive->GetMediaByte());

		if (type != "overlay") WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"),drive,newdrive->GetInfo());
		else WriteOut("Overlay %s on drive %c mounted.\n",temp_line.c_str(),drive);
		/* check if volume label is given and don't allow it to updated in the future */
		if (cmd->FindString("-label",label,true)) newdrive->dirCache.SetLabel(label.c_str(),iscdrom,false);
		/* For hard drives set the label to DRIVELETTER_Drive.
		 * For floppy drives set the label to DRIVELETTER_Floppy.
		 * This way every drive except cdroms should get a label.*/
		else if(type == "dir" || type == "overlay") {
			label = drive; label += "_DRIVE";
			newdrive->dirCache.SetLabel(label.c_str(),iscdrom,false);
		} else if(type == "floppy") {
			label = drive; label += "_FLOPPY";
			newdrive->dirCache.SetLabel(label.c_str(),iscdrom,true);
		}
		if(type == "floppy") incrementFDD();
		return;
showusage:
#if defined (WIN32) || defined(OS2)
	   WriteOut(MSG_Get("PROGRAM_MOUNT_USAGE"),"d:\\dosprogs","d:\\dosprogs");
#else
	   WriteOut(MSG_Get("PROGRAM_MOUNT_USAGE"),"~/dosprogs","~/dosprogs");		   
#endif
		return;
	}
};

static void MOUNT_ProgramStart(Program * * make) {
	*make=new MOUNT;
}

class MEM : public Program {
public:
	void Run(void) {
		/* Show conventional Memory */
		WriteOut("\n");

		Bit16u umb_start=dos_infoblock.GetStartOfUMBChain();
		Bit8u umb_flag=dos_infoblock.GetUMBChainState();
		Bit8u old_memstrat=DOS_GetMemAllocStrategy()&0xff;
		if (umb_start!=0xffff) {
			if ((umb_flag&1)==1) DOS_LinkUMBsToMemChain(0);
			DOS_SetMemAllocStrategy(0);
		}

		Bit16u seg,blocks;blocks=0xffff;
		DOS_AllocateMemory(&seg,&blocks);
		WriteOut(MSG_Get("PROGRAM_MEM_CONVEN"),blocks*16/1024);

		if (umb_start!=0xffff) {
			DOS_LinkUMBsToMemChain(1);
			DOS_SetMemAllocStrategy(0x40);	// search in UMBs only

			Bit16u largest_block=0,total_blocks=0,block_count=0;
			for (;; block_count++) {
				blocks=0xffff;
				DOS_AllocateMemory(&seg,&blocks);
				if (blocks==0) break;
				total_blocks+=blocks;
				if (blocks>largest_block) largest_block=blocks;
				DOS_AllocateMemory(&seg,&blocks);
			}

			Bit8u current_umb_flag=dos_infoblock.GetUMBChainState();
			if ((current_umb_flag&1)!=(umb_flag&1)) DOS_LinkUMBsToMemChain(umb_flag);
			DOS_SetMemAllocStrategy(old_memstrat);	// restore strategy

			if (block_count>0) WriteOut(MSG_Get("PROGRAM_MEM_UPPER"),total_blocks*16/1024,block_count,largest_block*16/1024);
		}

		/* Test for and show free XMS */
		reg_ax=0x4300;CALLBACK_RunRealInt(0x2f);
		if (reg_al==0x80) {
			reg_ax=0x4310;CALLBACK_RunRealInt(0x2f);
			Bit16u xms_seg=SegValue(es);Bit16u xms_off=reg_bx;
			reg_ah=8;
			CALLBACK_RunRealFar(xms_seg,xms_off);
			if (!reg_bl) {
				WriteOut(MSG_Get("PROGRAM_MEM_EXTEND"),reg_dx);
			}
		}	
		/* Test for and show free EMS */
		Bit16u handle;
		char emm[9] = { 'E','M','M','X','X','X','X','0',0 };
		if (DOS_OpenFile(emm,0,&handle)) {
			DOS_CloseFile(handle);
			reg_ah=0x42;
			CALLBACK_RunRealInt(0x67);
			WriteOut(MSG_Get("PROGRAM_MEM_EXPAND"),reg_bx*16);
		}
	}
};


static void MEM_ProgramStart(Program * * make) {
	*make=new MEM;
}

extern Bit32u floppytype;


class BOOT : public Program {
private:
   
	FILE *getFSFile_mounted(char const* filename, Bit32u *ksize, Bit32u *bsize, Bit8u *error) {
		//if return NULL then put in error the errormessage code if an error was requested
		bool tryload = (*error)?true:false;
		*error = 0;
		Bit8u drive;
		FILE *tmpfile;
		char fullname[DOS_PATHLENGTH];

		localDrive* ldp=0;
		if (!DOS_MakeName(const_cast<char*>(filename),fullname,&drive)) return NULL;

		try {		
			ldp=dynamic_cast<localDrive*>(Drives[drive]);
			if(!ldp) return NULL;

			tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
			if(tmpfile == NULL) {
				if (!tryload) *error=1;
				return NULL;
			}

			// get file size
			fseek(tmpfile,0L, SEEK_END);
			*ksize = (ftell(tmpfile) / 1024);
			*bsize = ftell(tmpfile);
			fclose(tmpfile);

			tmpfile = ldp->GetSystemFilePtr(fullname, "rb+");
			if(tmpfile == NULL) {
//				if (!tryload) *error=2;
//				return NULL;
				WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
				tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
				if(tmpfile == NULL) {
					if (!tryload) *error=1;
					return NULL;
				}
			}

			return tmpfile;
		}
		catch(...) {
			return NULL;
		}
	}
   
	FILE *getFSFile(char const * filename, Bit32u *ksize, Bit32u *bsize,bool tryload=false) {
		Bit8u error = tryload?1:0;
		FILE* tmpfile = getFSFile_mounted(filename,ksize,bsize,&error);
		if(tmpfile) return tmpfile;
		//File not found on mounted filesystem. Try regular filesystem
		std::string filename_s(filename);
		Cross::ResolveHomedir(filename_s);
		tmpfile = fopen_wrap(filename_s.c_str(),"rb+");
		if(!tmpfile) {
			if( (tmpfile = fopen_wrap(filename_s.c_str(),"rb")) ) {
				//File exists; So can't be opened in correct mode => error 2
//				fclose(tmpfile);
//				if(tryload) error = 2;
				WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
				fseek(tmpfile,0L, SEEK_END);
				*ksize = (ftell(tmpfile) / 1024);
				*bsize = ftell(tmpfile);
				return tmpfile;
			}
			// Give the delayed errormessages from the mounted variant (or from above)
			if(error == 1) WriteOut(MSG_Get("PROGRAM_BOOT_NOT_EXIST"));
			if(error == 2) WriteOut(MSG_Get("PROGRAM_BOOT_NOT_OPEN"));
			return NULL;
		}
		fseek(tmpfile,0L, SEEK_END);
		*ksize = (ftell(tmpfile) / 1024);
		*bsize = ftell(tmpfile);
		return tmpfile;
	}

	void printError(void) {
		WriteOut(MSG_Get("PROGRAM_BOOT_PRINT_ERROR"));
	}

	void disable_umb_ems_xms(void) {
		Section* dos_sec = control->GetSection("dos");
		dos_sec->ExecuteDestroy(false);
		char test[20];
		strcpy(test,"umb=false");
		dos_sec->HandleInputline(test);
		strcpy(test,"xms=false");
		dos_sec->HandleInputline(test);
		strcpy(test,"ems=false");
		dos_sec->HandleInputline(test);
		dos_sec->ExecuteInit(false);
     }

public:
   
	void Run(void) {
		//Hack To allow long commandlines
		ChangeToLongCmd();
		/* In secure mode don't allow people to boot stuff. 
		 * They might try to corrupt the data on it */
		if(control->SecureMode()) {
			WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
			return;
		}

		FILE *usefile_1=NULL;
		FILE *usefile_2=NULL;
		Bitu i=0; 
		Bit32u floppysize=0;
		Bit32u rombytesize_1=0;
		Bit32u rombytesize_2=0;
		Bit8u drive = 'A';
		std::string cart_cmd="";

		if(!cmd->GetCount()) {
			printError();
			return;
		}
		while(i<cmd->GetCount()) {
			if(cmd->FindCommand(i+1, temp_line)) {
				if((temp_line == "-l") || (temp_line == "-L")) {
					/* Specifying drive... next argument then is the drive */
					i++;
					if(cmd->FindCommand(i+1, temp_line)) {
						drive=toupper(temp_line[0]);
						if ((drive != 'A') && (drive != 'C') && (drive != 'D') && (drive != 'E')) {
							printError();
							return;
						}

					} else {
						printError();
						return;
					}
					i++;
					continue;
				}

				if((temp_line == "-e") || (temp_line == "-E")) {
					/* Command mode for PCJr cartridges */
					i++;
					if(cmd->FindCommand(i + 1, temp_line)) {
						for(size_t ct = 0;ct < temp_line.size();ct++) temp_line[ct] = toupper(temp_line[ct]);
						cart_cmd = temp_line;
					} else {
						printError();
						return;
					}
					i++;
					continue;
				}

				if ( i >= MAX_SWAPPABLE_DISKS ) {
					return; //TODO give a warning.
				}
				
				WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_OPEN"), temp_line.c_str());
				Bit32u rombytesize;
				FILE *usefile = getFSFile(temp_line.c_str(), &floppysize, &rombytesize);
				if(usefile != NULL) {
					delete diskSwap[i];
					diskSwap[i] = new imageDisk(usefile, (Bit8u *)temp_line.c_str(), floppysize, false);
					if (usefile_1==NULL) {
						usefile_1=usefile;
						rombytesize_1=rombytesize;
					} else {
						usefile_2=usefile;
						rombytesize_2=rombytesize;
					}
				} else {
					WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_NOT_OPEN"), temp_line.c_str());
					return;
				}

			}
			i++;
		}

		swapPosition = 0;

		swapInDisks();

		if(imageDiskList[drive-65]==NULL) {
			WriteOut(MSG_Get("PROGRAM_BOOT_UNABLE"), drive);
			return;
		}

		bootSector bootarea;
		imageDiskList[drive-65]->Read_Sector(0,0,1,(Bit8u *)&bootarea);
		if ((bootarea.rawdata[0]==0x50) && (bootarea.rawdata[1]==0x43) && (bootarea.rawdata[2]==0x6a) && (bootarea.rawdata[3]==0x72)) {
			if (machine!=MCH_PCJR) WriteOut(MSG_Get("PROGRAM_BOOT_CART_WO_PCJR"));
			else {
				Bit8u rombuf[65536];
				Bits cfound_at=-1;
				if (cart_cmd!="") {
					/* read cartridge data into buffer */
					fseek(usefile_1,0x200L, SEEK_SET);
					fread(rombuf, 1, rombytesize_1-0x200, usefile_1);

					char cmdlist[1024];
					cmdlist[0]=0;
					Bitu ct=6;
					Bits clen=rombuf[ct];
					char buf[257];
					if (cart_cmd=="?") {
						while (clen!=0) {
							strncpy(buf,(char*)&rombuf[ct+1],clen);
							buf[clen]=0;
							upcase(buf);
							strcat(cmdlist," ");
							strcat(cmdlist,buf);
							ct+=1+clen+3;
							if (ct>sizeof(cmdlist)) break;
							clen=rombuf[ct];
						}
						if (ct>6) {
							WriteOut(MSG_Get("PROGRAM_BOOT_CART_LIST_CMDS"),cmdlist);
						} else {
							WriteOut(MSG_Get("PROGRAM_BOOT_CART_NO_CMDS"));
						}
						for(Bitu dct=0;dct<MAX_SWAPPABLE_DISKS;dct++) {
							delete diskSwap[dct];
							diskSwap[dct]=NULL;
						}
						//fclose(usefile_1); //delete diskSwap closes the file
						return;
					} else {
						while (clen!=0) {
							strncpy(buf,(char*)&rombuf[ct+1],clen);
							buf[clen]=0;
							upcase(buf);
							strcat(cmdlist," ");
							strcat(cmdlist,buf);
							ct+=1+clen;

							if (cart_cmd==buf) {
								cfound_at=ct;
								break;
							}

							ct+=3;
							if (ct>sizeof(cmdlist)) break;
							clen=rombuf[ct];
						}
						if (cfound_at<=0) {
							if (ct>6) {
								WriteOut(MSG_Get("PROGRAM_BOOT_CART_LIST_CMDS"),cmdlist);
							} else {
								WriteOut(MSG_Get("PROGRAM_BOOT_CART_NO_CMDS"));
							}
							for(Bitu dct=0;dct<MAX_SWAPPABLE_DISKS;dct++) {
								delete diskSwap[dct];
								diskSwap[dct]=NULL;
							}
							//fclose(usefile_1); //Delete diskSwap closes the file
							return;
						}
					}
				}
				/* DOSBox-MB IMGMAKE patch. ========================================================================= */
				if((bootarea.rawdata[0]==0) && (bootarea.rawdata[1]==0)) {
					WriteOut_NoParsing("PROGRAM_BOOT_UNABLE");
					return;
				}				
				/* DOSBox-MB IMGMAKE patch. ========================================================================= */				
				disable_umb_ems_xms();
				void PreparePCJRCartRom(void);
				PreparePCJRCartRom();

				if (usefile_1==NULL) return;

				Bit32u sz1,sz2;
				FILE *tfile = getFSFile("system.rom", &sz1, &sz2, true);
				if (tfile!=NULL) {
					fseek(tfile, 0x3000L, SEEK_SET);
					Bit32u drd=(Bit32u)fread(rombuf, 1, 0xb000, tfile);
					if (drd==0xb000) {
						for(i=0;i<0xb000;i++) phys_writeb(0xf3000+i,rombuf[i]);
					}
					fclose(tfile);
				}

				if (usefile_2!=NULL) {
					fseek(usefile_2, 0x0L, SEEK_SET);
					fread(rombuf, 1, 0x200, usefile_2);
					PhysPt romseg_pt=host_readw(&rombuf[0x1ce])<<4;

					/* read cartridge data into buffer */
					fseek(usefile_2, 0x200L, SEEK_SET);
					fread(rombuf, 1, rombytesize_2-0x200, usefile_2);
					//fclose(usefile_2); //usefile_2 is in diskSwap structure which should be deleted to close the file

					/* write cartridge data into ROM */
					for(i=0;i<rombytesize_2-0x200;i++) phys_writeb(romseg_pt+i,rombuf[i]);
				}

				fseek(usefile_1, 0x0L, SEEK_SET);
				fread(rombuf, 1, 0x200, usefile_1);
				Bit16u romseg=host_readw(&rombuf[0x1ce]);

				/* read cartridge data into buffer */
				fseek(usefile_1,0x200L, SEEK_SET);
				fread(rombuf, 1, rombytesize_1-0x200, usefile_1);
				//fclose(usefile_1); //usefile_1 is in diskSwap structure which should be deleted to close the file

				/* write cartridge data into ROM */
				for(i=0;i<rombytesize_1-0x200;i++) phys_writeb((romseg<<4)+i,rombuf[i]);

				//Close cardridges
				for(Bitu dct=0;dct<MAX_SWAPPABLE_DISKS;dct++) {
					delete diskSwap[dct];
					diskSwap[dct]=NULL;
				}


				if (cart_cmd=="") {
					Bit32u old_int18=mem_readd(0x60);
					/* run cartridge setup */
					SegSet16(ds,romseg);
					SegSet16(es,romseg);
					SegSet16(ss,0x8000);
					reg_esp=0xfffe;
					CALLBACK_RunRealFar(romseg,0x0003);

					Bit32u new_int18=mem_readd(0x60);
					if (old_int18!=new_int18) {
						/* boot cartridge (int18) */
						SegSet16(cs,RealSeg(new_int18));
						reg_ip = RealOff(new_int18);
					} 
				} else {
					if (cfound_at>0) {
						/* run cartridge setup */
						SegSet16(ds,dos.psp());
						SegSet16(es,dos.psp());
						CALLBACK_RunRealFar(romseg,cfound_at);
					}
				}
			}
		} else {
			disable_umb_ems_xms();
			void RemoveEMSPageFrame(void);
			RemoveEMSPageFrame();
			WriteOut(MSG_Get("PROGRAM_BOOT_BOOT"), drive);
			for(i=0;i<512;i++) real_writeb(0, 0x7c00 + i, bootarea.rawdata[i]);

			/* create appearance of floppy drive DMA usage (Demon's Forge) */
			if (!IS_TANDY_ARCH && floppysize!=0) GetDMAChannel(2)->tcount=true;

			/* revector some dos-allocated interrupts */
			real_writed(0,0x01*4,0xf000ff53);
			real_writed(0,0x03*4,0xf000ff53);

			SegSet16(cs, 0);
			reg_ip = 0x7c00;
			SegSet16(ds, 0);
			SegSet16(es, 0);
			/* set up stack at a safe place */
			SegSet16(ss, 0x7000);
			reg_esp = 0x100;
			reg_esi = 0;
			reg_ecx = 1;
			reg_ebp = 0;
			reg_eax = 0;
			reg_edx = 0; //Head 0 drive 0
			reg_ebx= 0x7c00; //Real code probably uses bx to load the image
		}
	}
};

static void BOOT_ProgramStart(Program * * make) {
	*make=new BOOT;
}


class LOADROM : public Program {
public:
	void Run(void) {
		if (!(cmd->FindCommand(1, temp_line))) {
			WriteOut(MSG_Get("PROGRAM_LOADROM_SPECIFY_FILE"));
			return;
		}

		Bit8u drive;
		char fullname[DOS_PATHLENGTH];
		localDrive* ldp=0;
		if (!DOS_MakeName((char *)temp_line.c_str(),fullname,&drive)) return;

		try {
			/* try to read ROM file into buffer */
			ldp=dynamic_cast<localDrive*>(Drives[drive]);
			if(!ldp) return;

			FILE *tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
			if(tmpfile == NULL) {
				WriteOut(MSG_Get("PROGRAM_LOADROM_CANT_OPEN"));
				return;
			}
			fseek(tmpfile, 0L, SEEK_END);
			if (ftell(tmpfile)>0x8000) {
				WriteOut(MSG_Get("PROGRAM_LOADROM_TOO_LARGE"));
				fclose(tmpfile);
				return;
			}
			fseek(tmpfile, 0L, SEEK_SET);
			Bit8u rom_buffer[0x8000];
			Bitu data_read = fread(rom_buffer, 1, 0x8000, tmpfile);
			fclose(tmpfile);

			/* try to identify ROM type */
			PhysPt rom_base = 0;
			if (data_read >= 0x4000 && rom_buffer[0] == 0x55 && rom_buffer[1] == 0xaa &&
				(rom_buffer[3] & 0xfc) == 0xe8 && strncmp((char*)(&rom_buffer[0x1e]), "IBM", 3) == 0) {

				if (!IS_EGAVGA_ARCH) {
					WriteOut(MSG_Get("PROGRAM_LOADROM_INCOMPATIBLE"));
					return;
				}
				rom_base = PhysMake(0xc000, 0); // video BIOS
			}
			else if (data_read == 0x8000 && rom_buffer[0] == 0xe9 && rom_buffer[1] == 0x8f &&
				rom_buffer[2] == 0x7e && strncmp((char*)(&rom_buffer[0x4cd4]), "IBM", 3) == 0) {

				rom_base = PhysMake(0xf600, 0); // BASIC
			}

			if (rom_base) {
				/* write buffer into ROM */
				for (Bitu i=0; i<data_read; i++) phys_writeb(rom_base + i, rom_buffer[i]);

				if (rom_base == 0xc0000) {
					/* initialize video BIOS */
					phys_writeb(PhysMake(0xf000, 0xf065), 0xcf);
					reg_flags &= ~FLAG_IF;
					CALLBACK_RunRealFar(0xc000, 0x0003);
					LOG_MSG("Video BIOS ROM loaded and initialized.");
				}
				else WriteOut(MSG_Get("PROGRAM_LOADROM_BASIC_LOADED"));
			}
			else WriteOut(MSG_Get("PROGRAM_LOADROM_UNRECOGNIZED"));
		}
		catch(...) {
			return;
		}
	}
};

static void LOADROM_ProgramStart(Program * * make) {
	*make=new LOADROM;
}

class BIOSTEST : public Program {
public:
	void Run(void) {
		if (!(cmd->FindCommand(1, temp_line))) {
			WriteOut("Must specify BIOS file to load.\n");
			return;
		}

		Bit8u drive;
		char fullname[DOS_PATHLENGTH];
		localDrive* ldp = 0;
		if (!DOS_MakeName((char *)temp_line.c_str(), fullname, &drive)) return;

		try {
			/* try to read ROM file into buffer */
			ldp = dynamic_cast<localDrive*>(Drives[drive]);
			if (!ldp) return;

			FILE *tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
			if (tmpfile == NULL) {
				WriteOut("Can't open a file");
				return;
			}
			fseek(tmpfile, 0L, SEEK_END);
			if (ftell(tmpfile) > 64 * 1024) {
				WriteOut("BIOS File too large");
				fclose(tmpfile);
				return;
			}
			fseek(tmpfile, 0L, SEEK_SET);
			Bit8u buffer[64*1024];
			Bitu data_read = fread(buffer, 1, sizeof( buffer), tmpfile);
			fclose(tmpfile);

			Bit32u rom_base = PhysMake(0xf000, 0); // override regular dosbox bios
			/* write buffer into ROM */
			for (Bitu i = 0; i < data_read; i++) phys_writeb(rom_base + i, buffer[i]);

			//Start executing this bios
			memset(&cpu_regs, 0, sizeof(cpu_regs));
			memset(&Segs, 0, sizeof(Segs));

			
			SegSet16(cs, 0xf000);
			reg_eip = 0xfff0;
		}
		catch (...) {
			return;
		}
	}
};

static void BIOS_ProgramStart(Program * * make) {
	*make = new BIOSTEST;
}

/* DOSBox-MB IMGMAKE patch. ========================================================================= */
const Bit8u freedos_mbr[] = {
	0x33,0xC0,0x8E,0xC0,0x8E,0xD8,0x8E,0xD0,0xBC,0x00,0x7C,0xFC,0x8B,0xF4,0xBF,0x00,
	0x06,0xB9,0x00,0x01,0xF2,0xA5,0xEA,0x67,0x06,0x00,0x00,0x8B,0xD5,0x58,0xA2,0x4F, // 10h
	0x07,0x3C,0x35,0x74,0x23,0xB4,0x10,0xF6,0xE4,0x05,0xAE,0x04,0x8B,0xF0,0x80,0x7C, // 20h
	0x04,0x00,0x74,0x44,0x80,0x7C,0x04,0x05,0x74,0x3E,0xC6,0x04,0x80,0xE8,0xDA,0x00,
	0x8A,0x74,0x01,0x8B,0x4C,0x02,0xEB,0x08,0xE8,0xCF,0x00,0xB9,0x01,0x00,0x32,0xD1, // 40h
	0xBB,0x00,0x7C,0xB8,0x01,0x02,0xCD,0x13,0x72,0x1E,0x81,0xBF,0xFE,0x01,0x55,0xAA,
	0x75,0x16,0xEA,0x00,0x7C,0x00,0x00,0x80,0xFA,0x81,0x74,0x02,0xB2,0x80,0x8B,0xEA,
	0x42,0x80,0xF2,0xB3,0x88,0x16,0x41,0x07,0xBF,0xBE,0x07,0xB9,0x04,0x00,0xC6,0x06,
	0x34,0x07,0x31,0x32,0xF6,0x88,0x2D,0x8A,0x45,0x04,0x3C,0x00,0x74,0x23,0x3C,0x05, // 80h
	0x74,0x1F,0xFE,0xC6,0xBE,0x31,0x07,0xE8,0x71,0x00,0xBE,0x4F,0x07,0x46,0x46,0x8B,
	0x1C,0x0A,0xFF,0x74,0x05,0x32,0x7D,0x04,0x75,0xF3,0x8D,0xB7,0x7B,0x07,0xE8,0x5A,
	0x00,0x83,0xC7,0x10,0xFE,0x06,0x34,0x07,0xE2,0xCB,0x80,0x3E,0x75,0x04,0x02,0x74,
	0x0B,0xBE,0x42,0x07,0x0A,0xF6,0x75,0x0A,0xCD,0x18,0xEB,0xAC,0xBE,0x31,0x07,0xE8,
	0x39,0x00,0xE8,0x36,0x00,0x32,0xE4,0xCD,0x1A,0x8B,0xDA,0x83,0xC3,0x60,0xB4,0x01,
	0xCD,0x16,0xB4,0x00,0x75,0x0B,0xCD,0x1A,0x3B,0xD3,0x72,0xF2,0xA0,0x4F,0x07,0xEB,
	0x0A,0xCD,0x16,0x8A,0xC4,0x3C,0x1C,0x74,0xF3,0x04,0xF6,0x3C,0x31,0x72,0xD6,0x3C,
	0x35,0x77,0xD2,0x50,0xBE,0x2F,0x07,0xBB,0x1B,0x06,0x53,0xFC,0xAC,0x50,0x24,0x7F, //100h
	0xB4,0x0E,0xCD,0x10,0x58,0xA8,0x80,0x74,0xF2,0xC3,0x56,0xB8,0x01,0x03,0xBB,0x00, //110h
	0x06,0xB9,0x01,0x00,0x32,0xF6,0xCD,0x13,0x5E,0xC6,0x06,0x4F,0x07,0x3F,0xC3,0x0D, //120h
	0x8A,0x0D,0x0A,0x46,0x35,0x20,0x2E,0x20,0x2E,0x20,0x2E,0xA0,0x64,0x69,0x73,0x6B,
	0x20,0x32,0x0D,0x0A,0x0A,0x44,0x65,0x66,0x61,0x75,0x6C,0x74,0x3A,0x20,0x46,0x31, //140h
	0xA0,0x00,0x01,0x00,0x04,0x00,0x06,0x03,0x07,0x07,0x0A,0x0A,0x63,0x0E,0x64,0x0E,
	0x65,0x14,0x80,0x19,0x81,0x19,0x82,0x19,0x83,0x1E,0x93,0x24,0xA5,0x2B,0x9F,0x2F,
	0x75,0x33,0x52,0x33,0xDB,0x36,0x40,0x3B,0xF2,0x41,0x00,0x44,0x6F,0xF3,0x48,0x70,
	0x66,0xF3,0x4F,0x73,0xB2,0x55,0x6E,0x69,0xF8,0x4E,0x6F,0x76,0x65,0x6C,0xEC,0x4D, //180h
	0x69,0x6E,0x69,0xF8,0x4C,0x69,0x6E,0x75,0xF8,0x41,0x6D,0x6F,0x65,0x62,0xE1,0x46,
	0x72,0x65,0x65,0x42,0x53,0xC4,0x42,0x53,0x44,0xE9,0x50,0x63,0x69,0xF8,0x43,0x70,
	0xED,0x56,0x65,0x6E,0x69,0xF8,0x44,0x6F,0x73,0x73,0x65,0xE3,0x3F,0xBF,0x00,0x00, //1B0h
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x55,0xAA
	};
	
#ifdef WIN32
	#include <winioctl.h>
#endif

class IMGMAKE : public Program {
public:
#ifdef WIN32
	bool OpenDisk(HANDLE* f, OVERLAPPED* o, Bit8u* name) {
		o->hEvent = INVALID_HANDLE_VALUE;
		*f = CreateFile( (LPCSTR)name, GENERIC_READ | GENERIC_WRITE,
			0,    // exclusive access
			NULL, // default security attributes
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL );

		if (*f == INVALID_HANDLE_VALUE) return false;

		// init OVERLAPPED
		o->Internal = 0;
		o->InternalHigh = 0;
		o->Offset = 0;
		o->OffsetHigh = 0;
		o->hEvent = CreateEvent(
			NULL,   // default security attributes
			TRUE,   // manual-reset event
			FALSE,  // not signaled
			NULL    // no name
			);
		return true;
	}

	void CloseDisk(HANDLE f, OVERLAPPED* o) {
		if(f != INVALID_HANDLE_VALUE) CloseHandle(f);
		if(o->hEvent != INVALID_HANDLE_VALUE) CloseHandle(o->hEvent);
	}

	bool StartReadDisk(HANDLE f, OVERLAPPED* o, Bit8u* buffer, Bitu offset, Bitu size) {
		o->Offset = offset;
		if (!ReadFile(f, buffer, size, NULL, o) &&
			(GetLastError()==ERROR_IO_PENDING)) return true;
		return false;
	}

	// 0=still waiting, 1=catastrophic faliure, 2=success, 3=sector not found, 4=crc error
	Bitu CheckDiskReadComplete(HANDLE f, OVERLAPPED* o) {
		DWORD numret;
		BOOL b = GetOverlappedResult( f, o, &numret,false);
		if(b) return 2;
		else {
			int error = GetLastError();
			if(error==ERROR_IO_INCOMPLETE)			return 0;
			if(error==ERROR_FLOPPY_UNKNOWN_ERROR)	return 5;
			if(error==ERROR_CRC)					return 4;
			if(error==ERROR_SECTOR_NOT_FOUND)		return 3;
			return 1;
		}
	}

	Bitu ReadDisk(FILE* f, Bit8u driveletter, Bitu retries_max) {
		unsigned char data[36*2*512];
		HANDLE hFloppy;
		DWORD numret;
		OVERLAPPED o;
		DISK_GEOMETRY geom;

		Bit8u drivestring[] = "\\\\.\\x:"; drivestring[4]=driveletter;
		if(!OpenDisk(&hFloppy, &o, drivestring)) return false;

		// get drive geom
		DeviceIoControl( hFloppy, IOCTL_DISK_GET_DRIVE_GEOMETRY,NULL,0,
		&geom,sizeof(DISK_GEOMETRY),&numret,NULL);

		switch(geom.MediaType) {
			case F5_1Pt2_512: case F3_1Pt44_512: case F3_2Pt88_512:	case F3_720_512:
			case F5_360_512: case F5_320_512: case F5_180_512: case F5_160_512:
				break;
			default:
				CloseDisk(hFloppy,&o);
				return false;
		}
		Bitu total_sect_per_cyl = geom.SectorsPerTrack * geom.TracksPerCylinder;
		Bitu cyln_size = 512 * total_sect_per_cyl;

		WriteOut(MSG_Get("PROGRAM_IMGMAKE_FLREAD"),
			geom.Cylinders.LowPart,geom.TracksPerCylinder,
			geom.SectorsPerTrack,(cyln_size*geom.Cylinders.LowPart)/1024);
		WriteOut(MSG_Get("PROGRAM_IMGMAKE_FLREAD2"));

		for(Bitu i = 0; i < geom.Cylinders.LowPart; i++) {
			Bitu result;
			// for each cylinder
			WriteOut("%2u",i);

			if(!StartReadDisk(hFloppy, &o, &data[0], cyln_size*i, cyln_size)){
				CloseDisk(hFloppy,&o);
				return false;
			}
			do {
				result = CheckDiskReadComplete(hFloppy, &o);
				CALLBACK_Idle();
			}
			while (result==0);

			switch(result) {
			case 1:
				CloseDisk(hFloppy,&o);
				return false;
			case 2: // success
				for(Bitu m=0; m < cyln_size/512; m++) WriteOut("\xdb");
				break;
			case 3:
			case 4: // data errors
			case 5:
				for(Bitu k=0; k < total_sect_per_cyl; k++) {
					Bitu retries=retries_max;
restart_int:
					StartReadDisk(hFloppy, &o, &data[512*k], cyln_size*i + 512*k, 512);
					do {
						result = CheckDiskReadComplete(hFloppy, &o);
						CALLBACK_Idle();
					}
					while (result==0);

					switch(result) {
					case 1: // bad error
						CloseDisk(hFloppy,&o);
						return false;
					case 2: // success
						if(retries==retries_max) WriteOut("\xdb");
						else WriteOut("\b\b\b\xb1");
						break;
					case 3:
					case 4: // read errors
					case 5:
						if(retries!=retries_max) WriteOut("\b\b\b");
						retries--;
						switch(result) {
							case 3: WriteOut("x");
							case 4: WriteOut("!");
							case 5: WriteOut("?");
						}
						WriteOut("%2d",retries);

						if(retries)	goto restart_int;
						const Bit8u badfood[]="IMGMAKE BAD FLOPPY SECTOR \xBA\xAD\xF0\x0D";
						for(Bitu z = 0; z < 512/32; z++)
							memcpy(&data[512*k+z*32],badfood,32);
						WriteOut("\b\b");
						break;
					}
				}
				break;
			}
			fwrite(data, 512, total_sect_per_cyl, f);
			WriteOut("%2x%2x\n", data[0], data[1]);
		}
		// seek to 0
		StartReadDisk(hFloppy, &o, &data[0], 0, 512);
		CloseDisk(hFloppy,&o);
		return true;
	}
#endif

	void Run(void) {
		std::string disktype;
		std::string src;
		std::string filename;
		std::string path = "";
		std::string dpath;

		Bitu c, h, s, sectors;
		Bit64u size = 0;

		if(cmd->FindExist("-?")) {
			printHelp();
			return;
		}

/*
		this stuff is too frustrating

		// when only a filename is passed try to create the file on the current DOS path
		// if directory+filename are passed first see if directory is a host path, if not
		// maybe it is a DOSBox path.

		// split filename and path
		Bitu spos = temp_line.rfind('\\');
		if(spos==std::string::npos) {
			temp_line.rfind('/');
		}

		if(spos==std::string::npos) {
			// no path separator
			filename=temp_line;
		} else {
			path=temp_line.substr(0,spos);
			filename=temp_line.substr(spos+1,std::string::npos);
		}
		if(filename=="")

		char tbuffer[DOS_PATHLENGTH]= { 0 };
		if(path=="") {
			if(!DOS_GetCurrentDir(DOS_GetDefaultDrive()+1,tbuffer)){
				printHelp();
				return;
			}
			dpath=(std::string)tbuffer;
		}

		WriteOut("path %s, filename %s, dpath %s",
			path.c_str(),filename.c_str(),dpath.c_str());
		return;
*/

#ifdef WIN32
		// read from real floppy?
		if(cmd->FindString("-source",src,true)) {
			Bits retries = 10;
			cmd->FindInt("-retries",retries,true);
			if((retries < 1)||(retries > 99))  {
				printHelp();
				return;
			}
			if((src.length()!=1) || !isalpha(src.c_str()[0])) {
				// only one letter allowed
				printHelp();
				return;
			}

			// temp_line is the filename
			if (!(cmd->FindCommand(1, temp_line))) {
				printHelp();
				return;
			}

			// don't trash user's files
			FILE* f = fopen(temp_line.c_str(),"r");
			if(f) {
				fclose(f);
				WriteOut(MSG_Get("PROGRAM_IMGMAKE_FILE_EXISTS"),temp_line.c_str());
				return;
			}
			f = fopen(temp_line.c_str(),"wb+");
			if (!f) {
				WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANNOT_WRITE"),temp_line.c_str());
				return;
			}
			// maybe delete f if it failed?
			if(!ReadDisk(f, src.c_str()[0],retries))
				WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANT_READ_FLOPPY"));
			fclose(f);
			return;
		}
#endif
		// disk type
		if (!(cmd->FindString("-t",disktype,true))) {
			printHelp();
			return;
		}

		Bit8u mediadesc = 0xF8; // media descriptor byte; also used to differ fd and hd
		Bitu root_ent = 512; // FAT root directory entries: 512 is for harddisks
		if(disktype=="fd_160") {
			c = 40; h = 1; s = 8; mediadesc = 0xFE; root_ent = 56; // root_ent?
		} else if(disktype=="fd_180") {
			c = 40; h = 1; s = 9; mediadesc = 0xFC; root_ent = 56; // root_ent?
		} else if(disktype=="fd_200") {
			c = 40; h = 1; s = 10; mediadesc = 0xFC; root_ent = 56; // root_ent?
		} else if(disktype=="fd_320") {
			c = 40; h = 2; s = 8; mediadesc = 0xFF; root_ent = 112; // root_ent?
		} else if(disktype=="fd_360") {
			c = 40; h = 2; s = 9; mediadesc = 0xFD; root_ent = 112;
		} else if(disktype=="fd_400") {
			c = 40; h = 2; s = 10; mediadesc = 0xFD; root_ent = 112; // root_ent?
		} else if(disktype=="fd_720") {
			c = 80; h = 2; s = 9; mediadesc = 0xF9; root_ent = 112;
		} else if(disktype=="fd_1200") {
			c = 80; h = 2; s = 15; mediadesc = 0xF9; root_ent = 224;
		} else if(disktype=="fd_1440") {
			c = 80; h = 2; s = 18; mediadesc = 0xF0; root_ent = 224;
		} else if(disktype=="fd_2880") {
			c = 80; h = 2; s = 36; mediadesc = 0xF0; root_ent = 512; // root_ent?
		} else if(disktype=="hd_250") {
			c = 489; h = 16; s = 63;
		} else if(disktype=="hd_520") {
			c = 1023; h = 16; s = 63;
		} else if(disktype=="hd_2gig") {
			c = 1023; h = 64; s = 63;
		} else if(disktype=="hd_4gig") { // fseek only supports 2gb
			c = 1023; h = 130; s = 63;
		} else if(disktype=="hd_8gig") { // fseek only supports 2gb
			c = 1023; h = 255; s = 63;
		} else if(disktype=="hd_st251") { // old 40mb drive
			c = 820; h = 6; s = 17;
		} else if(disktype=="hd_st225") { // even older 20mb drive
			c = 615; h = 4; s = 17;
		} else if(disktype=="hd") {
			// get size from parameter
			std::string isize;
			if (!(cmd->FindString("-size",isize,true))) {
				// maybe -chs?
				if (!(cmd->FindString("-chs",isize,true))){
						// user forgot -size and -chs
						printHelp();
						return;
				}
				else {
					// got chs data: -chs 1023,16,63
					if(sscanf(isize.c_str(),"%u,%u,%u",&c,&h,&s) != 3) {
						printHelp();
						return;
					}
					// sanity-check chs values
					if((h>255)||(c>1023)||(s>63)) {
						printHelp();
						return;
					}
					size = c*h*s*512LL;
					if((size < 3*1024*1024) || (size > 0x1FFFFFFFFLL)) {
						// user picked geometry resulting in wrong size
						printHelp();
						return;
					}
				}
			} else {
				// got -size
				std::istringstream stream(isize);
				stream >> size;
				size *= 1024*1024LL; // size in megabytes
				// low limit: 3 megs, high limit: 2 gigs
				// Int13 limit would be 8 gigs
				if((size < 3*1024*1024LL) || (size > 0x1FFFFFFFFLL)) {
					// wrong size
					printHelp();
					return;
				}
				sectors = (Bitu)(size / 512);

				// Now that we finally have the proper size, figure out good CHS values
				h=2;
				while(h*1023*63 < sectors) h <<= 1;
				if(h>255) h=255;
				s=8;
				while(h*s*1023 < sectors) s *= 2;
				if(s>63) s=63;
				c=sectors/(h*s);
				if(c>1023) c=1023;
			}
		} else {
			// user passed a wrong -t argument
			printHelp();
			return;
		}

		std::string t2 = "";
		if(cmd->FindExist("-bat",true)) {
			t2 = "-bat";
		} else if(cmd->FindExist("-txt",true)) {
			t2 = "-txt";
		}

		size = c*h*s*512LL;
		Bits bootsect_pos = 0; // offset of the boot sector in clusters
		if(cmd->FindExist("-nofs",true) || (size>(2048*1024*1024LL))) {
			bootsect_pos = -1;
		}

		// temp_line is the filename
		if (!(cmd->FindCommand(1, temp_line))) {
			printHelp();
			return;
		}

		// don't trash user's files
		FILE* f = fopen(temp_line.c_str(),"r");
		if(f) {
			fclose(f);
			WriteOut(MSG_Get("PROGRAM_IMGMAKE_FILE_EXISTS"),temp_line.c_str());
			return;
		}

		WriteOut(MSG_Get("PROGRAM_IMGMAKE_PRINT_CHS"),c,h,s);
		WriteOut("%s\r\n",temp_line.c_str());
		LOG_MSG(MSG_Get("PROGRAM_IMGMAKE_PRINT_CHS"),c,h,s);

		// do it again for fixed chs values
		sectors = (Bitu)(size / 512);

		// create the image file
		f = fopen64(temp_line.c_str(),"wb+");
		if (!f) {
			WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANNOT_WRITE"),temp_line.c_str());
			return;
		}
		if(fseeko64(f,size-1,SEEK_SET)) {
			WriteOut(MSG_Get("PROGRAM_IMGMAKE_NOT_ENOUGH_SPACE"),size);
			return;
		}
		Bit8u bufferbyte=0;
		if(fwrite(&bufferbyte,1,1,f)!=1) {
			WriteOut(MSG_Get("PROGRAM_IMGMAKE_NOT_ENOUGH_SPACE"),size);
			return;
		}

		// Format the image if not unrequested (and image size<2GB)
		if(bootsect_pos > -1) {
			Bit8u sbuf[512];
			if(mediadesc == 0xF8) {
				// is a harddisk: write MBR
				memcpy(sbuf,freedos_mbr,512);
				// active partition
				sbuf[0x1be]=0x80;
				// start head - head 0 has the partition table, head 1 first partition
				sbuf[0x1bf]=1;
				// start sector with bits 8-9 of start cylinder in bits 6-7
				sbuf[0x1c0]=1;
				// start cylinder bits 0-7
				sbuf[0x1c1]=0;
				// OS indicator: DOS what else ;)
				sbuf[0x1c2]=0x06;
				// end head (0-based)
				sbuf[0x1c3]= h-1;
				// end sector with bits 8-9 of end cylinder (0-based) in bits 6-7
				sbuf[0x1c4]=s|(((c-1)&0x300)>>2);
				// end cylinder (0-based) bits 0-7
				sbuf[0x1c5]=(c-1)&0xFF;
				// sectors preceding partition1 (one head)
				host_writed(&sbuf[0x1c6],s);
				// length of partition1, align to chs value
				host_writed(&sbuf[0x1ca],((c-1)*h+(h-1))*s);

				// write partition table
				fseeko64(f,0,SEEK_SET);
				fwrite(&sbuf,512,1,f);
				bootsect_pos = s;
			}

			// set boot sector values
			memset(sbuf,0,512);
			// TODO boot code jump
			sbuf[0]=0xEB; sbuf[1]=0x3c; sbuf[2]=0x90;
			// OEM
			sprintf((char*)&sbuf[0x03],"MSDOS5.0");
			// bytes per sector: always 512
			host_writew(&sbuf[0x0b],512);
			// sectors per cluster: 1,2,4,8,16,...
			if(mediadesc == 0xF8) {
				Bitu cval = 1;
				while((sectors/cval) >= 65525) cval <<= 1;
				sbuf[0x0d]=cval;
			} else sbuf[0x0d]=sectors/0x1000 + 1; // FAT12 can hold 0x1000 entries TODO
			// TODO small floppys have 2 sectors per cluster?
			// reserverd sectors: 1 ( the boot sector)
			host_writew(&sbuf[0x0e],1);
			// Number of FATs - always 2
			sbuf[0x10] = 2;
			// Root entries - how are these made up? - TODO
			host_writew(&sbuf[0x11],root_ent);
			// sectors (under 32MB) - will OSes be sore if all HD's use large size?
			if(mediadesc != 0xF8) host_writew(&sbuf[0x13],c*h*s);
			// media descriptor
			sbuf[0x15]=mediadesc;
			// sectors per FAT
			// needed entries: (sectors per cluster)
			Bitu sect_per_fat=0;
			Bitu clusters = (sectors-1)/sbuf[0x0d]; // TODO subtract root dir too maybe
			if(mediadesc == 0xF8) sect_per_fat = (clusters*2)/512+1;
			else sect_per_fat = ((clusters*3)/2)/512+1;
			host_writew(&sbuf[0x16],sect_per_fat);
			// sectors per track
			host_writew(&sbuf[0x18],s);
			// heads
			host_writew(&sbuf[0x1a],h);
			// hidden sectors
			host_writed(&sbuf[0x1c],bootsect_pos);
			// sectors (large disk) - this is the same as partition length in MBR
			if(mediadesc == 0xF8) host_writed(&sbuf[0x20],sectors-s);
			// BIOS drive
			if(mediadesc == 0xF8) sbuf[0x24]=0x80;
			else sbuf[0x24]=0x00;
			// ext. boot signature
			sbuf[0x26]=0x29;
			// volume serial number
			// let's use the BIOS time (cheap, huh?)
			host_writed(&sbuf[0x27],mem_readd(BIOS_TIMER));
			// Volume label
			sprintf((char*)&sbuf[0x2b],"KEIN NAME    ");
			// file system type
			if(mediadesc == 0xF8) sprintf((char*)&sbuf[0x36],"FAT16   ");
			else sprintf((char*)&sbuf[0x36],"FAT12   ");
			// boot sector signature
			host_writew(&sbuf[0x1fe],0xAA55);

			// write the boot sector
			fseeko64(f,bootsect_pos*512,SEEK_SET);
			fwrite(&sbuf,512,1,f);
			// write FATs
			memset(sbuf,0,512);
			if(mediadesc == 0xF8) host_writed(&sbuf[0],0xFFFFFFF8);
			else host_writed(&sbuf[0],0xFFFFF0);
			// 1st FAT
			fseeko64(f,(bootsect_pos+1)*512,SEEK_SET);
			fwrite(&sbuf,512,1,f);
			// 2nd FAT
			fseeko64(f,(bootsect_pos+1+sect_per_fat)*512,SEEK_SET);
			fwrite(&sbuf,512,1,f);
		}
		// write VHD footer if requested, largely copied from RAW2VHD program, no license was included
		if((mediadesc = 0xF8) && (temp_line.find(".vhd"))) {
			int i;
			Bit8u footer[512];
			// basic information
			memcpy(footer,"conectix" "\0\0\0\2\0\1\0\0" "\xff\xff\xff\xff\xff\xff\xff\xff" "????rawv" "\0\1\0\0Wi2k",40);
			memset(footer+40,0,512-40);
			// time
			struct tm tm20000101 = { 0,0,0, 1,0,100, 0,0,0 };
			time_t basetime = mktime(&tm20000101);
			time_t vhdtime = time(NULL) - basetime;
			*(Bit32u*)(footer+0x18) = SDL_SwapBE32((__time32_t)vhdtime);
			// size and geometry
			*(Bit64u*)(footer+0x30) = *(Bit64u*)(footer+0x28) = SDL_SwapBE64(size);

			*(Bit16u*)(footer+0x38) = SDL_SwapBE16(c);
			*(Bit8u*)( footer+0x3A) = h;
			*(Bit8u*)( footer+0x3B) = s;
			*(Bit32u*)(footer+0x3C) = SDL_SwapBE32(2);

			// generate UUID
			for (i=0; i<16; ++i) {
				*(footer+0x44+i) = (Bit8u)(rand()>>4);
			}

			// calculate checksum
			Bit32u sum;
			for (i=0,sum=0; i<512; ++i) {
				sum += footer[i];
			}

			*(Bit32u*)(footer+0x40) = SDL_SwapBE32(~sum);

			// write footer
			fseeko64(f, 0L, SEEK_END);
			fwrite(&footer,512,1,f);
		}
		fclose(f);

		// create the batch file
		if( (t2 == "-bat") || (t2 == "-txt") ) {
			if(temp_line.length() > 3) {
				if(t2 == "-bat"){
					t2 = temp_line.substr(0,temp_line.length()-4);
					t2 = t2.append(".bat");
				
				}else if(t2 == "-txt"){
					t2 = temp_line.substr(0,temp_line.length()-4);
					t2 = t2.append(".txt");					
				}				
			} else {
				t2 = temp_line.append(".txt");							
			}
			WriteOut("%s\n",t2.c_str());
			f = fopen(t2.c_str(),"wb+");
			if (!f) {
				WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANNOT_WRITE"),t2.c_str());
				return;
			}
			fprintf(f,"imgmount c %s -size 512,%u,%u,%u\r\n",temp_line.c_str(),s,h,c);
			fclose(f);
		}
		return;
	}
	void printHelp() { // maybe hint parameter?
		WriteOut(MSG_Get("PROGRAM_IMGMOUNT_SYNTAX"));
	}
};

static void IMGMAKE_ProgramStart(Program * * make) {
	*make=new IMGMAKE;
}
/* DOSBox-MB IMGMAKE patch. ========================================================================= */ 	
// LOADFIX

class LOADFIX : public Program {
public:
	void Run(void);
};

void LOADFIX::Run(void) 
{
	Bit16u commandNr	= 1;
	Bit16u kb			= 64;
	if (cmd->FindCommand(commandNr,temp_line)) {
		if (temp_line[0]=='-') {
			char ch = temp_line[1];
			if ((*upcase(&ch)=='D') || (*upcase(&ch)=='F')) {
				// Deallocate all
				DOS_FreeProcessMemory(0x40);
				WriteOut(MSG_Get("PROGRAM_LOADFIX_DEALLOCALL"),kb);
				return;
			} else {
				// Set mem amount to allocate
				kb = atoi(temp_line.c_str()+1);
				if (kb==0) kb=64;
				commandNr++;
			}
		}
	}
	// Allocate Memory
	Bit16u segment;
	Bit16u blocks = kb*1024/16;
	if (DOS_AllocateMemory(&segment,&blocks)) {
		DOS_MCB mcb((Bit16u)(segment-1));
		mcb.SetPSPSeg(0x40);			// use fake segment
		WriteOut(MSG_Get("PROGRAM_LOADFIX_ALLOC"),kb);
		// Prepare commandline...
		if (cmd->FindCommand(commandNr++,temp_line)) {
			// get Filename
			char filename[128];
			safe_strncpy(filename,temp_line.c_str(),128);
			// Setup commandline
			bool ok;
			char args[256];
			args[0] = 0;
			do {
				ok = cmd->FindCommand(commandNr++,temp_line);
				if(sizeof(args)-strlen(args)-1 < temp_line.length()+1)
					break;
				strcat(args,temp_line.c_str());
				strcat(args," ");
			} while (ok);			
			// Use shell to start program
			DOS_Shell shell;
			shell.Execute(filename,args);
			DOS_FreeMemory(segment);		
			WriteOut(MSG_Get("PROGRAM_LOADFIX_DEALLOC"),kb);
		}
	} else {
		WriteOut(MSG_Get("PROGRAM_LOADFIX_ERROR"),kb);	
	}
}

static void LOADFIX_ProgramStart(Program * * make) {
	*make=new LOADFIX;
}

// RESCAN

class RESCAN : public Program {
public:
	void Run(void);
};

void RESCAN::Run(void) 
{
	bool all = false;
	
	Bit8u drive = DOS_GetDefaultDrive();
	
	if(cmd->FindCommand(1,temp_line)) {
		//-A -All /A /All 
		if(temp_line.size() >= 2 && (temp_line[0] == '-' ||temp_line[0] =='/')&& (temp_line[1] == 'a' || temp_line[1] =='A') ) all = true;
		else if(temp_line.size() == 2 && temp_line[1] == ':') {
			lowcase(temp_line);
			drive  = temp_line[0] - 'a';
		}
	}
	// Get current drive
	if (all) {
		for(Bitu i =0; i<DOS_DRIVES;i++) {
			if (Drives[i]) Drives[i]->EmptyCache();
		}
		WriteOut(MSG_Get("PROGRAM_RESCAN_SUCCESS"));
	} else {
		if (drive < DOS_DRIVES && Drives[drive]) {
			Drives[drive]->EmptyCache();
			WriteOut(MSG_Get("PROGRAM_RESCAN_SUCCESS"));
		}
	}
}

static void RESCAN_ProgramStart(Program * * make) {
	*make=new RESCAN;
}

class INTRO : public Program {
public:
	void DisplayMount(void) {
		/* Basic mounting has a version for each operating system.
		 * This is done this way so both messages appear in the language file*/
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_START"));
#if (WIN32)
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_WINDOWS"));
#else			
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_OTHER"));
#endif
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_END"));
	}

	void Run(void) {
		/* Only run if called from the first shell (Xcom TFTD runs any intro file in the path) */
		if(DOS_PSP(dos.psp()).GetParent() != DOS_PSP(DOS_PSP(dos.psp()).GetParent()).GetParent()) return;
		if(cmd->FindExist("cdrom",false)) {
			WriteOut(MSG_Get("PROGRAM_INTRO_CDROM"));
			return;
		}
		if(cmd->FindExist("mount",false)) {
			WriteOut("\033[2J");//Clear screen before printing
			DisplayMount();
			return;
		}
		if(cmd->FindExist("special",false)) {
			WriteOut(MSG_Get("PROGRAM_INTRO_SPECIAL"));
			return;
		}
		/* Default action is to show all pages */
		WriteOut(MSG_Get("PROGRAM_INTRO"));
		Bit8u c;Bit16u n=1;
		DOS_ReadFile (STDIN,&c,&n);
		DisplayMount();
		DOS_ReadFile (STDIN,&c,&n);
		WriteOut(MSG_Get("PROGRAM_INTRO_CDROM"));
		DOS_ReadFile (STDIN,&c,&n);
		WriteOut(MSG_Get("PROGRAM_INTRO_SPECIAL"));
	}
};

static void INTRO_ProgramStart(Program * * make) {
	*make=new INTRO;
}

bool ElTorito_ChecksumRecord(unsigned char *entry/*32 bytes*/) {
	unsigned int word,chk=0,i;

	for (i=0;i < 16;i++) {
		word = ((unsigned int)entry[0]) + ((unsigned int)entry[1] << 8);
		chk += word;
		entry += 2;
	}
	chk &= 0xFFFF;
	return (chk == 0);
}

		/************************************************************************************* EL_TORITO */
bool ElTorito_ScanForBootRecord(CDROM_Interface *drv,unsigned long &boot_record,unsigned long &el_torito_base) {
	char buffer[2048];
	unsigned int sec;

	for (sec=16;sec < 32;sec++) {
		if (!drv->ReadSectorsHost(buffer,false,sec,1))
			break;

		/* stop at terminating volume record */
		if (buffer[0] == 0xFF) break;

		/* match boot record and whether it conforms to El Torito */
		if (buffer[0] == 0x00 && memcmp(buffer+1,"CD001",5) == 0 && buffer[6] == 0x01 &&
			memcmp(buffer+7,"EL TORITO SPECIFICATION\0\0\0\0\0\0\0\0\0",32) == 0) {
			boot_record = sec;
			el_torito_base = (unsigned long)buffer[71] +
					((unsigned long)buffer[72] << 8UL) +
					((unsigned long)buffer[73] << 16UL) +
					((unsigned long)buffer[74] << 24UL);

			return true;
		}
	}

	return false;
}


/* C++ class implementing El Torito floppy emulation */
class imageDiskElToritoFloppy : public imageDisk {
public:
	/* Read_Sector and Write_Sector take care of geometry translation for us,
	 * then call the absolute versions. So, we override the absolute versions only */
	virtual Bit8u Read_AbsoluteSector(Bit32u sectnum, void * data) {
		unsigned char buffer[2048];

		bool GetMSCDEXDrive(unsigned char drive_letter,CDROM_Interface **_cdrom);

		CDROM_Interface *src_drive=NULL;
		if (!GetMSCDEXDrive(CDROM_drive-'A',&src_drive)) return 0x05;

		if (!src_drive->ReadSectorsHost(buffer,false,cdrom_sector_offset+(sectnum>>2)/*512 byte/sector to 2048 byte/sector conversion*/,1))
			return 0x05;

		memcpy(data,buffer+((sectnum&3)*512),512);
		return 0x00;
	}
	virtual Bit8u Write_AbsoluteSector(Bit32u sectnum, void * data) {
		return 0x05; /* fail, read only */
	}
	imageDiskElToritoFloppy(unsigned char new_CDROM_drive,unsigned long new_cdrom_sector_offset,unsigned char floppy_emu_type) : imageDisk(NULL,NULL,0,false) {
		diskimg = NULL;
		sector_size = 512;
		CDROM_drive = new_CDROM_drive;
		cdrom_sector_offset = new_cdrom_sector_offset;
		class_id = ID_EL_TORITO_FLOPPY;

		if (floppy_emu_type == 1) { /* 1.2MB */
			heads = 2;
			cylinders = 80;
			sectors = 15;
		}
		else if (floppy_emu_type == 2) { /* 1.44MB */
			heads = 2;
			cylinders = 80;
			sectors = 18;
		}
		else if (floppy_emu_type == 3) { /* 2.88MB */
			heads = 2;
			cylinders = 80;
			sectors = 36; /* FIXME: right? */
		}
		else {
			heads = 2;
			cylinders = 69;
			sectors = 14;
			fprintf(stderr,"BUG! unsupported floppy_emu_type in El Torito floppy object\n");
		}

		active = true;
	}
	virtual ~imageDiskElToritoFloppy() {
	}

	unsigned long cdrom_sector_offset;
	unsigned char CDROM_drive;
/*
	int class_id;

	bool hardDrive;
	bool active;
	FILE *diskimg;
	std::string diskname;
	Bit8u floppytype;

	Bit32u sector_size;
	Bit32u heads,cylinders,sectors;
	Bit32u reserved_cylinders;
	Bit64u current_fpos; */
};
		/************************************************************************************* EL_TORITO */

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline bool ExistsHDD (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

void IDE_InfoConnect(std::string type, int ide_index, bool ide_slave, bool autoconnect){
	
	std::string sIDEIndex  ="--";
	std::string sIDEChannel="--";
	std::string sIDEAutoCon="--";	
	std::string sIDEHDType ="--";	
			
	switch ( ide_index ) {
		case 0 : { sIDEIndex ="Primary"   ;break; }
		case 1 : { sIDEIndex ="Seconadry" ;break; } 
		case 2 : { sIDEIndex ="Tertiary"  ;break; }
		case 3 : { sIDEIndex ="Quaternary";break; }			
		default: sIDEIndex   ="";	
	}
			
	if ( ide_slave == false ) { sIDEChannel ="Master"; }
	if ( ide_slave == true  ) { sIDEChannel ="Slave "; }		
			
	if ( autoconnect == true){
		sIDEAutoCon = "[Auto Mode]";
	}
	if ( autoconnect == false){
		sIDEAutoCon = "[Jumper Mode]";
	}			
			
	LOG_MSG( "\nIDE Atapi: Connect %s %s To %s %s", type.c_str(), sIDEIndex.c_str() ,sIDEChannel.c_str(), sIDEAutoCon.c_str());			
}
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
class IMGMOUNT : public Program {
public:
	void Run(void) {
		
		/* Hack To allow long commandlines */
		ChangeToLongCmd();
		
		/* In secure mode don't allow people to change imgmount points. 
		 * Neither mount nor unmount */
		if(control->SecureMode()) {
			WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
			return;
		}
		DOS_Drive * newdrive = NULL;
		imageDisk * newImage = NULL;
		Bit32u imagesize;
		char drive;
		std::string label;
		std::vector<std::string> paths;
		std::string umount;
		
		/* Check for unmounting */
		if (cmd->FindString("-u",umount,false)) {
			WriteOut(UnmountHelper(umount[0]), toupper(umount[0]));
			return;
		}

		/************************************************************************************* IDE Emulation */
		bool ide_slave 	      = false;
		signed char ide_index = -1;
		std::string ideattach ="auto";
		/************************************************************************************* IDE Emulation */
		unsigned long el_torito_floppy_base=~0UL;
		unsigned char el_torito_floppy_type=0xFF;		
		char el_torito_cd_drive = 0;
		std::string el_torito;		
		/************************************************************************************* EL_TORITO */
		
		std::string type="hdd";
		
		/* DOSBox-MB IMGMAKE patch. ========================================================================= */
		/* 
			default to floppy for drive letters A and B and numbers 0 and 1
		*/
		
		if (!cmd->FindCommand(1,temp_line) || (temp_line.size() > 2) ||	((temp_line.size()>1) && (temp_line[1]!=':'))) {
			/*
				drive not valid
			*/
		} else {
			Bit8u tdr = toupper(temp_line[0]);
			
			if(tdr=='A'||tdr=='B'||tdr=='0'||tdr=='1'){
				
				type="floppy";
			}
		}
		
		/************************************************************************************* EL_TORITO */		
		cmd->FindString("-el-torito",el_torito,true);
		if (el_torito != "") {
			unsigned char entries[2048],*entry,ent_num=0;
			int header_platform = -1,header_count=0;
			bool header_final = false;
			int header_more = -1;
			
			el_torito_cd_drive = toupper(el_torito[0]);

			/* must be valid drive letter, C to Z */
			if (!isalpha(el_torito_cd_drive) || el_torito_cd_drive < 'C') {
				WriteOut("ElTorito BootCD:\n"
					/*   "= Requires a Proper Drive Letter Corresponding To Your CD-ROM drive\n"		*/
						 "= Erfordert einen geeigneten Laufwerksbuchstaben fuer Ihr CD-ROM-Laufwerk\n"
						 "");																				
				return;
			}

			/* drive must not exist (as a hard drive) */
			if (!imageDiskList[el_torito_cd_drive-'C'] != 0) {
				WriteOut("ElTorito BootCD:\n"
					/*	 "= CD-ROM drive [%c] specified already exists as a non-CD-ROM device\n"		*/
						 "= Das angegebene CD-ROM-Laufwerk [%c] ist bereits als Nicht-CD-ROM-Geraet vorhanden\n"
						 ,el_torito_cd_drive);
				return;
			}

			bool GetMSCDEXDrive(unsigned char drive_letter,CDROM_Interface **_cdrom);

			/* get the CD-ROM drive */
			CDROM_Interface *src_drive=NULL;
			if (!GetMSCDEXDrive(el_torito_cd_drive-'A',&src_drive)) {
				WriteOut("ElTorito BootCD:\n"
					/*	 "= CD-ROM drive specified is not actually a CD-ROM drive [%c]\n",				*/
						 "= Das angegebene CD-ROM-Laufwerk [%c] ist kein CD-ROM-Laufwerk\n",
						 el_torito_cd_drive);
				return;
			}

			/* FIXME: We only support the floppy emulation mode at this time.
			 *        "Superfloppy" or hard disk emulation modes are not yet implemented */
			if (type != "floppy") {
				WriteOut("ElTorito BootCD:\n"
					/*	 "= must be used with -t floppy at this time\n");								*/
						 "= muss zu diesem Zeitpunkt mit -t Floppy verwendet werden\n");	
				return;
			}

			/* Okay. Step #1: Scan the volume descriptors for the Boot Record. */
			unsigned long el_torito_base = 0,boot_record_sector = 0;
			if (!ElTorito_ScanForBootRecord(src_drive,boot_record_sector,el_torito_base)) {
				WriteOut("ElTorito BootCD:\n"
					/*	 "= Boot Record not found\n"													*/
						 "= Boot Record niocht gefunden\n" 
					);
				return;
			}

			fprintf(stderr,"El Torito emulation: Found ISO 9660 Boot Record in sector %lu, pointing to sector %lu\n",
				boot_record_sector,el_torito_base);
				

			/* Step #2: Parse the records. Each one is 32 bytes long */
			if (!src_drive->ReadSectorsHost(entries,false,el_torito_base,1)) {
				WriteOut("El Torito entries unreadable\n");
				return;
			}

			/* for more information about what this loop is doing, read:
			 * http://download.intel.com/support/motherboards/desktop/sb/specscdrom.pdf
			 */
			/* FIXME: Somebody find me an example of a CD-ROM with bootable code for both x86, PowerPC, and Macintosh.
			 *        I need an example of such a CD since El Torito allows multiple "headers" */
			/* TODO: Is it possible for this record list to span multiple sectors? */
			for (ent_num=0;ent_num < (2048/0x20);ent_num++) {
				entry = entries + (ent_num*0x20);

				if (memcmp(entry,"\0\0\0\0""\0\0\0\0""\0\0\0\0""\0\0\0\0""\0\0\0\0""\0\0\0\0""\0\0\0\0""\0\0\0\0",32) == 0)
					break;

				if (entry[0] == 0x01/*header*/) {
					if (!ElTorito_ChecksumRecord(entry)) {
						fprintf(stderr,"Warning: El Torito checksum error in header(0x01) entry\n");
						continue;
					}

					if (header_count != 0) {
						fprintf(stderr,"Warning: El Torito has more than one Header/validation entry\n");
						continue;
					}

					if (header_final) {
						fprintf(stderr,"Warning: El Torito has an additional header past the final header\n");
						continue;
					}

					header_more = -1;
					header_platform = entry[1];
					fprintf(stderr,"El Torito entry: first header platform=0x%02x\n",header_platform);
					header_count++;
				}
				else if (entry[0] == 0x90/*header, more follows*/ || entry[0] == 0x91/*final header*/) {
					if (header_final) {
						fprintf(stderr,"Warning: El Torito has an additional header past the final header\n");
						continue;
					}

					header_final = (entry[0] == 0x91);
					header_more = ((unsigned int)entry[2]) + (((unsigned int)entry[3]) << 8);
					header_platform = entry[1];
					fprintf(stderr,"El Torito entry: first header platform=0x%02x more=%u final=%u\n",header_platform,header_more,header_final);
					header_count++;
				}
				else {
					if (header_more == 0) {
						fprintf(stderr,"El Torito entry: Non-header entry count expired, ignoring record 0x%02x\n",entry[0]);
						continue;
					}
					else if (header_more > 0) {
						header_more--;
					}

					if (entry[0] == 0x44) {
						fprintf(stderr,"El Torito entry: ignoring extension record\n");
					}
					else if (entry[0] == 0x00/*non-bootable*/) {
						fprintf(stderr,"El Torito entry: ignoring non-bootable record\n");
					}
					else if (entry[0] == 0x88/*bootable*/) {
						if (header_platform == 0x00/*x86*/) {
							unsigned char mediatype = entry[1]&0xF;
							unsigned short load_segment = ((unsigned int)entry[2]) + (((unsigned int)entry[3]) << 8);
							unsigned char system_type = entry[4];
							unsigned short sector_count = ((unsigned int)entry[6]) + (((unsigned int)entry[7]) << 8);
							unsigned long load_rba = ((unsigned int)entry[8]) + (((unsigned int)entry[9]) << 8) +
								(((unsigned int)entry[10]) << 16) + (((unsigned int)entry[11]) << 24);

							fprintf(stderr,"El Torito entry: bootable x86 record mediatype=%u load_segment=0x%04x "
								"system_type=0x%02x sector_count=%u load_rba=%lu\n",
								mediatype,load_segment,system_type,sector_count,load_rba);

							/* already chose one, ignore */
							if (el_torito_floppy_base != ~0UL)
								continue;

							if (load_segment != 0 && load_segment != 0x7C0)
								fprintf(stderr,"El Torito boot warning: load segments other than 0x7C0 not supported yet\n");
							if (sector_count != 1)
								fprintf(stderr,"El Torito boot warning: sector counts other than 1 are not supported yet\n");

							if (mediatype < 1 || mediatype > 3) {
								fprintf(stderr,"El Torito boot entry: media types other than floppy emulation not supported yet\n");
								continue;
							}

							el_torito_floppy_base = load_rba;
							el_torito_floppy_type = mediatype;
						}
						else {
							fprintf(stderr,"El Torito entry: ignoring bootable non-x86 (platform_id=0x%02x) record\n",header_platform);
						}
					}
					else {
						fprintf(stderr,"El Torito entry: ignoring unknown record ID %02x\n",entry[0]);
					}
				}
			}

			if (el_torito_floppy_type == 0xFF || el_torito_floppy_base == ~0UL) {
				WriteOut("El Torito bootable floppy not found\n");
				return;
			}				
		}
		/************************************************************************************* EL_TORITO */		
		
		/* DOSBox-MB IMGMAKE patch. ========================================================================= */		
		std::string fstype="fat";
		
		cmd->FindString("-t",type,true);
		cmd->FindString("-fs",fstype,true);

		
		if (type == "cdrom"){
			type = "iso";		 
			/*
				Tiny hack for people who like to type -t cdrom
			*/
		}
		Bit8u mediaid;	
		
		if (type=="floppy" || type=="hdd" || type=="iso" || type == "autohdd") {
			Bit16u sizes[4];
			bool imgsizedetect=false;
			
			std::string str_size;
			mediaid=0xF8;

			int reserved_cylinders=0;
			std::string reservecyl;
			/* DOSBox-X: to please certain 32-bit drivers like Windows 3.1 WDCTRL, or to emulate older h/w configurations,
			 *           we allow the user or script to specify the number of reserved cylinders. older BIOSes were known
			 *           to subtract 1 or 2 additional cylinders from the total in the fixed disk param table. the -reservecyl
			 *           option allows the number we subtract from the total in INT 13H to be set */
			cmd->FindString("-reservecyl",reservecyl,true);
			if (reservecyl != ""){
				
				reserved_cylinders = atoi(reservecyl.c_str());
			}

			
			/************************************************************************************* IDE Emulation */
			/*                                  DOSBox-X: we allow "-ide" to allow controlling
												which IDE controller and slot to attach the hard
												disk/CD-ROM to
			*/
			cmd->FindString("-ide",ideattach,true);
															
			if (ideattach == "auto") {
				
				if (type == "floppy") {
				}
				else {
					IDE_Auto(ide_index,ide_slave);	
					IDE_InfoConnect(type, ide_index, ide_slave, true);
				}				
				
			}			
			else if (ideattach != "none" && isdigit(ideattach[0]) && ideattach[0] > '0') {
				/* 
					takes the form [controller]<m/s> such as: 1m for primary master
				*/				
				ide_index = ideattach[0] - '1';
				if (ideattach.length() >= 2){
																				
					ide_slave = (ideattach[1] == 's');
				} 				
				IDE_InfoConnect(type, ide_index, ide_slave, false);				
			}			
			/************************************************************************************* IDE Emulation */			
			
			if (type=="floppy") {
				mediaid		= 0xF0;
				ideattach	= "none";				
				
			} else if (type== "iso") {
				/*
					str_size="2048,1,65535,0";
					ignored, see drive_iso.cpp (AllocationInfo)
				*/
				mediaid		= 0xF8;		
				fstype 		= "iso";
			} 
			
			
			cmd->FindString("-size",str_size,true);		
			
			if ( (type=="hdd") && (str_size.size() == 0) ) {
				
				imgsizedetect = true;
				
			} else {
						
				char number[21] = { 0 };const char * scan = str_size.c_str();
				Bitu index = 0;Bitu count = 0;
				/* 
					Parse the str_size string
				*/
				while ( *scan && index < 20 && count < 4 ) {
					
					if ( *scan == ',' ) {
						
						number[index]  = 0;
						sizes[count++] = atoi(number);
						index 		   = 0;
					
					} else number[index++] = *scan;										
					scan++;
				}
				
				if (count < 4) {
					number[index] = 0; 					/* always goes correct as index is max 20 at this point.*/
					sizes[count]  = atoi(number);
				}
			}
		
			if(fstype=="fat" || fstype=="iso") {
				/* 
					get the drive letter
				*/
				
				if (!cmd->FindCommand(1,temp_line) || (temp_line.size() > 2) || ((temp_line.size()>1) && (temp_line[1]!=':'))) {
					WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_DRIVE"));
					return;
				}
				int i_drive = toupper(temp_line[0]);
				if (!isalpha(i_drive) || (i_drive - 'A') >= DOS_DRIVES || (i_drive - 'A') <0) {
					WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_DRIVE"));
					return;
				}
				drive = static_cast<char>(i_drive);
			} else if (fstype=="none") {
				cmd->FindCommand(1,temp_line);
				if ((temp_line.size() > 1) || (!isdigit(temp_line[0]))) {
					WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY2"));
					return;
				}
				drive=temp_line[0];
				if ((drive<'0') || (drive>=(MAX_DISK_IMAGES+'0'))) {
					WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY2"));
					return;
				}
			} else {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FORMAT_UNSUPPORTED"),fstype.c_str());
				return;
			}
			
			// find all file parameters, assuming that all option parameters have been removed
			
			/*
			  int cnt = 0;
			  a Counter for Multiple Images. If an Image can't found say me the Position/Slot.			  
			*/
			int ImageCNT = 0;
			
			while(cmd->FindCommand((unsigned int)(paths.size() + 2), temp_line) && temp_line.size()) {
				
				
				struct stat test; ImageCNT += 1;
				if (stat(temp_line.c_str(),&test)) {
					//See if it works if the ~ are written out
					std::string homedir(temp_line);
					Cross::ResolveHomedir(homedir);
					if(!stat(homedir.c_str(),&test)) {
						temp_line = homedir;
					} else {
						// convert dosbox filename to system filename
						char fullname[CROSS_LEN];
						char tmp[CROSS_LEN];
						safe_strncpy(tmp, temp_line.c_str(), CROSS_LEN);

						Bit8u dummy;
						if (!DOS_MakeName(tmp, fullname, &dummy) || strncmp(Drives[dummy]->GetInfo(),"local directory",15)) {
							WriteOut(MSG_Get("PROGRAM_IMGMOUNT_NON_LOCAL_DRIVE"));							
							WriteOut("Image not Found\n [at Position %d] in %s\n", ImageCNT, homedir.c_str());
							return;
						}

						localDrive *ldp = dynamic_cast<localDrive*>(Drives[dummy]);
						if (ldp==NULL) {
							WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FILE_NOT_FOUND"));
							return;
						}
						ldp->GetSystemFilename(tmp, fullname);
						temp_line = tmp;

						if (stat(temp_line.c_str(),&test)) {
							WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FILE_NOT_FOUND"));
							return;
						}
					}
				}
				if (S_ISDIR(test.st_mode)) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_MOUNT"));
					return;
				}
				paths.push_back(temp_line);
			}

			if (el_torito != "") {
				if (paths.size() != 0) {
					WriteOut("Do not specify files when mounting virtual floppy disk images from El Torito bootable CDs\n");
					return;
				}
			}
			else {
				if (paths.size() == 0) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_FILE"));				
					return;	
				}
				if (paths.size() == 1)
					temp_line = paths[0];
			}
		
			if (type == "autohdd"){
				type = "hdd";
					
				LOG_MSG("Detect VHD:");									
				FILE * diskfile = fopen64(temp_line.c_str(), "rb+");
				if (!diskfile) {

					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					WriteOut        ("Image: %s",temp_line.c_str());
					WriteOut        ("Image is Open and used from another Program?");					
					
					LOG_MSG         ("Autodetect: No Access to HDD %s",temp_line.c_str());
					LOG_MSG         ("Autodetect: Image is Open and used from another Program?\n");					
					E_Exit			("No Access to Image and can't open\n%s",temp_line.c_str());
				}
									
				fseeko64(diskfile, 0L, SEEK_END);
				Bit32u fcsize = (Bit32u)(ftello64(diskfile) / 512L);
				Bit8u buf[512];																	
				fseeko64(diskfile, -512, SEEK_CUR); 				// check for vhd signature
					
				if (fread(buf,sizeof(Bit8u),512,diskfile)<512) {
					fclose(diskfile);
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
					
				if(!strcmp((const char*)buf,"conectix")) {
					fcsize--;										// skip footer (512 bytes)
					sizes[0]=512;									// sector size
					sizes[1]=buf[0x3b];								// sectors
					sizes[2]=buf[0x3a];								// heads
					sizes[3]=SDL_SwapBE16(*(Bit16s*)(buf + 0x38));	// cylinders

					// Do translation (?)
					while((sizes[2] < 128) && (sizes[3] > 1023)) {
						sizes[2]<<=1;
						sizes[3]>>=1;
					}
					if (sizes[3]>1023) {
						// Set x/255/63
						sizes[2] = 255;
						sizes[3] = fcsize/sizes[2]/sizes[1];
					}
					LOG_MSG("Autodetect: Harddrive Image %s", temp_line.c_str());
					LOG_MSG("Autodetect: Harddrive Size     [%d Megabytes]", (sizes[1]*sizes[2]*sizes[3]*sizes[0]/1024)/1024);					
					LOG_MSG("Autodetect: Harddrive Geometry [%u,%u,%u,%u]", sizes[0], sizes[1], sizes[2], sizes[3]);
					
					if( sizes[3] > 1023){
						LOG_MSG("WARNING: cylinders>1023, INT13 will not work unless extensions are used");
					}									
				}
				else
				{										
					imgsizedetect = true;
					LOG_MSG("Autodetect: No VHD Image .. Using Default\n");	
				}				
				fseeko64(diskfile, 0L, SEEK_SET);														
				if (fread(buf,sizeof(Bit8u),512,diskfile)<512) {
					fclose(diskfile);
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
				fclose(diskfile);							
			}
				
			if(fstype=="fat") {
				
				if (el_torito != "") {
					WriteOut("El Torito bootable CD: -fs fat mounting not supported\n"); /* <- NTS: Someday!! */
					return;
				}				
				
				if (imgsizedetect) {

/* DOSBox-MB IMGMAKE patch. ========================================================================= */					
					bool yet_detected = false;
					FILE * diskfile = fopen64(temp_line.c_str(), "rb+");
/* DOSBox-MB IMGMAKE patch. ========================================================================= */					

					if (!diskfile) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
						return;
					}

/* DOSBox-MB IMGMAKE patch. ========================================================================= */										
					fseeko64(diskfile, 0L, SEEK_END);
					Bit32u fcsize = (Bit32u)(ftello64(diskfile) / 512L);
/* DOSBox-MB IMGMAKE patch. ========================================================================= */										

					Bit8u buf[512];

/* DOSBox-MB IMGMAKE patch. ========================================================================= */															
					// check for vhd signature
					fseeko64(diskfile, -512, SEEK_CUR);
					if (fread(buf,sizeof(Bit8u),512,diskfile)<512) {
						fclose(diskfile);
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
						return;
					}
					if(!strcmp((const char*)buf,"conectix")) {
						fcsize--;	// skip footer (512 bytes)
						sizes[0]=512;	// sector size
						sizes[1]=buf[0x3b];	// sectors
						sizes[2]=buf[0x3a];	// heads
						sizes[3]=SDL_SwapBE16(*(Bit16s*)(buf + 0x38));	// cylinders

						// Do translation (?)
						while((sizes[2] < 128) && (sizes[3] > 1023)) {
							sizes[2]<<=1;
							sizes[3]>>=1;
						}

						if (sizes[3]>1023) {
							// Set x/255/63
							sizes[2] = 255;
							sizes[3] = fcsize/sizes[2]/sizes[1];
						}

						LOG_MSG("VHD image detected: %u,%u,%u,%u",
						    sizes[0], sizes[1], sizes[2], sizes[3]);
						if(sizes[3]>1023) LOG_MSG("WARNING: cylinders>1023, INT13 will not work unless extensions are used");
						yet_detected = true;
					}

					fseeko64(diskfile, 0L, SEEK_SET);
/* DOSBox-MB IMGMAKE patch. ========================================================================= */															
					if (fread(buf,sizeof(Bit8u),512,diskfile)<512) {
						fclose(diskfile);
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
						return;
					}
					fclose(diskfile);
					
/* DOSBox-MB IMGMAKE patch. ========================================================================= */	
					// check it is not dynamic VHD image
					if(!strcmp((const char*)buf,"conectix")) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
						LOG_MSG("Dynamic VHD images are not supported");
						return;
					}
					// check MBR signature for unknown images
					if (!yet_detected && ((buf[510]!=0x55) || (buf[511]!=0xaa))) {
/* DOSBox-MB IMGMAKE patch. ========================================================================= */	

						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_GEOMETRY"));
						return;
					}

/* DOSBox-MB IMGMAKE patch. ========================================================================= */	
					// check MBR partition entry 1
					Bitu starthead = buf[0x1bf];
					Bitu startsect = buf[0x1c0]&0x3f-1;
					Bitu startcyl = buf[0x1c1]|((buf[0x1c0]&0xc0)<<2);
					Bitu endcyl = buf[0x1c5]|((buf[0x1c4]&0xc0)<<2);

					Bitu heads = buf[0x1c3]+1;
					Bitu sectors = buf[0x1c4]&0x3f;

					Bitu pe1_size = host_readd(&buf[0x1ca]);
					if(pe1_size!=0) {
						Bitu part_start = startsect + sectors*starthead +
							startcyl*sectors*heads;
						Bitu part_end = heads*sectors*endcyl;
						Bits part_len = part_end - part_start;
						// partition start/end sanity check
						// partition length should not exceed file length
						// real partition size can be a few cylinders less than pe1_size
						// if more than 1023 cylinders see if first partition fits
						// into 1023, else bail.
						if((part_len<0)||((Bitu)part_len > pe1_size)||(pe1_size > fcsize)||
							((pe1_size-part_len)/(sectors*heads)>2)||
							((pe1_size/(heads*sectors))>1023)) {
							//LOG_MSG("start(c,h,s) %u,%u,%u",startcyl,starthead,startsect);
							//LOG_MSG("endcyl %u heads %u sectors %u",endcyl,heads,sectors);
							//LOG_MSG("psize %u start %u end %u",pe1_size,part_start,part_end);
						} else if (!yet_detected) {
							sizes[0]=512; sizes[1]=sectors;
							sizes[2]=heads; sizes[3]=(Bit16u)(fcsize/(heads*sectors));
							if(sizes[3]>1023) sizes[3]=1023;
							yet_detected = true;
						}
					}
					if(!yet_detected) {
						// Try bximage disk geometry
						Bitu cylinders=(Bitu)(fcsize/(16*63));
						// Int13 only supports up to 1023 cylinders
						// For mounting unknown images we could go up with the heads to 255
						if ((cylinders*16*63==fcsize)&&(cylinders<1024)) {
							yet_detected=true;
							sizes[0]=512; sizes[1]=63; sizes[2]=16; sizes[3]=cylinders;
						}
					}

					if(yet_detected) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_AUTODET_VALUES"),sizes[0],sizes[1],sizes[2],sizes[3]);

					} else {
/* DOSBox-MB IMGMAKE patch. ========================================================================= */							
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_GEOMETRY"));
						return;
					}
					sizes[0]=512;	sizes[1]=63;	sizes[2]=16;	sizes[3]=sectors;
					LOG_MSG("AutoDetect: Harddrive Image %s", temp_line.c_str());
					LOG_MSG("Autodetect: Harddrive Size     [Autosized]", (sizes[1]*sizes[2]*sizes[3]*sizes[0]/1024)/1024);					
					LOG_MSG("Autodetect: Harddrive Geometry [Autosized]", sizes[0], sizes[1], sizes[2], sizes[3]);
					
					//LOG_MSG("autosized image file: %d:%d:%d:%d",sizes[0],sizes[1],sizes[2],sizes[3]);
				}

				if (Drives[drive-'A']) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_ALREADY_MOUNTED"));
					return;
				}

				std::vector<DOS_Drive*> imgDisks;
				std::vector<std::string>::size_type i;
				std::vector<DOS_Drive*>::size_type ct;
				
				for (i = 0; i < paths.size(); i++) {
					DOS_Drive* newDrive = new fatDrive(paths[i].c_str(),sizes[0],sizes[1],sizes[2],sizes[3],0);
					imgDisks.push_back(newDrive);
					if(!(dynamic_cast<fatDrive*>(newDrive))->created_successfully) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_CANT_CREATE"));
						for(ct = 0; ct < imgDisks.size(); ct++) {
							delete imgDisks[ct];
						}
						return;
					}
				}

				// Update DriveManager
				for(ct = 0; ct < imgDisks.size(); ct++) {
					DriveManager::AppendDisk(drive - 'A', imgDisks[ct]);
				}
				DriveManager::InitializeDrive(drive - 'A');

				// Set the correct media byte in the table 
				mem_writeb(Real2Phys(dos.tables.mediaid) + (drive - 'A') * 9, mediaid);
				
				/* Command uses dta so set it to our internal dta */
				RealPt save_dta = dos.dta();
				dos.dta(dos.tables.tempdta);

				for(ct = 0; ct < imgDisks.size(); ct++) {
					DriveManager::CycleDisks(drive - 'A', (ct == (imgDisks.size() - 1)));

					char root[7] = {drive,':','\\','*','.','*',0};
					DOS_FindFirst(root, DOS_ATTR_VOLUME); // force obtaining the label and saving it in dirCache
				}
				dos.dta(save_dta);
				
				// Multiple Imagemount ======================================================== BEGIN
				std::string lfw(paths[0]);
				int ddn = 0;
				
				for (i = 1; i < paths.size(); i++) {
					ddn = i;					
				}
				
				if (ddn > 0){						
					WriteOut("=========== Multiples Mount on Drive %c: ===========\n"
						     "            Press CTRL + F4 to Cycle through Disk's\n",drive);																
					WriteOut("Mounted:\n");							 
					
					for (i = 0; i < paths.size(); i++) {
						WriteOut("%s\n", paths[i].c_str());	
					}					
					WriteOut("====================================================\n");
					
				}else{
					WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"), drive, lfw.c_str());
					
				}
				
				// Multiple Imagemount ======================================================== END
				if (paths.size() == 1) {
					newdrive = imgDisks[0];
					switch (drive - 'A') {
					case 0:
					case 1:
						if(!((fatDrive *)newdrive)->loadedDisk->hardDrive) {
							if(imageDiskList[drive - 'A'] != NULL) delete imageDiskList[drive - 'A'];
							imageDiskList[drive - 'A'] = ((fatDrive *)newdrive)->loadedDisk;
							// If instructed, attach to IDE controller as ATA hard disk
							if (ide_index >= 0) IDE_Hard_Disk_Attach(ide_index,ide_slave,2);						
						}
						break;
					case 2:
					case 3:
						if(((fatDrive *)newdrive)->loadedDisk->hardDrive) {
							if(imageDiskList[drive - 'A'] != NULL) delete imageDiskList[drive - 'A'];
							imageDiskList[drive - 'A'] = ((fatDrive *)newdrive)->loadedDisk;
							// If instructed, attach to IDE controller as ATA hard disk
							if (ide_index >= 0) IDE_Hard_Disk_Attach(ide_index,ide_slave,3);							
							updateDPT();
						}
					break;
					}
				}
			} else if (fstype=="iso") {

				if (el_torito != "") {
					WriteOut("El Torito bootable CD: -fs iso mounting not supported\n"); /* <- NTS: Will never implement, either */
					return;
				}
				
				if (Drives[drive-'A']) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_ALREADY_MOUNTED"));
					return;
				}
				// create new drives for all images
				std::vector<DOS_Drive*> isoDisks;
				std::vector<std::string>::size_type i;
				std::vector<DOS_Drive*>::size_type ct;
				
				for (i = 0; i < paths.size(); i++) {
					int error = -1;
					DOS_Drive* newDrive = new isoDrive(drive, paths[i].c_str(), mediaid, error);
					isoDisks.push_back(newDrive);
					switch (error) {
						case 0  :	break;
						case 1  :	WriteOut(MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS"));	break;
						case 2  :	WriteOut(MSG_Get("MSCDEX_ERROR_NOT_SUPPORTED"));	break;
						case 3  :	WriteOut(MSG_Get("MSCDEX_ERROR_OPEN"));				break;
						case 4  :	WriteOut(MSG_Get("MSCDEX_TOO_MANY_DRIVES"));		break;
						case 5  :	WriteOut(MSG_Get("MSCDEX_LIMITED_SUPPORT"));		break;
						case 6  :	WriteOut(MSG_Get("MSCDEX_INVALID_FILEFORMAT"));		break;
						default :	WriteOut(MSG_Get("MSCDEX_UNKNOWN_ERROR"));			break;
					}
					// error: clean up and leave
					if (error) {
						for(ct = 0; ct < isoDisks.size(); ct++) {
							delete isoDisks[ct];
						}
						return;
					}
				}
				// Update DriveManager
				for(ct = 0; ct < isoDisks.size(); ct++) {
					DriveManager::AppendDisk(drive - 'A', isoDisks[ct]);
				}
				DriveManager::InitializeDrive(drive - 'A');
				
				// Set the correct media byte in the table 
				mem_writeb(Real2Phys(dos.tables.mediaid) + (drive - 'A') * 9, mediaid);
				
				int ddn = 0;
				std::string tmp(paths[0]);
				
				for (i = 1; i < paths.size(); i++) {
					ddn = i;
				}
				
				/*
					If instructed, attach to IDE controller as ATAPI CD-ROM device
				*/
				if (ide_index >= 0){
					IDE_CDROM_Attach(ide_index,ide_slave,drive - 'A');
					IDE_InfoConnect(fstype, ide_index, ide_slave, false);	
					LOG_MSG("IDE Atapi: CDROM %c: Mounted\n[%s]",drive - 'A', tmp.c_str());				
				}
			
				// Print status message (success)
				WriteOut("\n");				
				WriteOut(MSG_Get("MSCDEX_SUCCESS"));			
				if (ddn > 0){						
					WriteOut("=========== Multiples Mount on Drive %c: ===========\n"
						     "            Press CTRL + F3 to Cycle through CD's\n",drive);																
					WriteOut("Mounted:\n");							 
					
					for (i = 0; i < paths.size(); i++) {
						WriteOut("%s\n", paths[i].c_str());	
					}					
					WriteOut("====================================================\n");
					
				}else{
					WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"), drive, tmp.c_str());
					
				}

			} else if (el_torito != "") {
				newImage = new imageDiskElToritoFloppy(el_torito_cd_drive,el_torito_floppy_base,el_torito_floppy_type);
				
			} else {
				if (el_torito != "") {
					WriteOut("El Torito bootable CD: -fs none unexpected path (BUG)\n");
					return;
				}
				
/* DOSBox-MB IMGMAKE patch. ========================================================================= */					
				FILE *newDisk = fopen64(temp_line.c_str(), "rb+");			
/* DOSBox-MB IMGMAKE patch. ========================================================================= */					
				if (!newDisk) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
/* DOSBox-MB IMGMAKE patch. ========================================================================= */					
				fseeko64(newDisk,0L, SEEK_END);
				imagesize = (Bit32u)(ftello64(newDisk) / 1024);
/* DOSBox-MB IMGMAKE patch. ========================================================================= */					

				newImage = new imageDisk(newDisk, (Bit8u *)temp_line.c_str(), imagesize, (imagesize > 2880));
				if(imagesize>2880) newImage->Set_Geometry(sizes[2],sizes[3],sizes[1],sizes[0]);				
				if (reserved_cylinders > 0) newImage->Set_Reserved_Cylinders(reserved_cylinders);
			}
		} else {
			WriteOut(MSG_Get("PROGRAM_IMGMOUNT_TYPE_UNSUPPORTED"),type.c_str());
			return;
		}

		if (fstype=="none") {
			delete imageDiskList[drive-'0'];
			imageDiskList[drive-'0'] = newImage;
			updateDPT();
			
			WriteOut(MSG_Get("PROGRAM_IMGMOUNT_MOUNT_NUMBER"),drive-'0',temp_line.c_str());
			/*
				If instructed, attach to IDE controller as ATA hard disk
			*/
			if (ide_index >= 0) {
				IDE_Hard_Disk_Attach(ide_index,ide_slave,drive-'0');				
				IDE_InfoConnect(fstype, ide_index, ide_slave, false);			
			}
			
		} else {
			delete newImage;			
		}

		// check if volume label is given. be careful for cdrom
		//if (cmd->FindString("-label",label,true)) newdrive->dirCache.SetLabel(label.c_str());
		return;
	}
};

void IMGMOUNT_ProgramStart(Program * * make) {
	*make=new IMGMOUNT;
}


Bitu DOS_SwitchKeyboardLayout(const char* new_layout, Bit32s& tried_cp);
Bitu DOS_LoadKeyboardLayout(const char * layoutname, Bit32s codepage, const char * codepagefile);
const char* DOS_GetLoadedLayout(void);

class KEYB : public Program {
public:
	void Run(void);
};

void KEYB::Run(void) {
	if (cmd->FindCommand(1,temp_line)) {
		if (cmd->FindString("?",temp_line,false)) {
			WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
		} else {
			/* first parameter is layout ID */
			Bitu keyb_error=0;
			std::string cp_string;
			Bit32s tried_cp = -1;
			if (cmd->FindCommand(2,cp_string)) {
				/* second parameter is codepage number */
				tried_cp=atoi(cp_string.c_str());
				char cp_file_name[256];
				if (cmd->FindCommand(3,cp_string)) {
					/* third parameter is codepage file */
					strcpy(cp_file_name, cp_string.c_str());
				} else {
					/* no codepage file specified, use automatic selection */
					strcpy(cp_file_name, "auto");
				}

				keyb_error=DOS_LoadKeyboardLayout(temp_line.c_str(), tried_cp, cp_file_name);
			} else {
				keyb_error=DOS_SwitchKeyboardLayout(temp_line.c_str(), tried_cp);
			}
			switch (keyb_error) {
				case KEYB_NOERROR:
					WriteOut(MSG_Get("PROGRAM_KEYB_NOERROR"),temp_line.c_str(),dos.loaded_codepage);
					break;
				case KEYB_FILENOTFOUND:
					WriteOut(MSG_Get("PROGRAM_KEYB_FILENOTFOUND"),temp_line.c_str());
					WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
					break;
				case KEYB_INVALIDFILE:
					WriteOut(MSG_Get("PROGRAM_KEYB_INVALIDFILE"),temp_line.c_str());
					break;
				case KEYB_LAYOUTNOTFOUND:
					WriteOut(MSG_Get("PROGRAM_KEYB_LAYOUTNOTFOUND"),temp_line.c_str(),tried_cp);
					break;
				case KEYB_INVALIDCPFILE:
					WriteOut(MSG_Get("PROGRAM_KEYB_INVCPFILE"),temp_line.c_str());
					WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
					break;
				default:
					LOG(LOG_DOSMISC,LOG_ERROR)("KEYB:Invalid returncode %x",keyb_error);
					break;
			}
		}
	} else {
		/* no parameter in the command line, just output codepage info and possibly loaded layout ID */
		const char* layout_name = DOS_GetLoadedLayout();
		if (layout_name==NULL) {
			WriteOut(MSG_Get("PROGRAM_KEYB_INFO"),dos.loaded_codepage);
		} else {
			WriteOut(MSG_Get("PROGRAM_KEYB_INFO_LAYOUT"),dos.loaded_codepage,layout_name);
		}
	}
}

static void KEYB_ProgramStart(Program * * make) {
	*make=new KEYB;
}


void DOS_SetupPrograms(void) {
	/*Add Messages */
	MSG_Add("PROGRAM_MOUSE_INSTALL","Installed at PS/2 port.\n");
	MSG_Add("PROGRAM_MOUSE_UNINSTALL","Driver successfully unloaded...\n");
	MSG_Add("PROGRAM_MOUSE_ERROR","Already installed at PS/2 port.\n");
	MSG_Add("PROGRAM_MOUSE_NOINSTALLED","Driver is not installed.\n");
	MSG_Add("PROGRAM_MOUSE_HELP","Turns on/off mouse.\n\nMOUSE [/?] [/U]\n");	
	MSG_Add("PROGRAM_MOUNT_PHYS_CDROMS_NOT_SUPPORTED","Physical CDROMs aren't fully supported. IMGMOUNT may be more useful.\n");
	
	MSG_Add("PROGRAM_MOUNT_CDROMS_FOUND","CDROMs found: %d\n");	
	MSG_Add("PROGRAM_MOUNT_STATUS_FORMAT","%-5s  %-58s %-12s\n");
				// Multiple Imagemount ======================================================== BEGIN	
	MSG_Add("PROGRAM_MOUNT_STATUS_2","Mount Drive %c: %s\n");
				// Multiple Imagemount ======================================================== END	
	MSG_Add("PROGRAM_MOUNT_STATUS_3","Mount: %s in Drive %c\n");	
	MSG_Add("PROGRAM_MOUNT_STATUS_1","The currently mounted drives are:\n");
	MSG_Add("PROGRAM_MOUNT_ERROR_1","Directory %s doesn't exist.\n");
	MSG_Add("PROGRAM_MOUNT_ERROR_2","%s isn't a directory\n");
	MSG_Add("PROGRAM_MOUNT_ILL_TYPE","Illegal type %s\n");
	MSG_Add("PROGRAM_MOUNT_ALREADY_MOUNTED","Drive %c already mounted with %s\n");
	MSG_Add("PROGRAM_MOUNT_USAGE",
		"Usage \033[34;1mMOUNT Drive-Letter Local-Directory\033[0m\n"
		"For example: MOUNT c %s\n"
		"This makes the directory %s act as the C: drive inside DOSBox.\n"
		"The directory has to exist.\n");
	MSG_Add("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED","Drive %c isn't mounted.\n");
	MSG_Add("PROGRAM_MOUNT_UMOUNT_SUCCESS","Drive %c has successfully been removed.\n");
	MSG_Add("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL","Virtual Drives can not be unMOUNTed.\n");
	MSG_Add("PROGRAM_MOUNT_WARNING_WIN","\033[31;1mMounting c:\\ is NOT recommended. Please mount a (sub)directory next time.\033[0m\n");
	MSG_Add("PROGRAM_MOUNT_WARNING_OTHER","\033[31;1mMounting / is NOT recommended. Please mount a (sub)directory next time.\033[0m\n");

	MSG_Add("PROGRAM_MEM_CONVEN","%10d Kb free conventional memory\n");
	MSG_Add("PROGRAM_MEM_EXTEND","%10d Kb free extended memory\n");
	MSG_Add("PROGRAM_MEM_EXPAND","%10d Kb free expanded memory\n");
	MSG_Add("PROGRAM_MEM_UPPER","%10d Kb free upper memory in %d blocks (largest UMB %d Kb)\n");

	MSG_Add("PROGRAM_LOADFIX_ALLOC","%d kb allocated.\n");
	MSG_Add("PROGRAM_LOADFIX_DEALLOC","%d kb freed.\n");
	MSG_Add("PROGRAM_LOADFIX_DEALLOCALL","Used memory freed.\n");
	MSG_Add("PROGRAM_LOADFIX_ERROR","Memory allocation error.\n");

	MSG_Add("MSCDEX_SUCCESS","MSCDEX installed.\n");
	MSG_Add("MSCDEX_ERROR_MULTIPLE_CDROMS","MSCDEX: Failure: Drive-letters of multiple CD-ROM drives have to be continuous.\n");
	MSG_Add("MSCDEX_ERROR_NOT_SUPPORTED","MSCDEX: Failure: Not yet supported.\n");
	MSG_Add("MSCDEX_ERROR_PATH","MSCDEX: Specified location is not a CD-ROM drive.\n");
	MSG_Add("MSCDEX_ERROR_OPEN","MSCDEX: Failure: Invalid file or unable to open.\n");
	MSG_Add("MSCDEX_TOO_MANY_DRIVES","MSCDEX: Failure: Too many CD-ROM drives (max: 5). MSCDEX Installation failed.\n");
	MSG_Add("MSCDEX_LIMITED_SUPPORT","MSCDEX: Mounted subdirectory: limited support.\n");
	MSG_Add("MSCDEX_INVALID_FILEFORMAT","MSCDEX: Failure: File is either no ISO/CUE image or contains errors.\n");
	MSG_Add("MSCDEX_UNKNOWN_ERROR","MSCDEX: Failure: Unknown error.\n");

	MSG_Add("PROGRAM_RESCAN_SUCCESS","Drive cache cleared.\n");

	MSG_Add("PROGRAM_INTRO",
		"\033[2J\033[32;1mWelcome to DOSBox\033[0m, an x86 emulator with sound and graphics.\n"
		"DOSBox creates a shell for you which looks like old plain DOS.\n"
		"\n"
		"For information about basic mount type \033[34;1mintro mount\033[0m\n"
		"For information about CD-ROM support type \033[34;1mintro cdrom\033[0m\n"
		"For information about special keys type \033[34;1mintro special\033[0m\n"
		"For more information about DOSBox, go to \033[34;1mhttp://www.dosbox.com/wiki\033[0m\n"
		"\n"
		"\033[31;1mDOSBox will stop/exit without a warning if an error occurred!\033[0m\n"
		"\n"
		"\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_START",
		"\033[32;1mHere are some commands to get you started:\033[0m\n"
		"Before you can use the files located on your own filesystem,\n"
		"You have to mount the directory containing the files.\n"
		"\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_WINDOWS",
		"\033[44;1m\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n"
		"\xBA \033[32mmount c c:\\dosgames\\\033[37m will create a C drive with c:\\dosgames as contents.\xBA\n"
		"\xBA                                                                         \xBA\n"
		"\xBA \033[32mc:\\dosgames\\\033[37m is an example. Replace it with your own games directory.  \033[37m \xBA\n"
		"\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\033[0m\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_OTHER",
		"\033[44;1m\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n"
		"\xBA \033[32mmount c ~/dosgames\033[37m will create a C drive with ~/dosgames as contents.\xBA\n"
		"\xBA                                                                      \xBA\n"
		"\xBA \033[32m~/dosgames\033[37m is an example. Replace it with your own games directory.\033[37m  \xBA\n"
		"\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\033[0m\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_END",
		"When the mount has successfully completed you can type \033[34;1mc:\033[0m to go to your freshly\n"
		"mounted C-drive. Typing \033[34;1mdir\033[0m there will show its contents."
		" \033[34;1mcd\033[0m will allow you to\n"
		"enter a directory (recognised by the \033[33;1m[]\033[0m in a directory listing).\n"
		"You can run programs/files which end with \033[31m.exe .bat\033[0m and \033[31m.com\033[0m.\n"
		);
	MSG_Add("PROGRAM_INTRO_CDROM",
		"\033[2J\033[32;1mHow to mount a Real/Virtual CD-ROM Drive in DOSBox:\033[0m\n"
		"DOSBox provides CD-ROM emulation on a few levels.\n"
		"\n"
		"The \033[33mbasic\033[0m level works on all CD-ROM drives and normal directories.\n"
		"It installs MSCDEX and marks the files read-only.\n"
		"Usually this is enough for most games:\n"
		"\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom\033[0m   or   \033[34;1mmount d C:\\example -t cdrom\033[0m\n"
		"If it doesn't work you might have to tell DOSBox the label of the CD-ROM:\n"
		"\033[34;1mmount d C:\\example -t cdrom -label CDLABEL\033[0m\n"
		"\n"
		"The \033[33mhigher\033[0m level adds CD-ROM image mounting support.\n"
		"Therefore only works on supported CD-ROM images:\n"
		"\033[34;1mimgmount d \033[0;31mD:\\example.img\033[34;1m -t cdrom\033[0m\n"
		"\n"
		"Replace \033[0;31mD:\\\033[0m with the location of your CD-ROM.\n"
		"Replace \033[0;31mD:\\example.img\033[0m with the location of your CD-ROM image.\n"
		);
	MSG_Add("PROGRAM_INTRO_SPECIAL",
		"\033[2J\033[32;1mSpecial keys:\033[0m\n"
		"These are the default keybindings.\n"
		"They can be changed in the \033[33mkeymapper\033[0m.\n"
		"\n"
		"\033[33;1mALT-ENTER\033[0m   : Go full screen and back.\n"
		"\033[33;1mALT-PAUSE\033[0m   : Pause DOSBox.\n"
		"\033[33;1mALT-F9\033[0m      : Set Mixer [All Channels] Volume Up.\n"
		"\033[33;1mALT-F10\033[0m     : Set Mixer [All Channels] Volume Down.\n"			
		"\033[33;1mALT-F12\033[0m     : Unlock speed Fix (turbo button/fast forward).\n"			
		"\033[33;1mALT-F12\033[0m     : Unlock speed (turbo button/fast forward).\n"		
		"\033[33;1mCTRL-F1\033[0m     : Start the \033[33mkeymapper\033[0m.\n"
		"\033[33;1mCTRL-F3\033[0m     : Cyle/Swap/Update Directory Cache for CD-ROM's.\n"		
		"\033[33;1mCTRL-F4\033[0m     : Cyle/Swap/Update Directory Cache for Floppy's.\n"		
		"\033[33;1mCTRL-F5\033[0m     : Save a screenshot.\n"
		"\033[33;1mCTRL-F6\033[0m     : Start/Stop recording sound output to a wave file.\n"
		"\033[33;1mCTRL-F7\033[0m     : Decrease frameskip.\n"
		"\033[33;1mCTRL-F8\033[0m     : Increase frameskip.\n"
		"\033[33;1mCTRL-F9\033[0m     : Kill DOSBox.\n"
		"\033[33;1mCTRL-F10\033[0m    : Capture/Release the mouse.\n"
		"\033[33;1mCTRL-F11\033[0m    : Slow down emulation (Decrease DOSBox Cycles).\n"
		"\033[33;1mCTRL-F12\033[0m    : Speed up emulation (Increase DOSBox Cycles).\n"		
		"\033[33;1mCTRL-ALT-F5\033[0m : Start/Stop creating a movie of the screen.\n"				
		"\033[33;1mCTRL-ALT-F7\033[0m : Start/Stop recording of OPL commands.\n"
		"\033[33;1mCTRL-ALT-F8\033[0m : Start/Stop the recording of raw MIDI commands.\n"		
	
		);
	MSG_Add("PROGRAM_BOOT_NOT_EXIST","Bootdisk file does not exist.  Failing.\n");
	MSG_Add("PROGRAM_BOOT_NOT_OPEN","Cannot open bootdisk file.  Failing.\n");
	MSG_Add("PROGRAM_BOOT_WRITE_PROTECTED","Image file is read-only! Might create problems.\n");
	MSG_Add("PROGRAM_BOOT_PRINT_ERROR","This command boots DOSBox from either a floppy or hard disk image.\n\n"
		"For this command, one can specify a succession of floppy disks swappable\n"
		"by pressing Ctrl-F4, and -l specifies the mounted drive to boot from.  If\n"
		"no drive letter is specified, this defaults to booting from the A drive.\n"
		"The only bootable drive letters are A, C, and D.  For booting from a hard\n"
		"drive (C or D), the image should have already been mounted using the\n"
		"\033[34;1mIMGMOUNT\033[0m command.\n\n"
		"The syntax of this command is:\n\n"
		"\033[34;1mBOOT [diskimg1.img diskimg2.img] [-l driveletter]\033[0m\n"
		);
	MSG_Add("PROGRAM_BOOT_UNABLE","Unable to boot off of drive %c\n");
	MSG_Add("PROGRAM_BOOT_IMAGE_OPEN","Opening image file: %s\n");
	MSG_Add("PROGRAM_BOOT_IMAGE_NOT_OPEN","Cannot open %s");
	MSG_Add("PROGRAM_BOOT_BOOT","Booting from drive %c...\n");
	MSG_Add("PROGRAM_BOOT_CART_WO_PCJR","PCjr cartridge found, but machine is not PCjr");
	MSG_Add("PROGRAM_BOOT_CART_LIST_CMDS","Available PCjr cartridge commandos:%s");
	MSG_Add("PROGRAM_BOOT_CART_NO_CMDS","No PCjr cartridge commandos found");

	MSG_Add("PROGRAM_LOADROM_SPECIFY_FILE","Must specify ROM file to load.\n");
	MSG_Add("PROGRAM_LOADROM_CANT_OPEN","ROM file not accessible.\n");
	MSG_Add("PROGRAM_LOADROM_TOO_LARGE","ROM file too large.\n");
	MSG_Add("PROGRAM_LOADROM_INCOMPATIBLE","Video BIOS not supported by machine type.\n");
	MSG_Add("PROGRAM_LOADROM_UNRECOGNIZED","ROM file not recognized.\n");
	MSG_Add("PROGRAM_LOADROM_BASIC_LOADED","BASIC ROM loaded.\n");

	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY_DRIVE","Must specify drive letter to mount image at.\n");
	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY2","Must specify drive number (0 or 3) to mount image at (0,1=fda,fdb;2,3=hda,hdb).\n");
	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY_GEOMETRY",
		"For \033[33mCD-ROM\033[0m images:   \033[34;1mIMGMOUNT drive-letter location-of-image -t iso\033[0m\n"
		"\n"
		"For \033[33mhardrive\033[0m images: Must specify drive geometry for hard drives:\n"
		"bytes_per_sector, sectors_per_cylinder, heads_per_cylinder, cylinder_count.\n"
		"\033[34;1mIMGMOUNT drive-letter location-of-image -size bps,spc,hpc,cyl\033[0m\n");
	MSG_Add("PROGRAM_IMGMOUNT_INVALID_IMAGE","Could not load image file.\n"
		"Check that the path is correct and the image is accessible.\n");
	MSG_Add("PROGRAM_IMGMOUNT_INVALID_GEOMETRY","Could not extract drive geometry from image.\n"
		"Use parameter -size bps,spc,hpc,cyl to specify the geometry.\n");
		
/* DOSBox-MB IMGMAKE patch. ========================================================================= */		
	MSG_Add("PROGRAM_IMGMOUNT_AUTODET_VALUES","Image geometry auto detection: -size %u,%u,%u,%u\n");		
/* DOSBox-MB IMGMAKE patch. ========================================================================= */
	
	MSG_Add("PROGRAM_IMGMOUNT_TYPE_UNSUPPORTED","Type \"%s\" is unsupported. Specify \"hdd\" or \"floppy\" or\"iso\".\n");
	MSG_Add("PROGRAM_IMGMOUNT_FORMAT_UNSUPPORTED","Format \"%s\" is unsupported. Specify \"fat\" or \"iso\" or \"none\".\n");
	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY_FILE","Must specify file-image to mount.\n");
	MSG_Add("PROGRAM_IMGMOUNT_FILE_NOT_FOUND","Image file not found.\n");
	MSG_Add("PROGRAM_IMGMOUNT_MOUNT","To mount directories, use the \033[34;1mMOUNT\033[0m command, not the \033[34;1mIMGMOUNT\033[0m command.\n");
	MSG_Add("PROGRAM_IMGMOUNT_ALREADY_MOUNTED","Drive already mounted at that letter.\n");
	MSG_Add("PROGRAM_IMGMOUNT_CANT_CREATE","Can't create drive from file.\n");
	MSG_Add("PROGRAM_IMGMOUNT_MOUNT_NUMBER","Drive number %d mounted as %s\n");
	MSG_Add("PROGRAM_IMGMOUNT_NON_LOCAL_DRIVE", "The image must be on a host or local drive.\n");
	MSG_Add("PROGRAM_IMGMOUNT_MULTIPLE_NON_CUEISO_FILES", "Using multiple files is only supported for cue/iso images.\n");
	
/* DOSBox-MB IMGMAKE patch. ========================================================================= */	
	MSG_Add("PROGRAM_IMGMOUNT_SYNTAX",
		    "\033[2J\033[32;1mCreates Floppy or Harddisk Images. Syntax:\033[0m\n"
		    "IMGMAKE file [-t] [-size] [-chs] [-nofs] [-source] [-r retries] [-bat] [-txt]\n"
		    "File       : Image file that is to be created  on the Host. [-t Image type].\n\n"
		    "\033[33;1mFloppy Disk templates\033[0m\n"
		    " fd_160: 160KB  fd_180: 180KB  fd_200 : 200KB  fd_320 : 320KB   fd_360 : 360KB\n"
			" fd_400: 400KB  fd_720: 720KB  fd_1200: 1.2MB  fd_1440: 1.44MB  fd_2880: 2.88MB\n"
		    "\033[33;1mHarddisk templates:\033[0m\n"
		    " hd_st225: 20MB    hd_st251: 40MB    hd_250 : 250MB   hd_520 : 520MB\n"
		    " hd_2gig :  2GB    hd_4gig : 4GB     hd_8gig: 8GB    (8GB is Maximum)\n\n"
		    "\033[33;1mCustom Harddisk Images: hd (requires -size or -chs)\033[0m\n"		   
		    " -size    : The size of a custom harddisk image in MB.\n"
		    " -chs     : Disk geometry in cylinders (1-1023), Heads( 1-255), Sectors (1-63).\n"
		    " -nofs    : Add this parameter if a blank image should be created.\n"
		    " -bat/-txt: Creates a Batch/Text file with the IMGMOUNT command for this image.\n"
#ifdef WIN32
		    " -source  : Drive letter. If specified the image is read from a floppy disk.\n"
		    " -retries : How often to retry to read from a bad floppy disk (1-99).\n\n"
#endif
		    " \033[33;1mExamples:\033[0m\n"
		    "  \033[33mimgmake c:\\image.img -t fd_1440\033[0m          - create a 1.44MB floppy image\n"
		    "  \033[33mimgmake c:\\image.img -t hd -size 100\033[0m     - create a 100MB hdd image\n"
		    "  \033[33mimgmake c:\\image.img -t hd -chs 130,2,17\033[0m - create a special hd image\n"
#ifdef WIN32
		    "  \033[33mimgmake c:\\image.img -source a\033[0m           - read image from physical drive A"
#endif
		);
#ifdef WIN32
	MSG_Add("PROGRAM_IMGMAKE_FLREAD",
		"Disk geometry: %d Cylinders, %d Heads, %d Sectors, %d Kilobytes\n\n");
	MSG_Add("PROGRAM_IMGMAKE_FLREAD2",
		"\xdb =good, \xb1 =good after retries, ! =CRC error, x =sector not found, ? =unknown\n\n");
#endif
	MSG_Add("PROGRAM_IMGMAKE_FILE_EXISTS","The file \"%s\" already exists.\n");
	MSG_Add("PROGRAM_IMGMAKE_CANNOT_WRITE","The file \"%s\" cannot be opened for writing.\n");
	MSG_Add("PROGRAM_IMGMAKE_NOT_ENOUGH_SPACE","Not enough space availible for the image file. Need %u bytes.\n");
	MSG_Add("PROGRAM_IMGMAKE_PRINT_CHS","Creating an image file with %u cylinders, %u heads and %u sectors\n");
	MSG_Add("PROGRAM_IMGMAKE_CANT_READ_FLOPPY","\n\nUnable to read floppy.");
/* DOSBox-MB IMGMAKE patch. ========================================================================= */
	

	MSG_Add("PROGRAM_KEYB_INFO","Codepage %i has been loaded\n");
	MSG_Add("PROGRAM_KEYB_INFO_LAYOUT","Codepage %i has been loaded for layout %s\n");
	MSG_Add("PROGRAM_KEYB_SHOWHELP",
		"\033[32;1mKEYB\033[0m [keyboard layout ID[ codepage number[ codepage file]]]\n\n"
		"Some examples:\n"
		"  \033[32;1mKEYB\033[0m: Display currently loaded codepage.\n"
		"  \033[32;1mKEYB\033[0m sp: Load the spanish (SP) layout, use an appropriate codepage.\n"
		"  \033[32;1mKEYB\033[0m sp 850: Load the spanish (SP) layout, use codepage 850.\n"
		"  \033[32;1mKEYB\033[0m sp 850 mycp.cpi: Same as above, but use file mycp.cpi.\n");
	MSG_Add("PROGRAM_KEYB_NOERROR","Keyboard layout %s loaded for codepage %i\n");
	MSG_Add("PROGRAM_KEYB_FILENOTFOUND","Keyboard file %s not found\n\n");
	MSG_Add("PROGRAM_KEYB_INVALIDFILE","Keyboard file %s invalid\n");
	MSG_Add("PROGRAM_KEYB_LAYOUTNOTFOUND","No layout in %s for codepage %i\n");
	MSG_Add("PROGRAM_KEYB_INVCPFILE","None or invalid codepage file for layout %s\n\n");

	/* Commands
		\033[33; == TAB
		\033[2J\033[32;1m -- TEXT GRÜN -- \033[0m
		\033[33m
		\033[33;1m
	*/
	
	/*regular setup*/
	PROGRAMS_MakeFile("BIOSTEST.COM", BIOS_ProgramStart);	
	PROGRAMS_MakeFile("BOOT.COM",BOOT_ProgramStart);
	PROGRAMS_MakeFile("INTRO.COM",INTRO_ProgramStart);
	PROGRAMS_MakeFile("IMGMAKE.COM", IMGMAKE_ProgramStart);	
	PROGRAMS_MakeFile("IMGMOUNT.COM", IMGMOUNT_ProgramStart);
	PROGRAMS_MakeFile("KEYB.COM", KEYB_ProgramStart);	
	PROGRAMS_MakeFile("LOADFIX.COM",LOADFIX_ProgramStart);	
	PROGRAMS_MakeFile("LOADROM.COM", LOADROM_ProgramStart);	
	PROGRAMS_MakeFile("MEM.COM",MEM_ProgramStart);	
	PROGRAMS_MakeFile("MOUNT.COM",MOUNT_ProgramStart);
	PROGRAMS_MakeFile("MOUSE.COM", MOUSE_ProgramStart);	
	PROGRAMS_MakeFile("RESCAN.COM",RESCAN_ProgramStart);
}
