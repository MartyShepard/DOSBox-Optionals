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
 *  You should have received a copy of the GNU General Public License along along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <string.h>
#include <math.h>
#include "dosbox.h"
#include "inout.h"
#include "setup.h"
#include "joystick.h"
#include "pic.h"
#include "support.h"


//TODO: higher axis can't be mapped. Find out why again

//Set to true, to enable automated switching back to square on circle mode if the inputs are outside the cirle.
#define SUPPORT_MAP_AUTO 0

#define RANGE 64
#define TIMEOUT 10

#define OHMS 120000/2
#define JOY_S_CONSTANT 0.0000242
#define S_PER_OHM 0.000000011

struct JoyStick {
	enum {JOYMAP_SQUARE,JOYMAP_CIRCLE,JOYMAP_INBETWEEN} mapstate;
	bool enabled;
	float xpos, ypos; //position as set by SDL.
	double xtick, ytick;
	Bitu xcount, ycount;
	bool button[2];
	int deadzone; //Deadzone (value between 0 and 100) interpreted as percentage.
	bool transformed; //Whether xpos,ypos have been converted to xfinal and yfinal. Cleared when new xpos orypos have been set
	float xfinal, yfinal; //position returned to the game for stick 0. 

	void clip() {
		if (xfinal > 1.0) xfinal = 1.0;
		else if (xfinal < -1.0) xfinal = -1.0;
		if (yfinal > 1.0) yfinal = 1.0;
		else if (yfinal < -1.0) yfinal = -1.0;
	}

	void fake_digital() {
		if (xpos > 0.5f) xfinal = 1.0f;
		else if (xpos < -0.5f) xfinal = -1.0f;
		else xfinal = 0.0f;
		if (ypos > 0.5f) yfinal = 1.0f;
		else if (ypos < -0.5f) yfinal = -1.0f;
		else yfinal = 0.0f;
	}

	void transform_circular(){
		float r = sqrtf(xpos * xpos + ypos * ypos);
		if (r == 0.0) {xfinal = xpos; yfinal = ypos; return;}
		float deadzone_f = deadzone / 100.0f;
		float s = 1.0f - deadzone_f;
		if (r < deadzone_f) {
			xfinal = yfinal = 0.0f;
			return;
		}

		float deadzonescale = (r - deadzone_f) / s; //r if deadzone=0;
		float xa = fabsf(xpos);
		float ya = fabsf(ypos);
		float maxpos = (ya>xa?ya:xa);
		xfinal = xpos * deadzonescale/maxpos;
		yfinal = ypos * deadzonescale/maxpos;
	}

	void transform_square() {
		float deadzone_f = deadzone / 100.0f;
		float s = 1.0f - deadzone_f;

		if (xpos > deadzone_f) {
			xfinal = (xpos - deadzone_f)/ s;
		} else if ( xpos < -deadzone_f) {
			xfinal = (xpos + deadzone_f) / s;
		} else xfinal = 0.0f;
		if (ypos > deadzone_f) {
			yfinal = (ypos - deadzone_f)/ s;
		} else if ( ypos < - deadzone_f) {
			yfinal = (ypos + deadzone_f) / s;
		} else yfinal = 0.0f;
	}

#if SUPPORT_MAP_AUTO
	void transform_inbetween(){
		//First transform to a circle and crop the values to -1.0 -> 1.0
		//then keep on doing this in future calls until it is safe to switch square mapping
		// safe = 0.95 as ratio  for both axis, or in deadzone
		transform_circular();
		clip();


		float xrate = xpos / xfinal;
		float yrate = ypos / yfinal;
		if (xrate > 0.95 && yrate > 0.95) {
			mapstate = JOYMAP_SQUARE; //TODO misschien xfinal=xpos...
			//LOG_MSG("switched to square %f %f",xrate,yrate);
		}
	}
#endif
	void transform_input(){
		if (transformed) return;
		transformed = true;
		if (deadzone == 100) fake_digital();
		else {
			if (mapstate == JOYMAP_SQUARE) transform_square();
			else if (mapstate == JOYMAP_CIRCLE) transform_circular();
#if SUPPORT_MAP_AUTO
			if (mapstate ==  JOYMAP_INBETWEEN) transform_inbetween(); //No else here
#endif
			clip();
		}
	}


};

JoystickType joytype;
HostControlType HostCtrlType;
static JoyStick stick[3];
static bool JoystickInvertY[4];

static Bitu last_write = 0;
static bool write_active = false;
static bool swap34 = false;
bool button_wrapping_enabled = true;

extern bool autofire; //sdl_mapper.cpp
extern int  nJoyButtons;

/*
* Joystick Invert Patch
*/


