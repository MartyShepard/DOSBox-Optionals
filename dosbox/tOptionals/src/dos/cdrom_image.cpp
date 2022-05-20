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

// #define DEBUG 1
#ifdef DEBUG
#include <time.h>
#include <chrono>
#endif

#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <limits.h> //GCC 2.95
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <assert.h>
#include "cdrom.h"
#include "drives.h"
#include "support.h"
#include "setup.h"
#include "shell.h"

#if defined(_MSC_VER)
	#include "../src/libs/libchdr/chd.h"
	#include "../src/libs/libchdr/libchdr_chd.c"
	#include "../src/libs/libchdr/libchdr_cdrom.c"
	#include "../src/libs/libchdr/libchdr_flac.c"
	#include "../src/libs/libchdr/libchdr_huffman.c"
	#include "../src/libs/libchdr/libchdr_bitstream.c"
	#include "../src/libs/libchdr/flac/stream_decoder.c"
	#include "../src/libs/libchdr/flac/bitreader.c"
	#include "../src/libs/libchdr/flac/format.c"
	#include "../src/libs/libchdr/flac/cpu.c"
	#include "../src/libs/libchdr/flac/crc.c"
	#include "../src/libs/libchdr/flac/fixed.c"
	#include "../src/libs/libchdr/flac/lpc.c"
	#include "../src/libs/libchdr/flac/md5.c"
	#include "../src/libs/libchdr/flac/memory.c"
	#if defined(WIN32)
	#include "../src/libs/libchdr/flac/windows_unicode_filenames.c"
	#endif
	#include "../src/libs/libchdr/lzma/LzmaDec.c"
	#include "../src/libs/libchdr/lzma/LzmaEnc.c"
	#include "../src/libs/libchdr/lzma/LzFind.c"
#else
	#include "src/libs/libchdr/chd.h"
	#include "src/libs/libchdr/libchdr_chd.c"
	#include "src/libs/libchdr/libchdr_cdrom.c"
	#include "src/libs/libchdr/libchdr_flac.c"
	#include "src/libs/libchdr/libchdr_huffman.c"
	#include "src/libs/libchdr/libchdr_bitstream.c"
	#include "src/libs/libchdr/flac/stream_decoder.c"
	#include "src/libs/libchdr/flac/bitreader.c"
	#include "src/libs/libchdr/flac/format.c"
	#include "src/libs/libchdr/flac/cpu.c"
	#include "src/libs/libchdr/flac/crc.c"
	#include "src/libs/libchdr/flac/fixed.c"
	#include "src/libs/libchdr/flac/lpc.c"
	#include "src/libs/libchdr/flac/md5.c"
	#include "src/libs/libchdr/flac/memory.c"
	#if defined(WIN32)
	#include "src/libs/libchdr/flac/windows_unicode_filenames.c"
	#endif
	#include "src/libs/libchdr/lzma/LzmaDec.c"
	#include "src/libs/libchdr/lzma/LzmaEnc.c"
	#include "src/libs/libchdr/lzma/LzFind.c"
#endif
#if !defined(WIN32)
#include <libgen.h>
#else
#include <string.h>
#endif

#if defined(WORDS_BIGENDIAN)
#define IS_BIGENDIAN true
#else
#define IS_BIGENDIAN false
#endif

using namespace std;

#define MAX_LINE_LENGTH 512
#define MAX_FILENAME_LENGTH 256

static char  empty_char = 0;
static char* empty_strg = &empty_char;

//#define CDROM_DEBUG;

