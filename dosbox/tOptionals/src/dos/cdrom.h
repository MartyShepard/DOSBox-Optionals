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



#ifndef __CDROM_INTERFACE__
#define __CDROM_INTERFACE__

#define MAX_ASPI_CDROM	5

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include "dosbox.h"
#include "mem.h"
#include "mixer.h"

#if defined(_MSC_VER)
	#include <SDL2\include\SDL.h>
	#include <SDL2\include\SDL_thread.h>
#else
	#include <SDL.h>
	#include <SDL_thread.h>
#endif

#include "../libs/decoders/SDL_sound.h"

#if defined(_MSC_VER)
	#include "..\libs\libchdr\chd.h"
#else
	#include "src/libs/libchdr/chd.h"
#endif

#define RAW_SECTOR_SIZE		2352
#define COOKED_SECTOR_SIZE	2048
#define AUDIO_DECODE_BUFFER_SIZE 16512
// 16512 is 16384 + 128, enough for four 4KB decode audio chunks plus 128 bytes extra
// which accomodate the leftovers from typically callbacks, which minimizes our wrap size.

/** @name Frames / MSF Conversion Functions
 *  Conversion functions from frames to Minute/Second/Frames and vice versa
 */
/*@{*/
#define CD_FPS	75
#define FRAMES_TO_MSF(f, M,S,F)	{					\
	int value = f;							\
	*(F) = value%CD_FPS;						\
	value /= CD_FPS;						\
	*(S) = value%60;						\
	value /= 60;							\
	*(M) = value;							\
}
#define MSF_TO_FRAMES(M, S, F)	((M)*60*CD_FPS+(S)*CD_FPS+(F))
/*@}*/

typedef struct SMSF {
	unsigned char min;
	unsigned char sec;
	unsigned char fr;
} TMSF;

typedef struct SCtrl {
	Bit8u	out[4];			// output channel
	Bit8u	vol[4];			// channel volume
} TCtrl;

extern int CDROM_GetMountType(char* path, int force);

	
class CDROM_Interface
{
public:
//	CDROM_Interface						(void);
	virtual ~CDROM_Interface			(void) {};

	virtual bool	SetDevice			(char* path, int forceCD) = 0;

	virtual bool	GetUPC				(unsigned char& attr, char* upc) = 0;

	virtual bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut) = 0;
	virtual bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr) = 0;
	virtual bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos) = 0;
	virtual bool	GetAudioStatus		(bool& playing, bool& pause) = 0;
	virtual bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen) = 0;

	virtual bool	PlayAudioSector		(unsigned long start,unsigned long len) = 0;
	virtual bool	PauseAudio			(bool resume) = 0;
	virtual bool	StopAudio			(void) = 0;
	virtual void	ChannelControl		(TCtrl /*ctrl*/) { return; };
	
	virtual bool	ReadSectors			(PhysPt buffer, bool raw, unsigned long sector, unsigned long num) = 0;

	/* This is needed for IDE hack, who's buffer does not exist in DOS physical memory */
	virtual bool	ReadSectorsHost			(void* buffer, bool raw, unsigned long sector, unsigned long num) = 0;	
	virtual bool	LoadUnloadMedia		(bool unload) = 0;
	
	virtual void	InitNewMedia		(void) {};
};	

class CDROM_Interface_Fake : public CDROM_Interface
{
public:
	bool	SetDevice			(char* /*path*/, int /*forceCD*/) { return true; };
	bool	GetUPC				(unsigned char& attr, char* upc) { attr = 0; strcpy(upc,"UPC"); return true; };
	bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	bool	GetAudioStatus		(bool& playing, bool& pause);
	bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);
	bool	PlayAudioSector		(unsigned long /*start*/,unsigned long /*len*/) { return true; };
	bool	PauseAudio			(bool /*resume*/) { return true; };
	bool	StopAudio			(void) { return true; };
	void	ChannelControl		(TCtrl /*ctrl*/) { return; };
	bool	ReadSectors			(PhysPt /*buffer*/, bool /*raw*/, unsigned long /*sector*/, unsigned long /*num*/) { return true; };
	/* This is needed for IDE hack, who's buffer does not exist in DOS physical memory */	
	bool	ReadSectorsHost			(void* buffer, bool raw, unsigned long sector, unsigned long num);	
	bool	LoadUnloadMedia		(bool /*unload*/) { return true; };
};	

class CDROM_Interface_Image : public CDROM_Interface
{
private:
	class TrackFile {
	protected:
		TrackFile(Bit16u chunkSize) : chunkSize(chunkSize) {}
	public:
		virtual bool read(uint8_t*buffer, int64_t seek, int count) = 0;
		virtual bool   seek(int64_t offset) = 0;
		virtual uint16_t decode(uint8_t*buffer) = 0;
		virtual uint16_t getEndian() = 0;
		virtual uint32_t getRate() = 0;
		virtual uint8_t  getChannels() = 0;
		virtual int64_t getLength() = 0;
		virtual ~TrackFile() { };
		virtual void setAudioPosition(Bit32u pos) = 0;
		const Bit16u   chunkSize = 0;
		Bit32u audio_pos = UINT32_MAX; // last position when playing audio
	};
	
	class BinaryFile : public TrackFile {
	public:
		BinaryFile(const char *filename, bool &error);
		~BinaryFile();
		bool read(uint8_t*buffer, int64_t seek, int count);
		bool   seek(int64_t offset);
		uint16_t decode(uint8_t*buffer);
		uint16_t getEndian();
		uint32_t getRate() { return 44100; }
		uint8_t  getChannels() { return 2; }
		int64_t getLength();
		void setAudioPosition(Bit32u pos) { audio_pos = pos; }
	private:
		BinaryFile();
		std::ifstream *file;
	};
	

