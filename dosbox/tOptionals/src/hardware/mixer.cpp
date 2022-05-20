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


/*
	Remove the sdl code from here and have it handled in the sdlmain.
	That should call the mixer start from there or something.
*/

#include <string.h>
#include <sys/types.h>
#include <math.h>

#if defined (WIN32)
//Midi listing
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

#if defined(_MSC_VER)
#include "SDL2/include/SDL.h"
#else
#include "SDL2/SDL.h"
#endif
#include "mem.h"
#include "pic.h"
#include "dosbox.h"
#include "mixer.h"
#include "timer.h"
#include "setup.h"
#include "cross.h"
#include "support.h"
#include "mapper.h"
#include "hardware.h"
#include "programs.h"
#include "midi.h"

extern void GFX_SetTitle(Bit32s cycles ,int frameskip,bool paused);

#define MIXER_SSIZE 4
#define MIXER_SHIFT 14
#define MIXER_REMAIN ((1<<MIXER_SHIFT)-1)

#define MIXER_VOLSHIFT 13

#define FREQ_SHIFT 14
#define FREQ_NEXT ( 1 << FREQ_SHIFT)
#define FREQ_MASK ( FREQ_NEXT -1 )

#define TICK_SHIFT 24
#define TICK_NEXT ( 1 << TICK_SHIFT)
#define TICK_MASK (TICK_NEXT -1)

Bit32s MixerVolDownUpKeysL = 0;
Bit32s MixerVolDownUpKeysR = 0;
Bit32s MixerVolMuteSaveL   = 0;
Bit32s MixerVolMuteSaveR   = 0;

Bit32s MixerVolDownUpCDA_L = 0;
Bit32s MixerVolDownUpCDA_R = 0;

static INLINE Bit16s MIXER_CLIP(Bits SAMP) {
	if (SAMP < MAX_AUDIO) {
		if (SAMP > MIN_AUDIO)
			return SAMP;
		else return MIN_AUDIO;
	} else return MAX_AUDIO;
}

static struct {
	Bit32s	work[MIXER_BUFSIZE][2];
			//Write/Read pointers for the buffer
	Bitu    work_in,work_out,work_wrap;
	Bitu 	pos,done;
	Bitu 	needed, min_needed, max_needed;
	//For every millisecond tick how many samples need to be generated
	Bit32u 	tick_add,tick_remain;
	Bit32u 	tick_counter;
	float 	mastervol[2];
	MixerChannel * channels;
	bool 	nosound;
	Bit32u 	freq;
	Bit32u 	blocksize;
	bool sampleaccurate;
} mixer;

bool Mixer_SampleAccurate() {
	return mixer.sampleaccurate;
}
	
Bit8u MixTemp[MIXER_BUFSIZE];

MixerChannel * MIXER_AddChannel(MIXER_Handler handler,Bitu freq,const char * name) {
	MixerChannel * chan=new MixerChannel();
	chan->scale = 1.0;
	chan->handler=handler;
	chan->name=name;
	chan->next=mixer.channels;
	chan->SetVolume(1,1);
	chan->enabled=false;
	chan->interpolate = false;
	chan->SetFreq(freq); //Sets interpolate as well.
	chan->last_samples_were_silence = true;
	chan->last_samples_were_stereo = false;
	chan->offset[0] = 0;
	chan->offset[1] = 0;
	mixer.channels = chan;
	return chan;
}

MixerChannel * MIXER_FindChannel(const char * name) {
	MixerChannel * chan=mixer.channels;
	while (chan) {
		if (!_stricmp(chan->name,name)) break;
		chan=chan->next;
	}
	return chan;
}

void MIXER_DelChannel(MixerChannel* delchan) {
	MixerChannel * chan=mixer.channels;
	MixerChannel * * where=&mixer.channels;
	while (chan) {
		if (chan==delchan) {
			*where=chan->next;
			delete delchan;
			return;
		}
		where=&chan->next;
		chan=chan->next;
	}
}

void MixerChannel::UpdateVolume(void) {
	volmul[0]=(Bits)((1 << MIXER_VOLSHIFT)*scale*volmain[0]*mixer.mastervol[0]);
	volmul[1]=(Bits)((1 << MIXER_VOLSHIFT)*scale*volmain[1]*mixer.mastervol[1]);
	MixerVolDownUpKeysL = mixer.mastervol[0]*100;
	MixerVolDownUpKeysR = mixer.mastervol[1]*100;	
	//LOG_MSG("SND: Volume Mixer: %2d:%2d",MixerVolDownUpKeysL,MixerVolDownUpKeysR);	
	// Zeige Lautstärke im Fenster	
	GFX_SetTitle(-1,-1,false);	
}

void MixerChannel::SetVolume(float _left,float _right) {
	volmain[0]=_left;
	volmain[1]=_right;
	UpdateVolume();
}

void MixerChannel::SetScale( float f ) {
	scale = f;
	UpdateVolume();
}