#if defined (C_DEBUG) && defined (CDROM_DEBUG)
char* get_time() {
	static time_t rawtime;
	struct tm* ptime;
    static char time_str[] = "00:00:00";

	time(&rawtime);
    ptime = localtime(&rawtime);
    sprintf(time_str, "%02d:%02d:%02d", ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
	return time_str;
}
#endif


	struct ImageTrack *ofAudioTrack ;

CDROM_Interface_Image::BinaryFile::BinaryFile(const char *filename, bool &error)
	:TrackFile(RAW_SECTOR_SIZE)
{
	file = new ifstream(filename, ios::in | ios::binary);
	error = (file == NULL) || (file->fail());
}

CDROM_Interface_Image::BinaryFile::~BinaryFile()
{
	delete file;
	file = NULL;
}

bool CDROM_Interface_Image::BinaryFile::read(uint8_t*buffer, int64_t seek, int count)
{
	file->seekg(seek, ios::beg);
	file->read((char*)buffer, count);
	return !(file->fail());
}

int64_t CDROM_Interface_Image::BinaryFile::getLength()
{
	file->seekg(0, ios::end);
	int length = (int)file->tellg();
	if (file->fail()) return -1;
	return length;
}

Bit16u CDROM_Interface_Image::BinaryFile::getEndian()
{
	// Image files are read into native-endian byte-order
	#if defined(WORDS_BIGENDIAN)
	return AUDIO_S16MSB;
	#else
	return AUDIO_S16LSB;
	#endif
}


bool CDROM_Interface_Image::BinaryFile::seek(int64_t offset)
{
	file->seekg(offset, ios::beg);
	return !file->fail();
}

Bit16u CDROM_Interface_Image::BinaryFile::decode(uint8_t*buffer)
{
	file->read((char*)buffer, chunkSize);
	return file->gcount();
}

CDROM_Interface_Image::AudioFile::AudioFile(const char *filename, bool &error)
	:TrackFile(4096)
{
	// Use the audio file's actual sample rate and number of channels as opposed to overriding
	Sound_AudioInfo desired = {AUDIO_S16, 0, 0};
	sample = Sound_NewSampleFromFile(filename, &desired, chunkSize);
	if (sample) {
		error = false;
		std::string filename_only(filename);
		filename_only = filename_only.substr(filename_only.find_last_of("\\/") + 1);
		LOG_MSG("CDROM: Loaded %s [%d Hz %d-channel]", filename_only.c_str(), this->getRate(), this->getChannels());
	} else
		error = true;
}

/**
 *  Seek takes in a Redbook CD-DA byte offset relative to the track's start
 *  time and returns true if the seek succeeded.
 * 
 *  When dealing with a raw bin/cue file, this requested byte position maps
 *  one-to-one with the bytes in raw binary image, as we see used in the
 *  BinaryTrack::seek() function.  However, when dealing with codec-based
 *  tracks, we need the codec's help to seek to the equivalent redbook position
 *  within the track, regardless of the track's sampling rate, bit-depth,
 *  or number of channels.  To do this, we convert the byte offset to a
 *  time-offset, and use the Sound_Seek() function to move the read position.
 */
CDROM_Interface_Image::AudioFile::~AudioFile()
{
	Sound_FreeSample(sample);
}

bool CDROM_Interface_Image::AudioFile::seek(int64_t offset)
{
	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
		const auto begin = std::chrono::steady_clock::now();
	#endif

	if (audio_pos == (uint32_t)offset) {
	
	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
		LOG_MSG("CDROM: seek to %u avoided with position-tracking", offset);
	#endif
		return true;
    }
	
	// Convert the byte-offset to a time offset (milliseconds)
	const bool result = Sound_Seek(sample, lround(offset/176.4f));

	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
	const auto end = std::chrono::steady_clock::now();
	LOG_MSG("%s CDROM: seek(%u) took %f ms", get_time(), offset, chrono::duration <double, milli> (end - begin).count());
	#endif

	return result;
}

Bit16u CDROM_Interface_Image::AudioFile::decode(uint8_t*buffer)
{
	const uint16_t bytes = Sound_Decode(sample);
	memcpy(buffer, sample->buffer, bytes);
	return bytes;
}

Bit16u CDROM_Interface_Image::AudioFile::getEndian()
{
	return sample->actual.format;
}

Bit32u CDROM_Interface_Image::AudioFile::getRate()
{
	Bit32u rate(0);
	if (sample) {
		rate = sample->actual.rate;
	}
	return rate;
}

Bit8u CDROM_Interface_Image::AudioFile::getChannels()
{
	Bit8u channels(0);
	if (sample) {
		channels = sample->actual.channels;
	}
	return channels;
}

int64_t CDROM_Interface_Image::AudioFile::getLength()
{
	int length(-1);

	// GetDuration returns milliseconds ... but getLength needs Red Book bytes.
	const int duration_ms = Sound_GetDuration(sample);
	if (duration_ms > 0) {
		// ... so convert ms to "Red Book bytes" by multiplying with 176.4f,
		// which is 44,100 samples/second * 2-channels * 2 bytes/sample
		// / 1000 milliseconds/second
		length = round(duration_ms * 176.4f);
	}
	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
		LOG_MSG("%s CDROM: AudioFile::getLength is %d bytes", get_time(), length);
    #endif

	return length;
}

CDROM_Interface_Image::CHDFile::CHDFile(const char* filename, bool& error)
    :TrackFile(RAW_SECTOR_SIZE) // CDAudioCallBack needs 2352
{
    error = chd_open(filename, CHD_OPEN_READ, NULL, &this->chd) != CHDERR_NONE;
	
    if (!error)
	{
        this->header      		= chd_get_header(this->chd);
        this->hunk_buffer 		= new uint8_t[this->header->hunkbytes];
		this->hunk_buffer_next 	= new uint8_t[this->header->hunkbytes];
    }
}

CDROM_Interface_Image::CHDFile::~CHDFile()
{
    // Guard: only cleanup if needed
    if (this->chd)
	{
        chd_close(this->chd);
        this->chd = nullptr;
    }

    if (this->hunk_buffer)
	{
        delete[] this->hunk_buffer;
        this->hunk_buffer = nullptr;
    }

}


bool CDROM_Interface_Image::CHDFile::read(uint8_t* buffer, int64_t offset, int count)
{
    // we can not read more than a single sector currently
    if (count > RAW_SECTOR_SIZE) {
        return false;
    }

	uint64_t needed_hunk = (uint64_t)offset / (uint64_t)this->header->hunkbytes;
	
    // EOF
    if ((needed_hunk) > this->header->totalhunks)
	{
        return false;
    }

    // read new hunk if needed
    if (needed_hunk != this->hunk_buffer_index) {
      
		if (chd_read(this->chd, needed_hunk, this->hunk_buffer) != CHDERR_NONE)
		{
            return false;
		}
		
        this->hunk_buffer_index = needed_hunk;
				
    }

    // copy data
    // the overlying read code thinks there is a sync header    
	// so for 2048 sector size images we need to subtract 16 from the offset to account for the missing sync header
    uint8_t* source = this->hunk_buffer + ((uint64_t)offset - (uint64_t)needed_hunk * this->header->hunkbytes) - ((uint64_t)16 * this->skip_sync);
    memcpy(buffer, source, min(count, RAW_SECTOR_SIZE));

    return true;
}

int64_t CDROM_Interface_Image::CHDFile::getLength()
{
	int64_t length(-1);

	// GetDuration returns milliseconds ... but getLength needs Red Book bytes.
	const int64_t duration_ms = this->header->logicalbytes;

	if (duration_ms > 0) {
		// ... so convert ms to "Red Book bytes" by multiplying with 176.4f,
		// which is 44,100 samples/second * 2-channels * 2 bytes/sample
		// / 1000 milliseconds/second
		length = round(duration_ms * 176.4f);
	}

	return length;

}

uint16_t CDROM_Interface_Image::CHDFile::getEndian()
{
	// CHD: no idea about this, chaning this did not fix the cd audio noise
	// Image files are read into native-endian byte-order
#if defined(WORDS_BIGENDIAN)
	return AUDIO_S16MSB;
#else
	return AUDIO_S16LSB;
#endif
}

bool CDROM_Interface_Image::CHDFile::seek(int64_t offset)
{
	// only checks if seek range is valid ? only used for audio ?
	// only used by PlayAudioSector ?
	#ifdef DEBUG
		LOG_MSG("CHD: Seek %d",offset);
    #endif		
		if ((uint32_t)((uint64_t)offset / this->header->hunkbytes) < this->header->hunkcount) {
			return true;
		}
		else {
			return false;
		}
}

static void Endian_A16_Swap(void* src, uint32_t nelements)
{
	uint32_t i;
	uint8_t* nsrc = (uint8_t*)src;

	for (i = 0; i < nelements; i++)
	{
		uint8_t tmp = nsrc[i * 2];

		nsrc[i * 2] = nsrc[i * 2 + 1];
		nsrc[i * 2 + 1] = tmp;
	}
}

uint16_t CDROM_Interface_Image::CHDFile::decode(uint8_t* buffer)
{
    // reads one sector of CD audio ?

    assert(this->audio_pos % 2448 == 0);

    if (this->read(buffer, this->audio_pos, RAW_SECTOR_SIZE)) {
								
        // chd uses 2448
        this->audio_pos += 2448;

        // no idea if other platforms need this but on windows this is needed or there is only noise
        Endian_A16_Swap(buffer, 588 * 2);
		
		#ifdef DEBUG		
			LOG_MSG("CHD: Decode %d (Audio Pos), %d (Buffer) %d",audio_pos, buffer, RAW_SECTOR_SIZE);
		#endif
        // we only read the raw audio nothing else
        return RAW_SECTOR_SIZE;
    }
	//SDL_Delay(1);
    return 0;
}

// initialize static members
int CDROM_Interface_Image::refCount = 0;
CDROM_Interface_Image* CDROM_Interface_Image::images[26] = {};
CDROM_Interface_Image::imagePlayer CDROM_Interface_Image::player;
	
CDROM_Interface_Image::CDROM_Interface_Image(Bit8u subUnit)
                      :subUnit(subUnit)
{
	images[subUnit] = this;
	if (refCount == 0) {
		if (player.channel == NULL) {
			// channel is kept dormant except during cdrom playback periods
			player.channel = MIXER_AddChannel(&CDAudioCallBack, 0, "CDAUDIO");
			player.channel->Enable(false);
			
			#ifdef DEBUG
				LOG_MSG("CDROM: Initialized with %d-byte circular buffer", AUDIO_DECODE_BUFFER_SIZE);
			#endif
		}
	}
	refCount++;
}

CDROM_Interface_Image::~CDROM_Interface_Image()
{
	refCount--;
	if (player.cd == this) player.cd = NULL;
	ClearTracks();
	// Stop playback before wiping out the CD Player
	if (refCount == 0) {
		StopAudio();
		MIXER_DelChannel(player.channel);
		player.channel = NULL;
		LOG_MSG("CDROM: Audio channel freed");
	}
}

void CDROM_Interface_Image::InitNewMedia(){}

bool CDROM_Interface_Image::SetDevice(char* path, int /*forceCD*/)
{
	const bool result = LoadCueSheet(path) || LoadIsoFile(path) || LoadChdFile(path);
	if 	( !result )
	{
		
		/* print error message on dosbox console */
		char buf[MAX_LINE_LENGTH];
		snprintf(buf, MAX_LINE_LENGTH, "\nCould not load image file:\n\r%s\n\r\n", path);
		Bit16u size = (Bit16u)strlen(buf);
		DOS_WriteFile(STDOUT, (Bit8u*)buf, &size);
	
		/* Add a Pause */
		Bit8u  c;
		Bit16u n = 1;
		DOS_ReadFile(STDIN, &c, &n);
		if (c==0)
		{
			DOS_ReadFile(STDIN, &c, &n); // read extended key	
		}
	}
	return result;	
}

bool CDROM_Interface_Image::GetUPC(unsigned char& attr, char* upc)
{
	attr = 0;
	strcpy(upc, this->mcn.c_str());
    #ifdef DEBUG
		LOG_MSG("%s CDROM: GetUPC=%s", get_time(), upc);
	#endif	



	return true;
}

bool CDROM_Interface_Image::GetAudioTracks(int& stTrack, int& end, TMSF& leadOut)
{
	stTrack = 1;
	end = (int)(tracks.size() - 1);
	FRAMES_TO_MSF(tracks[tracks.size() - 1].start + 150, &leadOut.min, &leadOut.sec, &leadOut.fr);
	//FRAMES_TO_MSF(tracks[tracks.size() - 1].start, &leadOut.min, &leadOut.sec, &leadOut.fr);
	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
	LOG_MSG("%s CDROM: GetAudioTracks, stTrack=%d, end=%d, leadOut.min=%d, leadOut.sec=%d, leadOut.fr=%d",
      get_time(),
	  stTrack,
	  end,
	  leadOut.min,
	  leadOut.sec,
	  leadOut.fr);
	#endif

	return true;
}

bool CDROM_Interface_Image::GetAudioTrackInfo(int track, TMSF& start, unsigned char& attr)
{
	if (track < 1 || track > (int)tracks.size()) return false;
	FRAMES_TO_MSF(tracks[track - 1].start + 150, &start.min, &start.sec, &start.fr);
	attr = tracks[track - 1].attr;

	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
	LOG_MSG("%s CDROM: GetAudioTrackInfo track=%d MSF %02d:%02d:%02d, attr=%u",
	  get_time(),
	  track,
	  start.min,
	  start.sec,
	  start.fr,
	  attr
	);
	#endif
	
	return true;
}

bool CDROM_Interface_Image::GetAudioSub(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos)
{
	int cur_track = GetTrack(player.currFrame);
	if (cur_track < 1) return false;
	track = (unsigned char)cur_track;
	attr = tracks[track - 1].attr;
	index = 1;
	FRAMES_TO_MSF(player.currFrame + 150, &absPos.min, &absPos.sec, &absPos.fr);
	FRAMES_TO_MSF(player.currFrame - tracks[track - 1].start +150, &relPos.min, &relPos.sec, &relPos.fr);
	
    #ifdef DEBUG
	LOG_MSG("%s CDROM: GetAudioSub attr=%u, track=%u, index=%u", get_time(), attr, track, index);

	LOG_MSG("%s CDROM: GetAudioSub absoute  offset (%d), MSF=%d:%d:%d",
      get_time(),
	  player.currFrame + 150,
	  absPos.min,
	  absPos.sec,
	  absPos.fr);
	LOG_MSG("%s CDROM: GetAudioSub relative offset (%d), MSF=%d:%d:%d",
      get_time(),
	  player.currFrame - tracks[track - 1].start + 150,
	  relPos.min,
	  relPos.sec,
	  relPos.fr);
	  */
	#endif
	
	return true;
}

bool CDROM_Interface_Image::GetAudioStatus(bool& playing, bool& pause)
{
	playing = player.isPlaying;
	pause = player.isPaused;

	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
		LOG_MSG("CDROM: GetAudioStatus playing=%d, paused=%d", get_time(), playing, pause);
	#endif	

	return true;
}

bool CDROM_Interface_Image::GetMediaTrayStatus(bool& mediaPresent, bool& mediaChanged, bool& trayOpen)
{
	mediaPresent = true;
	mediaChanged = false;
	trayOpen = false;

	#ifdef DEBUG
	LOG_MSG("%s CDROM: GetMediaTrayStatus present=%d, changed=%d, open=%d", get_time(), mediaPresent, mediaChanged, trayOpen);
	#endif

	return true;
}

bool CDROM_Interface_Image::PlayAudioSector(unsigned long start, unsigned long len)
{
	bool is_playable(false);
	const int track = GetTrack(start) - 1;

	// The CDROM Red Book standard allows up to 99 tracks, which includes the data track
	if ( track < 0 || track > 99 )
	{
		LOG_MSG("Game tried to load track #%d, which is invalid", track);
	}

	// Attempting to play zero sectors is a no-op
	else if (len == 0)
	{
		LOG_MSG("Game tried to play zero sectors, skipping");
	}

	// The maximum storage achieved on a CDROM was ~900MB or just under 100 minutes
	// with overburning, so use this threshold to sanity-check the start sector.
	else if (start > 450000)
	{
		LOG_MSG("Game tried to read sector %lu, which is beyond the 100-minute maximum of a CDROM", start);
	}

	// We can't play audio from a data track (as it would result in garbage/static)
	else if(track >= 0 && tracks[track].attr == 0x40)
	{
		LOG_MSG("Game tries to play the data track. Not doing this");

	}

	// Checks passed, setup the audio stream
	else {
		TrackFile* trackFile = tracks[track].file;

		// Convert the playback start sector to a time offset (milliseconds) relative to the track
		const Bit32u offset = tracks[track].skip + (start - tracks[track].start) * tracks[track].sectorSize;
		is_playable = trackFile->seek(offset);

		// only initialize the player elements if our track is playable
		if (is_playable)
		{
			trackFile->setAudioPosition(offset);
			const Bit8u channels = trackFile->getChannels();
			const Bit32u rate = trackFile->getRate();

			player.cd = this;
			//player.bufLen = 0;
			player.trackFile = trackFile;
			player.startFrame = start;
			player.currFrame = start;
			player.numFrames = len;
			player.bufferPos = 0;
			player.bufferConsumed = 0;
			player.isPlaying = true;
			player.isPaused = false;

			if ( (!IS_BIGENDIAN && trackFile->getEndian() == AUDIO_S16SYS) || ( IS_BIGENDIAN && trackFile->getEndian() != AUDIO_S16SYS) )
			{
				player.addSamples = channels ==  2  ? &MixerChannel::AddSamples_s16 \
													: &MixerChannel::AddSamples_m16;
			 }
			else
			{
				player.addSamples = channels ==  2  ? &MixerChannel::AddSamples_s16_nonnative \
				                                    : &MixerChannel::AddSamples_m16_nonnative;
			}
			
			const float bytesPerMs = (float)(rate * channels * 2 / 1000.0);
			/*
			* CHD vs Cue
			* PC Audio Tracks always in 2352 SectorSize, Cue/Bin Files played ok
			* chd comes in sectorsize 2448, each Track play and end with ~2 seconds more.
			* With 2 seconds more it play the nextrack and stopped at the begin and jump
			* to the old track
			*/
			int FixSectorSize = tracks[track].sectorSize;

			if (tracks[track].sectorSize == 2448)
				FixSectorSize = 2352;

			player.playbackTotal = lround(len * FixSectorSize/*tracks[track].sectorSize*/ * bytesPerMs / 176.4);
			player.playbackRemaining = player.playbackTotal;

			#ifdef DEBUG
			LOG_MSG(
			   "%s CDROM: Playing track %d at %.1f KHz %d-channel at start sector %lu (%.1f minute-mark), seek %u (skip=%d,dstart=%d,secsize=%d), for %lu sectors (%.1f seconds)",
			   get_time(),
			   track,
			   rate/1000.0,
			   channels,
			   start,
			   offset * (1/10584000.0),
			   offset,
			   tracks[track].skip,
			   tracks[track].start,
			   FixSectorSize/*tracks[track].sectorSize*/,
			   len,
			   player.playbackRemaining / (1000 * bytesPerMs)
			);
			#endif

			// start the channel!
			player.channel->SetFreq(rate);
			player.channel->Enable(true);
		}
	}
	if (!is_playable) StopAudio();
	return is_playable;
}

bool CDROM_Interface_Image::PauseAudio(bool resume)
{
	player.isPaused = !resume;
	if (player.channel)
	{
		player.channel->Enable(resume);
	}
	#ifdef DEBUG
	LOG_MSG("%s CDROM: PauseAudio, state=%s", get_time(), resume ? "resumed" : "paused");
	#endif	
	return true;
}

bool CDROM_Interface_Image::StopAudio(void)
{
	// Only switch states if needed
		player.isPlaying = false;
		player.isPaused = false;
	
	if (player.channel){
		player.channel->Enable(false);
	}	
	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
		LOG_MSG("%s CDROM: StopAudio", get_time());
	#endif
	return true;
}

void CDROM_Interface_Image::ChannelControl(TCtrl ctrl)
{
	// Guard: Bail if our mixer channel hasn't been allocated
	if (!player.channel)
		return;

	player.ctrlUsed = (ctrl.out[0]!=0 || ctrl.out[1]!=1 || ctrl.vol[0]<0xfe || ctrl.vol[1]<0xfe);
	player.ctrlData = ctrl;

	// Adjust the volume of our mixer channel as defined by the application
	player.channel->SetScale(static_cast<float>(ctrl.vol[0]),  // left vol
	                         static_cast<float>(ctrl.vol[1])); // right vol
}

bool CDROM_Interface_Image::ReadSectors(PhysPt buffer, bool raw, unsigned long sector, unsigned long num)
{
	int sectorSize = raw ? RAW_SECTOR_SIZE : COOKED_SECTOR_SIZE;
	Bitu buflen = num * sectorSize;
	Bit8u* buf = new Bit8u[buflen];
	
	bool success = true; //Gobliiins reads 0 sectors
	for(unsigned long i = 0; i < num; i++) {
		success = ReadSector(&buf[i * sectorSize], raw, sector + i);
		if (!success) break;
	}

	MEM_BlockWrite(buffer, buf, buflen);
	delete[] buf;
	return success;
}

bool CDROM_Interface_Image::ReadSectorsHost(void *buffer, bool raw, unsigned long sector, unsigned long num)
{
	int sectorSize = raw ? RAW_SECTOR_SIZE : COOKED_SECTOR_SIZE;
	//Bitu buflen = num * sectorSize;     
	
	bool success = true; //Gobliiins reads 0 sectors
	for(unsigned long i = 0; i < num; i++) {
		success = ReadSector((Bit8u*)buffer + (i * sectorSize), raw, sector + i);
		if (!success) break;
	}
	
	return success;
}

bool CDROM_Interface_Image::LoadUnloadMedia(bool /*unload*/)
{
	return true;
}

int CDROM_Interface_Image::GetTrack(int sector)
{
	vector<Track>::iterator i = tracks.begin();
	vector<Track>::iterator end = tracks.end() - 1;
	
	while(i != end) {
		Track &curr = *i;
		Track &next = *(i + 1);
		if (curr.start <= sector && sector < next.start) return curr.number;
		i++;
	}
	return -1;
}

bool CDROM_Interface_Image::ReadSector(uint8_t* buffer, bool raw, unsigned long sector)
{
	int track = GetTrack(sector) - 1;
	if (track < 0) return false;

	int64_t seek = (int64_t)tracks[track].skip + ((int64_t)(sector - tracks[track].start)) * (int64_t)tracks[track].sectorSize;
	int length = (raw ? RAW_SECTOR_SIZE : COOKED_SECTOR_SIZE);
	if (tracks[track].sectorSize != RAW_SECTOR_SIZE && raw) return false;
	if ((tracks[track].sectorSize == RAW_SECTOR_SIZE || tracks[track].sectorSize == 2448) && !tracks[track].mode2 && !raw) seek += 16;
	if (tracks[track].mode2 && !raw) seek += 24;

	// LOG_MSG("CDROM: ReadSector track=%d, desired raw=%s, sector=%ld, length=%d", track, raw ? "true":"false", sector, length);
	return tracks[track].file->read(buffer, seek, length);
}

void printProgress(double percentage, const char* msg)
{
	// 60 is the number of characters in the full progress bar
	int val  = (int)(percentage * 100);
	int lpad = (int)(percentage * 60);
	int rpad = 60 - lpad;
	#ifdef DEBUG
	LOG_MSG("\r%3d%% [%.*s%*s] - %s", val, lpad,  "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||", rpad, "", msg);
	#endif
	fflush(stdout);
}

void CDROM_Interface_Image::CDAudioCallBack(Bitu len)
{
	// Our member object "playbackRemaining" holds the
	// exact number of stream-bytes we need to play before meeting the
	// DOS program's desired playback duration in sectors. We simply
	// decrement this counter each callback until we're done.
	if (len == 0 || !player.isPlaying || player.isPaused) return;


	// determine bytes per request (16-bit samples)
	const Bit8u channels = player.trackFile->getChannels();
	const Bit8u bytes_per_request = channels * 2;
	Bit16u total_requested = len * bytes_per_request;

	while (total_requested > 0) {
		Bit16u requested = total_requested;

		// Every now and then the callback wants a big number of bytes,
		// which can exceed our circular buffer. In these cases we need
		// read through as many iteration of our circular buffer as needed.
		if (total_requested > AUDIO_DECODE_BUFFER_SIZE) {
			requested = AUDIO_DECODE_BUFFER_SIZE;
			total_requested -= AUDIO_DECODE_BUFFER_SIZE;
		}
		else {
			total_requested = 0;
		}

		// Three scenarios in order of probabilty:
		//
		// 1. Consume: If our decoded circular buffer is sufficiently filled to
		//             satify the requested size, then feed the callback with
		//             the requested number of bytes.
		//
		// 2. Wrap:    If we've decoded and consumed to edge of our buffer, then
		//             we need to wrap any remaining decoded-but-not-consumed
		//             samples back around to the front of the buffer.
		//
		// 3. Fill:    When out circular buffer is too depleted to satisfy the
		//             requested size, then perform chunked-decode reads from
		//             the audio-codec to either fill our buffer or satify our
		//             remaining playback - whichever is smaller.
		//
		while (true) {

			// 1. Consume
			// ==========
			if (player.bufferPos - player.bufferConsumed >= requested) {
				if (player.ctrlUsed) {
					for (Bit8u i=0; i < channels; i++) {
						Bit16s  sample;
						Bit16s* samples = (Bit16s*)&player.buffer[player.bufferConsumed];
						for (Bitu pos = 0; pos < requested / bytes_per_request; pos++) {
							#if defined(WORDS_BIGENDIAN)
							sample = (Bit16s)host_readw((HostPt) & samples[pos * 2 + player.ctrlData.out[i]]);
							#else
							sample = samples[pos * 2 + player.ctrlData.out[i]];
							#endif
							samples[pos * 2 + i] = (Bit16s)(sample * player.ctrlData.vol[i] / 255.0);
						}
					}
				}
				// uses either the stereo or mono and native or nonnative AddSamples call assigned during construction
				(player.channel->*player.addSamples)(requested / bytes_per_request, (Bit16s*)(player.buffer + player.bufferConsumed) );
				player.bufferConsumed += requested;
				player.playbackRemaining -= requested;

				// Games can query the current Red Book MSF frame-position, so we keep that up-to-date here.
				// We scale the final number of frames by the percent complete, which
				// avoids having to keep track of the euivlent number of Red Book frames
				// read (which would involve coverting the compressed streams data-rate into
				// CDROM Red Book rate, which is more work than simply scaling).
				//
				const float playbackPercentSoFar = static_cast<float>(player.playbackTotal - player.playbackRemaining) / player.playbackTotal;
				player.currFrame = player.startFrame + ceil(player.numFrames * playbackPercentSoFar);
				break;
				printProgress( (player.bufferPos - player.bufferConsumed)/(float)AUDIO_DECODE_BUFFER_SIZE, "consume");
			}

			// 2. Wrap
			// =======
			else {
				memcpy(player.buffer,
					   player.buffer + player.bufferConsumed,
					   player.bufferPos - player.bufferConsumed);
				player.bufferPos -= player.bufferConsumed;
				player.bufferConsumed = 0;
				printProgress( (player.bufferPos - player.bufferConsumed)/(float)AUDIO_DECODE_BUFFER_SIZE, "wrap");
			}

			// 3. Fill
			// =======
			const Bit16u chunkSize = player.trackFile->chunkSize;
			while(AUDIO_DECODE_BUFFER_SIZE - player.bufferPos >= chunkSize &&
			      (player.bufferPos - player.bufferConsumed < player.playbackRemaining ||
				   player.bufferPos - player.bufferConsumed < requested) ) {

				const Bit16u decoded = player.trackFile->decode(player.buffer + player.bufferPos);
				player.bufferPos += decoded;

				// if we decoded less than expected, which could be due to EOF or if the CUE file specified
				// an exact "INDEX 01 MIN:SEC:FRAMES" value but the compressed track is ever-so-slightly less than
				// that specified, then simply pad with zeros.
				const Bit16s underDecode = chunkSize - decoded;
				if (underDecode > 0) {

					#if defined (C_DEBUG) && defined (CDROM_DEBUG)
						LOG_MSG("%s CDROM: Underdecoded by %d. Feeding mixer with zeros.", get_time(), underDecode);
                    #endif

					memset(player.buffer + player.bufferPos, 0, underDecode);
					player.bufferPos += underDecode;
				}
				printProgress( (player.bufferPos - player.bufferConsumed)/(float)AUDIO_DECODE_BUFFER_SIZE, "fill");
			} // end of fill-while
		} // end of decode and fill loop
	} // end while total_requested

	if (player.playbackRemaining <= 0) {
		player.cd->StopAudio();
		printProgress( (player.bufferPos - player.bufferConsumed)/(float)AUDIO_DECODE_BUFFER_SIZE, "stop");
	}
}

//////////////////////////////////////////////////////////////////////////////

bool CDROM_Interface_Image::LoadIsoFile(char* filename)
{
	tracks.clear();
	
	// data track
	Track track = {0, 0, 0, 0, 0, 0, false, NULL};
	bool error;
	track.file = new BinaryFile(filename, error);
	if (error) {
		delete track.file;
		track.file = NULL;
		return false;
	}
	track.number = 1;
	track.attr = 0x40;//data
	
	// try to detect iso type
	if (CanReadPVD(track.file, COOKED_SECTOR_SIZE, false)) {
		track.sectorSize = COOKED_SECTOR_SIZE;
		track.mode2 = false;
	} else if (CanReadPVD(track.file, RAW_SECTOR_SIZE, false)) {
		track.sectorSize = RAW_SECTOR_SIZE;
		track.mode2 = false;		
	} else if (CanReadPVD(track.file, 2336, true)) {
		track.sectorSize = 2336;
		track.mode2 = true;		
	} else if (CanReadPVD(track.file, RAW_SECTOR_SIZE, true)) {
		track.sectorSize = RAW_SECTOR_SIZE;
		track.mode2 = true;		
	} else if (CanReadPVD(track.file, 2448, false)) {
		track.sectorSize = 2448;
		track.mode2 = false;		
	} else {
		delete track.file;
		track.file = NULL;
		return false;
	}
	int64_t len = track.file->getLength();
	track.length = (int)(len / track.sectorSize);
	
	#if defined (C_DEBUG)
		LOG_MSG("LoadIsoFile: %s\n"
								   "	Track		= %d, 0x40\n"
								   "	SectorSize	= %d"
								   "	Mode2		= %s"
								   "    Lenght		= %d", filename, track.number, track.sectorSize, track.mode2 ? "true" : "false",track.length);
	#endif
	
		tracks.push_back(track);
	
	// leadout track
	track.number = 2;
	track.attr = 0;
	track.start = track.length;
	track.length = 0;
	track.file = NULL;
	tracks.push_back(track);
	return true;
}

bool CDROM_Interface_Image::CanReadPVD(TrackFile *file, int sectorSize, bool mode2)
{
	Bit8u pvd[COOKED_SECTOR_SIZE];
	int seek = 16 * sectorSize;	// first vd is located at sector 16
	if ((sectorSize == RAW_SECTOR_SIZE || sectorSize == 2448) && !mode2) seek += 16;
	if (mode2) seek += 24;
	file->read(pvd, seek, COOKED_SECTOR_SIZE);
	// pvd[0] = descriptor type, pvd[1..5] = standard identifier, pvd[6] = iso version (+8 for High Sierra)
	return ((pvd[0] == 1 && !strncmp((char*)(&pvd[1]), "CD001", 5) && pvd[6] == 1) ||
			(pvd[8] == 1 && !strncmp((char*)(&pvd[9]), "CDROM", 5) && pvd[14] == 1));
}

#if defined(WIN32)
static string dirname(char * file) {
	char * sep = strrchr(file, '\\');
	if (sep == NULL)
		sep = strrchr(file, '/');
	if (sep == NULL)
		return "";
	else {
		int len = (int)(sep - file);
		char tmp[MAX_FILENAME_LENGTH];
		safe_strncpy(tmp, file, len+1);
		return tmp;
	}
}
#endif

bool CDROM_Interface_Image::LoadCueSheet(char *cuefile)
{
	Track track = {0, 0, 0, 0, 0, 0, false, NULL};
	tracks.clear();
	int shift = 0;
	int currPregap = 0;
	int totalPregap = 0;
	int prestart = -1;
	bool success;
	bool canAddTrack = false;
	ImageTrack.used = false;
	char tmp[MAX_FILENAME_LENGTH];	// dirname can change its argument
	safe_strncpy(tmp, cuefile, MAX_FILENAME_LENGTH);
	string pathname(dirname(tmp));
	ifstream in;
	in.open(cuefile, ios::in);
	if (in.fail()) return false;
	
	int TrackCount;
	string type;
	LOG_MSG("CDROM: Open %s",cuefile);
	
	while(!in.eof()) {
		// get next line
		char buf[MAX_LINE_LENGTH];
		in.getline(buf, MAX_LINE_LENGTH);
		if (in.fail() && !in.eof()) return false;  // probably a binary file
		istringstream line(buf);
				
		string command;
		GetCueKeyword(command, line);
		
		if (command == "TRACK") {
			if (canAddTrack) success = AddTrack(track, shift, prestart, totalPregap, currPregap);
			else success = true;
			
			track.start = 0;
			track.skip = 0;
			currPregap = 0;
			prestart = -1;
	
			line >> track.number;			
			GetCueKeyword(type, line);
			
			if (type == "AUDIO") {
				track.sectorSize = RAW_SECTOR_SIZE;
				track.attr = 0;
				track.mode2 = false;				
			} else if (type == "MODE1/2048") {
				track.sectorSize = COOKED_SECTOR_SIZE;
				track.attr = 0x40;
				track.mode2 = false;
			} else if (type == "MODE1/2352") {
				track.sectorSize = RAW_SECTOR_SIZE;
				track.attr = 0x40;
				track.mode2 = false;
			} else if (type == "MODE2/2336") {
				track.sectorSize = 2336;
				track.attr = 0x40;
				track.mode2 = true;
			} else if (type == "MODE2/2352") {
				track.sectorSize = RAW_SECTOR_SIZE;
				track.attr = 0x40;
				track.mode2 = true;
			} else if (type == "MODE1/2448") {
				track.sectorSize = 2448;
				track.attr = 0x40;
				track.mode2 = false;				
			} else success = false;						
			
			canAddTrack = true;
		}
		else if (command == "INDEX") {
			int index;
			line >> index;
			int frame;
			success = GetCueFrame(frame, line);

			if (index == 1) track.start = frame;
			else if (index == 0) prestart = frame;
			// ignore other indices
						
			//LOG_MSG("CDROM: TRACK %d (TYPE = %s)(SectorSize = %d)(Attr = %d)(Frame = %d)",track.number, type.c_str(), track.sectorSize, track.attr, track.start);			
			// Versuche die Frame (Offset) Position zu bekommen		
			
			if (track.number == 2 && type == "AUDIO" && track.attr == 0 && track.start > 0  && ImageTrack.used == false)
			{
				LOG_MSG("CDROM: -> 1st AUDIO OFFSET %d", track.start);				
				ImageTrack.offset 	  = track.start;				
				ImageTrack.sectorsize = track.sectorSize;
				ImageTrack.attr 	  = track.attr;
				ImageTrack.mode2 	  = track.mode2;					
				ImageTrack.used 	  = true;				
			}
			
		}
		else if (command == "FILE") {
			if (canAddTrack) success = AddTrack(track, shift, prestart, totalPregap, currPregap);
			else success = true;
			canAddTrack = false;

			string filename;
			GetCueString(filename, line);
			GetRealFileName(filename, pathname);
			string type;
			GetCueKeyword(type, line);

			track.file = NULL;
			bool error = true;
			if (type == "BINARY") {
				track.file = new BinaryFile(filename.c_str(), error);
			}
			else
				track.file = new AudioFile(filename.c_str(), error);
				// SDL_Sound first tries using a decoder having a matching registered extension
				// as the filename, and then falls back to trying each decoder before finally
				// giving up.
			if (error) {
				delete track.file;
				track.file = NULL;				
				success = false;
			}
		}
		else if (command == "PREGAP") success = GetCueFrame(currPregap, line);
		else if (command == "CATALOG") success = GetCueString(mcn, line);
		// ignored commands
		else if (command == "CDTEXTFILE" || command == "FLAGS" || command == "ISRC"
			|| command == "PERFORMER" || command == "POSTGAP" || command == "REM"
			|| command == "SONGWRITER" || command == "TITLE" || command == "") success = true;
		// failure
		else {
			delete track.file;
			track.file = NULL;
			success = false;
		}
		
		
		
		if (!success) return false;
	}
	// add last track
	if (!AddTrack(track, shift, prestart, totalPregap, currPregap)) return false;
	
	// add leadout track
	track.number++;
	track.attr = 0;//sync with load iso
	track.start = 0;
	track.length = 0;
	track.file = NULL;
	if(!AddTrack(track, shift, -1, totalPregap, 0)) return false;

	return true;
}

std::vector<string> split_string_to_list(const std::string& str, const std::string& delim)
{
	std::vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

typedef struct ChdInfo_
{
  chd_file		   * chd;
  core_file		   * image_file;
  const chd_header * header;
  char			   * hunk_buffer;
  int			   current_hunk_id;
} ChdInfo;
ChdInfo * pChdInfo = NULL;


int CDROM_Interface_Image::CHD_PreLoadFile( char* chdfile )
{

	char Meta[256];		/* Metadaten   */
	Bit8u  MetaFlg;		/* Meta Flags  */
	Bit32u MetaLen;		/* Meta Lenght */
	Bit32u MetaTag;		/* Tag Result  */

	int MetaVersion = 0;/* MetaVersion */
	int  Index = 0;		/* Index Nummer*/
	int	 Track = 0;		/* Track Nummer*/
	int  Frames= 0;		/* FRAMES:	   */
	int  Pre   = 0;		/* PREGAB:	   */
	int  Post  = 0;		/* POSTGAB:	   */

	char Type[32];		/* Mode Type   */
	char PGT[32];		/* PGTYPE:	   */
	char PGS[32];		/* PGSUB:	   */
	char SubType[32];	/* SUBTYPE:    */

	tracks.clear();
	ChdInfoCount.MetaVersion = 0;
	ChdInfoCount.Tracks		 = 0;

	
  
	if (pChdInfo != NULL) 
		free(pChdInfo);

	pChdInfo = (ChdInfo*)malloc(sizeof(ChdInfo));

	ChdInfoCount.Error = chd_open(chdfile , CHD_OPEN_READ, NULL, &pChdInfo->chd);
	if (ChdInfoCount.Error != CHDERR_NONE)
	{
		LOG_MSG("[%d] CHD: Open Error %d (File: %s)",__LINE__, ChdInfoCount.Error, __FILE__);
		return -1;
	}
		

	pChdInfo->header = chd_get_header(pChdInfo->chd);
  
	while (true) {
		ChdInfoCount.Error = chd_get_metadata(pChdInfo->chd, CDROM_TRACK_METADATA2_TAG, Index, &Meta, sizeof(Meta), &MetaLen, &MetaTag, &MetaFlg);
		if (ChdInfoCount.Error == CHDERR_NONE)
		{
			if (sscanf(Meta, CDROM_TRACK_METADATA2_FORMAT, &Track, Type, SubType, &Frames,&Pre, PGT,PGS, &Post) != 8)
			{
				LOG_MSG("CHD: Invalid Track New Metadata: ", Meta, "\n");
				return -1;
			}
			MetaVersion = 2;
			LOG_MSG("CHD [Track: %02d] [Type: %10s] [Subtype: %4s] [Frames: %06d ] [PreGAP: %03d]", Track, Type, SubType, Frames, Pre);

		}
		else
		{
			ChdInfoCount.Error = chd_get_metadata(pChdInfo->chd, CDROM_TRACK_METADATA_TAG, Index, &Meta, sizeof(Meta), &MetaLen, &MetaTag, &MetaFlg);
			if (ChdInfoCount.Error != CHDERR_NONE)
			{
				/* Meta v1 and V2 failed */
				break;
			}
			if (sscanf(Meta, CDROM_TRACK_METADATA_FORMAT, &Track, Type, SubType, &Frames) != 4)
			{
				LOG_MSG("[%d] CHD: Meta Data Open Error %d (File: %s)", __LINE__, ChdInfoCount.Error, __FILE__);
				LOG_MSG("[%d] CHD: Invalid Track Metadata: ", Meta, "\n");
				return -1;
			}
			MetaVersion = 1;
			LOG_MSG("CHD Format1 [Track: %02d] [Type: %10s] [Subtype: %4s] [Frames: %06d ]", Track, Type, SubType, Frames);
		}
		Index++;
	}
	ChdInfoCount.Tracks		 = Index;
	ChdInfoCount.MetaVersion = MetaVersion;
	LOG_MSG("\n\n");
	return MetaVersion;
}

bool CDROM_Interface_Image::LoadChdFile(char* chdfile)
{

	/*
		ToDo:
			- check if this is a CD and not an HDD CHD
	*/
	int MetaVersion = CHD_PreLoadFile(chdfile);
	if (MetaVersion == -1)
		return false;

	bool     error			= true;
	CHDFile* file			= new CHDFile(chdfile, error);
	Track    track			= { 0, 0, 0, 0, 0, 0, false, file };
	int      shift			= 0;
	int		 index			= 0;
	int      totalPregap	= 0;
	int      prestart		= -1;
	int		 frames			= 0;
	int		 frames_pregap  = 0;
	int		 frames_postgap = 0;
	Bit32u	 MetadataLen	= 0;
	Bit32u	 tag			= 0;
	Bit8u	 Indexflag		= 0;
	ImageTrack.used			= false;

	char   MetadataOut[256];

	char Type[32];		/* Mode Type   */
	char PGT[32];		/* PGTYPE:	   */
	char PGS[32];		/* PGSUB:	   */
	char SubType[32];	/* SUBTYPE:    */

	int    total_frames = 0;
	for (int TrackIndex = 0; TrackIndex < ChdInfoCount.Tracks; TrackIndex++)
	{		
		switch (MetaVersion)
		{
			case 1:
			{
				chd_get_metadata(file->getChd(), CDROM_TRACK_METADATA_TAG, TrackIndex, &MetadataOut, sizeof(MetadataOut), &MetadataLen,nullptr,&Indexflag);
				sscanf(MetadataOut, CDROM_TRACK_METADATA_FORMAT, &track.number, Type/*value*/, SubType, &frames);
			}
			break;
			case 2:
			{
				chd_get_metadata(file->getChd(), CDROM_TRACK_METADATA2_TAG, TrackIndex, &MetadataOut, sizeof(MetadataOut), &MetadataLen, nullptr, &Indexflag);
				sscanf(MetadataOut, CDROM_TRACK_METADATA2_FORMAT, &track.number, Type/*value*/, SubType, &frames, &frames_pregap, PGT, PGS, &frames_postgap);
			}
		}

		track.start = 0;
		track.skip	= 0;
		prestart	= -1;

		if (strcmp(Type, "AUDIO") == 0)
		{
			track.sectorSize = RAW_SECTOR_SIZE;
			track.attr		 = 0;
			track.mode2		 = false;			
		}
		else if (strcmp(Type, "MODE1") == 0)
		{
			track.sectorSize = COOKED_SECTOR_SIZE;
			track.attr		 = 0x40;
			track.mode2		 = false;
			file->skip_sync  = true;
		}
		else if (strcmp(Type, "MODE1_RAW") == 0) {
			track.sectorSize = RAW_SECTOR_SIZE;
			track.attr		 = 0x41;
			track.mode2		 = false;
		}
		else if (strcmp(Type, "MODE2") == 0) {
			track.sectorSize = 2336;
			track.attr		 = 0x40;
			track.mode2		 = true;
		}
		else if (strcmp(Type, "MODE2_FORM1") == 0) {
			track.sectorSize = COOKED_SECTOR_SIZE;
			track.attr		 = 0x40;
			track.mode2		 = true;
			file->skip_sync  = true;
		}
		else if (strcmp(Type, "MODE2_FORM2") == 0) {
			track.sectorSize = 2336;
			track.attr		 = 0x40;
			track.mode2		 = true;
		}
		else if (strcmp(Type, "MODE2_RAW") == 0) {
			track.sectorSize = RAW_SECTOR_SIZE;
			track.attr		 = 0x40;
			track.mode2		 = true;
		}
		else {
			LOG_MSG("[%d] Unknown Track Typ in Metadata %s",__LINE__, Type);// unknown track mode
			return false;
		};

		/* is subchannel data even supported? */
		if ( strcmp(SubType , "RAW") == 0 )
		{
			track.sectorSize += 96;
			LOG_MSG("[%d] Track Use RAW SectorSize (+96) [Sector Size = %d]", __LINE__, track.sectorSize);// unknown track mode
		}

		track.start		= total_frames;
		total_frames   += frames;
		track.length	= frames;
		
		//LOG_MSG("track.number %d value %s, attr %d, start %d",track.number, Type,track.attr, track.start);
		
		/* CHD has the pregap added to FRAMES
		 * this is needed or track.start and track.length
		 * do not match with LoadCueSheet for the same image
		 * creates the same track list as LoadCueSheet
		 */

		if (frames_pregap == 150)
		{
			prestart	  = track.start;
			track.start  += frames_pregap;
			track.length -= frames_pregap;
		}

		track.sectorSize = 2448; /* chd always uses 2448, fixed in PlayAudioSector */
		
		if (!AddTrack(track, shift, prestart, totalPregap, 0))
			return false;
	}

	// no tracks found
	if (tracks.empty())
		return false;

		track.number++;
		track.attr		= 0; // sync with load iso
		track.start		= 0;
		track.length	= 0;
		track.file		= NULL;

	if (!AddTrack(track, shift, -1, totalPregap, 0))
		return false;

	return true;
}



bool CDROM_Interface_Image::AddTrack(Track &curr, int &shift, int prestart, int &totalPregap, int currPregap)
{
	// frames between index 0(prestart) and 1(curr.start) must be skipped
	int skip;
	if (prestart > 0) {
		if (prestart > curr.start) return false;
		skip = curr.start - prestart;
	} else skip = 0;
	
	// first track (track number must be 1)
	if (tracks.empty()) {
		if (curr.number != 1) return false;
		curr.skip = skip * curr.sectorSize;
		curr.start += currPregap;
		totalPregap = currPregap;
		tracks.push_back(curr);
		return true;
	}
	
	Track &prev = *(tracks.end() - 1);

	// current track consumes data from the same file as the previous
	if (prev.file == curr.file) {
			curr.start += shift;
		if (!prev.length) {
			prev.length = curr.start + totalPregap - prev.start - skip;
		}
		curr.skip += prev.skip + prev.length * prev.sectorSize + skip * curr.sectorSize;
		totalPregap += currPregap;
		curr.start += totalPregap;
	// current track uses a different file as the previous track
	} else {
			if (!prev.length) {
				int tmp = prev.file->getLength() - prev.skip;
				prev.length = tmp / prev.sectorSize;
				if (tmp % prev.sectorSize != 0) prev.length++; // padding
			}
		curr.start += prev.start + prev.length + currPregap;
		curr.skip = skip * curr.sectorSize;
		shift += prev.start + prev.length;
		totalPregap = currPregap;
	}
	
	#if defined (C_DEBUG) && defined (CDROM_DEBUG)
	LOG_MSG("%s CDROM: AddTrack cur.start=%d cur.len=%d cur.start+len=%d | prev.start=%d prev.len=%d prev.start+len=%d",
	        get_time(),
	        curr.start, curr.length, curr.start + curr.length,
	        prev.start, prev.length, prev.start + prev.length);
	#endif

	// error checks
	if (curr.number <= 1) return false;
	if (prev.number + 1 != curr.number) return false;
	if (curr.start < prev.start + prev.length) return false;
	if (curr.length < 0) return false;
	
	tracks.push_back(curr);
	return true;
}

bool CDROM_Interface_Image::HasDataTrack(void)
{
	//Data track has attribute 0x40
	for(track_it it = tracks.begin(); it != tracks.end(); it++) {
		if ((*it).attr == 0x40) return true;
	}
	return false;
}


bool CDROM_Interface_Image::GetRealFileName(string &filename, string &pathname)
{
	// check if file exists
	struct stat test;
	if (stat(filename.c_str(), &test) == 0) return true;
	
	// check if file with path relative to cue file exists
	string tmpstr(pathname + "/" + filename);
	if (stat(tmpstr.c_str(), &test) == 0) {
		filename = tmpstr;
		return true;
	}
	// finally check if file is in a dosbox local drive
	char fullname[CROSS_LEN];
	char tmp[CROSS_LEN];
	safe_strncpy(tmp, filename.c_str(), CROSS_LEN);
	Bit8u drive;
	if (!DOS_MakeName(tmp, fullname, &drive)) return false;
	
	localDrive *ldp = dynamic_cast<localDrive*>(Drives[drive]);
	if (ldp) {
		ldp->GetSystemFilename(tmp, fullname);
		if (stat(tmp, &test) == 0) {
			filename = tmp;
			return true;
		}
	}
#if defined (WIN32) || defined(OS2)
	//Nothing
#else
	//Consider the possibility that the filename has a windows directory seperator (inside the CUE file) 
	//which is common for some commercial rereleases of DOS games using DOSBox

	string copy = filename;
	size_t l = copy.size();
	for (size_t i = 0; i < l;i++) {
		if(copy[i] == '\\') copy[i] = '/';
	}

	if (stat(copy.c_str(), &test) == 0) {
		filename = copy;
		return true;
	}

	tmpstr = pathname + "/" + copy;
	if (stat(tmpstr.c_str(), &test) == 0) {
		filename = tmpstr;
		return true;
	}

#endif
	return false;
}

bool CDROM_Interface_Image::GetCueKeyword(string &keyword, istream &in)
{
	in >> keyword;
	for(Bitu i = 0; i < keyword.size(); i++) keyword[i] = toupper(keyword[i]);
	
	return true;
}

bool CDROM_Interface_Image::GetCueFrame(int &frames, istream &in)
{
	string msf;
	in >> msf;
	int min, sec, fr;
	bool success = sscanf(msf.c_str(), "%d:%d:%d", &min, &sec, &fr) == 3;
	frames = MSF_TO_FRAMES(min, sec, fr);
	
	return success;
}

bool CDROM_Interface_Image::GetCueString(string &str, istream &in)
{
	int pos = (int)in.tellg();
	in >> str;
	if (str[0] == '\"') {
		if (str[str.size() - 1] == '\"') {
			str.assign(str, 1, str.size() - 2);
		} else {
			in.seekg(pos, ios::beg);
			char buffer[MAX_FILENAME_LENGTH];
			in.getline(buffer, MAX_FILENAME_LENGTH, '\"');	// skip
			in.getline(buffer, MAX_FILENAME_LENGTH, '\"');
			str = buffer;
		}
	}
	return true;
}

void CDROM_Interface_Image::ClearTracks()
{
	vector<Track>::iterator i = tracks.begin();
	vector<Track>::iterator end = tracks.end();

	TrackFile* last = NULL;	
	while(i != end) {
		Track &curr = *i;
		if (curr.file != last) {
			delete curr.file;
			last = curr.file;
		}
		i++;
	}
	tracks.clear();
}

void CDROM_Image_Destroy(Section*) {
	Sound_Quit();
}

void CDROM_Image_Init(Section* sec) {
	sec->AddDestroyFunction(CDROM_Image_Destroy, false);
	Sound_Init();
}
