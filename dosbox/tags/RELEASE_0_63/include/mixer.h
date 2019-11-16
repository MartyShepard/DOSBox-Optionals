/*
 *  Copyright (C) 2002-2004  The DOSBox Team
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

typedef void (*MIXER_MixHandler)(Bit8u * sampdate,Bit32u len);
typedef void (*MIXER_Handler)(Bitu len);

enum BlahModes {
	MIXER_8MONO,MIXER_8STEREO,
	MIXER_16MONO,MIXER_16STEREO
};

enum MixerModes {
	M_8M,M_8S,
	M_16M,M_16S,
};

#define MIXER_BUFSIZE (16*1024)
#define MIXER_BUFMASK (MIXER_BUFSIZE-1)
extern Bit8u MixTemp[MIXER_BUFSIZE];

#define MAX_AUDIO ((1<<(16-1))-1)
#define MIN_AUDIO -(1<<(16-1))

class MixerChannel {
public:
	void SetVolume(float _left,float _right);
	void UpdateVolume(void);
	void SetFreq(Bitu _freq);
	void Mix(Bitu _needed);
	void AddSilence(void);			//Fill up until needed
	template<bool _8bit,bool stereo>
	void AddSamples(Bitu len,void * data);
	void AddSamples_m8(Bitu len,Bit8u * data);
	void AddSamples_s8(Bitu len,Bit8u * data);
	void AddSamples_m16(Bitu len,Bit16s * data);
	void AddSamples_s16(Bitu len,Bit16s * data);
	void AddStretched(Bitu len,Bit16s * data);		//Strech block up into needed data
	void FillUp(void);
	void Enable(bool _yesno);
	MIXER_Handler handler;
	float volmain[2];
	Bit32s volmul[2];
	Bitu freq_add,freq_index;
	Bitu done,needed;
	Bits last[2];
	char * name;
	bool enabled;
	MixerChannel * next;
};

MixerChannel * MIXER_AddChannel(MIXER_Handler handler,Bitu freq,char * name);
MixerChannel * MIXER_FindChannel(const char * name);

/* PC Speakers functions, tightly related to the timer functions */

void PCSPEAKER_SetCounter(Bitu cntr,Bitu mode);
void PCSPEAKER_SetType(Bitu mode);