void MixerChannel::SetScale(float _left, float _right) {
	// Constrain application-defined volume between 0% and 100%
	const float min_volume(0.0);
	const float max_volume(1.0);
	_left  = clamp(_left,  min_volume, max_volume);
	_right = clamp(_right, min_volume, max_volume);
	if (scalex[0] != _left || scalex[1] != _right) {
		scalex[0] = _left;
		scalex[1] = _right;
		UpdateVolume();
	}
}

void MixerChannel::Enable(bool _yesno) {
	if (_yesno==enabled) return;
	enabled=_yesno;
	if (enabled) {
		freq_counter = 0;
		SDL_LockAudio();
		if (done<mixer.done) done=mixer.done;
		SDL_UnlockAudio();
	}
}

void MixerChannel::SetFreq(Bitu freq) {
	freq_add=(freq<<FREQ_SHIFT)/mixer.freq;

	if (freq != mixer.freq) {
		interpolate = true;
	} else {
		interpolate = false;
	}
}

void MixerChannel::Mix(Bitu _needed) {
	needed=_needed;
	while (enabled && needed>done) {
		Bitu left = (needed - done);
		left *= freq_add;
		left  = (left >> FREQ_SHIFT) + ((left & FREQ_MASK)!=0);		
		handler(left);
	}
}

void MixerChannel::AddSilence(void) {
	if (done < needed) {
		if(prevSample[0] == 0 && prevSample[1] == 0) {
			done = needed;
			//Make sure the next samples are zero when they get switched to prev
			nextSample[0] = 0;
			nextSample[1] = 0;
			//This should trigger an instant request for new samples
			freq_counter = FREQ_NEXT;
		} else {
			bool stereo = last_samples_were_stereo;
			//Position where to write the data
			Bitu mixpos = mixer.pos + done;
			while (done < needed) {
				// Maybe depend on sample rate. (the 4)
				if (prevSample[0] > 4)       nextSample[0] = prevSample[0] - 4;
				else if (prevSample[0] < -4) nextSample[0] = prevSample[0] + 4;
				else nextSample[0] = 0;
				if (prevSample[1] > 4)       nextSample[1] = prevSample[1] - 4;
				else if (prevSample[1] < -4) nextSample[1] = prevSample[1] + 4;
				else nextSample[1] = 0;

				mixpos &= MIXER_BUFMASK;
				Bit32s* write = mixer.work[mixpos];

				write[0] += prevSample[0] * volmul[0];
				write[1] += (stereo ? prevSample[1] : prevSample[0]) * volmul[1];

				prevSample[0] = nextSample[0];
				prevSample[1] = nextSample[1];
				mixpos++;
				done++;
				freq_counter = FREQ_NEXT;
			} 
		}
	}
	last_samples_were_silence = true;
	offset[0] = offset[1] = 0;
}

//4 seems to work . Disabled for now
#define MIXER_UPRAMP_STEPS 0
#define MIXER_UPRAMP_SAVE 512

