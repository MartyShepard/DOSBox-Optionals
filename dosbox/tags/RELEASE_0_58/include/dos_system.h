/*
 *  Copyright (C) 2002  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DOSSYSTEM_H_
#define DOSSYSTEM_H_

#include <string.h>
#include <vector>
#include "dosbox.h"
#include "cross.h"

#define DOS_NAMELENGTH 12
#define DOS_NAMELENGTH_ASCII (DOS_NAMELENGTH+1)
#define DOS_FCBNAME 15
#define DOS_DIRDEPTH 8
#define DOS_PATHLENGTH 80
#define DOS_TEMPSIZE 1024

enum {
	DOS_ATTR_READ_ONLY=	0x01,
	DOS_ATTR_HIDDEN=	0x02,
	DOS_ATTR_SYSTEM=	0x04,
	DOS_ATTR_VOLUME=	0x08,
	DOS_ATTR_DIRECTORY=	0x10,
	DOS_ATTR_ARCHIVE=	0x20
};

struct FileStat_Block {
	Bit32u size;
	Bit16u time;
	Bit16u date;
	Bit16u attr;
};

class DOS_DTA;

class DOS_File {
public:
	DOS_File()		{ name=0; };
	virtual	~DOS_File(){};
	virtual bool	Read(Bit8u * data,Bit16u * size)=0;
	virtual bool	Write(Bit8u * data,Bit16u * size)=0;
	virtual bool	Seek(Bit32u * pos,Bit32u type)=0;
	virtual bool	Close()=0;
	virtual Bit16u	GetInformation(void)=0;
	virtual void	SetName(const char* _name)	{ if (name) delete[] name; name = new char[strlen(_name)+1]; strcpy(name,_name); }
	virtual char*	GetName(void)				{ return name; };
	virtual bool	IsOpen()					{ return open; };
	virtual bool	IsName(const char* _name)	{ if (!name) return false; return strcmp(name,_name)==0; };
	Bit8u type;
	Bit32u flags;
	Bit16u time;
	Bit16u date;
	Bit16u attr;
	Bit32u size;
	bool open;
	char* name;
/* Some Device Specific Stuff */
};

class DOS_Device : public DOS_File {
public:
/* Some Device Specific Stuff */
	char * name;
	Bit8u fhandle;	
};

#define MAX_OPENDIRS 16 

class DOS_Drive_Cache {
public:
	DOS_Drive_Cache					(void);
	DOS_Drive_Cache					(const char* path);
	~DOS_Drive_Cache				(void);

	typedef enum TDirSort { NOSORT, ALPHABETICAL, DIRALPHABETICAL, ALPHABETICALREV, DIRALPHABETICALREV };

	void		SetBaseDir			(const char* path);
	void		SetDirSort			(TDirSort sort) { sortDirType = sort; };
	bool		OpenDir				(const char* path, Bit16u& id);
	bool		ReadDir				(Bit16u id, char* &result);

	void		ExpandName			(char* path);
	char*		GetExpandName		(const char* path);
	bool		GetShortName		(const char* fullname, char* shortname);
	
	void		CacheOut			(const char* path, bool ignoreLastDir = false);
	void		AddEntry			(const char* path, bool checkExist = false);
	void		DeleteEntry			(const char* path, bool ignoreLastDir = false);

	void		EmptyCache			(void);

	class CFileInfo {
	public:	
		~CFileInfo(void) {
			for (Bit32u i=0; i<fileList.size(); i++) delete fileList[i];
			fileList.clear();
			longNameList.clear();
			outputList.clear();
		};
		char		orgname		[CROSS_LEN];
		char		shortname	[DOS_NAMELENGTH_ASCII];
		bool		isDir;
		Bitu		nextEntry;
		Bitu		shortNr;
		Bitu		compareCount;
		// contents
		std::vector<CFileInfo*>	fileList;
		std::vector<CFileInfo*>	longNameList;
		std::vector<CFileInfo*>	outputList;
	};

private:

