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

#ifndef DOSBOX_BIOS_DISK_H
#define DOSBOX_BIOS_DISK_H

#include <stdio.h>
#ifndef DOSBOX_MEM_H
#include "mem.h"
#endif
#ifndef DOSBOX_DOS_INC_H
#include "dos_inc.h"
#endif
#ifndef DOSBOX_BIOS_H
#include "bios.h"
#endif

/* The Section handling Bios Disk Access */
#define BIOS_MAX_DISK 20

#define MAX_SWAPPABLE_DISKS 40
struct diskGeo {
	Bit32u ksize;  /* Size in kilobytes */
	Bit16u secttrack; /* Sectors per track */
	Bit16u headscyl;  /* Heads per cylinder */
	Bit16u cylcount;  /* Cylinders per side */
	Bit16u biosval;   /* Type to return from BIOS */
};
extern diskGeo DiskGeometryList[];

class imageDisk  {
public:
	enum {		
		ID_BASE=0,
		ID_EL_TORITO_FLOPPY
	};
	int class_id;
	
	Bit8u Read_Sector(Bit32u head,Bit32u cylinder,Bit32u sector,void * data);
	Bit8u Write_Sector(Bit32u head,Bit32u cylinder,Bit32u sector,void * data);
	Bit8u Read_AbsoluteSector(Bit32u sectnum, void * data);
	Bit8u Write_AbsoluteSector(Bit32u sectnum, void * data);
	
	/* INT13, IDE              ========================================================================= */	
	void Set_Reserved_Cylinders(Bitu resCyl);
	Bit32u Get_Reserved_Cylinders();
	/* INT13, IDE              ========================================================================= */	
	
	void Set_Geometry(Bit32u setHeads, Bit32u setCyl, Bit32u setSect, Bit32u setSectSize);
	void Get_Geometry(Bit32u * getHeads, Bit32u *getCyl, Bit32u *getSect, Bit32u *getSectSize);
	Bit8u GetBiosType(void);
	Bit32u getSectSize(void);
	imageDisk(FILE *imgFile, Bit8u *imgName, Bit32u imgSizeK, bool isHardDisk);
	~imageDisk() { if(diskimg != NULL) { fclose(diskimg); }	};

	bool hardDrive;
	bool active;
	FILE *diskimg;
	Bit8u diskname[512];
	Bit8u floppytype;

	Bit32u sector_size;
	Bit32u heads,cylinders,sectors;
	/* INT13, IDE              ========================================================================= */		
	Bit32u reserved_cylinders;
	/* DOSBox-MB IMGMAKE patch. ========================================================================= */
	Bit64u current_fpos;
	/* DOSBox-MB IMGMAKE patch. ========================================================================= */	
private:
	
	enum { NONE,READ,WRITE } last_action;
	volatile int refcount = 0;
public:
	int Addref() {
		return ++refcount;
	}
	int Release() {
		int ret = --refcount;
		if (ret < 0) {
			fprintf(stderr, "WARNING: imageDisk Release() changed refcount to %d\n", ret);
			abort();
		}
		if (ret == 0) delete this;
		return ret;
	}
};

void updateDPT(void);
void incrementFDD(void);


#define MAX_HDD_IMAGES 23
#define MAX_DISK_IMAGES (2 + MAX_HDD_IMAGES)

extern bool imageDiskChange[MAX_DISK_IMAGES];
extern imageDisk *imageDiskList[MAX_DISK_IMAGES];
extern imageDisk *diskSwap[MAX_SWAPPABLE_DISKS];
extern Bit32s swapPosition;
extern Bit16u imgDTASeg; /* Real memory location of temporary DTA pointer for fat image disk access */
extern RealPt imgDTAPtr; /* Real memory location of temporary DTA pointer for fat image disk access */
extern DOS_DTA *imgDTA;

void swapInDisks(void);
void swapInNextDisk(bool clicked, bool FLOPPY, bool CDROM);
bool getSwapRequest(void);

#endif