template<class Type,bool stereo,bool signeddata,bool nativeorder>
inline void MixerChannel::AddSamples(Bitu len, const Type* data) {
	last_samples_were_stereo = stereo;

	//Position where to write the data
	Bitu mixpos = mixer.pos + done;
	//Position in the incoming data
	Bitu pos = 0;
	//Mix and data for the full length
	while (1) {
		//Does new data need to get read?
		while (freq_counter >= FREQ_NEXT) {
			//Would this overflow the source data, then it's time to leave
			if (pos >= len) {
				last_samples_were_silence = false;
#if MIXER_UPRAMP_STEPS > 0
				if (offset[0] || offset[1]) {
					//Should be safe to do, as the value inside offset is 16 bit while offset itself is at least 32 bit
					offset[0] = (offset[0]*(MIXER_UPRAMP_STEPS-1))/MIXER_UPRAMP_STEPS;
					offset[1] = (offset[1]*(MIXER_UPRAMP_STEPS-1))/MIXER_UPRAMP_STEPS;
					if (offset[0] < MIXER_UPRAMP_SAVE && offset[0] > -MIXER_UPRAMP_SAVE) offset[0] = 0;
					if (offset[1] < MIXER_UPRAMP_SAVE && offset[1] > -MIXER_UPRAMP_SAVE) offset[1] = 0;
				}
#endif
				return;
			}
			freq_counter -= FREQ_NEXT;
			prevSample[0] = nextSample[0];
			if (stereo) {
				prevSample[1] = nextSample[1];
			}
			if ( sizeof( Type) == 1) {
				if (!signeddata) {
					if (stereo) {
						nextSample[0]=(((Bit8s)(data[pos*2+0] ^ 0x80)) << 8);
						nextSample[1]=(((Bit8s)(data[pos*2+1] ^ 0x80)) << 8);
					} else {
						nextSample[0]=(((Bit8s)(data[pos] ^ 0x80)) << 8);
					}
				} else {
					if (stereo) {
						nextSample[0]=(data[pos*2+0] << 8);
						nextSample[1]=(data[pos*2+1] << 8);
					} else {
						nextSample[0]=(data[pos] << 8);
					}
				}
			//16bit and 32bit both contain 16bit data internally
			} else  {
				if (signeddata) {
					if (stereo) {
						if (nativeorder) {
							nextSample[0]=data[pos*2+0];
							nextSample[1]=data[pos*2+1];
						} else {
							if ( sizeof( Type) == 2) {
								nextSample[0]=(Bit16s)host_readw((HostPt)&data[pos*2+0]);
								nextSample[1]=(Bit16s)host_readw((HostPt)&data[pos*2+1]);
							} else {
								nextSample[0]=(Bit32s)host_readd((HostPt)&data[pos*2+0]);
								nextSample[1]=(Bit32s)host_readd((HostPt)&data[pos*2+1]);
							}
						}
					} else {
						if (nativeorder) {
							nextSample[0] = data[pos];
						} else {
							if ( sizeof( Type) == 2) {
								nextSample[0]=(Bit16s)host_readw((HostPt)&data[pos]);
							} else {
								nextSample[0]=(Bit32s)host_readd((HostPt)&data[pos]);
							}
						}
					}
				} else {
					if (stereo) {
						if (nativeorder) {
							nextSample[0]=(Bits)data[pos*2+0]-32768;
							nextSample[1]=(Bits)data[pos*2+1]-32768;
						} else {
							if ( sizeof( Type) == 2) {
								nextSample[0]=(Bits)host_readw((HostPt)&data[pos*2+0])-32768;
								nextSample[1]=(Bits)host_readw((HostPt)&data[pos*2+1])-32768;
							} else {
								nextSample[0]=(Bits)host_readd((HostPt)&data[pos*2+0])-32768;
								nextSample[1]=(Bits)host_readd((HostPt)&data[pos*2+1])-32768;
							}
						}
					} else {
						if (nativeorder) {
							nextSample[0]=(Bits)data[pos]-32768;
						} else {
							if ( sizeof( Type) == 2) {
								nextSample[0]=(Bits)host_readw((HostPt)&data[pos])-32768;
							} else {
								nextSample[0]=(Bits)host_readd((HostPt)&data[pos])-32768;
							}
						}
					}
				}
			}
			//This sample has been handled now, increase position
			pos++;
#if MIXER_UPRAMP_STEPS > 0
			if (last_samples_were_silence && pos == 1) {
				offset[0] = nextSample[0] - prevSample[0];
				if (stereo) offset[1] = nextSample[1] - prevSample[1];
				//Don't bother with small steps.
				if (offset[0] < (MIXER_UPRAMP_SAVE*4) && offset[0] > (-MIXER_UPRAMP_SAVE*4)) offset[0] = 0;
				if (offset[1] < (MIXER_UPRAMP_SAVE*4) && offset[1] > (-MIXER_UPRAMP_SAVE*4)) offset[1] = 0;
			}

			if (offset[0] || offset[1]) {
				nextSample[0] = nextSample[0] - (offset[0]*(MIXER_UPRAMP_STEPS*static_cast<Bits>(len)-static_cast<Bits>(pos))) /( MIXER_UPRAMP_STEPS*static_cast<Bits>(len) );
				nextSample[1] = nextSample[1] - (offset[1]*(MIXER_UPRAMP_STEPS*static_cast<Bits>(len)-static_cast<Bits>(pos))) /( MIXER_UPRAMP_STEPS*static_cast<Bits>(len) );
			}
#endif			
		}
		//Where to write
		mixpos &= MIXER_BUFMASK;
		Bit32s* write = mixer.work[mixpos];
		if (!interpolate) {
			write[0] += prevSample[0] * volmul[0];
			write[1] += (stereo ? prevSample[1] : prevSample[0]) * volmul[1];
		}
		else {
			Bits diff_mul = freq_counter & FREQ_MASK;
			Bits sample = prevSample[0] + (((nextSample[0] - prevSample[0]) * diff_mul) >> FREQ_SHIFT);
			write[0] += sample*volmul[0];
			if (stereo) {
				sample = prevSample[1] + (((nextSample[1] - prevSample[1]) * diff_mul) >> FREQ_SHIFT);
			}
			write[1] += sample*volmul[1];
		}
		//Prepare for next sample
		freq_counter += freq_add;
		mixpos++;
		done++;
	}
}

void MixerChannel::AddStretched(Bitu len,Bit16s * data) {
	if (done >= needed) {
		LOG_MSG("Can't add, buffer full");
		return;
	}
	//Target samples this inputs gets stretched into
	Bitu outlen = needed - done;
	Bitu index = 0;
	Bitu index_add = (len << FREQ_SHIFT)/outlen;
	Bitu mixpos = mixer.pos + done;
	done = needed;
	Bitu pos = 0;

	while (outlen--) {
		Bitu new_pos = index >> FREQ_SHIFT;
		if (pos != new_pos) {
			pos = new_pos;
			//Forward the previous sample
			prevSample[0] = data[0];
			data++;
		}
		Bits diff = data[0] - prevSample[0];
		Bits diff_mul = index & FREQ_MASK;
		index += index_add;
		mixpos &= MIXER_BUFMASK;
		Bits sample = prevSample[0] + ((diff * diff_mul) >> FREQ_SHIFT);
		mixer.work[mixpos][0] += sample * volmul[0];
		mixer.work[mixpos][1] += sample * volmul[1];
		mixpos++;
	}
}

