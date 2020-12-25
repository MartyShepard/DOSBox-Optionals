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


char* get_time() {
	static time_t rawtime;
	struct tm* ptime;
    static char time_str[] = "00:00:00";

	time(&rawtime);
    ptime = localtime(&rawtime);
    sprintf(time_str, "%02d:%02d:%02d", ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
	return time_str;
}


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

bool CDROM_Interface_Image::BinaryFile::read(Bit8u *buffer, int seek, int count)
{
	file->seekg(seek, ios::beg);
	file->read((char*)buffer, count);
	return !(file->fail());
}

int CDROM_Interface_Image::BinaryFile::getLength()
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


bool CDROM_Interface_Image::BinaryFile::seek(Bit32u offset)
{
	file->seekg(offset, ios::beg);
	return !file->fail();
}

Bit16u CDROM_Interface_Image::BinaryFile::decode(Bit8u *buffer)
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

CDROM_Interface_Image::AudioFile::~AudioFile()
{
	Sound_FreeSample(sample);
}

bool CDROM_Interface_Image::AudioFile::seek(Bit32u offset)
{
	#ifdef DEBUG
	const auto begin = std::chrono::steady_clock::now();
	#endif

	// Convert the byte-offset to a time offset (milliseconds)
	const bool result = Sound_Seek(sample, lround(offset/176.4f));

	#ifdef DEBUG
	const auto end = std::chrono::steady_clock::now();
	LOG_MSG("%s CDROM: seek(%u) took %f ms", get_time(), offset, chrono::duration <double, milli> (end - begin).count());
	#endif

	return result;
}

Bit16u CDROM_Interface_Image::AudioFile::decode(Bit8u *buffer)
{
	const Bit16u bytes = Sound_Decode(sample);
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

int CDROM_Interface_Image::AudioFile::getLength()
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
    #ifdef DEBUG
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
        this->hunk_buffer 		= new Bit8u[this->header->hunkbytes];
		this->hunk_buffer_next 	= new Bit8u[this->header->hunkbytes];
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


bool CDROM_Interface_Image::CHDFile::read(Bit8u* buffer, int offset, int count)
{
    // we can not read more than a single sector currently
    if (count > RAW_SECTOR_SIZE) {
        return false;
    }

    int needed_hunk = offset / this->header->hunkbytes;

    // EOF
    if (needed_hunk > this->header->totalhunks) {
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

int CDROM_Interface_Image::CHDFile::getLength()
{
	return this->header->logicalbytes;
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

bool CDROM_Interface_Image::CHDFile::seek(Bit32u offset)
{
	// only checks if seek range is valid ? only used for audio ?
	// only used by PlayAudioSector ?
	#ifdef DEBUG
		LOG_MSG("CHD: Seek %d",offset);
    #endif		
	if ((offset / this->header->hunkbytes) < this->header->hunkcount) {
		return true;
	}
	else {
		return false;
	}
}

static void Endian_A16_Swap(void* src, Bit32u nelements)
{
	Bit32u i;
	Bit8u* nsrc = (Bit8u*)src;

	for (i = 0; i < nelements; i++)
	{
		Bit8u tmp = nsrc[i * 2];

		nsrc[i * 2] = nsrc[i * 2 + 1];
		nsrc[i * 2 + 1] = tmp;
	}
}

uint16_t CDROM_Interface_Image::CHDFile::decode(Bit8u* buffer)
{
    // reads one sector of CD audio ?

    assert(this->audio_pos % 2448 == 0);

    if (this->read(buffer, this->audio_pos, RAW_SECTOR_SIZE)) {
					
		
		
        // chd uses 2448
        this->audio_pos += 2448;

        // no idea if other platforms need this but on windows this is needed or there is only noise
        Endian_A16_Swap(buffer, 588 * 2);
		
		LOG_MSG("CHD: Decode %d (Audio Pos), %d (Buffer) %d",audio_pos, buffer, RAW_SECTOR_SIZE);

        // we only read the raw audio nothing else
			
        return RAW_SECTOR_SIZE;
    }
	SDL_Delay(1);
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
	
	if (player.cd == this)
	{
		player.cd = NULL;
	}
	
	ClearTracks();
	
	if (refCount == 0)
	{
		StopAudio();
		MIXER_DelChannel(player.channel);
		player.channel = NULL;
		    #ifdef DEBUG
				LOG_MSG("CDROM: Audio channel freed");
			#endif	
	
	}
}

void CDROM_Interface_Image::InitNewMedia(){}

bool CDROM_Interface_Image::SetDevice(char* path, int forceCD)
{
	(void)forceCD;//UNUSED
	
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

	#ifdef DEBUG
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

    #ifdef DEBUG
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
	FRAMES_TO_MSF(player.currFrame - tracks[track - 1].start + 150, &relPos.min, &relPos.sec, &relPos.fr);

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
	#endif
	
	return true;
}

bool CDROM_Interface_Image::GetAudioStatus(bool& playing, bool& pause)
{
	playing = player.isPlaying;
	pause = player.isPaused;

    #ifdef DEBUG
		LOG_MSG("%s CDROM: GetAudioStatus playing=%d, paused=%d", get_time(), playing, pause);
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
			player.playbackTotal = lround(len * tracks[track].sectorSize * bytesPerMs / 176.4);
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
			   tracks[track].sectorSize,
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
	#ifdef DEBUG
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

bool CDROM_Interface_Image::LoadUnloadMedia(bool unload)
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



bool CDROM_Interface_Image::ReadSector(uint8_t *buffer, bool raw, unsigned long sector)
{
	int track = GetTrack(sector) - 1;
	if (track < 0) return false;

	int seek = tracks[track].skip + (sector - tracks[track].start) * tracks[track].sectorSize;
	
	int length = (raw ? RAW_SECTOR_SIZE : COOKED_SECTOR_SIZE);
	
	if (tracks[track].sectorSize != RAW_SECTOR_SIZE && raw)
	{ 
		return false;
	}

	switch ( tracks[track].sectorSize )
	{		
		case 2448:	
		case RAW_SECTOR_SIZE:
		{
			if ( !tracks[track].mode2 && !raw )
			{
				/*
					Mode2 = false
				*/				
				seek += 16;
				SDL_Delay(1);
			}				
		}
		break;		
	}

	if (tracks[track].mode2 && !raw)
	{
		seek += 24;
	}
    #ifdef DEBUG
	LOG_MSG("CDROM: ReadSector track=%d, desired raw=%s, sector=%ld, length=%d", track, raw ? "true":"false", sector, length);
	#endif
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

					#ifdef DEBUG
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
	track.length = track.file->getLength() / track.sectorSize;
	// LOG_MSG("LoadIsoFile: %s, track 1, 0x40, sectorSize=%d, mode2=%s", filename, track.sectorSize, track.mode2 ? "true":"false");

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
	char tmp[MAX_FILENAME_LENGTH];	// dirname can change its argument
	safe_strncpy(tmp, cuefile, MAX_FILENAME_LENGTH);
	string pathname(dirname(tmp));
	ifstream in;
	in.open(cuefile, ios::in);
	if (in.fail()) return false;
	
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
			string type;
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

//////////////////////////////////////////////////////////////////////////////

typedef struct
{
   Bit8u ctl_addr;
   Bit32u fad_start;
   Bit32u fad_end;
   Bit32u file_offset;
   Bit32u sector_size;
   FILE *fp;
   int file_size;
   int file_id;
   int interleaved_sub;
   Bit32u frames;
   Bit32u extraframes;
   Bit32u pregap;
   Bit32u postgap;
   Bit32u physframeofs;
   Bit32u chdframeofs;
   Bit32u logframeofs;
   int isZip;
   char* filename;
} track_info_struct;

typedef struct
{
   Bit32u fad_start;
   Bit32u fad_end;
   track_info_struct *track;
   int track_num;
} session_info_struct;

typedef struct
{
   int session_num;
   session_info_struct *session;
} disc_info_struct;

static disc_info_struct disc;

//////////////////////////////////////////////////////////////////////////////

typedef struct ChdInfo_ {
  chd_file *chd;
  core_file * image_file;
  const chd_header * header;
  char * hunk_buffer;
  int current_hunk_id;
} ChdInfo;

ChdInfo * pChdInfo = NULL;

void CHD_LogInfo( char* chdfile )
{
  int  trak_number;
  
  char track_type[64];
  char track_subtype[64];  
  char pg_type[64];
  char pg_sub_type[64];
  
  int  frame 		= 0;
  int  pregap 		= 0;  
  int  postgap 		= 0;
  int  num_tracks 	= 0;  
  int meta_outlen 	= 512 * 1024;
  
  Bit8u * buf 		= (Bit8u *)malloc(meta_outlen);
  Bit32u resultlen;
  Bit32u resulttag;
  Bit8u  resultflags;
  
	if (pChdInfo != NULL) { free(pChdInfo); }

	pChdInfo = (ChdInfo *)malloc(sizeof(ChdInfo));
	  
	memset(pChdInfo, 0, sizeof(ChdInfo));

	track_info_struct trk[100];
	  
	memset(trk, 0, sizeof(trk));

  		
	chd_open(chdfile , CHD_OPEN_READ, NULL, &pChdInfo->chd);
		
	pChdInfo->header = chd_get_header(pChdInfo->chd);

	trk[num_tracks].fad_start = frame + pregap + 150;

	LOG_MSG(".. CHD CDROM IMAGE INFO ====================== \n");
  
	while ( chd_get_metadata(pChdInfo->chd, 0, num_tracks, buf, meta_outlen, &resultlen, &resulttag, &resultflags) == CHDERR_NONE )
	{

		LOG_MSG("%s", buf);
		
		switch (resulttag)
		{
			case CDROM_TRACK_METADATA_TAG:
			{
				sscanf((const char*)buf, CDROM_TRACK_METADATA_FORMAT, &trak_number, track_type, track_subtype, &frame);
				pregap 	= 0;
				postgap = 0;
				sprintf(pg_type, "NONE");
			}
			break;
		
			case CDROM_TRACK_METADATA2_TAG:
			{
				sscanf((const char*)buf, CDROM_TRACK_METADATA2_FORMAT, &trak_number, track_type, track_subtype, &frame, &pregap, pg_type, pg_sub_type, &postgap);
			}
			break;			
		}
		num_tracks++;
	}
	LOG_MSG(".. ==================================================== \n");  
	
	free(buf);	
	
}


bool CDROM_Interface_Image::LoadChdFile(char* chdfile)
{
	/*
		ToDo:
			- check if this is a CD and not an HDD CHD
			- check for CDROM_TRACK_METADATA_TAG or CDROM_OLD_METADATA_TAG (?)
	*/
	tracks.clear();
	bool     error		= true;
	CHDFile* file 		= new CHDFile(chdfile, error);
	Track    track 		= { 0, 0, 0, 0, 0, 0, false, file };
	int      shift 		= 0;
	int      currPregap = 0;
	int      totalPregap= 0;
	int      prestart 	= -1;
	int  	 pregap 	= 0; 
	int  	 postgap	= 0;
	
	char 	 pg_type[64];
	if (!error)
	{
		
		 CHD_LogInfo( chdfile );

  
		// iterate over all track entries
		char   metadata[256];
		int    index = 0;
		UINT32 count = 0;
		UINT32 tag = 0;
		UINT8  flags = 0;
		int    total_frames = 0;
		while (chd_get_metadata(file->getChd(), 0, index, &metadata, 256, &count, &tag, &flags) == CHDERR_NONE) {
			
			switch (tag)
			{
				case CDROM_TRACK_METADATA_TAG:
				{
				pregap = 0;
				postgap = 0;
				sprintf(pg_type, "NONE");
				}
				break;
				
				case CDROM_TRACK_METADATA2_TAG:
				{
				}
				break;
			}		  
		  
			// parse track
			// TRACK:1 TYPE:MODE1_RAW SUBTYPE:NONE FRAMES:206931 PREGAP:0 P
			std::vector<string> tokens = split_string_to_list(metadata, " ");
			track.start = 0;
			track.skip = 0;
			currPregap = 0;
			prestart = -1;
			for (int i = 0; i < tokens.size(); i++) {
				// "TRACK:1" > "TRACK" "1"
				std::vector<string> track_meta = split_string_to_list(tokens[i], ":");
				std::string         key = track_meta[0];
				std::string         value = track_meta[1];
				
				if (key == "TRACK") {
					// track index
					track.number = std::stoi(value);
				}
				
				else if (key == "TYPE") {
					// track type
                    if (value == "AUDIO") {
                        track.sectorSize = RAW_SECTOR_SIZE;
                        track.attr       = 0;
                        track.mode2      = false;
                    } else if (value == "MODE1") {
                        track.sectorSize = COOKED_SECTOR_SIZE;
                        track.attr       = 0x40;
                        track.mode2      = false;
                        file->skip_sync  = true;
                    } else if (value == "MODE1_RAW") {
                        track.sectorSize = RAW_SECTOR_SIZE;
                        track.attr       = 0x41;
                        track.mode2      = false;
                    } else if (value == "MODE2") {
                        track.sectorSize = 2336;
                        track.attr       = 0x40;
                        track.mode2      = true;
                    } else if (value == "MODE2_FORM1") {
                        track.sectorSize = COOKED_SECTOR_SIZE;
                        track.attr       = 0x40;
                        track.mode2      = true;
                        file->skip_sync  = true;
                    } else if (value == "MODE2_FORM2") {
                        track.sectorSize = 2336;
                        track.attr       = 0x40;
                        track.mode2      = true;
                    } else if (value == "MODE2_RAW") {
                        track.sectorSize = RAW_SECTOR_SIZE;
                        track.attr       = 0x40;
                        track.mode2      = true;												   								
					} else {
						// unknown track mode
						return false;
					};
				}
				else if (key == "SUBTYPE")
				{
					if (value == "RAW") {
						track.sectorSize += 96;
						// is subchannel data even supported?
					}
				}
				else if (key == "FRAMES")
				{
					int frames 		= std::stoi(value);
					track.start 	= total_frames;
					total_frames   += frames;
					track.length 	= frames;
				}
				else if (key == "PREGAP")
				{
					currPregap = std::stoi(value);
				}

			}

			// chd has the pregap added to FRAMES
			// this is needed or track.start and track.length do not match with LoadCueSheet for the same image
			// creates the same track list as LoadCueSheet
			if (currPregap) {
				prestart = track.start;
				track.start += currPregap;
				track.length -= currPregap;
			}

			track.sectorSize = 2448; // chd always uses 2448
			if (!AddTrack(track, shift, prestart, totalPregap, 0)) {
				return false;
			}

			index += 1;
		}

		// no tracks found
		if (tracks.empty()) return false;

		// add leadout track
		track.number++;
		track.attr = 0; // sync with load iso
		track.start = 0;
		track.length = 0;
		track.file = NULL;
		if (!AddTrack(track, shift, -1, totalPregap, 0)) return false;

		return true;

	}
	else {
		return false;
	}
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
	
	#ifdef DEBUG
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