static Bitu read_p201(Bitu port,Bitu iolen) {
	/* Reset Joystick to 0 after TIMEOUT ms */
	if(write_active && ((PIC_Ticks - last_write) > TIMEOUT)) {
		write_active = false;
		stick[0].xcount = 0;
		stick[1].xcount = 0;
		stick[0].ycount = 0;
		stick[1].ycount = 0;		
		
		/* Joystick Support for Saitek Flight Systems with more Axes and Buttons */
		if (joytype == JOY_CHVP )
		{
			stick[2].xcount = 0;
			stick[2].ycount = 0;			
		}
//		LOG_MSG("reset by time %d %d",PIC_Ticks,last_write);
	}

	/**  Format of the byte to be returned:       
	**                        | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	**                        +-------------------------------+
	**                          |   |   |   |   |   |   |   |
	**  Joystick B, Button 2 ---+   |   |   |   |   |   |   +--- Joystick A, X Axis
	**  Joystick B, Button 1 -------+   |   |   |   |   +------- Joystick A, Y Axis
	**  Joystick A, Button 2 -----------+   |   |   +----------- Joystick B, X Axis
	**  Joystick A, Button 1 ---------------+   +--------------- Joystick B, Y Axis
	**/
	Bit8u ret=0xff;
	if (stick[0].enabled) {
		if (stick[0].xcount) stick[0].xcount--; else ret&=~1;
		if (stick[0].ycount) stick[0].ycount--; else ret&=~2;
		if (stick[0].button[0]) ret&=~16;
		if (stick[0].button[1]) ret&=~32;
	}
	if (stick[1].enabled) {
		if (stick[1].xcount) stick[1].xcount--; else ret&=~4;		
		if (stick[1].ycount) stick[1].ycount--; else ret&=~8;		
		if (stick[1].button[0]) ret&=~64;
		if (stick[1].button[1]) ret&=~128;
	}
	
	/* Joystick Support for Saitek Flight Systems with more Axes and Buttons */
	if ( (stick[2].enabled) && (joytype == JOY_CHVP) )
	{
		if (stick[2].xcount) stick[2].xcount--; else ret&=~4;
		if (stick[2].ycount) stick[2].ycount--; else ret&=~8;	
		if (stick[2].button[0]) ret&=~64;
		if (stick[2].button[1]) ret&=~128;
	}							
	return ret;
}

static Bitu read_p201_timed(Bitu port,Bitu iolen) {
	Bit8u ret=0xff;
	double currentTick = PIC_FullIndex();
	if( stick[0].enabled ){
		if( stick[0].xtick < currentTick ) ret &=~1;
		if( stick[0].ytick < currentTick ) ret &=~2;
	}
	if( stick[1].enabled ){
		if( stick[1].xtick < currentTick ) ret &=~4;
		if( stick[1].ytick < currentTick ) ret &=~8;
	}
	
	/* Joystick Support for Saitek Flight Systems with more Axes and Buttons */	
	if ( ( stick[2].enabled ) && (joytype == JOY_CHVP) )
	{
		if( stick[2].xtick < currentTick ) ret &=~4;
		if( stick[2].ytick < currentTick ) ret &=~8;
	}				

	if (stick[0].enabled) {
		if (stick[0].button[0]) ret&=~16;
		if (stick[0].button[1]) ret&=~32;
	}
	if (stick[1].enabled) {
		if (stick[1].button[0]) ret&=~64;
		if (stick[1].button[1]) ret&=~128;
	}
	
	/* Joystick Support for Saitek Flight Systems with more Axes and Buttons */	
	if ( ( stick[2].enabled ) && (joytype == JOY_CHVP) )
	{
		if (stick[2].button[0]) ret&=~64;
		if (stick[2].button[1]) ret&=~128;
	}			
	return ret;
}