void MixerChannel::AddStretchedStereo(Bitu len/*combined L+R samples*/,Bit16s * data) {
	if (done>=needed) {
		LOG_MSG("Can't add, buffer full");	
		return;
	}
	Bitu outlen=needed-done;Bits diff,diff2;
	freq_index=0;
	Bitu temp_add=(len << MIXER_SHIFT)/outlen;
	Bitu mixpos=mixer.pos+done;done=needed;
	Bitu pos=0;
	diff=data[0]-last[0];
	diff2=data[1]-last[1];
	while (outlen--) {
		Bitu new_pos=freq_index >> MIXER_SHIFT;
		if (pos<new_pos) {
			pos=new_pos;
			last[0]+=diff;
			last[1]+=diff2;
			diff=data[pos*2]-last[0];
			diff2=data[pos*2+1]-last[1];
		}
		Bits diff_mul=freq_index & MIXER_REMAIN;
		freq_index+=temp_add;
		mixpos&=MIXER_BUFMASK;
		Bits sample;
		sample=last[0]+((diff*diff_mul) >> MIXER_SHIFT);
		mixer.work[mixpos][0]+=sample*volmul[0];
		sample=last[1]+((diff2*diff_mul) >> MIXER_SHIFT);
		mixer.work[mixpos][1]+=sample*volmul[1];
		mixpos++;
	}
}

void MixerChannel::AddSamples_m8(Bitu len, const Bit8u * data) {
	AddSamples<Bit8u,false,false,true>(len,data);
}
void MixerChannel::AddSamples_s8(Bitu len,const Bit8u * data) {
	AddSamples<Bit8u,true,false,true>(len,data);
}
void MixerChannel::AddSamples_m8s(Bitu len,const Bit8s * data) {
	AddSamples<Bit8s,false,true,true>(len,data);
}
void MixerChannel::AddSamples_s8s(Bitu len,const Bit8s * data) {
	AddSamples<Bit8s,true,true,true>(len,data);
}
void MixerChannel::AddSamples_m16(Bitu len,const Bit16s * data) {
	AddSamples<Bit16s,false,true,true>(len,data);
}
void MixerChannel::AddSamples_s16(Bitu len,const Bit16s * data) {
	AddSamples<Bit16s,true,true,true>(len,data);
}
void MixerChannel::AddSamples_m16u(Bitu len,const Bit16u * data) {
	AddSamples<Bit16u,false,false,true>(len,data);
}
void MixerChannel::AddSamples_s16u(Bitu len,const Bit16u * data) {
	AddSamples<Bit16u,true,false,true>(len,data);
}
void MixerChannel::AddSamples_m32(Bitu len,const Bit32s * data) {
	AddSamples<Bit32s,false,true,true>(len,data);
}
void MixerChannel::AddSamples_s32(Bitu len,const Bit32s * data) {
	AddSamples<Bit32s,true,true,true>(len,data);
}
void MixerChannel::AddSamples_m16_nonnative(Bitu len,const Bit16s * data) {
	AddSamples<Bit16s,false,true,false>(len,data);
}
void MixerChannel::AddSamples_s16_nonnative(Bitu len,const Bit16s * data) {
	AddSamples<Bit16s,true,true,false>(len,data);
}
void MixerChannel::AddSamples_m16u_nonnative(Bitu len,const Bit16u * data) {
	AddSamples<Bit16u,false,false,false>(len,data);
}
void MixerChannel::AddSamples_s16u_nonnative(Bitu len,const Bit16u * data) {
	AddSamples<Bit16u,true,false,false>(len,data);
}
void MixerChannel::AddSamples_m32_nonnative(Bitu len,const Bit32s * data) {
	AddSamples<Bit32s,false,true,false>(len,data);
}
void MixerChannel::AddSamples_s32_nonnative(Bitu len,const Bit32s * data) {
	AddSamples<Bit32s,true,true,false>(len,data);
}

void MixerChannel::FillUp(void) {
	SDL_LockAudio();
	if (done < mixer.done) {
		SDL_UnlockAudio();
		return;
	}
	float index = PIC_TickIndex();
	Mix((Bitu)(index * mixer.needed));
	SDL_UnlockAudio();
}

extern bool ticksLocked;
static inline bool Mixer_irq_important(void) {
	/* In some states correct timing of the irqs is more important than
	 * non stuttering audio */
	return (ticksLocked || (CaptureState & (CAPTURE_WAVE|CAPTURE_VIDEO)));
}