	bool		RemoveTrailingDot	(char* shortname);
	Bit16s		GetLongName			(CFileInfo* info, char* shortname);
	void		CreateShortName		(CFileInfo* dir, CFileInfo* info);
	Bit16u		CreateShortNameID	(CFileInfo* dir, const char* name);
	bool		SetResult			(CFileInfo* dir, char * &result, Bit16u entryNr);
	bool		IsCachedIn			(CFileInfo* dir);
	CFileInfo*	FindDirInfo			(const char* path, char* expandedPath);
	bool		RemoveSpaces		(char* str);
	bool		OpenDir				(CFileInfo* dir, char* path, Bit16u& id);
	void		CreateEntry			(CFileInfo* dir, const char* name);
	Bit16u		GetFreeID			(CFileInfo* dir);
	void		Clear				(void);


	CFileInfo*	dirBase;
	char		dirPath				[CROSS_LEN];
	char		basePath			[CROSS_LEN];
	bool		dirFirstTime;
	TDirSort	sortDirType;
	CFileInfo*	save_dir;
	char		save_path			[CROSS_LEN];
	char		save_expanded		[CROSS_LEN];

	Bit16u		srchNr;
	CFileInfo*	dirSearch			[MAX_OPENDIRS];
	char		dirSearchName		[MAX_OPENDIRS];
	bool		free				[MAX_OPENDIRS];

};

class DOS_No_Drive_Cache {
public:
	DOS_No_Drive_Cache				(void) {};
	DOS_No_Drive_Cache				(const char* path);
	~DOS_No_Drive_Cache				(void) {};

	typedef enum TDirSort { NOSORT, ALPHABETICAL, DIRALPHABETICAL, ALPHABETICALREV, DIRALPHABETICALREV };

	void		SetBaseDir			(const char* path);
	void		SetDirSort			(TDirSort sort) {};
	bool		OpenDir				(const char* path, Bit16u& id);
	bool		ReadDir				(Bit16u id, char * &result);

	void		ExpandName			(char* path) {};
	char*		GetExpandName		(const char* path) { return (char*)path; };
	bool		GetShortName		(const char* fullname, char* shortname) { return false; };
	
	void		CacheOut			(const char* path, bool ignoreLastDir = false) {};
	void		AddEntry			(const char* path, bool checkExists = false) {};
	void		DeleteEntry			(const char* path, bool ignoreLastDir = false) {};

	void		SetCurrentEntry		(Bit16u entry) {};
	Bit16u		GetCurrentEntry		(void) { return 0; };

	void		EmptyCache			(void) {};

public:
	char		basePath			[CROSS_LEN];
	char		dirPath				[CROSS_LEN];
};

class DOS_Drive {
public:
	DOS_Drive();
	virtual ~DOS_Drive(){};
	virtual bool FileOpen(DOS_File * * file,char * name,Bit32u flags)=0;
	virtual bool FileCreate(DOS_File * * file,char * name,Bit16u attributes)=0;
	virtual bool FileUnlink(char * _name)=0;
	virtual bool RemoveDir(char * _dir)=0;
	virtual bool MakeDir(char * _dir)=0;
	virtual bool TestDir(char * _dir)=0;
	virtual bool FindFirst(char * _dir,DOS_DTA & dta)=0;
	virtual bool FindNext(DOS_DTA & dta)=0;
	virtual bool GetFileAttr(char * name,Bit16u * attr)=0;
	virtual bool Rename(char * oldname,char * newname)=0;
	virtual bool AllocationInfo(Bit16u * _bytes_sector,Bit8u * _sectors_cluster,Bit16u * _total_clusters,Bit16u * _free_clusters)=0;
	virtual bool FileExists(const char* name)=0;
	virtual bool FileStat(const char* name, FileStat_Block * const stat_block)=0;
	virtual Bit8u GetMediaByte(void)=0;
	virtual void SetDir(const char* path) { strcpy(curdir,path); };
	virtual void EmptyCache(void) { dirCache.EmptyCache(); };
	char * GetInfo(void);
	char curdir[DOS_PATHLENGTH];
	char info[256];

	DOS_Drive_Cache dirCache;
};

enum { OPEN_READ=0,OPEN_WRITE=1,OPEN_READWRITE=2 };
enum { DOS_SEEK_SET=0,DOS_SEEK_CUR=1,DOS_SEEK_END=2};


/*
 A multiplex handler should read the registers to check what function is being called 
 If the handler returns false dos will stop checking other handlers 
*/

typedef bool (MultiplexHandler)(void);
void DOS_AddMultiplexHandler(MultiplexHandler * handler);

void DOS_AddDevice(DOS_Device * adddev);
void VFILE_Register(const char * name,Bit8u * data,Bit32u size);
#endif

