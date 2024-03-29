/*
 *  Copyright (C) 2002-2013  The DOSBox Team
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

#include <math.h>
#include <string.h>
#include "dosbox.h"
#include "inout.h"
#include "mixer.h"
#include "mem.h"
#include "setup.h"
#include "pic.h"
#include "dma.h"
#include "mame/sn76496_Ps1.h"

extern bool PS1AudioCard;
#define DAC_CLOCK 1000000
// 950272?
#define MAX_OUTPUT 0x7fff
#define STEP 0x10000

// Powers of 2 please!
#define FIFOSIZE		2048
#define FIFOSIZE_MASK	( FIFOSIZE - 1 )

#define FIFO_NEARLY_EMPTY_VAL	128
#define FIFO_NEARLY_FULL_VAL	( FIFOSIZE - 128 )

#define FRAC_SHIFT		12		// Fixed precision.

// Nearly full and half full flags (somewhere) on the SN74V2x5/IDT72V2x5 datasheet (just guessing on the hardware).
#define FIFO_HALF_FULL		0x00
#define FIFO_NEARLY_FULL	0x00

#define FIFO_READ_AVAILABLE	0x10	// High when the interrupt can't do anything but wait (cleared by reading 0200?).
#define FIFO_FULL			0x08	// High when we can't write any more.
#define FIFO_EMPTY			0x04	// High when we can write direct values???
#define FIFO_NEARLY_EMPTY	0x02	// High when we can write more to the FIFO (or, at least, there are 0x700 bytes free).
#define FIFO_IRQ			0x01	// High when IRQ was triggered by the DAC?

std::string nCurrent_Ps1SND = "";

struct PS1AUDIO
{
	// Native stuff.
	MixerChannel * chanDAC;
	MixerChannel * chanSN;
	bool enabledDAC;
	bool enabledSN;
	Bitu last_writeDAC;
	Bitu last_writeSN;
	int SampleRate;

	// SN76496.
	struct SN76496 sn;

	// "DAC".
	Bit8u FIFO[FIFOSIZE];
	Bit16u FIFO_RDIndex;
	Bit16u FIFO_WRIndex;
	bool Playing;
	bool CanTriggerIRQ;
	Bit32u Rate;
	Bitu RDIndexHi;			// FIFO_RDIndex << FRAC_SHIFT
	Bitu Adder;				// Step << FRAC_SHIFT
	Bitu Pending;			// Bytes to go << FRAC_SHIFT

	// Regs.
	Bit8u Status;		// 0202 RD
	Bit8u Command;		// 0202 WR / 0200 RD
	Bit8u Data;			// 0200 WR
	Bit8u Divisor;		// 0203 WR
	Bit8u Unknown;		// 0204 WR (Reset?)
};

static struct PS1AUDIO ps1;

static Bit8u PS1SOUND_CalcStatus(void)
{
	Bit8u Status = ps1.Status & FIFO_IRQ;
	if( !ps1.Pending ) {
		Status |= FIFO_EMPTY;
	}
	if( ( ps1.Pending < ( FIFO_NEARLY_EMPTY_VAL << FRAC_SHIFT ) ) && ( ( ps1.Command & 3 ) == 3 ) ) {
		Status |= FIFO_NEARLY_EMPTY;
	}
	if( ps1.Pending > ( ( FIFOSIZE - 1 ) << FRAC_SHIFT ) ) {
//	if( ps1.Pending >= ( ( FIFOSIZE - 1 ) << FRAC_SHIFT ) ) { // OK
		// Should never be bigger than FIFOSIZE << FRAC_SHIFT...?
		Status |= FIFO_FULL;
	}
	if( ps1.Pending > ( FIFO_NEARLY_FULL_VAL << FRAC_SHIFT ) ) {
		Status |= FIFO_NEARLY_FULL;
	}
	if( ps1.Pending >= ( ( FIFOSIZE >> 1 ) << FRAC_SHIFT ) ) {
		Status |= FIFO_HALF_FULL;
	}
	return Status;
}

static void PS1DAC_Reset(bool bTotal)
{
	PIC_DeActivateIRQ( 7 );
	ps1.Data = 0x80;
	memset( ps1.FIFO, 0x80, FIFOSIZE );
	ps1.FIFO_RDIndex = 0;
	ps1.FIFO_WRIndex = 0;
	if( bTotal ) ps1.Rate = 0xFFFFFFFF;
	ps1.RDIndexHi = 0;
	if( bTotal ) ps1.Adder = 0;	// Be careful with this, 5 second timeout and Space Quest 4!
	ps1.Pending = 0;
	ps1.Status = PS1SOUND_CalcStatus();
	ps1.Playing = true;
	ps1.CanTriggerIRQ = false;
}


#include "regs.h"
static void PS1SOUNDWrite(Bitu port,Bitu data,Bitu iolen) {
	(void)iolen;//UNUSED
	if( port != 0x0205 ) {
		ps1.last_writeDAC=PIC_Ticks;
		if (!ps1.enabledDAC) {
			ps1.chanDAC->Enable(true);
			ps1.enabledDAC=true;
		}
	}
	else
	{
		ps1.last_writeSN=PIC_Ticks;
		if (!ps1.enabledSN) {
			ps1.chanSN->Enable(true);
			ps1.enabledSN=true;
		}
	}

#if C_DEBUG != 0
	if( ( port != 0x0205 ) && ( port != 0x0200 ) )
		LOG(LOG_PS1,LOG_NORMAL)("PS1 WR %04X,%02X (%04X:%08X)",(int)port,(int)data,(int)SegValue(cs),(int)reg_eip);
#endif
	switch(port)
	{
		case 0x0200:
			// Data - insert into FIFO.
			ps1.Data = (Bit8u)data;
			ps1.Status = PS1SOUND_CalcStatus();
			if( !( ps1.Status & FIFO_FULL ) )
			{
				ps1.FIFO[ ps1.FIFO_WRIndex++ ]=(Bit8u)data;
				ps1.FIFO_WRIndex &= FIFOSIZE_MASK;
				ps1.Pending += ( 1 << FRAC_SHIFT );
				if( ps1.Pending > ( FIFOSIZE << FRAC_SHIFT ) ) {
					ps1.Pending = FIFOSIZE << FRAC_SHIFT;
				}
			}
			break;
		case 0x0202:
			// Command.
			ps1.Command = (Bit8u)data;
			if( data & 3 ) ps1.CanTriggerIRQ = true;
//			switch( data & 3 )
//			{
//				case 0: // Stop?
//					ps1.Adder = 0;
//					break;
//			}
			break;
		case 0x0203:
			{
				// Clock divisor (maybe trigger first IRQ here).
				ps1.Divisor = (Bit8u)data;
				ps1.Rate = (Bit32u)( DAC_CLOCK / ( data + 1 ) );
				// 22050 << FRAC_SHIFT / 22050 = 1 << FRAC_SHIFT
				ps1.Adder = ( ps1.Rate << FRAC_SHIFT ) / (unsigned int)ps1.SampleRate;
				if( ps1.Rate > 22050 )
				{
//					if( ( ps1.Command & 3 ) == 3 ) {
//						LOG_MSG("Attempt to set DAC rate too high (%dhz).",ps1.Rate);
//					}
					//ps1.Divisor = 0x2C;
					//ps1.Rate = 22050;
					//ps1.Adder = 0;	// Not valid.
				}
				ps1.Status = PS1SOUND_CalcStatus();
				if( ( ps1.Status & FIFO_NEARLY_EMPTY ) && ( ps1.CanTriggerIRQ ) )
				{
					// Generate request for stuff.
					ps1.Status |= FIFO_IRQ;
					ps1.CanTriggerIRQ = false;
					PIC_ActivateIRQ( 7 );
				}
			}
			break;
		case 0x0204:
			// Reset? (PS1MIC01 sets it to 08 for playback...)
			ps1.Unknown = (Bit8u)data;
			if( !data )
				PS1DAC_Reset(true);
			break;
		case 0x0205:
			SN76496Write(&ps1.sn,port,data);
			break;
		default:break;
	}
}

static Bitu PS1SOUNDRead(Bitu port,Bitu iolen) {
	(void)iolen;//UNUSED
	ps1.last_writeDAC=PIC_Ticks;
	if (!ps1.enabledDAC) {
		ps1.chanDAC->Enable(true);
		ps1.enabledDAC=true;
	}
#if C_DEBUG != 0
	LOG(LOG_PS1,LOG_NORMAL)("PS1 RD %04X (%04X:%08X)",(int)port,(int)SegValue(cs),(int)reg_eip);
#endif
	switch(port)
	{
		case 0x0200:
			// Read last command.
			ps1.Status &= ~FIFO_READ_AVAILABLE;
			return ps1.Command;
		case 0x0202:
			{
//				LOG_MSG("PS1 RD %04X (%04X:%08X)",port,SegValue(cs),reg_eip);

				// Read status / clear IRQ?.
				Bit8u Status = ps1.Status = PS1SOUND_CalcStatus();
// Don't do this until we have some better way of detecting the triggering and ending of an IRQ.
//				ps1.Status &= ~FIFO_IRQ;
				return Status;
			}
		case 0x0203:
			// Stunt Island / Roger Rabbit 2 setup.
			return ps1.Divisor;
		case 0x0205:
		case 0x0206:
			// Bush Buck detection.
			return 0;
		default:break;
	}
	return 0xFF;
}

static void PS1SOUNDUpdate(Bitu length)
{
	if ((ps1.last_writeDAC+5000)<PIC_Ticks) {
		ps1.enabledDAC=false;
		ps1.chanDAC->Enable(false);
		// Excessive?
		PS1DAC_Reset(false);
	}
	Bit8u * buffer=(Bit8u *)MixTemp;

	Bits pending = 0;
	Bitu add = 0;
	Bitu pos=ps1.RDIndexHi;
	Bitu count=length;

	if( ps1.Playing )
	{
		ps1.Status = PS1SOUND_CalcStatus();
		pending = (Bits)ps1.Pending;
		add = ps1.Adder;
		if( ( ps1.Status & FIFO_NEARLY_EMPTY ) && ( ps1.CanTriggerIRQ ) )
		{
			// More bytes needed.

			//PIC_AddEvent( ??, ??, ?? );
			ps1.Status |= FIFO_IRQ;
			ps1.CanTriggerIRQ = false;
			PIC_ActivateIRQ( 7 );
		}
	}

	while (count)
	{
		unsigned int out;

		if( pending <= 0 ) {
			pending = 0;
			while( count-- ) *(buffer++) = 0x80;	// Silence.
			break;
			//pos = ( ( ps1.FIFO_RDIndex - 1 ) & FIFOSIZE_MASK ) << FRAC_SHIFT;	// Stay on last byte.
		}
		else
		{
			out = ps1.FIFO[ pos >> FRAC_SHIFT ];
			pos += add;
			pos &= ( ( FIFOSIZE << FRAC_SHIFT ) - 1 );
			pending -= (Bits)add;
		}

		*(buffer++) = out;
		count--;
	}
	// Update positions and see if we can clear the FIFO_FULL flag.
	ps1.RDIndexHi = pos;
//	if( ps1.FIFO_RDIndex != ( pos >> FRAC_SHIFT ) ) ps1.Status &= ~FIFO_FULL;
	ps1.FIFO_RDIndex = (Bit16u)(pos >> FRAC_SHIFT);
	if( pending < 0 ) pending = 0;
	ps1.Pending = (Bitu)pending;

	ps1.chanDAC->AddSamples_m8(length,MixTemp);
}

static void PS1SN76496Update(Bitu length)
{
	if ((ps1.last_writeSN+5000)<PIC_Ticks) {
		ps1.enabledSN=false;
		ps1.chanSN->Enable(false);
	}

	Bit16s * buffer=(Bit16s *)MixTemp;
	SN76496Update(&ps1.sn,buffer,length);
	ps1.chanSN->AddSamples_m16(length,(Bit16s *)MixTemp);
}

#include "regs.h"
//static void PS1SOUND_Write(Bitu port,Bitu data,Bitu iolen) {
//	LOG_MSG("Write PS1 dac %X val %X (%04X:%08X)",port,data,SegValue(cs),reg_eip);
//}

class PS1SOUND: public Module_base {
private:
	IO_ReadHandleObject ReadHandler[2];
	IO_WriteHandleObject WriteHandler[2];
	MixerObject MixerChanDAC, MixerChanSN;
public:
	PS1SOUND(Section* configuration):Module_base(configuration){
				
		Section_prop * section=static_cast<Section_prop *>(configuration);
		
		PS1AudioCard= useSoundPS1 = false;


		if ((strcmp(section->Get_string("ps1audio"),"true")!=0) &&
			(strcmp(section->Get_string("ps1audio"),"on")!=0)   &&
			(strcmp(section->Get_string("ps1audio"),"auto")!=0))
				return;

		PS1AudioCard = useSoundPS1 = true;
		nCurrent_Ps1SND = "IBM PS/1 Audio Card (SN76496)";
		PS1AudioCard	= useSoundPS1 = true;

		LOG_MSG("PS/1: Sound Emulation Enabled");

		// Ports 0x0200-0x0205 (let normal code handle the joystick at 0x0201).
		ReadHandler[0].Install(0x200,&PS1SOUNDRead,IO_MB);
		ReadHandler[1].Install(0x202,&PS1SOUNDRead,IO_MB,6); //5); //3);

		WriteHandler[0].Install(0x200,PS1SOUNDWrite,IO_MB);
		WriteHandler[1].Install(0x202,PS1SOUNDWrite,IO_MB,4);

		Bit32u sample_rate = (Bit32u)section->Get_int("ps1audiorate");
		LOG_MSG("PS/1: + Rate : %dhz", sample_rate);
		ps1.chanDAC=MixerChanDAC.Install(&PS1SOUNDUpdate,sample_rate,"PS1 DAC");
		ps1.chanSN=MixerChanSN.Install(&PS1SN76496Update,sample_rate,"PS1 SN76496");

		ps1.SampleRate=(Bit32u)sample_rate;
		ps1.enabledDAC=false;
		ps1.enabledSN=false;
		ps1.last_writeDAC = 0;
		ps1.last_writeSN = 0;
		PS1DAC_Reset(true);
		Bit32u PS1Clock = 4000000;

		PS1Clock = (Bit32u)section->Get_int("ps1audioclock");
		if ( PS1Clock >= 1 )		
		{
			/*
				> Jmk wrote:
				> Judging by what I've read in that technical document, it looks like
				  the sound chip is fed by a 4 Mhz clock instead of a ~3.5 Mhz clock.
				> 
				> So, there's a line in ps1_sound.cpp that looks like this:
				> SN76496Reset( &ps1.sn, 3579545, sample_rate );
				> 
				> Instead, it should look like this:
				> SN76496Reset( &ps1.sn, 4000000, sample_rate );
				> 
				> That should fix it! Mind you, that was with the old code
				 (it was 0.72 I worked with) which may have been updated since,
				 but the same principle applies.

				* NTS: I do not have anything to test this change! --J.C.
				* SN76496Reset( &ps1.sn, 3579545, sample_rate );
				* SN76496Reset( &ps1.sn, 4000000, sample_rate );
			*/			
			SN76496Reset( &ps1.sn, PS1Clock, sample_rate );
			if 		( PS1Clock == 3579545 )
				LOG_MSG("PS/1: + Clock: ~3.5Mhz");
			
			else if ( PS1Clock == 4000000 )
				LOG_MSG("PS/1: + Clock: 4.0Mhz");			
			
			else
				LOG_MSG("PS/1: + Clock: %dMhz clock (User Settings)",PS1Clock);
		}
		else
		{
			SN76496Reset( &ps1.sn, 4000000, sample_rate );
			LOG_MSG("PS/1: + Clock: 4.0Mhz");			
		}

	}
	~PS1SOUND(){ }
};