static Bit32u calc_tickadd(Bit32u freq) {
	#if TICK_SHIFT > 16
		Bit64u freq64 = static_cast<Bit64u>(freq);
		freq64 = (freq64<<TICK_SHIFT)/1000;
		Bit32u r = static_cast<Bit32u>(freq64);
		return r;
	#else
		return (freq<<TICK_SHIFT)/1000;
	#endif
}


/* Mix a certain amount of new samples */
static void MIXER_MixData(Bitu needed) {
	MixerChannel * chan=mixer.channels;
	while (chan) {
		chan->Mix(needed);
		chan=chan->next;
	}
	if (CaptureState & (CAPTURE_WAVE|CAPTURE_VIDEO)) {
		Bit16s convert[1024][2];
		Bitu added=needed-mixer.done;
		if (added>1024)
			added=1024;
		Bitu readpos=(mixer.pos+mixer.done)&MIXER_BUFMASK;
		for (Bitu i=0;i<added;i++) {
			Bits sample=mixer.work[readpos][0] >> MIXER_VOLSHIFT;
			convert[i][0]=MIXER_CLIP(sample);
			sample=mixer.work[readpos][1] >> MIXER_VOLSHIFT;
			convert[i][1]=MIXER_CLIP(sample);
			readpos=(readpos+1)&MIXER_BUFMASK;
		}
		CAPTURE_AddWave( mixer.freq, added, (Bit16s*)convert );
	}
	//Reset the the tick_add for constant speed
	if( Mixer_irq_important() && !mixer.sampleaccurate )
		mixer.tick_add = calc_tickadd(mixer.freq);
	mixer.done = needed;
}

static void MIXER_MixPICEvent(Bitu val) {
	SDL_LockAudio();
	if (mixer.needed > 0) MIXER_MixData(mixer.needed);
	mixer.tick_counter+=mixer.tick_add;
	mixer.needed+=(mixer.tick_counter>>MIXER_SHIFT);
	mixer.tick_counter&=MIXER_REMAIN;
	SDL_UnlockAudio();

	PIC_AddEvent(MIXER_MixPICEvent,1000.0 / mixer.freq,0);
}

static void MIXER_Mix(void) {
	SDL_LockAudio();
	MIXER_MixData(mixer.needed);
	mixer.tick_counter += mixer.tick_add;
	mixer.needed+=(mixer.tick_counter >> TICK_SHIFT);
	mixer.tick_counter &= TICK_MASK;
	SDL_UnlockAudio();
}

static void MIXER_Mix_NoSound(void) {
	MIXER_MixData(mixer.needed);
	/* Clear piece we've just generated */
	for (Bitu i=0;i<mixer.needed;i++) {
		mixer.work[mixer.pos][0]=0;
		mixer.work[mixer.pos][1]=0;
		mixer.pos=(mixer.pos+1)&MIXER_BUFMASK;
	}
	/* Reduce count in channels */
	for (MixerChannel * chan=mixer.channels;chan;chan=chan->next) {
		if (chan->done>mixer.needed) chan->done-=mixer.needed;
		else chan->done=0;
	}
	/* Set values for next tick */
	mixer.tick_counter += mixer.tick_add;
	mixer.needed = (mixer.tick_counter >> TICK_SHIFT);
	mixer.tick_counter &= TICK_MASK;
	mixer.done=0;
}
#define INDEX_SHIFT_LOCAL 14