static void write_p201(Bitu port,Bitu val,Bitu iolen) {
	/* Store writetime index */
	write_active = true;
	last_write = PIC_Ticks;
	if (stick[0].enabled) {
		stick[0].transform_input();
		if (JoystickInvertY[0])
		{
			stick[0].ycount = (Bitu)(((-stick[0].yfinal) * RANGE) + RANGE);
		}
		else
		{
			stick[0].ycount = (Bitu)((stick[0].yfinal * RANGE) + RANGE);
		}
		stick[0].xcount=(Bitu)((stick[0].xfinal*RANGE)+RANGE);	
		
	}
	if (stick[1].enabled) {
		stick[1].xcount = (Bitu)(((swap34 ? stick[1].ypos : stick[1].xpos) * RANGE) + RANGE);
		if (JoystickInvertY[1])
		{
			stick[1].ycount = (Bitu)(((swap34 ? stick[1].xpos : -stick[1].ypos) * RANGE) + RANGE);
		}
		else
		{
			stick[1].ycount = (Bitu)(((swap34 ? stick[1].xpos : stick[1].ypos) * RANGE) + RANGE);
		}
		
	}
	
	/* Joystick Support for Saitek Flight Systems with more Axes and Buttons */	
	if ( ( stick[2].enabled ) && (joytype == JOY_CHVP) )
	{
		stick[2].xcount=(Bitu)(((swap34? stick[2].ypos : stick[2].xpos)*RANGE)+RANGE);
		if (JoystickInvertY[2])
		{
			stick[2].ycount = (Bitu)(((swap34 ? stick[2].xpos : -stick[2].ypos) * RANGE) + RANGE);
		}
		else
		{
			stick[2].ycount = (Bitu)(((swap34 ? stick[2].xpos : stick[2].ypos) * RANGE) + RANGE);
		}

	}																		 

}
static void write_p201_timed(Bitu port,Bitu val,Bitu iolen) {
	// Axes take time = 24.2 microseconds + ( 0.011 microsecons/ohm * resistance )
	// to reset to 0
	// Pre-calculate the time at which each axis hits 0 here
	double currentTick = PIC_FullIndex();
	if (stick[0].enabled) {
		stick[0].transform_input();
		if (JoystickInvertY[0])
		{
			stick[0].ytick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
				(double)(((-stick[0].yfinal + 1.0) * OHMS)));
		}
		else
		{
			stick[0].ytick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
				(double)(((stick[0].yfinal + 1.0) * OHMS)));
		}

		stick[0].xtick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
			(double)(((stick[0].xfinal + 1.0) * OHMS)));


	}
	if (stick[1].enabled) {
		if (JoystickInvertY[1])
		{
			stick[1].ytick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
				(double)((swap34 ? stick[1].xpos : stick[1].ypos) + 1.0) * OHMS);
		}
		else
		{
			stick[1].ytick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
				(double)((swap34 ? stick[1].xpos : -stick[1].ypos) + 1.0) * OHMS);
		}
		stick[1].xtick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
			(double)((swap34 ? stick[1].ypos : stick[1].xpos) + 1.0) * OHMS);

	}
	/* Joystick Support for Saitek Flight Systems with more Axes and Buttons
	   Swap Axis Support. By Axis 3/4 (Axis 4(Y) is Throttle, Axis 3(x) is Left and Right Strafe) 
	*/	
	 
	if ( ( stick[2].enabled ) && (joytype == JOY_CHVP) )
	{
		stick[2].transform_input();
		if (JoystickInvertY[2])
		{
			stick[2].ytick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
				(double)((swap34 ? -stick[2].ypos : stick[1].xpos) + 1.0) * OHMS);
		}
		else
		{
			stick[2].ytick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
				(double)((swap34 ? stick[2].ypos : stick[1].xpos) + 1.0) * OHMS);
		}
		stick[2].xtick = currentTick + 1000.0 * (JOY_S_CONSTANT + S_PER_OHM *
			(double)((swap34 ? stick[2].ypos : stick[1].xpos) + 1.0) * OHMS);


	}
}

void JOYSTICK_Enable(Bitu which,bool enabled) {
	if (which<2) stick[which].enabled = enabled;
}

void JOYSTICK_Button(Bitu which,Bitu num,bool pressed) {
	//if ((which<2) && (num<2)) 
		stick[which].button[num] = pressed;
	
}

void JOYSTICK_Move_X(Bitu which,float x) {
	if(which > 1) return;
	if (stick[which].xpos == x) return;
	stick[which].xpos = x;
	stick[which].transformed = false;
//	if( which == 0 || joytype != JOY_FCS)  
//		stick[which].applied_conversion; //todo 
}

void JOYSTICK_Move_Y(Bitu which,float y) {
	if(which > 1) return;
	if (stick[which].ypos == y) return;
	stick[which].ypos = y;
	stick[which].transformed = false;
}

bool JOYSTICK_IsEnabled(Bitu which) {
	if (which<3) return stick[which].enabled;
	return false;
}

bool JOYSTICK_GetButton(Bitu which, Bitu num) {
	if ((which<3) && (num<2)) return stick[which].button[num];
	return false;
}

float JOYSTICK_GetMove_X(Bitu which) {
	if (which > 1) return 0.0f;
	if (which == 0) { stick[0].transform_input(); return stick[0].xfinal;}
	return stick[1].xpos;
}

float JOYSTICK_GetMove_Y(Bitu which) {
	if (which > 1) return 0.0f;
	if (which == 0) { stick[0].transform_input(); return stick[0].yfinal;}
	return stick[1].ypos;
}