static PS1SOUND* test;

void PS1SOUND_ShutDown(Section* sec) {
	delete test;	
}

void PS1SOUND_Init(Section* sec) {
	LOG(LOG_PS1,LOG_NORMAL)("PS/1: Initializing IBM PS/1 Sound Emulation");
	test = new PS1SOUND(sec);	
	sec->AddDestroyFunction(&PS1SOUND_ShutDown,true);
}



/*
ykhwong svn-daum 2012-02-20
static globals:
struct PS1AUDIO ps1
	// - static 'new' ptr
	MixerChannel * chanDAC;
	MixerChannel * chanSN;
	// - pure data
	bool enabledDAC;
	bool enabledSN;
	Bitu last_writeDAC;
	Bitu last_writeSN;
	int SampleRate;
	// - pure struct data
	struct SN76496 sn;
	// - pure data
	Bit8u FIFO[FIFOSIZE];
	Bit16u FIFO_RDIndex;
	Bit16u FIFO_WRIndex;
	bool Playing;
	bool CanTriggerIRQ;
	Bit32u Rate;
	Bitu RDIndexHi;
	Bitu Adder;
	Bitu Pending;
	// - pure data
	Bit8u Status;
	Bit8u Command;
	Bit8u Data;
	Bit8u Divisor;
	Bit8u Unknown;
// - static 'new' ptr
static PS1SOUND* test;
	// - static data
	IO_ReadHandleObject ReadHandler[2];
	IO_WriteHandleObject WriteHandler[2];
	MixerObject MixerChanDAC, MixerChanSN;
*/