static void SDLCALL MIXER_CallBack(void* /*userdata*/, Uint8* stream, int len) {
	memset(stream, 0, int(len));
	Bitu need=(Bitu)len/MIXER_SSIZE;
	Bit16s * output=(Bit16s *)stream;
	Bitu reduce;
	Bitu pos;
	//Local resampling counter to manipulate the data when sending it off to the callback
	Bitu index_add = (1 << INDEX_SHIFT_LOCAL);
	Bitu index = (index_add % need) ? need : 0;
	Bits sample;
	/* sample accurate mode */
	if (mixer.sampleaccurate) {
		/* FIXME: Need stretching, min/max considerations, muting the buffer, etc. */
		reduce = need;
		if (reduce > mixer.done) reduce = mixer.done;
		index_add = (reduce << MIXER_SHIFT) / need;
	}
	
	/* Enough room in the buffer ? */
	else if (mixer.done < need) {
//		LOG_MSG("Full underrun need %d, have %d, min %d", need, mixer.done, mixer.min_needed);
		if ((need - mixer.done) > (need >> 7)) //Max 1 percent stretch.
			return;
		reduce = mixer.done;
		index_add = (reduce << INDEX_SHIFT_LOCAL) / need;
		mixer.tick_add = calc_tickadd(mixer.freq+mixer.min_needed);
	} else if (mixer.done < mixer.max_needed) {
		Bitu left = mixer.done - need;
		if (left < mixer.min_needed) {
			if( !Mixer_irq_important() ) {
				Bitu needed = mixer.needed - need;
				Bitu diff = (mixer.min_needed>needed?mixer.min_needed:needed) - left;
				mixer.tick_add = calc_tickadd(mixer.freq+(diff*3));
				left = 0; //No stretching as we compensate with the tick_add value
			} else {
				left = (mixer.min_needed - left);
				left = 1 + (2*left) / mixer.min_needed; //left=1,2,3
			}
//			LOG_MSG("needed underrun need %d, have %d, min %d, left %d", need, mixer.done, mixer.min_needed, left);
			reduce = need - left;
			index_add = (reduce << INDEX_SHIFT_LOCAL) / need;
		} else {
			reduce = need;
			index_add = (1 << INDEX_SHIFT_LOCAL);
//			LOG_MSG("regular run need %d, have %d, min %d, left %d", need, mixer.done, mixer.min_needed, left);

			/* Mixer tick value being updated:
			 * 3 cases:
			 * 1) A lot too high. >division by 5. but maxed by 2* min to prevent too fast drops.
			 * 2) A little too high > division by 8
			 * 3) A little to nothing above the min_needed buffer > go to default value
			 */
			Bitu diff = left - mixer.min_needed;
			if(diff > (mixer.min_needed<<1)) diff = mixer.min_needed<<1;
			if(diff > (mixer.min_needed>>1))
				mixer.tick_add = calc_tickadd(mixer.freq-(diff/5));
			else if (diff > (mixer.min_needed>>2))
				mixer.tick_add = calc_tickadd(mixer.freq-(diff>>3));
			else
				mixer.tick_add = calc_tickadd(mixer.freq);
		}
	} else {
		/* There is way too much data in the buffer */
//		LOG_MSG("overflow run need %d, have %d, min %d", need, mixer.done, mixer.min_needed);
		if (mixer.done > MIXER_BUFSIZE)
			index_add = MIXER_BUFSIZE - 2*mixer.min_needed;
		else
			index_add = mixer.done - 2*mixer.min_needed;
		index_add = (index_add << INDEX_SHIFT_LOCAL) / need;
		reduce = mixer.done - 2* mixer.min_needed;
		mixer.tick_add = calc_tickadd(mixer.freq-(mixer.min_needed/5));
	}
	/* Reduce done count in all channels */
	for (MixerChannel * chan=mixer.channels;chan;chan=chan->next) {
		if (chan->done>reduce) chan->done-=reduce;
		else chan->done=0;
	}

	// Reset mixer.tick_add when irqs are important
	if( Mixer_irq_important() && !mixer.sampleaccurate )
		mixer.tick_add = calc_tickadd(mixer.freq);

	mixer.done -= reduce;
	mixer.needed -= reduce;
	pos = mixer.pos;
	mixer.pos = (mixer.pos + reduce) & MIXER_BUFMASK;
	if(need != reduce) {
		while (need--) {
			Bitu i = (pos + (index >> INDEX_SHIFT_LOCAL)) & MIXER_BUFMASK;
			index += index_add;
			sample=mixer.work[i][0]>>MIXER_VOLSHIFT;
			*output++=MIXER_CLIP(sample);
			sample=mixer.work[i][1]>>MIXER_VOLSHIFT;
			*output++=MIXER_CLIP(sample);
		}
		/* Clean the used buffer */
		while (reduce--) {
			pos &= MIXER_BUFMASK;
			mixer.work[pos][0]=0;
			mixer.work[pos][1]=0;
			pos++;
		}
	} else {
		while (reduce--) {
			pos &= MIXER_BUFMASK;
			sample=mixer.work[pos][0]>>MIXER_VOLSHIFT;
			*output++=MIXER_CLIP(sample);
			sample=mixer.work[pos][1]>>MIXER_VOLSHIFT;
			*output++=MIXER_CLIP(sample);
			mixer.work[pos][0]=0;
			mixer.work[pos][1]=0;
			pos++;
		}
	}
}

#undef INDEX_SHIFT_LOCAL

static void MIXER_Stop(Section* /*sec*/) {
}

class MIXER : public Program {
public:
	void MakeVolume(char * scan,float & vol0,float & vol1) {
		Bitu w=0;
		bool db=(toupper(*scan)=='D');
		if (db) scan++;
		while (*scan) {
			if (*scan==':') {
				++scan;w=1;
			}
			char * before=scan;
			float val=(float)strtod(scan,&scan);
			if (before==scan) {
				++scan;continue;
			}
			if (!db) val/=100;
			else val=powf(10.0f,(float)val/20.0f);
			if (val<0) val=1.0f;
			if (!w) {
				vol0=val;
			} else {
				vol1=val;
			}
		}
		if (!w) vol1=vol0;
	}