	class AudioFile : public TrackFile {
	public:
		AudioFile(const char *filename, bool &error);
		~AudioFile();
		bool   read(uint8_t*buffer, int64_t seek, int count) { return false; }
		bool   seek(int64_t offset);
		Bit16u decode(uint8_t*buffer);
		Bit16u getEndian();
		uint32_t getRate();
		uint8_t  getChannels();
		int64_t getLength();
		void setAudioPosition(Bit32u pos) { (void)pos;/*unused*/ }
	private:
		AudioFile();
		Sound_Sample *sample;
	};

	class CHDFile : public TrackFile {
	public:
		CHDFile(const char* filename, bool &error);
		~CHDFile();

		CHDFile() = delete;
		CHDFile(const CHDFile&) = delete;
		CHDFile& operator= (const CHDFile&) = delete;

		bool            read(uint8_t*buffer, int64_t seek, int count);
		bool            seek(int64_t offset);
		uint16_t        decode(uint8_t* buffer);
		uint16_t        getEndian();
		uint32_t        getRate() { return 44100; }
		uint8_t         getChannels() { return 2; }
		int64_t         getLength();
		void setAudioPosition(uint32_t pos) { audio_pos = pos; }
		
		chd_file* getChd() { return this->chd; }
	private:
		std::ifstream *file;
		chd_file* chd 				= nullptr;
		const chd_header* header 	= nullptr; // chd header
				/*
					TODO: cache more than one hunk
					or wait for https://github.com/rtissera/libchdr/issues/36
				*/
              Bit8u*     hunk_buffer       = nullptr; // buffer to hold one hunk // size of hunks in CHD up to 1 MiB
              Bit8u*     hunk_buffer_next  = nullptr; // index + 1 prefetch
              int          hunk_buffer_index = -1;      // hunk index for buffer
    public:
              bool         skip_sync         = false;   // this will fail if a CHD contains 2048 and 2352 sector tracks
     };
	
public:
	struct Track {
		int number;
		int attr;
		int start;
		int length;
		int skip;
		int sectorSize;	
		bool mode2;		
		TrackFile *file;
	};
	
	struct TrackCHDCount {
		int Tracks;
		int MetaVersion;
		chd_error Error;
	}ChdInfoCount;

	struct ImageAudioTrack {
		int sectorsize;
		int attr;
		int offset;
		int length;
		bool mode2;
		bool used;		
	}ImageTrack;
	
	CDROM_Interface_Image		(Bit8u subUnit);
	virtual ~CDROM_Interface_Image	(void);
	void	InitNewMedia		(void);
	bool	SetDevice		(char* path, int forceCD);
	bool	GetUPC			(unsigned char& attr, char* upc);
	bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	bool	GetAudioSub		(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	bool	GetAudioStatus		(bool& playing, bool& pause);
	bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);
	bool	PlayAudioSector		(unsigned long start,unsigned long len);
	bool	PauseAudio		(bool resume);
	bool	StopAudio		(void);
	void	ChannelControl		(TCtrl ctrl);
	bool	ReadSectors		(PhysPt buffer, bool raw, unsigned long sector, unsigned long num);
	bool	LoadUnloadMedia		(bool unload);
	bool	ReadSector		(Bit8u *buffer, bool raw, unsigned long sector);
	/* This is needed for IDE hack, who's buffer does not exist in DOS physical memory */
	bool	ReadSectorsHost			(void* buffer, bool raw, unsigned long sector, unsigned long num);	
	bool	HasDataTrack		(void);
	
static	CDROM_Interface_Image* images[26];

private:
	// player
static	void	CDAudioCallBack(Bitu len);
	int	GetTrack(int sector);

static  struct imagePlayer {
		CDROM_Interface_Image *cd;
		MixerChannel   *channel;

		Bit8u   buffer[AUDIO_DECODE_BUFFER_SIZE];
		Bit32u  startFrame;
		Bit32u  currFrame;
		Bit32u  numFrames;
		bool    isPlaying;
		bool    isPaused;
		bool    ctrlUsed;
		TCtrl   ctrlData;
		TrackFile* trackFile;
		void     (MixerChannel::*addSamples) (Bitu, const Bit16s*);
		Bit32u   playbackTotal;
		int      playbackRemaining;
		Bit16u   bufferPos;
		Bit16u   bufferConsumed;
	} player;
	
	void 	ClearTracks();
	bool	LoadIsoFile(char *filename);
	
	bool	CanReadPVD(TrackFile *file, int sectorSize, bool mode2);
	// cue sheet processing
	bool	LoadCueSheet(char *cuefile);
	bool  LoadChdFile(char* chdfile);
	bool  LoadChdFileNew(chd_file* chd, CHDFile* file);
	int     CHD_PreLoadFile(char* chdfile);
	bool	GetRealFileName(std::string& filename, std::string& pathname);
	bool	GetCueKeyword(std::string &keyword, std::istream &in);
	bool	GetCueFrame(int &frames, std::istream &in);
	bool	GetCueString(std::string &str, std::istream &in);
	// bool	AddTrack(Track &curr, int &shift, int prestart, int &totalPregap, int currPregap, int frameFromCue);
	bool	AddTrack(Track &curr, int &shift, int prestart, int &totalPregap, int currPregap);

static	int	refCount;
	std::vector<Track>	tracks;
typedef	std::vector<Track>::iterator	track_it;
	std::string	mcn;
	Bit8u	subUnit;
};

#endif /* __CDROM_INTERFACE__ */