class JOYSTICK:public Module_base{
private:
	IO_ReadHandleObject ReadHandler;
	IO_WriteHandleObject WriteHandler;
public:
	JOYSTICK(Section* configuration):Module_base(configuration){
		Section_prop * section = static_cast<Section_prop *>(configuration);
		const char * type = section->Get_string("joysticktype");
		if (!_stricmp(type,"none"))         joytype = JOY_NONE;
		else if (!_stricmp(type,"false"))   joytype = JOY_NONE;
		else if (!_stricmp(type,"auto"))    joytype = JOY_AUTO;
		else if (!_stricmp(type,"2axis"))   joytype = JOY_2AXIS;
		else if (!_stricmp(type,"4axis"))   joytype = JOY_4AXIS;
		else if (!_stricmp(type,"4axis_2")) joytype = JOY_4AXIS_2;
		else if (!_stricmp(type,"fcs"))     joytype = JOY_FCS;
		else if (!_stricmp(type,"fcslw"))   joytype = JOY_FCSLW;
		else if (!_stricmp(type,"ch"))      joytype = JOY_CH;
		else if (!_stricmp(type,"chvp"))    joytype = JOY_CHVP;
		else if (!_stricmp(type,"chgs"))    joytype = JOY_CHGS;
		else if (!_stricmp(type,"qshot6"))  joytype = JOY_SHOT6;
		else if (!_stricmp(type,"capcom"))  joytype = JOY_CAPCOM;
		else if (!_stricmp(type,"inter6"))  joytype = JOY_INTER6;
		else if (!_stricmp(type,"fcs2"))    joytype = JOY_FCSII;
		else if (!_stricmp(type,"wheel"))    joytype = JOY_WHEEL;
		
		else joytype = JOY_AUTO;

		
		const char * host = section->Get_string("joystickhost");
		if (!_stricmp(host,"default"))      HostCtrlType = HOSTJOY_DEFAULT;
		else if (!_stricmp(host,"saitekx45"))   HostCtrlType = HOSTJOY_SAITEKX45;
		else if (!_stricmp(host,"lgex3dpro"))   HostCtrlType = HOSTJOY_LEXT3DPRO;
		else if (!_stricmp(host,"tfhotasone"))   HostCtrlType = HOSTJOY_TFLIGHTHO;
		else if (!_stricmp(host,"ldriveforcepro"))   HostCtrlType = HOSTJOY_LDRIVINGF;
		else HostCtrlType = HOSTJOY_DEFAULT;
		
		JoystickInvertY[0] = section->Get_bool("invertyaxis1");
		JoystickInvertY[1] = section->Get_bool("invertyaxis2");
		JoystickInvertY[3] = section->Get_bool("invertyaxia3");
		bool timed = section->Get_bool("timed");
		if (timed) {
			ReadHandler.Install(0x201,read_p201_timed,IO_MB);
			WriteHandler.Install(0x201,write_p201_timed,IO_MB);
		} else {
			ReadHandler.Install(0x201,read_p201,IO_MB);
			WriteHandler.Install(0x201,write_p201,IO_MB);
		}

		autofire = section->Get_bool("autofire");
		swap34 = section->Get_bool("swap34");
		button_wrapping_enabled = section->Get_bool("buttonwrap");
		stick[0].xtick = stick[0].ytick = stick[1].xtick =
		                 stick[1].ytick = PIC_FullIndex();
		stick[0].xpos = stick[0].ypos = stick[1].xpos = stick[1].ypos = 0.0f;
		stick[0].transformed = false;


		stick[0].mapstate = JoyStick::JOYMAP_SQUARE;
		bool circ = section->Get_bool("circularinput");
		if (circ) stick[0].mapstate = JoyStick::JOYMAP_CIRCLE;
		stick[0].deadzone = section->Get_int("deadzone");
		
		nJoyButtons =  section->Get_int("buttons");

		/* Force Buttons Config for Special Types */
		switch(joytype){
			case JOY_WHEEL:
			case JOY_FCSLW:
			{
				nJoyButtons = 4;
			}
			case JOY_CH:
			case JOY_SHOT6:
			case JOY_CAPCOM:
			case JOY_INTER6:
			{
				nJoyButtons = 6;
			}
			break;
			case JOY_CHVP:
			case JOY_CHGS:			
			{
				nJoyButtons = 10;
			}
			break;			
			
		}
		
	}
};
static JOYSTICK* test;

void JOYSTICK_Destroy(Section* sec) {
	delete test;
}

void JOYSTICK_Init(Section* sec) {
	test = new JOYSTICK(sec);
	sec->AddDestroyFunction(&JOYSTICK_Destroy,true); 
}