	void Run(void) {
		if(cmd->FindExist("/LISTMIDI")) {
			ListMidi();
			return;
		}
		if (cmd->FindString("MASTER",temp_line,false)) {
			MakeVolume((char *)temp_line.c_str(),mixer.mastervol[0],mixer.mastervol[1]);
		}
		MixerChannel * chan = mixer.channels;
		while (chan) {
			if (cmd->FindString(chan->name,temp_line,false)) {
				MakeVolume((char *)temp_line.c_str(),chan->volmain[0],chan->volmain[1]);
			}
			chan->UpdateVolume();
			chan = chan->next;
		}
		if (cmd->FindExist("/NOSHOW")) return;
		
		WriteOut("\n");
		WriteOut(" Channel Name   Main\t  Main(dB)\n");
		ShowVolume("MASTER",mixer.mastervol[0],mixer.mastervol[1]);
		for (chan = mixer.channels;chan;chan = chan->next){
			ShowVolume(chan->name,chan->volmain[0],chan->volmain[1]);
		}
		WriteOut("\n");		
	}
private:
	void ShowVolume(const char * name,float vol0,float vol1) {
		WriteOut("   %-12s %3.0f:%-3.0f\t  %+3.2f:%-+3.2f \n",name,vol0*100,vol1*100,20*log(vol0)/log(10.0f),20*log(vol1)/log(10.0f));		
	}

	void ListMidi(){
		if(midi.handler) midi.handler->ListAll(this);
	};

};

static void MIXER_ProgramStart(Program * * make) {
	*make=new MIXER;
}

MixerChannel* MixerObject::Install(MIXER_Handler handler,Bitu freq,const char * name){
	if(!installed) {
		if(strlen(name) > 31) E_Exit("Too long mixer channel name");
		safe_strncpy(m_name,name,32);
		installed = true;
		return MIXER_AddChannel(handler,freq,name);
	} else {
		E_Exit("already added mixer channel.");
		return 0; //Compiler happy
	}
}

MixerObject::~MixerObject(){
	if(!installed) return;
	MIXER_DelChannel(MIXER_FindChannel(m_name));
}

static void MIXER_UpdateVol(float delta) {
	for (int i=0;i<2;i++) {
		mixer.mastervol[i] += delta;	
		if (mixer.mastervol[i] < 0.0f) mixer.mastervol[i] = 0.0f;
		else if (mixer.mastervol[i] > 1.0f) mixer.mastervol[i] = 1.0f;
	}
	for (MixerChannel * chan=mixer.channels;chan;chan=chan->next)
		chan->UpdateVolume();
}
 
static void MIXER_VolDown(bool pressed) {
	MIXER_UpdateVol(-0.01f);
}

static void MIXER_VolUp(bool pressed) {
	MIXER_UpdateVol(+0.01f);
}

static void MIXER_VolMute(bool pressed) {
	/* Todo Volume Mute */
}

void MIXER_UpdateVolCD(float delta, bool manual, const char* channame) {

	MixerChannel* chan = mixer.channels;	
	while (chan) {
		if (strcmp(chan->name, channame) == 0) {
			for (int i = 0; i < 2; i++) {

				if (manual) {
					chan->volmain[i] += delta;
				}
				else {
					chan->volmain[i] = delta;
				}

				if (chan->volmain[i] < 0.0f) {
					chan->volmain[i] = 0.0f;						
				}
				else if (chan->volmain[i] > 1.0f) {
					chan->volmain[i] = 1.0f;
				}
			}
			chan->UpdateVolume();
			MixerVolDownUpCDA_L = chan->volmain[0] * 100;
			MixerVolDownUpCDA_R = chan->volmain[1] * 100;
			LOG_MSG("Volume Command: -----------------");
			LOG_MSG("Volume Command: MIXER %s %2d:%2d", chan->name, MixerVolDownUpCDA_L, MixerVolDownUpCDA_R);
			LOG_MSG("Volume Command: -----------------\n");
			break;
		}			
		chan = chan->next;
	}
}

static void MIXER_VolDownCD(bool pressed) {
	MIXER_UpdateVolCD(-0.01f, true, "CDAUDIO");
}

static void MIXER_VolUpCD(bool pressed) {
	MIXER_UpdateVolCD(+0.01f, true, "CDAUDIO");
}



void MIXER_Init(Section* sec) {
	sec->AddDestroyFunction(&MIXER_Stop);

	Section_prop * section=static_cast<Section_prop *>(sec);
	/* Read out config section */
	mixer.freq=section->Get_int("rate");
	mixer.nosound=section->Get_bool("nosound");
	mixer.blocksize=section->Get_int("blocksize");
	mixer.sampleaccurate=section->Get_bool("accurate");

	/* Initialize the internal stuff */
	mixer.channels=0;
	mixer.pos=0;
	mixer.done=0;
	memset(mixer.work,0,sizeof(mixer.work));
	mixer.mastervol[0]=1.0f;
	mixer.mastervol[1]=1.0f;
	
	// Zeige Lautstärke im Fenster	
	MixerVolDownUpKeysL = mixer.mastervol[0]*100;
	MixerVolDownUpKeysR = mixer.mastervol[1]*100;
	MixerVolMuteSaveL   = MixerVolDownUpKeysL;
	MixerVolMuteSaveR   = MixerVolDownUpKeysR;
	
	/* Start the Mixer using SDL Sound at 22 khz */
	SDL_AudioSpec spec;
	SDL_AudioSpec obtained;

	spec.freq=mixer.freq;
	spec.format=AUDIO_S16LSB;
	spec.channels=2;
	spec.callback=MIXER_CallBack;
	spec.userdata=NULL;
	spec.samples=(Uint16)mixer.blocksize;

	mixer.tick_remain=0;
	mixer.tick_counter=0;
	if (mixer.nosound) {
		LOG_MSG("MIXER: No Sound Mode Selected.");
		mixer.tick_add=calc_tickadd(mixer.freq);
		TIMER_AddTickHandler(MIXER_Mix_NoSound);
	} else if (SDL_OpenAudio(&spec, &obtained) <0 ) {
		mixer.nosound = true;
		LOG_MSG("MIXER: Can't open audio: %s , running in nosound mode.",SDL_GetError());
		mixer.tick_add=calc_tickadd(mixer.freq);
		TIMER_AddTickHandler(MIXER_Mix_NoSound);
	} else {
		if ((mixer.freq != static_cast<Bit32u>(obtained.freq)) || (mixer.blocksize != obtained.samples))
			LOG_MSG("MIXER: Got different values from SDL: freq %d, blocksize %d",obtained.freq,obtained.samples);
		
			mixer.freq=(unsigned int)obtained.freq;
			mixer.blocksize=obtained.samples;
		if (mixer.sampleaccurate) {
			/* mixer.tick_add=1 << MIXER_SHIFT; /* one sample per tick */
			mixer.tick_add=calc_tickadd(mixer.freq);
			TIMER_AddTickHandler(MIXER_Mix);			
			/* PIC_AddEvent(MIXER_MixPICEvent,1000.0 / mixer.freq,0); */
		}
		else {			
			mixer.tick_add=calc_tickadd(mixer.freq);
			TIMER_AddTickHandler(MIXER_Mix);
		}
			SDL_PauseAudio(0);
	}
	
	mixer.work_wrap = MIXER_BUFSIZE;

	if (mixer.work_wrap <= mixer.blocksize) E_Exit("blocksize too large");

    {
		mixer.tick_counter = (mixer.freq % 125) ? TICK_NEXT : 0;

        int ms = section->Get_int("prebuffer");
	
		if (ms < 0){
			ms = 25;	
		} 
			  
		if ( (ms <= 1000) && ( mixer.blocksize == 512) ){
			LOG_MSG("MIXER B: %d",ms);
	
			// More for Windows 95/98 Settings. Its need a small Prebuffer and Small Blocksize
			mixer.min_needed = (unsigned int)ms;
			mixer.max_needed = (unsigned int)(mixer.freq * mixer.blocksize);	
			mixer.needed	 = mixer.min_needed * (mixer.freq % 1000)/2;				
			
		} else {
					
			//mixer.min_needed=section->Get_int("prebuffer");
			mixer.min_needed=ms;
			if (mixer.min_needed>10000u) {
				mixer.min_needed=10000u;	
			}
			
			mixer.min_needed=((unsigned int)mixer.min_needed * (unsigned int)mixer.freq)/1000u;
	
			mixer.max_needed=mixer.blocksize * 2 + 2 * mixer.min_needed;
			mixer.needed=mixer.min_needed+1;			
		}	
    }
	
			LOG_MSG("MIXER: Frequenzy    : %d",mixer.freq);	
			LOG_MSG("MIXER: Needed       : %d",mixer.needed);	
			LOG_MSG("MIXER: Needed (Min) : %d",mixer.min_needed);
			LOG_MSG("MIXER: Needed (Max) : %d\n\n",mixer.max_needed);
			
	if ( (section->Get_bool("UseMediaKeys") == false ) ){
		MAPPER_AddHandler(MIXER_VolDown,MK_f9,MMOD2,"volumedown","Vol Down");
		MAPPER_AddHandler(MIXER_VolUp,MK_f10,MMOD2,"volumeup","Vol Up");
	}else{
		/* Volume Media Keys */
		MAPPER_AddHandler(MIXER_VolDown,MK_ACVD,0,"volumedown","Vol Down");
		MAPPER_AddHandler(MIXER_VolUp,MK_ACVU,0,"volumeup","Vol Up");
		MAPPER_AddHandler(MIXER_VolMute,MK_ACVM,0,"volumemute","Vol Mute");	
	}	
	/* TODO
	MAPPER_AddHandler(MIXER_VolDownCD, MK_f10,MMOD2, "volumedowncd", "Vol Down (CD)");
	MAPPER_AddHandler(MIXER_VolUpCD, MK_f10, MMOD1|MMOD2, "volumeupcd", "Vol Up (CD)");
	*/
	PROGRAMS_MakeFile("MIXER.COM",MIXER_ProgramStart);
}
