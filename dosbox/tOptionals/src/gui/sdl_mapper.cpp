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


#include <vector>
#include <list>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


#if defined(_MSC_VER)
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_thread.h"
#include "SDL2/include/SDL_endian.h"
#else
#include "SDL2/SDL.h"
#include "SDL_thread.h"
#include <SDL_endian.h>
#endif

#include "dosbox.h"
#include "video.h"
#include "keyboard.h"
#include "joystick.h"
#include "mouse.h"
#include "support.h"
#include "mapper.h"
#include "setup.h"
#include "pic.h"
#include "control.h"

enum {
	CLR_BLACK=0,
	CLR_GREY=1,
	CLR_WHITE=2,
	CLR_RED=3,
	CLR_BLUE=4,
	CLR_GREEN=5,
	CLR_LAST
};

enum BB_Types {
	BB_Next,BB_Add,BB_Del,
	BB_Save,BB_Exit
};

enum BC_Types {
	BC_Mod1,BC_Mod2,BC_Mod3,
	BC_Hold
};

#define BMOD_Mod1 0x0001
#define BMOD_Mod2 0x0002
#define BMOD_Mod3 0x0004

#define BFLG_Hold 0x0001
#define BFLG_Repeat 0x0004


#define MAXSTICKS 8
#define MAXACTIVE 16
#define MAXBUTTON 36  // Use 36 for Android (KEYCODE_BUTTON_1..16 are mapped to SDL buttons 20..35) //#define MAXBUTTON 32
#define MAXBUTTON_CAP 16
#define MAXAXIS 10
#define MAXHAT 2

class CEvent;
class CHandlerEvent;
class CButton;
class CBind;
class CBindGroup;

static void SetActiveEvent(CEvent * event);
static void SetActiveBind(CBind * _bind);
extern Bit8u int10_font_14[256 * 14];

static std::vector<CEvent *> events;
static std::vector<CButton *> buttons;
static std::vector<CBindGroup *> bindgroups;
static std::vector<CHandlerEvent *> handlergroup;
typedef std::list<CBind *> CBindList;
typedef std::list<CEvent *>::iterator CEventList_it;
typedef std::list<CBind *>::iterator CBindList_it;
typedef std::vector<CButton *>::iterator CButton_it;
typedef std::vector<CEvent *>::iterator CEventVector_it;
typedef std::vector<CHandlerEvent *>::iterator CHandlerEventVector_it;
typedef std::vector<CBindGroup *>::iterator CBindGroup_it;

static CBindList holdlist;


class CEvent {
public:
	CEvent(char const * const _entry) {
		safe_strncpy(entry,_entry,16);
		events.push_back(this);
		bindlist.clear();
		activity=0;
		current_value=0;
	}
	void AddBind(CBind * bind);
	virtual ~CEvent() {}
	virtual void Active(bool yesno)=0;
	virtual void ActivateEvent(bool ev_trigger,bool skip_action)=0;
	virtual void DeActivateEvent(bool ev_trigger)=0;
	void DeActivateAll(void);
	void SetValue(Bits value){
		current_value=value;
	}
	Bits GetValue(void) {
		return current_value;
	}
	char * GetName(void) { return entry; }
	virtual bool IsTrigger(void)=0;
	CBindList bindlist;
protected:
	Bitu activity;
	char entry[16];
	Bits current_value;
};

/* class for events which can be ON/OFF only: key presses, joystick buttons, joystick hat */
class CTriggeredEvent : public CEvent {
public:
	CTriggeredEvent(char const * const _entry) : CEvent(_entry) {}
	virtual bool IsTrigger(void) {
		return true;
	}
	void ActivateEvent(bool ev_trigger,bool skip_action) {
		if (current_value>25000) {
			/* value exceeds boundary, trigger event if not active */
			if (!activity && !skip_action) Active(true);
			if (activity<32767) activity++;
		} else {
			if (activity>0) {
				/* untrigger event if it is fully inactive */
				DeActivateEvent(ev_trigger);
				activity=0;
			}
		}
	}
	void DeActivateEvent(bool /*ev_trigger*/) {
		activity--;
		if (!activity) Active(false);
	}
};

/* class for events which have a non-boolean state: joystick axis movement */
class CContinuousEvent : public CEvent {
public:
	CContinuousEvent(char const * const _entry) : CEvent(_entry) {}
	virtual bool IsTrigger(void) {
		return false;
	}
	void ActivateEvent(bool ev_trigger,bool skip_action) {
		if (ev_trigger) {
			activity++;
			if (!skip_action) Active(true);
		} else {
			/* test if no trigger-activity is present, this cares especially
			   about activity of the opposite-direction joystick axis for example */
			if (!GetActivityCount()) Active(true);
		}
	}
	void DeActivateEvent(bool ev_trigger) {
		if (ev_trigger) {
			if (activity>0) activity--;
			if (activity==0) {
				/* test if still some trigger-activity is present,
				   adjust the state in this case accordingly */
				if (GetActivityCount()) RepostActivity();
				else Active(false);
			}
		} else {
			if (!GetActivityCount()) Active(false);
		}
	}
	virtual Bitu GetActivityCount(void) {
		return activity;
	}
	virtual void RepostActivity(void) {}
};

class CBind {
public:
	virtual ~CBind () {
		list->remove(this);
//		event->bindlist.remove(this);
	}
	CBind(CBindList * _list) {
		list=_list;
		_list->push_back(this);
		mods=flags=0;
		event=0;
		active=holding=false;
	}
	void AddFlags(char * buf) {
		if (mods & BMOD_Mod1) strcat(buf," mod1");
		if (mods & BMOD_Mod2) strcat(buf," mod2");
		if (mods & BMOD_Mod3) strcat(buf," mod3");
		if (flags & BFLG_Hold) strcat(buf," hold");
	}
	void SetFlags(char * buf) {
		char * word;
		while (*(word=StripWord(buf))) {
			if (!_stricmp(word,"mod1")) mods|=BMOD_Mod1;
			if (!_stricmp(word,"mod2")) mods|=BMOD_Mod2;
			if (!_stricmp(word,"mod3")) mods|=BMOD_Mod3;
			if (!_stricmp(word,"hold")) flags|=BFLG_Hold;
		}
	}
	void ActivateBind(Bits _value,bool ev_trigger,bool skip_action=false) {
		if (event->IsTrigger()) {
			/* use value-boundary for on/off events */
			if (_value>25000) {
				event->SetValue(_value);
				if (active) return;
				event->ActivateEvent(ev_trigger,skip_action);
				active=true;
			} else {
				if (active) {
					event->DeActivateEvent(ev_trigger);
					active=false;
				}
			}
		} else {
			/* store value for possible later use in the activated event */
			event->SetValue(_value);
			event->ActivateEvent(ev_trigger,false);
		}
	}
	void DeActivateBind(bool ev_trigger) {
		if (event->IsTrigger()) {
			if (!active) return;
			active=false;
			if (flags & BFLG_Hold) {
				if (!holding) {
					holdlist.push_back(this);
					holding=true;
					return;
				} else {
					holdlist.remove(this);
					holding=false;
				}
			}
			event->DeActivateEvent(ev_trigger);
		} else {
			/* store value for possible later use in the activated event */
			event->SetValue(0);
			event->DeActivateEvent(ev_trigger);
		}
	}
	virtual void ConfigName(char * buf)=0;
	virtual void BindName(char * buf)=0;
   
	Bitu mods,flags;
	Bit16s value;
	CEvent * event;
	CBindList * list;
	bool active,holding;
};


void CEvent::AddBind(CBind * bind) {
	bindlist.push_front(bind);
	bind->event=this;
}
void CEvent::DeActivateAll(void) {
	for (CBindList_it bit=bindlist.begin();bit!=bindlist.end();bit++) {
		(*bit)->DeActivateBind(true);
	}
}



class CBindGroup {			
public:
	CBindGroup() {
		bindgroups.push_back(this);
	}
	void ActivateBindList(CBindList * list,Bits value,bool ev_trigger);
	void DeactivateBindList(CBindList * list,bool ev_trigger);
	virtual CBind * CreateConfigBind(char *&buf)=0;
	virtual CBind * CreateEventBind(SDL_Event * event)=0;

	virtual bool CheckEvent(SDL_Event * event)=0;
	virtual const char * ConfigStart(void)=0;
	virtual const char * BindStart(void)=0;
	virtual ~CBindGroup (void) { }

protected:

};


class CKeyBind;
class CKeyBindGroup;

class CKeyBind : public CBind {
public:
	CKeyBind(CBindList * _list,SDL_Scancode _key) : CBind(_list) {
		key = _key;
	}
	void BindName(char * buf) {
		sprintf(buf,"Key %s",SDL_GetScancodeName(key));
	}
	void ConfigName(char * buf) {
		sprintf(buf,"key %d",key);
	}
public:
	SDL_Scancode key;
};

class CKeyBindGroup : public  CBindGroup {
public:
	CKeyBindGroup(Bitu _keys) : CBindGroup (){
		lists=new CBindList[_keys];
		for (Bitu i=0;i<_keys;i++) lists[i].clear();
		keys=_keys;
		configname="key";
	}
	~CKeyBindGroup() { delete[] lists; }
	CBind * CreateConfigBind(char *& buf) {
		if (strncasecmp(buf,configname,strlen(configname))) return 0;
		StripWord(buf);char * num=StripWord(buf);
		Bitu code=ConvDecWord(num);
		CBind * bind=CreateKeyBind((SDL_Scancode)code);
		return bind;
	}
	CBind * CreateEventBind(SDL_Event * event) {
		if (event->type!=SDL_KEYDOWN) return 0;
		return CreateKeyBind(event->key.keysym.scancode);
	};
	bool CheckEvent(SDL_Event * event) {
		if (event->type!=SDL_KEYDOWN && event->type!=SDL_KEYUP) return false;
		Bitu key = event->key.keysym.scancode;
		if (event->type==SDL_KEYDOWN) ActivateBindList(&lists[key],0x7fff,true);
		else DeactivateBindList(&lists[key],true);
		return 0;
	}
	CBind * CreateKeyBind(SDL_Scancode _key) {
		return new CKeyBind(&lists[(Bitu)_key],_key);
	}
private:
	const char * ConfigStart(void) {
		return configname;
	}
	const char * BindStart(void) {
		return "Key";
	}
protected:
	const char * configname;
	CBindList * lists;
	Bitu keys;
};

#define MAX_VJOY_BUTTONS 10
#define MAX_VJOY_HAT 16
#define MAX_VJOY_AXIS 8
static struct {
	bool button_pressed[MAX_VJOY_BUTTONS];
	Bit16s axis_pos[MAX_VJOY_AXIS];
	bool hat_pressed[MAX_VJOY_HAT];
} virtual_joysticks[2];


class CJAxisBind;
class CJButtonBind;
class CJHatBind;

class CJAxisBind : public CBind {
public:
	CJAxisBind(CBindList * _list,CBindGroup * _group,Bitu _axis,bool _positive) : CBind(_list){
		group = _group;
		axis = _axis;
		positive = _positive;
	}
	void ConfigName(char * buf) {
		sprintf(buf,"%s axis %" sBitfs(d) " %d",group->ConfigStart(),axis,(positive ? 1 : 0));
	}
	void BindName(char * buf) {
		sprintf(buf,"%s Axis %" sBitfs(d) "%s",group->BindStart(),axis,(positive ? "+" : "-"));
	}
protected:
	CBindGroup * group;
	Bitu axis;
	bool positive;
};

class CJButtonBind : public CBind {
public:
	CJButtonBind(CBindList * _list,CBindGroup * _group,Bitu _button) : CBind(_list) {
		group = _group;
		button=_button;
	}
	void ConfigName(char * buf) {
		sprintf(buf,"%s button %" sBitfs(d),group->ConfigStart(),button);
	}
	void BindName(char * buf) {
		sprintf(buf,"%s Button %" sBitfs(d),group->BindStart(),button);
	}
protected:
	CBindGroup * group;
	Bitu button;
};

class CJHatBind : public CBind {
public:
	CJHatBind(CBindList * _list,CBindGroup * _group,Bitu _hat,Bit8u _dir) : CBind(_list) {
		group = _group;
		hat   = _hat;
		dir   = _dir;
		/* allow only one hat position */
		if (dir&SDL_HAT_UP) dir=SDL_HAT_UP;
		else if (dir&SDL_HAT_RIGHT) dir=SDL_HAT_RIGHT;
		else if (dir&SDL_HAT_DOWN) dir=SDL_HAT_DOWN;
		else if (dir&SDL_HAT_LEFT) dir=SDL_HAT_LEFT;
		else E_Exit("MAPPER:JOYSTICK:Invalid hat position");
	}
	void ConfigName(char * buf) {
		sprintf(buf,"%s hat %" sBitfs(d) " %d",group->ConfigStart(),hat,dir);
	}
	void BindName(char * buf) {
		sprintf(buf,"%s Hat %" sBitfs(d) " %s",group->BindStart(),hat,(dir==SDL_HAT_UP)?"up":
														((dir==SDL_HAT_RIGHT)?"right":
														((dir==SDL_HAT_DOWN)?"down":"left")));
	}
protected:
	CBindGroup * group;
	Bitu hat;
	Bit8u dir;
};

bool autofire = false;
int  nJoyButtons = 0 ;
int  nPadLayout  = 0 ;

class CStickBindGroup : public  CBindGroup {
public:
	CStickBindGroup(Bitu _stick,Bitu _emustick,bool _dummy=false) : CBindGroup (){
		stick=_stick;		// the number of the physical device (SDL numbering|)
		emustick=_emustick;	// the number of the emulated device
		sprintf(configname,"stick_%" sBitfs(d),emustick);

		sdl_joystick=NULL;
		axes=0;	buttons=0; hats=0;
		button_wrap=0;
		button_cap=0; axes_cap=0; hats_cap=0;
		emulated_buttons=0; emulated_axes=0; emulated_hats=0;

		is_dummy=_dummy;
		if (_dummy) return;

		// initialize binding lists and position data
		pos_axis_lists=new CBindList[MAXAXIS];
		neg_axis_lists=new CBindList[MAXAXIS];
		button_lists=new CBindList[MAXBUTTON];
		hat_lists=new CBindList[4];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) {
			button_autofire[i]=0;
			old_button_state[i]=0;
		}
		for(i=0;i<16;i++) old_hat_state[i]=0;
		for (i=0; i<MAXAXIS; i++) {
			old_pos_axis_state[i]=false;
			old_neg_axis_state[i]=false;
		}

		// initialize emulated joystick state
		emulated_axes=2;
		
		if ( nJoyButtons != 0 ){
			emulated_buttons = nJoyButtons;
		} else {
			emulated_buttons=2;
			nJoyButtons = emulated_buttons;
		}
		emulated_hats=0;
		JOYSTICK_Enable(emustick,true);

		sdl_joystick=SDL_JoystickOpen(_stick);
		if (sdl_joystick==NULL) {
			button_wrap=emulated_buttons;
			return;
		}

		axes=SDL_JoystickNumAxes(sdl_joystick);
		if (axes > MAXAXIS) axes = MAXAXIS;
		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		
		hats=SDL_JoystickNumHats(sdl_joystick);
		if (hats > MAXHAT) hats = MAXHAT;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;
		
		buttons=SDL_JoystickNumButtons(sdl_joystick);

		button_wrap=buttons;
		button_cap=buttons;
		if (button_wrapping_enabled) {
			button_wrap=emulated_buttons;
			if (buttons>MAXBUTTON_CAP) button_cap = MAXBUTTON_CAP;
		}
		if (button_wrap > MAXBUTTON) button_wrap = MAXBUTTON;

		LOG_MSG("SDL : Using joystick %s\n"
		        "      Axes    : %" sBitfs(d) "\n"
				"      Buttons : %" sBitfs(d) "\n"
				"      %s %" sBitfs(d) "\n",SDL_JoystickNameForIndex(stick),axes,buttons,(hats>1)?"Hats   : ":"Hat     : ",hats);
	}
	~CStickBindGroup() {
		SDL_JoystickClose(sdl_joystick);
		delete[] pos_axis_lists;
		delete[] neg_axis_lists;
		delete[] button_lists;
		delete[] hat_lists;
	}

	CBind * CreateConfigBind(char *& buf) {
		if (strncasecmp(configname,buf,strlen(configname))) return 0;
		StripWord(buf);char * type=StripWord(buf);
		CBind * bind=0;
		if (!_stricmp(type,"axis")) {
			Bitu ax=ConvDecWord(StripWord(buf));
			bool pos=ConvDecWord(StripWord(buf)) > 0;
			bind=CreateAxisBind(ax,pos);
		} else if (!_stricmp(type,"button")) {
			Bitu but=ConvDecWord(StripWord(buf));			
			bind=CreateButtonBind(but);
		} else if (!_stricmp(type,"hat")) {
			Bitu hat=ConvDecWord(StripWord(buf));			
			Bit8u dir=(Bit8u)ConvDecWord(StripWord(buf));			
			bind=CreateHatBind(hat,dir);
		}
		return bind;
	}
	CBind * CreateEventBind(SDL_Event * event) {

			/* Bug?! ... 
			   With two Joysticks plugged in USB. On JOY_4AXIS, JOY_4AXIS_2, FCS, CH
			   the binding let DOSBox crash if you press  a Button, Move Joystick or Hat.
			   
			   In my Tests i have on 
			       Host id1: Logitech F710 Rumble Pad
				   Host id2: Saitek45X
			   
			   From SDL i get the Joytick Name from Host id1: Saitek45X
			   From SDL i get the Joytick Name from Host id2: F710 Rumble Pad
			   			  
			   But the Buttons association works. ??? Crazy
			   
			   The next Routine try to catch the crash .....
			 */  
			if (stick >= 0 && stick < SDL_NumJoysticks())
			{
				SDL_JoystickGUID DefGUID, DevGUID;
				char const *cStrGUID = "";
					
				DefGUID = SDL_JoystickGetGUIDFromString(cStrGUID);
					
				for (int i  = 0; i < SDL_NumJoysticks(); ++i){
					DevGUID = SDL_JoystickGetDeviceGUID(i);
						
					if (memcmp(&DefGUID, &DevGUID, sizeof(SDL_JoystickGUID)))
					{					
						LOG_MSG("SDL: Joystick Binding Catched: %s",SDL_JoystickNameForIndex(i) );
						stick = i;		
						if ( joytype == JOY_4AXIS_2){
							break;
						}
					}						
				}
			}
			else
			{
				LOG_MSG("SDL: Joystick Not found (device disconnected?)");
				return 0;
			}	
			
		if (event->type==SDL_JOYAXISMOTION) {
			if (event->jaxis.which!=stick) return 0;
#if defined (REDUCE_JOYSTICK_POLLING)
			if (event->jaxis.axis>=emulated_axes) return 0;
#endif
			if (abs(event->jaxis.value)<25000) return 0;
			return CreateAxisBind(event->jaxis.axis,event->jaxis.value>0);
			
		} else if (event->type==SDL_JOYBUTTONDOWN) {
			if (event->jbutton.which!=stick) return 0;

#if defined (REDUCE_JOYSTICK_POLLING)
			return CreateButtonBind(event->jbutton.button%button_wrap);
#else
			return CreateButtonBind(event->jbutton.button);		
#endif
		} else if (event->type==SDL_JOYHATMOTION) {
			if (event->jhat.which!=stick) return 0;
			if (event->jhat.value==0) return 0;
			if (event->jhat.value>(SDL_HAT_UP|SDL_HAT_RIGHT|SDL_HAT_DOWN|SDL_HAT_LEFT)) return 0;
			return CreateHatBind(event->jhat.hat,event->jhat.value);
		} else return 0;
	}

	virtual bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		Bitu but = 0;
		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick) {				
					if(jaxis->axis == 0)
						JOYSTICK_Move_X(emustick,(float)(jaxis->value/32768.0));
					else if(jaxis->axis == 1)
						JOYSTICK_Move_Y(emustick,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
					JOYSTICK_Button(emustick,but,state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		if (is_dummy) return;
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[emustick].button_pressed[i])
				button_pressed[i % button_wrap]=true;
		}
		for (i=0; i<emulated_buttons; i++) {
			if (autofire && (button_pressed[i]))
				JOYSTICK_Button(emustick,i,(++button_autofire[i])&1);
			else
				JOYSTICK_Button(emustick,i,button_pressed[i]);
		}

		JOYSTICK_Move_X(emustick,((float)virtual_joysticks[emustick].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(emustick,((float)virtual_joysticks[emustick].axis_pos[1])/32768.0f);
	}

	void ActivateJoystickBoundEvents() {
		if (GCC_UNLIKELY(sdl_joystick==NULL)) return;

		Bitu i;

		bool button_pressed[MAXBUTTON];
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		/* read button states */
		for (i=0; i<button_cap; i++) {
			if (SDL_JoystickGetButton(sdl_joystick,i))
				button_pressed[i % button_wrap]=true;
		}
		for (i=0; i<button_wrap; i++) {
			/* activate binding if button state has changed */
			if (button_pressed[i]!=old_button_state[i]) {
				if (button_pressed[i]) ActivateBindList(&button_lists[i],32767,true);
				else DeactivateBindList(&button_lists[i],true);
				old_button_state[i]=button_pressed[i];
			}
		}

		for (i=0; i<axes_cap; i++) {
			Sint16 caxis_pos=SDL_JoystickGetAxis(sdl_joystick,i);
			/* activate bindings for joystick position */
			if (caxis_pos>1) {
				if (old_neg_axis_state[i]) {
					DeactivateBindList(&neg_axis_lists[i],false);
					old_neg_axis_state[i] = false;
				}
				ActivateBindList(&pos_axis_lists[i],caxis_pos,false);
				old_pos_axis_state[i] = true;
			} else if (caxis_pos<-1) {
				if (old_pos_axis_state[i]) {
					DeactivateBindList(&pos_axis_lists[i],false);
					old_pos_axis_state[i] = false;
				}
				if (caxis_pos!=-32768) caxis_pos=(Sint16)abs(caxis_pos);
				else caxis_pos=32767;
				ActivateBindList(&neg_axis_lists[i],caxis_pos,false);
				old_neg_axis_state[i] = true;
			} else {
				/* center */
				if (old_pos_axis_state[i]) {
					DeactivateBindList(&pos_axis_lists[i],false);
					old_pos_axis_state[i] = false;
				}
				if (old_neg_axis_state[i]) {
					DeactivateBindList(&neg_axis_lists[i],false);
					old_neg_axis_state[i] = false;
				}
			}
		}

		for (i=0; i<hats_cap; i++) {
			Uint8 chat_state=SDL_JoystickGetHat(sdl_joystick,i);

			/* activate binding if hat state has changed */
			if ((chat_state & SDL_HAT_UP) != (old_hat_state[i] & SDL_HAT_UP)) {
				if (chat_state & SDL_HAT_UP) ActivateBindList(&hat_lists[(i<<2)+0],32767,true);
				else DeactivateBindList(&hat_lists[(i<<2)+0],true);
			}
			if ((chat_state & SDL_HAT_RIGHT) != (old_hat_state[i] & SDL_HAT_RIGHT)) {
				if (chat_state & SDL_HAT_RIGHT) ActivateBindList(&hat_lists[(i<<2)+1],32767,true);
				else DeactivateBindList(&hat_lists[(i<<2)+1],true);
			}
			if ((chat_state & SDL_HAT_DOWN) != (old_hat_state[i] & SDL_HAT_DOWN)) {
				if (chat_state & SDL_HAT_DOWN) ActivateBindList(&hat_lists[(i<<2)+2],32767,true);
				else DeactivateBindList(&hat_lists[(i<<2)+2],true);
			}
			if ((chat_state & SDL_HAT_LEFT) != (old_hat_state[i] & SDL_HAT_LEFT)) {
				if (chat_state & SDL_HAT_LEFT) ActivateBindList(&hat_lists[(i<<2)+3],32767,true);
				else DeactivateBindList(&hat_lists[(i<<2)+3],true);
			}
			old_hat_state[i]=chat_state;
		}
	}

private:
	CBind * CreateAxisBind(Bitu axis,bool positive) {
		if (axis<emulated_axes) {
			if (positive) return new CJAxisBind(&pos_axis_lists[axis],this,axis,positive);
			else return new CJAxisBind(&neg_axis_lists[axis],this,axis,positive);
		}
		return NULL;
	}
	CBind * CreateButtonBind(Bitu button) {		
		if (button<button_wrap) 
			return new CJButtonBind(&button_lists[button],this,button);			
		return NULL;
	}
	CBind * CreateHatBind(Bitu hat,Bit8u value) {
		if (hat < hats_cap) return NULL;
		Bitu hat_dir;
		if (value&SDL_HAT_UP) hat_dir=0;
		else if (value&SDL_HAT_RIGHT) hat_dir=1;
		else if (value&SDL_HAT_DOWN) hat_dir=2;
		else if (value&SDL_HAT_LEFT) hat_dir=3;
		else return NULL;
		return new CJHatBind(&hat_lists[(hat<<2)+hat_dir],this,hat,value);
	}
	const char * ConfigStart(void) {
		return configname;
	}
	const char * BindStart(void) {
		if (sdl_joystick!=NULL) return SDL_JoystickNameForIndex(stick);
		else return "[missing joystick]";
	}

protected:
	CBindList * pos_axis_lists;
	CBindList * neg_axis_lists;
	CBindList * button_lists;
	CBindList * hat_lists;
	Bitu stick,emustick,axes,buttons,hats,emulated_axes,emulated_buttons,emulated_hats;
	Bitu button_wrap,button_cap,axes_cap,hats_cap;
	SDL_Joystick * sdl_joystick;
	char configname[10];
	Bitu button_autofire[MAXBUTTON];
	bool old_button_state[MAXBUTTON];
	bool old_pos_axis_state[MAXAXIS];
	bool old_neg_axis_state[MAXAXIS];
	Uint8 old_hat_state[16];
	bool is_dummy;
};

class C4AxisBindGroup : public  CStickBindGroup {
public:
	C4AxisBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=4;

		if ( nJoyButtons != 0 ){
			emulated_buttons = nJoyButtons;
		} else {
			emulated_buttons=4;
			nJoyButtons = emulated_buttons;
		}
		
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;

		JOYSTICK_Enable(1,true);
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		Bitu but = 0;

		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 4) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
					JOYSTICK_Button((but >> 1),(but & 1),state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}
		for (i=0; i<emulated_buttons; i++) {
			
			if (autofire && (button_pressed[i]))				
				JOYSTICK_Button(i>>1,i&1,(++button_autofire[i])&1);
			else			
				JOYSTICK_Button(i>>1,i&1,button_pressed[i]);
		}

		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
		JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
		JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);
	}
};



class CFCSBindGroup : public  CStickBindGroup {
public:
	CFCSBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=4;

		if ( nJoyButtons != 0 ){
			emulated_buttons = nJoyButtons;
		} else {
			emulated_buttons=4;
			nJoyButtons = emulated_buttons;
		}
		
		old_hat_position = 0;
		//emulated_hats=1;
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;

		JOYSTICK_Enable(1,true);
		JOYSTICK_Move_Y(1,1.0);
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		SDL_JoyHatEvent * jhat = NULL;
		Bitu but = 0;

		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick) {
					if(jaxis->axis == 0)
						JOYSTICK_Move_X(0,(float)(jaxis->value/32768.0));
					else if(jaxis->axis == 1)
						JOYSTICK_Move_Y(0,(float)(jaxis->value/32768.0));
					else if(jaxis->axis == 2)
						JOYSTICK_Move_X(1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYHATMOTION:
				jhat = &event->jhat;
				if(jhat->which == stick) DecodeHatPosition(jhat->value);
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
						JOYSTICK_Button((but >> 1),(but & 1),state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}
		for (i=0; i<emulated_buttons; i++) {
			if (autofire && (button_pressed[i]))
				JOYSTICK_Button(i>>1,i&1,(++button_autofire[i])&1);
			else
				JOYSTICK_Button(i>>1,i&1,button_pressed[i]);
		}

		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
		JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);

		Uint8 hat_pos=0;
		if (virtual_joysticks[0].hat_pressed[0]) hat_pos|=SDL_HAT_UP;
		else if (virtual_joysticks[0].hat_pressed[2]) hat_pos|=SDL_HAT_DOWN;
		if (virtual_joysticks[0].hat_pressed[3]) hat_pos|=SDL_HAT_LEFT;
		else if (virtual_joysticks[0].hat_pressed[1]) hat_pos|=SDL_HAT_RIGHT;

		if (hat_pos!=old_hat_position) {
			DecodeHatPosition(hat_pos);
			old_hat_position=hat_pos;
		}
	}

private:
	Uint8 old_hat_position;

	void DecodeHatPosition(Uint8 hat_pos) {
		switch(hat_pos) {
			case SDL_HAT_CENTERED:
				JOYSTICK_Move_Y(1,1.0);
				break;
			case SDL_HAT_UP:
				JOYSTICK_Move_Y(1,-1.0);
				break;
			case SDL_HAT_RIGHT:
				JOYSTICK_Move_Y(1,-0.5);
				break;
			case SDL_HAT_DOWN:
				JOYSTICK_Move_Y(1,0.0);
				break;
			case SDL_HAT_LEFT:
				JOYSTICK_Move_Y(1,0.5);
				break;
			case SDL_HAT_LEFTUP:
				if(JOYSTICK_GetMove_Y(1) < 0)
					JOYSTICK_Move_Y(1,0.5);
				else
					JOYSTICK_Move_Y(1,-1.0);
				break;
			case SDL_HAT_RIGHTUP:
				if(JOYSTICK_GetMove_Y(1) < -0.7)
					JOYSTICK_Move_Y(1,-0.5);
				else
					JOYSTICK_Move_Y(1,-1.0);
				break;
			case SDL_HAT_RIGHTDOWN:
				if(JOYSTICK_GetMove_Y(1) < -0.2)
					JOYSTICK_Move_Y(1,0.0);
				else
					JOYSTICK_Move_Y(1,-0.5);
				break;
			case SDL_HAT_LEFTDOWN:
				if(JOYSTICK_GetMove_Y(1) > 0.2)
					JOYSTICK_Move_Y(1,0.0);
				else
					JOYSTICK_Move_Y(1,0.5);
				break;
		}
	}
};

class CFCSLWBindGroup : public  CStickBindGroup {
public:
	CFCSLWBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		
		emulated_axes=6;
		emulated_buttons=4;		
		//emulated_hats=1;
		
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;

		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;
				
		JOYSTICK_Enable(1,true);			
		button_state=0;
	}

	// bool CheckEvent(SDL_Event * event) {
		// SDL_JoyAxisEvent * jaxis = NULL;
		// SDL_JoyButtonEvent * jbutton = NULL;
		// SDL_JoyHatEvent * jhat = NULL;
		// Bitu but = 0;
		// static unsigned const button_magic[6]={0x02,0x04,0x10,0x100,0x20,0x200};
		// static unsigned const hat_magic[2][5]={{0x8888,0x8000,0x800,0x80,0x08},{0x5440,0x4000,0x400,0x40,0x1000}};
		// switch(event->type) {
			// case SDL_JOYAXISMOTION:
				// jaxis = &event->jaxis;
				// if(jaxis->which == stick && jaxis->axis < 6) {
					// if(jaxis->axis & 1)
						// JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					// else
						// JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				// }
				// break;
			// case SDL_JOYHATMOTION:
				// jhat = &event->jhat;
				// if(jhat->which == stick && jhat->hat < 2) {
					// if(jhat->value == SDL_HAT_CENTERED)
						// button_state&=~hat_magic[jhat->hat][0];
					// if(jhat->value & SDL_HAT_UP)
						// button_state|=hat_magic[jhat->hat][1];
					// if(jhat->value & SDL_HAT_RIGHT)
						// button_state|=hat_magic[jhat->hat][2];
					// if(jhat->value & SDL_HAT_DOWN)
						// button_state|=hat_magic[jhat->hat][3];
					// if(jhat->value & SDL_HAT_LEFT)
						// button_state|=hat_magic[jhat->hat][4];
				// }
				// break;
			// case SDL_JOYBUTTONDOWN:
				// jbutton = &event->jbutton;
				// but = jbutton->button % emulated_buttons;
				// if (jbutton->which == stick)
					// button_state|=button_magic[but];
				// break;
			// case SDL_JOYBUTTONUP:
				// jbutton = &event->jbutton;
				// but = jbutton->button % emulated_buttons;
				// if (jbutton->which == stick)
					// button_state&=~button_magic[but];
				// break;
		// }

		// unsigned i;
		// Bit16u j;
		// j=button_state;
		// for(i=0;i<16;i++) if (j & 1) break; else j>>=1;
		// JOYSTICK_Button(0,0,i&1);
		// JOYSTICK_Button(0,1,(i>>1)&1);
		// JOYSTICK_Button(1,0,(i>>2)&1);
		// JOYSTICK_Button(1,1,(i>>3)&1);
		// return false;
	// }

	void UpdateJoystick() {
		static unsigned const button_priority[4]={7,11,13,14};//7,11,13,14,5,6,5,1,4,2
		static unsigned const hat_priority[2][4]={{0,1,2,3},{8,9,10,12}};

		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		/* ==================================================================================================== */
		switch(HostCtrlType)
		{			
			case HOSTJOY_DEFAULT:			
			case HOSTJOY_SAITEKX45:			
			{						 
				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);	
				JOYSTICK_Move_X(2,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
				JOYSTICK_Move_Y(2,((float)virtual_joysticks[0].axis_pos[5])/32768.0f);	
		
			}
			break;	
			case HOSTJOY_LEXT3DPRO:			
			{
				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);	
				JOYSTICK_Move_X(2,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);
				JOYSTICK_Move_Y(2,((float)virtual_joysticks[0].axis_pos[5])/32768.0f);							 
			}
			break;	
			case HOSTJOY_TFLIGHTHO:			
			{
				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[5])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);	
				JOYSTICK_Move_X(2,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);
				JOYSTICK_Move_Y(2,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);							 
			}
			break;							
		}
		/* ==================================================================================================== */
		

		
		

		//JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
		//JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);		

        Bitu bt_state=15;

        Bitu i;
        for (i=0; i<(hats<2?hats:2); i++) {
            Uint8 hat_pos=0;
            if (virtual_joysticks[0].hat_pressed[(i<<2)+0]) hat_pos|=SDL_HAT_UP;
            else if (virtual_joysticks[0].hat_pressed[(i<<2)+2]) hat_pos|=SDL_HAT_DOWN;
            if (virtual_joysticks[0].hat_pressed[(i<<2)+3]) hat_pos|=SDL_HAT_LEFT;
            else if (virtual_joysticks[0].hat_pressed[(i<<2)+1]) hat_pos|=SDL_HAT_RIGHT;

            if (hat_pos & SDL_HAT_UP)
                if (bt_state>hat_priority[i][0]) bt_state=hat_priority[i][0];
            if (hat_pos & SDL_HAT_DOWN)
                if (bt_state>hat_priority[i][1]) bt_state=hat_priority[i][1];
            if (hat_pos & SDL_HAT_RIGHT)
                if (bt_state>hat_priority[i][2]) bt_state=hat_priority[i][2];
            if (hat_pos & SDL_HAT_LEFT)
                if (bt_state>hat_priority[i][3]) bt_state=hat_priority[i][3];
        }
	
        bool button_pressed[MAXBUTTON];
        for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
        for (i=0; i<MAX_VJOY_BUTTONS; i++) {
            if (virtual_joysticks[0].button_pressed[i])
                button_pressed[i % button_wrap]=true;
            
        }
		
        for (i=0; i<4; i++) {
            if ((button_pressed[i]) && (bt_state>button_priority[i]))
			{
				
			//	if ( (i >= 0) &&  (i <= 5) && (button_pressed[i]) )
			//	{
					bt_state=button_priority[i];
					//LOG_MSG("%d",bt_state);
						
				// } else if  ( (i == 6) &&  (button_pressed[i] ) )
						
				// {
					// if (bt_state>15) bt_state=15;
					// JOYSTICK_Button(0,1,(button_priority[8]&4)==0);							
					// JOYSTICK_Button(1,0,(button_priority[8]&2)==0);			
					// JOYSTICK_Button(0,1,(button_priority[9]&4)==0);							
					// JOYSTICK_Button(1,1,(button_priority[9]&1)==0);	
					// return;
						
				// } else if  ( (i >= 7) &&  (button_pressed[i] ) )
						
				// {
					// if (bt_state>15) bt_state=15;
					// JOYSTICK_Button(0,0,(button_priority[i]&7)==0);
					// JOYSTICK_Button(0,1,(button_priority[i]&4)==0);							
					// JOYSTICK_Button(1,0,(button_priority[i]&2)==0);
					// JOYSTICK_Button(1,1,(button_priority[i]&1)==0);	
					// return;						
				// }																
			}
        }
		
        if (bt_state>15) bt_state=15;		
        JOYSTICK_Button(0,0,(bt_state&8)==0);
        JOYSTICK_Button(0,1,(bt_state&4)==0);
        JOYSTICK_Button(1,0,(bt_state&2)==0);
        JOYSTICK_Button(1,1,(bt_state&1)==0);	
	}

protected:
	Bit16u button_state;
};

class CCHBindGroup : public  CStickBindGroup {
public:
	CCHBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=4;
		emulated_buttons=6;
		//emulated_hats=1;
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;

		JOYSTICK_Enable(1,true);
		button_state=0;
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		SDL_JoyHatEvent * jhat = NULL;
		Bitu but = 0;
		static unsigned const button_magic[6]={0x02,0x04,0x10,0x100,0x20,0x200};
		static unsigned const hat_magic[2][5]={{0x8888,0x8000,0x800,0x80,0x08},
							   {0x5440,0x4000,0x400,0x40,0x1000}};
		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 4) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYHATMOTION:
				jhat = &event->jhat;
				if(jhat->which == stick && jhat->hat < 2) {
					if(jhat->value == SDL_HAT_CENTERED)
						button_state&=~hat_magic[jhat->hat][0];
					if(jhat->value & SDL_HAT_UP)
						button_state|=hat_magic[jhat->hat][1];
					if(jhat->value & SDL_HAT_RIGHT)
						button_state|=hat_magic[jhat->hat][2];
					if(jhat->value & SDL_HAT_DOWN)
						button_state|=hat_magic[jhat->hat][3];
					if(jhat->value & SDL_HAT_LEFT)
						button_state|=hat_magic[jhat->hat][4];
				}
				break;
			case SDL_JOYBUTTONDOWN:
				jbutton = &event->jbutton;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick)
					button_state|=button_magic[but];
				break;
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick)
					button_state&=~button_magic[but];
				break;
		}

		unsigned i;
		Bit16u j;
		j=button_state;
		for(i=0;i<16;i++) if (j & 1) break; else j>>=1;
		JOYSTICK_Button(0,0,i&1);
		JOYSTICK_Button(0,1,(i>>1)&1);
		JOYSTICK_Button(1,0,(i>>2)&1);
		JOYSTICK_Button(1,1,(i>>3)&1);
		return false;
	}

	void UpdateJoystick() {
		static unsigned const button_priority[6]={7,11,13,14,5,6};
		static unsigned const hat_priority[2][4]={{0,1,2,3},{8,9,10,12}};

		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
		JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
		JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);

		Bitu bt_state=15;

		Bitu i;
		for (i=0; i<(hats<2?hats:2); i++) {
			Uint8 hat_pos=0;
			if (virtual_joysticks[0].hat_pressed[(i<<2)+0]) hat_pos|=SDL_HAT_UP;
			else if (virtual_joysticks[0].hat_pressed[(i<<2)+2]) hat_pos|=SDL_HAT_DOWN;
			if (virtual_joysticks[0].hat_pressed[(i<<2)+3]) hat_pos|=SDL_HAT_LEFT;
			else if (virtual_joysticks[0].hat_pressed[(i<<2)+1]) hat_pos|=SDL_HAT_RIGHT;

			if (hat_pos & SDL_HAT_UP)
				if (bt_state>hat_priority[i][0]) bt_state=hat_priority[i][0];
			if (hat_pos & SDL_HAT_DOWN)
				if (bt_state>hat_priority[i][1]) bt_state=hat_priority[i][1];
			if (hat_pos & SDL_HAT_RIGHT)
				if (bt_state>hat_priority[i][2]) bt_state=hat_priority[i][2];
			if (hat_pos & SDL_HAT_LEFT)
				if (bt_state>hat_priority[i][3]) bt_state=hat_priority[i][3];
		}

		bool button_pressed[MAXBUTTON];
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}
		for (i=0; i<6; i++) {
			if ((button_pressed[i]) && (bt_state>button_priority[i]))
				bt_state=button_priority[i];
		}

		if (bt_state>15) bt_state=15;
		JOYSTICK_Button(0,0,(bt_state&8)==0);
		JOYSTICK_Button(0,1,(bt_state&4)==0);
		JOYSTICK_Button(1,0,(bt_state&2)==0);
		JOYSTICK_Button(1,1,(bt_state&1)==0);
	}

protected:
	Bit16u button_state;
};

/* ====================================================================================== VIRTUAL PILOT */
class CCHVPBindGroup : public  CStickBindGroup {
public:
	CCHVPBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=6;
		emulated_buttons=10;
		//emulated_hats=1;
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;
				
		JOYSTICK_Enable(1,true);			
		button_state=0;
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		SDL_JoyHatEvent * jhat = NULL;
		Bitu but = 0;
		static unsigned const button_magic[6]={0x02,0x04,0x10,0x100,0x20,0x200};
		static unsigned const hat_magic[2][5]={{0x8888,0x8000,0x800,0x80,0x08},{0x5440,0x4000,0x400,0x40,0x1000}};
		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 6) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYHATMOTION:
				jhat = &event->jhat;
				if(jhat->which == stick && jhat->hat < 2) {
					if(jhat->value == SDL_HAT_CENTERED)
						button_state&=~hat_magic[jhat->hat][0];
					if(jhat->value & SDL_HAT_UP)
						button_state|=hat_magic[jhat->hat][1];
					if(jhat->value & SDL_HAT_RIGHT)
						button_state|=hat_magic[jhat->hat][2];
					if(jhat->value & SDL_HAT_DOWN)
						button_state|=hat_magic[jhat->hat][3];
					if(jhat->value & SDL_HAT_LEFT)
						button_state|=hat_magic[jhat->hat][4];
				}
				break;
			case SDL_JOYBUTTONDOWN:
				jbutton = &event->jbutton;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick)
					button_state|=button_magic[but];
				break;
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick)
					button_state&=~button_magic[but];
				break;
		}

		unsigned i;
		Bit16u j;
		j=button_state;
		for(i=0;i<16;i++) if (j & 1) break; else j>>=1;
		JOYSTICK_Button(0,0,i&1);
		JOYSTICK_Button(0,1,(i>>1)&1);
		JOYSTICK_Button(1,0,(i>>2)&1);
		JOYSTICK_Button(1,1,(i>>3)&1);
		return false;
	}

	void UpdateJoystick() {
		static unsigned const button_priority[10]={7,11,13,14,5,6,5,1,4,2};
		static unsigned const hat_priority[2][4]={{0,1,2,3},{8,9,10,12}};

		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		/* ==================================================================================================== */
		switch(HostCtrlType)
		{			
			case HOSTJOY_DEFAULT:			
			case HOSTJOY_SAITEKX45:			
			{						 
				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);	
				JOYSTICK_Move_X(2,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
				JOYSTICK_Move_Y(2,((float)virtual_joysticks[0].axis_pos[5])/32768.0f);	
		
			}
			break;	
			case HOSTJOY_LEXT3DPRO:			
			{
				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);	
				JOYSTICK_Move_X(2,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);
				JOYSTICK_Move_Y(2,((float)virtual_joysticks[0].axis_pos[5])/32768.0f);							 
			}
			break;	
			case HOSTJOY_TFLIGHTHO:			
			{
				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[5])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);	
				JOYSTICK_Move_X(2,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);
				JOYSTICK_Move_Y(2,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);							 
			}
			break;							
		}
		/* ==================================================================================================== */
		

		
		

		//JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f);
		//JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);		

        Bitu bt_state=15;

        Bitu i;
        for (i=0; i<(hats<2?hats:2); i++) {
            Uint8 hat_pos=0;
            if (virtual_joysticks[0].hat_pressed[(i<<2)+0]) hat_pos|=SDL_HAT_UP;
            else if (virtual_joysticks[0].hat_pressed[(i<<2)+2]) hat_pos|=SDL_HAT_DOWN;
            if (virtual_joysticks[0].hat_pressed[(i<<2)+3]) hat_pos|=SDL_HAT_LEFT;
            else if (virtual_joysticks[0].hat_pressed[(i<<2)+1]) hat_pos|=SDL_HAT_RIGHT;

            if (hat_pos & SDL_HAT_UP)
                if (bt_state>hat_priority[i][0]) bt_state=hat_priority[i][0];
            if (hat_pos & SDL_HAT_DOWN)
                if (bt_state>hat_priority[i][1]) bt_state=hat_priority[i][1];
            if (hat_pos & SDL_HAT_RIGHT)
                if (bt_state>hat_priority[i][2]) bt_state=hat_priority[i][2];
            if (hat_pos & SDL_HAT_LEFT)
                if (bt_state>hat_priority[i][3]) bt_state=hat_priority[i][3];
        }
	
        bool button_pressed[MAXBUTTON];
        for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
        for (i=0; i<MAX_VJOY_BUTTONS; i++) {
            if (virtual_joysticks[0].button_pressed[i])
                button_pressed[i % button_wrap]=true;
            
        }
		
        for (i=0; i<10; i++) {
            if ((button_pressed[i]) && (bt_state>button_priority[i]))
			{
				
				if ( (i >= 0) &&  (i <= 5) && (button_pressed[i]) )
				{
					bt_state=button_priority[i];
					//LOG_MSG("%d",bt_state);
						
				} else if  ( (i == 6) &&  (button_pressed[i] ) )
						
				{
					if (bt_state>15) bt_state=15;
					JOYSTICK_Button(0,1,(button_priority[8]&4)==0);							
					JOYSTICK_Button(1,0,(button_priority[8]&2)==0);			
					JOYSTICK_Button(0,1,(button_priority[9]&4)==0);							
					JOYSTICK_Button(1,1,(button_priority[9]&1)==0);	
					return;
						
				} else if  ( (i >= 7) &&  (button_pressed[i] ) )
						
				{
					if (bt_state>15) bt_state=15;
					JOYSTICK_Button(0,0,(button_priority[i]&7)==0);
					JOYSTICK_Button(0,1,(button_priority[i]&4)==0);							
					JOYSTICK_Button(1,0,(button_priority[i]&2)==0);
					JOYSTICK_Button(1,1,(button_priority[i]&1)==0);	
					return;						
				}																
			}
        }
		
        if (bt_state>15) bt_state=15;		
        JOYSTICK_Button(0,0,(bt_state&8)==0);
        JOYSTICK_Button(0,1,(bt_state&4)==0);
        JOYSTICK_Button(1,0,(bt_state&2)==0);
        JOYSTICK_Button(1,1,(bt_state&1)==0);	
	}

protected:
	Bit16u button_state;
};

/* ======================================================================================CH GAMESTICK 1.4 as Xbox/PSX/F710 */
class CCHGSBindGroup : public  CStickBindGroup {
public:
	CCHGSBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=6;
		emulated_buttons=10;
		//emulated_hats=1;
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;
				
		JOYSTICK_Enable(1,true);			
		
		button_state=0;
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		SDL_JoyHatEvent * jhat = NULL;
		Bitu but = 0;
		static unsigned const button_magic[6]={0x02,0x04,0x10,0x100,0x20,0x200};
		static unsigned const hat_magic[2][5]={{0x8888,0x8000,0x800,0x80,0x08},{0x5440,0x4000,0x400,0x40,0x1000}};
		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 6) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYHATMOTION:
				jhat = &event->jhat;
				if(jhat->which == stick && jhat->hat < 2) {
					if(jhat->value == SDL_HAT_CENTERED)
						button_state&=~hat_magic[jhat->hat][0];
					if(jhat->value & SDL_HAT_UP)
						button_state|=hat_magic[jhat->hat][1];
					if(jhat->value & SDL_HAT_RIGHT)
						button_state|=hat_magic[jhat->hat][2];
					if(jhat->value & SDL_HAT_DOWN)
						button_state|=hat_magic[jhat->hat][3];
					if(jhat->value & SDL_HAT_LEFT)
						button_state|=hat_magic[jhat->hat][4];
				}
				break;
			case SDL_JOYBUTTONDOWN:
				jbutton = &event->jbutton;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick)
					button_state|=button_magic[but];
				break;
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick)
					button_state&=~button_magic[but];
				break;
		}

		unsigned i;
		Bit16u j;
		j=button_state;
		for(i=0;i<16;i++) if (j & 1) break; else j>>=1;
		JOYSTICK_Button(0,0,i&1);
		JOYSTICK_Button(0,1,(i>>1)&1);
		JOYSTICK_Button(1,0,(i>>2)&1);
		JOYSTICK_Button(1,1,(i>>3)&1);
		return false;
	}

	void UpdateJoystick() {
		static unsigned const button_priority[10]={13,7,11,14,5,6,5,1,4,2};
		static unsigned const hat_priority[2][4]={{0,1,2,3},{8,9,10,12}};

		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
		JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[5])/(-32768.0f));
		JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[2])/(-32768.0f));		

        Bitu bt_state=15;

        Bitu i;
        for (i=0; i<(hats<2?hats:2); i++) {
            Uint8 hat_pos=0;
            if (virtual_joysticks[0].hat_pressed[(i<<2)+0])
			{
				hat_pos |= SDL_HAT_UP;
				LOG_MSG("hat_pos |= SDL_HAT_UP;: %d", hat_pos);
			}
			else if (virtual_joysticks[0].hat_pressed[(i << 2) + 2])
			{
				hat_pos |= SDL_HAT_DOWN;
				LOG_MSG("hat_pos |= SDL_HAT_DOWN;: %d", hat_pos);
			}
			if (virtual_joysticks[0].hat_pressed[(i << 2) + 3])
			{
				hat_pos |= SDL_HAT_LEFT;
				LOG_MSG("hat_pos |= SDL_HAT_LEFT;: %d", hat_pos);
			}
			else if (virtual_joysticks[0].hat_pressed[(i << 2) + 1])
			{
				hat_pos |= SDL_HAT_RIGHT;
				LOG_MSG("hat_pos |= SDL_HAT_RIGHT;: %d", hat_pos);
			}

            if (hat_pos & SDL_HAT_UP)
                if (bt_state>hat_priority[i][0]) bt_state=hat_priority[i][0];
            if (hat_pos & SDL_HAT_DOWN)
                if (bt_state>hat_priority[i][1]) bt_state=hat_priority[i][1];
            if (hat_pos & SDL_HAT_RIGHT)
                if (bt_state>hat_priority[i][2]) bt_state=hat_priority[i][2];
            if (hat_pos & SDL_HAT_LEFT)
                if (bt_state>hat_priority[i][3]) bt_state=hat_priority[i][3];
        }
	
        bool button_pressed[MAXBUTTON];
        for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
        for (i=0; i<MAX_VJOY_BUTTONS; i++) {
            if (virtual_joysticks[0].button_pressed[i])
                button_pressed[i % button_wrap]=true;
            
        }
		
        for (i=0; i<10; i++) {
            if ((button_pressed[i]) && (bt_state>button_priority[i]))
			{
				
				if ( (i >= 0) &&  (i <= 5) && (button_pressed[i]) )
				{
					bt_state=button_priority[i];
					//LOG_MSG("%d",bt_state);
						
				} else if  ( (i == 6) &&  (button_pressed[i] ) )
						
				{
					if (bt_state>15) bt_state=15;
					JOYSTICK_Button(0,1,(button_priority[8]&4)==0);							
					JOYSTICK_Button(1,0,(button_priority[8]&2)==0);			
					JOYSTICK_Button(0,1,(button_priority[9]&4)==0);							
					JOYSTICK_Button(1,1,(button_priority[9]&1)==0);	
					return;
						
				} else if  ( (i >= 7) &&  (button_pressed[i] ) )
						
				{
					if (bt_state>15) bt_state=15;
					JOYSTICK_Button(0,0,(button_priority[i]&7)==0);
					JOYSTICK_Button(0,1,(button_priority[i]&4)==0);							
					JOYSTICK_Button(1,0,(button_priority[i]&2)==0);
					JOYSTICK_Button(1,1,(button_priority[i]&1)==0);	
					return;						
				}																
			}
        }
		
        if (bt_state>15) bt_state=15;		
        JOYSTICK_Button(0,0,(bt_state&8)==0);
        JOYSTICK_Button(0,1,(bt_state&4)==0);
        JOYSTICK_Button(1,0,(bt_state&2)==0);
        JOYSTICK_Button(1,1,(bt_state&1)==0);	
	}

protected:
	Bit16u button_state;
};

/* ====================================================================================== QUICKSHOT 6 BUTTON */
class CShot6BindGroup : public  CStickBindGroup {
public:
	CShot6BindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=2;
		emulated_buttons=6;

		nJoyButtons = emulated_buttons;
		
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;

		JOYSTICK_Enable(1,true);
		JOYSTICK_Enable(2,true);	
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		Bitu but = 0;

		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 2) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
					JOYSTICK_Button((but >> 1),(but & 1),state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}		
		for (i=0; i<7; i++) {
 
			if( i <= 3 )
			{						
				JOYSTICK_Button(i>>1,i&1,button_pressed[i]);	
			}
			
			else if ( i == 4 )
			{
				if (button_pressed[4] == 1){	
					JOYSTICK_Move_X(1,-0.250000f);				
								
				}else{
					JOYSTICK_Move_X(1,0.0f);	
				}
			}
			else if ( i == 5 )
			{
				if (button_pressed[5] == 1){	
					JOYSTICK_Move_Y(1,-0.250000f);				
								
				}else{
					JOYSTICK_Move_Y(1,0.0f);	
				}
			}				
		}
		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);	
		// JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[3])/32768.0f);
		// JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[4])/32768.0f);			
	}
};
/* ====================================================================================== CAPCOM 6 BUTTON PAD */
class CCPADBindGroup : public  CStickBindGroup {
public:
	CCPADBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=2;
		emulated_buttons=6;
		nJoyButtons = emulated_buttons;
		
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;

		JOYSTICK_Enable(1,true);		
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		Bitu but = 0;

		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 2) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
					JOYSTICK_Button((but >> 1),(but & 1),state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}		
		for (i=0; i<7; i++) {

			if( i == 0 )
			{		
				
				JOYSTICK_Button(i>>1,i&1,button_pressed[2]);	
			}
			else if ( i == 2 )
			{
				JOYSTICK_Button(i>>1,i&1,button_pressed[0]);
			}	

			else if ( i == 1 )
			{
				JOYSTICK_Button(i>>1,i&1,button_pressed[3]);
			}
			
			else if ( i == 3 )
			{
				JOYSTICK_Button(i>>1,i&1,button_pressed[1]);
			}			
			
			else if ( i == 4 )
			{
				if (button_pressed[4] == 1){	
					// Achse X 3- Button Z
					JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[3])-0.50);				
								
				}else{
					JOYSTICK_Move_X(1,0.0f);	
				}					
			}
			else if ( i == 5 )
			{
				// Achse X 4- Button C
				if (button_pressed[5] == 1){	
					JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[4])-0.50);				
								
				}else{
					JOYSTICK_Move_Y(1,0.0f);	
				}
			}				
		}
		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);	
	}
};

/* ====================================================================================== INTERACT 6 BUTTON PAD */
class CIntA6BindGroup : public  CStickBindGroup {
public:
	CIntA6BindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=2;
		emulated_buttons=6;

		nJoyButtons = emulated_buttons;
		
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;

		JOYSTICK_Enable(1,true);
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		Bitu but = 0;

		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 2) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
					JOYSTICK_Button((but >> 1),(but & 1),state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}		
		for (i=0; i<7; i++) {
 
			if( i <= 3 )
			{						
				JOYSTICK_Button(i>>1,i&1,button_pressed[i]);	
			}
			
			else if ( i == 4 )
			{
				if (button_pressed[4] == 1){	
					JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])-0.523438);					
								
				}else{
					JOYSTICK_Move_X(1,0.0f);	
				}
			}
			else if ( i == 5 )
			{
				if (button_pressed[5] == 1){	
					JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[3])-0.523438);					
								
				}else{
					JOYSTICK_Move_Y(1,0.0f);	
				}
			}				
		}
		JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
		JOYSTICK_Move_Y(0,((float)virtual_joysticks[0].axis_pos[1])/32768.0f);
		
		//if ( JOYSTICK_GetMove_X(1) != 0.0f ){
			//LOG_MSG("Move %f",JOYSTICK_GetMove_X(1));
		//}		
	}
};

class CWheelBindGroup : public  CStickBindGroup {
public:
	CWheelBindGroup(Bitu _stick,Bitu _emustick) : CStickBindGroup (_stick,_emustick){
		emulated_axes=6;

		if ( nJoyButtons != 0 ){
			emulated_buttons = nJoyButtons;
		} else {
			emulated_buttons=4;
			nJoyButtons = emulated_buttons;
		}
		
		if (button_wrapping_enabled) button_wrap=emulated_buttons;

		axes_cap=emulated_axes;
		if (axes_cap>axes) axes_cap=axes;
		hats_cap=emulated_hats;
		if (hats_cap>hats) hats_cap=hats;

		JOYSTICK_Enable(1,true);
	}

	bool CheckEvent(SDL_Event * event) {
		SDL_JoyAxisEvent * jaxis = NULL;
		SDL_JoyButtonEvent * jbutton = NULL;
		Bitu but = 0;

		switch(event->type) {
			case SDL_JOYAXISMOTION:
				jaxis = &event->jaxis;
				if(jaxis->which == stick && jaxis->axis < 6) {
					if(jaxis->axis & 1)
						JOYSTICK_Move_Y(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
					else
						JOYSTICK_Move_X(jaxis->axis>>1 & 1,(float)(jaxis->value/32768.0));
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				jbutton = &event->jbutton;
				bool state;
				state=jbutton->type==SDL_JOYBUTTONDOWN;
				but = jbutton->button % emulated_buttons;
				if (jbutton->which == stick) {
					JOYSTICK_Button((but >> 1),(but & 1),state);
				}
				break;
		}
		return false;
	}

	virtual void UpdateJoystick() {
		/* query SDL joystick and activate bindings */
		ActivateJoystickBoundEvents();

		bool button_pressed[MAXBUTTON];
		Bitu i;
		for (i=0; i<MAXBUTTON; i++) button_pressed[i]=false;
		for (i=0; i<MAX_VJOY_BUTTONS; i++) {
			if (virtual_joysticks[0].button_pressed[i])
				button_pressed[i % button_wrap]=true;
			
		}
		for (i=0; i<emulated_buttons; i++) {
			
			if (autofire && (button_pressed[i]))				
				JOYSTICK_Button(i>>1,i&1,(++button_autofire[i])&1);
			else			
				JOYSTICK_Button(i>>1,i&1,button_pressed[i]);
		}

				JOYSTICK_Move_X(0,((float)virtual_joysticks[0].axis_pos[0])/32768.0f);
				JOYSTICK_Move_Y(1,((float)virtual_joysticks[0].axis_pos[1])/32768.0f); // Brake
				JOYSTICK_Move_X(1,((float)virtual_joysticks[0].axis_pos[2])/32768.0f); // Gas					
	}
};


static struct CMapper {
	SDL_Window * window;
	SDL_Rect draw_rect;
	SDL_Surface * draw_surface_nonpaletted; // Needed for SDL_BlitScaled
	SDL_Surface * surface;
	SDL_Surface * draw_surface;
	bool exit;
	CEvent * aevent;				//Active Event
	CBind * abind;					//Active Bind
	CBindList_it abindit;			//Location of active bind in list
	bool redraw;
	bool addbind;
	Bitu mods;
	struct {
		Bitu num_groups,num;
		CStickBindGroup * stick[MAXSTICKS];
	} sticks;
	std::string filename;
} mapper;

void CBindGroup::ActivateBindList(CBindList * list,Bits value,bool ev_trigger) {
	Bitu validmod=0;
	CBindList_it it;
	for (it=list->begin();it!=list->end();it++) {
		if (((*it)->mods & mapper.mods) == (*it)->mods) {
			if (validmod<(*it)->mods) validmod=(*it)->mods;
		}
	}
	for (it=list->begin();it!=list->end();it++) {
		if (validmod==(*it)->mods) (*it)->ActivateBind(value,ev_trigger);
	}
}

void CBindGroup::DeactivateBindList(CBindList * list,bool ev_trigger) {
	CBindList_it it;
	for (it=list->begin();it!=list->end();it++) {
		(*it)->DeActivateBind(ev_trigger);
	}
}

static void DrawText(Bitu x,Bitu y,const char * text,Bit8u color) {
	Bit8u * draw=((Bit8u *)mapper.draw_surface->pixels)+(y*mapper.draw_surface->w)+x;
	while (*text) {
		Bit8u * font=&int10_font_14[(*text)*14];
		Bitu i,j;Bit8u * draw_line=draw;
		for (i=0;i<14;i++) {
			Bit8u map=*font++;
			for (j=0;j<8;j++) {
				if (map & 0x80) *(draw_line+j)=color;
				else *(draw_line+j)=CLR_BLACK;
				map<<=1;
			}
			draw_line+=mapper.draw_surface->w;
		}
		text++;draw+=8;
	}
}

class CButton {
public:
	virtual ~CButton(){};
	CButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy) {
		x=_x;y=_y;dx=_dx;dy=_dy;
		buttons.push_back(this);
		color=CLR_WHITE;
		enabled=true;
	}
	virtual void Draw(void) {
		if (!enabled) return;
		Bit8u * point=((Bit8u *)mapper.draw_surface->pixels)+(y*mapper.draw_surface->w)+x;
		for (Bitu lines=0;lines<dy;lines++)  {
			if (lines==0 || lines==(dy-1)) {
				for (Bitu cols=0;cols<dx;cols++) *(point+cols)=color;
			} else {
				*point=color;*(point+dx-1)=color;
			}
			point+=mapper.draw_surface->w;
		}
	}
	virtual bool OnTop(Bitu _x,Bitu _y) {
		return ( enabled && (_x>=x) && (_x<x+dx) && (_y>=y) && (_y<y+dy));
	}
	virtual void BindColor(void) {}
	virtual void Click(void) {}
	void Enable(bool yes) { 
		enabled=yes; 
		mapper.redraw=true;
	}
	void SetColor(Bit8u _col) { color=_col; }
protected:
	Bitu x,y,dx,dy;
	Bit8u color;
	bool enabled;
};

class CTextButton : public CButton {
public:
	CTextButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy,const char * _text) : CButton(_x,_y,_dx,_dy) { text=_text;}
	void Draw(void) {
		if (!enabled) return;
		CButton::Draw();
		DrawText(x+2,y+2,text,color);
	}
protected:
	const char * text;
};

class CClickableTextButton : public CTextButton {
public:
	CClickableTextButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy,const char * _text) : CTextButton(_x,_y,_dx,_dy,_text) {}
	void BindColor(void) {
		this->SetColor(CLR_WHITE);
	}
};

class CEventButton;
static CEventButton * last_clicked = NULL;

class CEventButton : public CClickableTextButton {
public:
	CEventButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy,const char * _text,CEvent * _event) 
	: CClickableTextButton(_x,_y,_dx,_dy,_text) 	{ 
		event=_event;	
	}
	void BindColor(void) {
		this->SetColor(event->bindlist.begin()==event->bindlist.end() ? CLR_GREY : CLR_WHITE);
	}
	void Click(void) {
		if (last_clicked) last_clicked->BindColor();
		this->SetColor(CLR_GREEN);
		SetActiveEvent(event);
		last_clicked=this;
	}
protected:
	CEvent * event;
};

class CCaptionButton : public CButton {
public:
	CCaptionButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy) : CButton(_x,_y,_dx,_dy){
		caption[0]=0;
	}
	void Change(const char * format,...) GCC_ATTRIBUTE(__format__(__printf__,2,3));

	void Draw(void) {
		if (!enabled) return;
		DrawText(x+2,y+2,caption,color);
	}
protected:
	char caption[128];
};

void CCaptionButton::Change(const char * format,...) {
	va_list msg;
	va_start(msg,format);
	vsprintf(caption,format,msg);
	va_end(msg);
	mapper.redraw=true;
}		

static void change_action_text(const char* text,Bit8u col);

static void MAPPER_SaveBinds(void);
class CBindButton : public CClickableTextButton {
public:	
	CBindButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy,const char * _text,BB_Types _type) 
	: CClickableTextButton(_x,_y,_dx,_dy,_text) 	{ 
		type=_type;
	}
	void Click(void) {
		switch (type) {
		case BB_Add: 
			mapper.addbind=true;
			SetActiveBind(0);
			change_action_text("Press a Key/Joystick Button or Move The Joystick. (ESC For Abort)",CLR_RED);
			break;
		case BB_Del:
			if (mapper.abindit!=mapper.aevent->bindlist.end())  {
				delete (*mapper.abindit);
				mapper.abindit=mapper.aevent->bindlist.erase(mapper.abindit);
				if (mapper.abindit==mapper.aevent->bindlist.end()) 
					mapper.abindit=mapper.aevent->bindlist.begin();
			}
			if (mapper.abindit!=mapper.aevent->bindlist.end()) SetActiveBind(*(mapper.abindit));
			else SetActiveBind(0);
			break;
		case BB_Next:
			if (mapper.abindit!=mapper.aevent->bindlist.end()) 
				mapper.abindit++;
			if (mapper.abindit==mapper.aevent->bindlist.end()) 
				mapper.abindit=mapper.aevent->bindlist.begin();
			SetActiveBind(*(mapper.abindit));
			break;
		case BB_Save:
			MAPPER_SaveBinds();
			break;
		case BB_Exit:   
			mapper.exit=true;
			break;
		}
	}
protected:
	BB_Types type;
};

class CCheckButton : public CClickableTextButton {
public:	
	CCheckButton(Bitu _x,Bitu _y,Bitu _dx,Bitu _dy,const char * _text,BC_Types _type) 
	: CClickableTextButton(_x,_y,_dx,_dy,_text) 	{ 
		type=_type;
	}
	void Draw(void) {
		if (!enabled) return;
		bool checked=false;
		switch (type) {
		case BC_Mod1:
			checked=(mapper.abind->mods&BMOD_Mod1)>0;
			break;
		case BC_Mod2:
			checked=(mapper.abind->mods&BMOD_Mod2)>0;
			break;
		case BC_Mod3:
			checked=(mapper.abind->mods&BMOD_Mod3)>0;
			break;
		case BC_Hold:
			checked=(mapper.abind->flags&BFLG_Hold)>0;
			break;
		}
		if (checked) {
			Bit8u * point=((Bit8u *)mapper.draw_surface->pixels)+((y+2)*mapper.draw_surface->w)+x+dx-dy+2;
			for (Bitu lines=0;lines<(dy-4);lines++)  {
				memset(point,color,dy-4);
				point+=mapper.draw_surface->w;
			}
		}
		CClickableTextButton::Draw();
	}
	void Click(void) {
		switch (type) {
		case BC_Mod1:
			mapper.abind->mods^=BMOD_Mod1;
			break;
		case BC_Mod2:
			mapper.abind->mods^=BMOD_Mod2;
			break;
		case BC_Mod3:
			mapper.abind->mods^=BMOD_Mod3;
			break;
		case BC_Hold:
			mapper.abind->flags^=BFLG_Hold;
			break;
		}
		mapper.redraw=true;
	}
protected:
	BC_Types type;
};

class CKeyEvent : public CTriggeredEvent {
public:
	CKeyEvent(char const * const _entry,KBD_KEYS _key) : CTriggeredEvent(_entry) {
		key=_key;
	}
	void Active(bool yesno) {
		KEYBOARD_AddKey(key,yesno);
	};
	KBD_KEYS key;
};

class CMouseButtonEvent : public CTriggeredEvent {
public:
	CMouseButtonEvent(char const * const _entry,Bit8u _button) : CTriggeredEvent(_entry) {
		button=_button;
	}
	void Active(bool yesno) {
		if (yesno)
			Mouse_ButtonPressed(button);
		else
			Mouse_ButtonReleased(button);
	}
	Bit8u button;
};


class CJAxisEvent : public CContinuousEvent {
public:
	CJAxisEvent(char const * const _entry,Bitu _stick,Bitu _axis,bool _positive,CJAxisEvent * _opposite_axis) : CContinuousEvent(_entry) {
		stick=_stick;
		axis=_axis;
		positive=_positive;
		opposite_axis=_opposite_axis;
		if (_opposite_axis) {
			_opposite_axis->SetOppositeAxis(this);
		}
	}
	void Active(bool /*moved*/) {
		virtual_joysticks[stick].axis_pos[axis]=(Bit16s)(GetValue()*(positive?1:-1));
	}
	virtual Bitu GetActivityCount(void) {
		return activity|opposite_axis->activity;
	}
	virtual void RepostActivity(void) {
		/* caring for joystick movement into the opposite direction */
		opposite_axis->Active(true);
	}
protected:
	void SetOppositeAxis(CJAxisEvent * _opposite_axis) {
		opposite_axis=_opposite_axis;
	}
	Bitu stick,axis;
	bool positive;
	CJAxisEvent * opposite_axis;
};

class CJButtonEvent : public CTriggeredEvent {
public:
	CJButtonEvent(char const * const _entry,Bitu _stick,Bitu _button) : CTriggeredEvent(_entry) {
		stick=_stick;
		button=_button;
	}
	void Active(bool pressed) {
		virtual_joysticks[stick].button_pressed[button]=pressed;
	}
protected:
	Bitu stick,button;
};

class CJHatEvent : public CTriggeredEvent {
public:
	CJHatEvent(char const * const _entry,Bitu _stick,Bitu _hat,Bitu _dir) : CTriggeredEvent(_entry) {
		stick=_stick;
		hat=_hat;
		dir=_dir;
	}
	void Active(bool pressed) {
		virtual_joysticks[stick].hat_pressed[(hat<<2)+dir]=pressed;
	}
protected:
	Bitu stick,hat,dir;
};


class CModEvent : public CTriggeredEvent {
public:
	CModEvent(char const * const _entry,Bitu _wmod) : CTriggeredEvent(_entry) {
		wmod=_wmod;
	}
	void Active(bool yesno) {
		if (yesno) mapper.mods|=(1 << (wmod-1));
		else mapper.mods&=~(1 << (wmod-1));
	};
protected:
	Bitu wmod;
};

class CHandlerEvent : public CTriggeredEvent {
public:
	CHandlerEvent(char const * const _entry,MAPPER_Handler * _handler,MapKeys _key,Bitu _mod,char const * const _buttonname) : CTriggeredEvent(_entry) {
		handler=_handler;
		defmod=_mod;
		defkey=_key;
		buttonname=_buttonname;
		handlergroup.push_back(this);
	}
	void Active(bool yesno) {
		(*handler)(yesno);
	};
	const char * ButtonName(void) {
		return buttonname;
	}
	void MakeDefaultBind(char * buf) {
		Bitu key=0;
		switch (defkey) {
		case MK_f1:case MK_f2:case MK_f3:case MK_f4:
		case MK_f5:case MK_f6:case MK_f7:case MK_f8:
		case MK_f9:case MK_f10:case MK_f11:case MK_f12:	
			key=SDL_SCANCODE_F1+(defkey-MK_f1);
			break;
		case MK_return:
			key=SDL_SCANCODE_RETURN;
			break;
		case MK_kpminus:
			key=SDL_SCANCODE_KP_MINUS;
			break;
		case MK_scrolllock:
			key=SDL_SCANCODE_SCROLLLOCK;
			break;
		case MK_pause:
			key=SDL_SCANCODE_PAUSE;
			break;
		case MK_printscreen:
			key=SDL_SCANCODE_PRINTSCREEN;
			break;
		case MK_home: 
			key=SDL_SCANCODE_HOME; 
			break;
		/* Quit Dosbox Options			*/
		case MK_WHDA:
			key=SDL_SCANCODE_KP_MULTIPLY;
			break;
		case MK_WHDB:
			key=SDL_SCANCODE_KP_MINUS;
			break;	
		case MK_WHDC:
			key=SDL_SCANCODE_KP_PLUS;
			break;
		case MK_WHDD:
			key=SDL_SCANCODE_KP_DIVIDE;
			break;				
		/* Volume Up Down Media Keys */
		case MK_ACVM:
			key=SDL_SCANCODE_MUTE;
			break;	
		case MK_ACVD:
			key=SDL_SCANCODE_VOLUMEDOWN;
			break;	
		case MK_ACVU:
			key=SDL_SCANCODE_VOLUMEUP;
			break;				
		}
		sprintf(buf,"%s \"key %" sBitfs(d) "%s%s%s\"",
			entry,
			key,
			defmod & 1 ? " mod1" : "",
			defmod & 2 ? " mod2" : "",
			defmod & 4 ? " mod3" : ""
		);
	}

protected:
	MapKeys defkey;
	Bitu defmod;
	MAPPER_Handler * handler;
public:
	const char * buttonname;
};


static struct {
	CCaptionButton *  event_title;
	CCaptionButton *  bind_title;
	CCaptionButton *  selected;
	CCaptionButton *  action;
	CBindButton * save;
	CBindButton * exit;   
	CBindButton * add;
	CBindButton * del;
	CBindButton * next;
	CCheckButton * mod1,* mod2,* mod3,* hold;
} bind_but;


static void change_action_text(const char* text,Bit8u col) {
	bind_but.action->Change(text,"");
	bind_but.action->SetColor(col);
}


static void SetActiveBind(CBind * _bind) {
	mapper.abind=_bind;
	if (_bind) {
		bind_but.bind_title->Enable(true);
		char buf[256];_bind->BindName(buf);
		bind_but.bind_title->Change("BIND:%s",buf);
		bind_but.del->Enable(true);
		bind_but.next->Enable(true);
		bind_but.mod1->Enable(true);
		bind_but.mod2->Enable(true);
		bind_but.mod3->Enable(true);
		bind_but.hold->Enable(true);
	} else {
		bind_but.bind_title->Enable(false);
		bind_but.del->Enable(false);
		bind_but.next->Enable(false);
		bind_but.mod1->Enable(false);
		bind_but.mod2->Enable(false);
		bind_but.mod3->Enable(false);
		bind_but.hold->Enable(false);
	}
}

static void SetActiveEvent(CEvent * event) {
	mapper.aevent=event;
	mapper.redraw=true;
	mapper.addbind=false;
	bind_but.event_title->Change("EVENT:%s",event ? event->GetName(): "none");
	if (!event) {
		change_action_text("               Select an event to change.",CLR_WHITE);
		bind_but.add->Enable(false);
		SetActiveBind(0);
	} else {
		change_action_text("Select a different event or hit the Add/ Delete/ Next Buttons.",CLR_WHITE);
		mapper.abindit=event->bindlist.begin();
		if (mapper.abindit!=event->bindlist.end()) {
			SetActiveBind(*(mapper.abindit));
		} else SetActiveBind(0);
		bind_but.add->Enable(true);
	}
}

extern SDL_Window * GFX_SetSDLSurfaceWindow(Bit16u width, Bit16u height);
extern SDL_Rect GFX_GetSDLSurfaceSubwindowDims(Bit16u width, Bit16u height);
extern void GFX_UpdateDisplayDimensions(int width, int height);

static void DrawButtons(void) {
	SDL_FillRect(mapper.draw_surface,0,CLR_BLACK);
	for (CButton_it but_it = buttons.begin();but_it!=buttons.end();but_it++) {
		(*but_it)->Draw();
	}
	// We can't just use SDL_BlitScaled (say for Android) in one step
	SDL_BlitSurface(mapper.draw_surface, NULL, mapper.draw_surface_nonpaletted, NULL);
	SDL_BlitScaled(mapper.draw_surface_nonpaletted, NULL, mapper.surface, &mapper.draw_rect);
	//SDL_BlitSurface(mapper.draw_surface, NULL, mapper.surface, NULL);
	SDL_UpdateWindowSurface(mapper.window);
}

static CMouseButtonEvent * AddMouseButtonEvent(Bitu x,Bitu y,Bitu dx,Bitu dy,char const * const title,char const * const entry,Bit8u button) {
	char buf[64];
	strcpy(buf,"mouse_");
	strcat(buf,entry);
	CMouseButtonEvent * event=new CMouseButtonEvent(buf,button);
	new CEventButton(x,y,dx,dy,title,event);
	return event;
}


static CKeyEvent * AddKeyButtonEvent(Bitu x,Bitu y,Bitu dx,Bitu dy,char const * const title,char const * const entry,KBD_KEYS key) {
	char buf[64];
	strcpy(buf,"key_");
	strcat(buf,entry);
	CKeyEvent * event=new CKeyEvent(buf,key);
	new CEventButton(x,y,dx,dy,title,event);
	return event;
}

static CJAxisEvent * AddJAxisButton(Bitu x,Bitu y,Bitu dx,Bitu dy,char const * const title,Bitu stick,Bitu axis,bool positive,CJAxisEvent * opposite_axis) {
	char buf[64];
	sprintf(buf,"jaxis_%" sBitfs(d) "_%" sBitfs(d) "%s",stick,axis,(positive ? "+" : "-"));
	CJAxisEvent	* event=new CJAxisEvent(buf,stick,axis,positive,opposite_axis);
	new CEventButton(x,y,dx,dy,title,event);
	return event;
}
static CJAxisEvent * AddJAxisButton_hidden(Bitu stick,Bitu axis,bool positive,CJAxisEvent * opposite_axis) {
	char buf[64];
	sprintf(buf,"jaxis_%" sBitfs(d) "_%" sBitfs(d) "%s",stick,axis, (positive ? "+" : "-") );
	return new CJAxisEvent(buf,stick,axis,positive,opposite_axis);
}

static void AddJButtonButton(Bitu x,Bitu y,Bitu dx,Bitu dy,char const * const title,Bitu stick,Bitu button) {
	char buf[64];
	sprintf(buf,"jbutton_%" sBitfs(d) "_%" sBitfs(d),stick,button);
	CJButtonEvent * event=new CJButtonEvent(buf,stick,button);
	new CEventButton(x,y,dx,dy,title,event);
}
static void AddJButtonButton_hidden(Bitu stick,Bitu button) {
	char buf[64];
	sprintf(buf,"jbutton_%" sBitfs(d) "_%" sBitfs(d),stick,button);
	new CJButtonEvent(buf,stick,button);
}

static void AddJHatButton(Bitu x,Bitu y,Bitu dx,Bitu dy,char const * const title,Bitu _stick,Bitu _hat,Bitu _dir) {
	char buf[64];
	sprintf(buf,"jhat_%" sBitfs(d) "_%" sBitfs(d) "_%" sBitfs(d),_stick,_hat,_dir);
	CJHatEvent * event=new CJHatEvent(buf,_stick,_hat,_dir);
	new CEventButton(x,y,dx,dy,title,event);
}

static void AddJHatButton_hidden(Bitu _stick,Bitu _hat,Bitu _dir) {
    char buf[64];
	sprintf(buf,"jhat_%" sBitfs(d) "_%" sBitfs(d) "_%" sBitfs(d),_stick,_hat,_dir);
    new CJHatEvent(buf,_stick,_hat,_dir);
}

static void AddModButton(Bitu x,Bitu y,Bitu dx,Bitu dy,char const * const title,Bitu _mod) {
	char buf[64];
	sprintf(buf,"mod_%" sBitfs(d),_mod);
	CModEvent * event=new CModEvent(buf,_mod);
	new CEventButton(x,y,dx,dy,title,event);
}

struct KeyBlock {
	const char * title;
	const char * entry;
	KBD_KEYS key;
};
static KeyBlock combo_f[12]={
	{"F1","f1",KBD_f1},		{"F2","f2",KBD_f2},		{"F3","f3",KBD_f3},
	{"F4","f4",KBD_f4},		{"F5","f5",KBD_f5},		{"F6","f6",KBD_f6},
	{"F7","f7",KBD_f7},		{"F8","f8",KBD_f8},		{"F9","f9",KBD_f9},
	{"F10","f10",KBD_f10},	{"F11","f11",KBD_f11},	{"F12","f12",KBD_f12},
};

static KeyBlock combo_1[14]={
	{"`~","grave",KBD_grave},	{"1!","1",KBD_1},	{"2@","2",KBD_2},
	{"3#","3",KBD_3},			{"4$","4",KBD_4},	{"5%","5",KBD_5},
	{"6^","6",KBD_6},			{"7&","7",KBD_7},	{"8*","8",KBD_8},
	{"9(","9",KBD_9},			{"0)","0",KBD_0},	{"-_","minus",KBD_minus},
	{"=+","equals",KBD_equals},	{"\x1B","bspace",KBD_backspace},
};

static KeyBlock combo_2[12]={
	{"Q","q",KBD_q},			{"W","w",KBD_w},	{"E","e",KBD_e},
	{"R","r",KBD_r},			{"T","t",KBD_t},	{"Y","y",KBD_y},
	{"U","u",KBD_u},			{"I","i",KBD_i},	{"O","o",KBD_o},
	{"P","p",KBD_p},			{"[{","lbracket",KBD_leftbracket},
	{"]}","rbracket",KBD_rightbracket},	
};

static KeyBlock combo_3[12]={
	{"A","a",KBD_a},			{"S","s",KBD_s},	{"D","d",KBD_d},
	{"F","f",KBD_f},			{"G","g",KBD_g},	{"H","h",KBD_h},
	{"J","j",KBD_j},			{"K","k",KBD_k},	{"L","l",KBD_l},
	{";:","semicolon",KBD_semicolon},				{"'\"","quote",KBD_quote},
	{"\\|","backslash",KBD_backslash},
};

static KeyBlock combo_4[11]={
	{"<>","lessthan",KBD_extra_lt_gt},
	{"Z","z",KBD_z},			{"X","x",KBD_x},	{"C","c",KBD_c},
	{"V","v",KBD_v},			{"B","b",KBD_b},	{"N","n",KBD_n},
	{"M","m",KBD_m},			{",<","comma",KBD_comma},
	{".>","period",KBD_period},						{"/?","slash",KBD_slash},
};

static CKeyEvent * caps_lock_event=NULL;
static CKeyEvent * num_lock_event=NULL;

static void CreateLayout(void) {
	Bitu i;
	/* Create the buttons for the Keyboard */
#define BW 28
#define BH 20
#define DX 5
#define PX(_X_) ((_X_)*BW + DX)
#define PY(_Y_) (10+(_Y_)*BH)
	AddKeyButtonEvent(PX(0),PY(0),BW,BH,"ESC","esc",KBD_esc);
	for (i=0;i<12;i++) AddKeyButtonEvent(PX(2+i),PY(0),BW,BH,combo_f[i].title,combo_f[i].entry,combo_f[i].key);
	for (i=0;i<14;i++) AddKeyButtonEvent(PX(  i),PY(1),BW,BH,combo_1[i].title,combo_1[i].entry,combo_1[i].key);

	AddKeyButtonEvent(PX(0),PY(2),BW*2,BH,"TAB","tab",KBD_tab);
	for (i=0;i<12;i++) AddKeyButtonEvent(PX(2+i),PY(2),BW,BH,combo_2[i].title,combo_2[i].entry,combo_2[i].key);

	AddKeyButtonEvent(PX(14),PY(2),BW*2,BH*2,"ENTER","enter",KBD_enter);
	
	caps_lock_event=AddKeyButtonEvent(PX(0),PY(3),BW*2,BH,"CLCK","capslock",KBD_capslock);
	for (i=0;i<12;i++) AddKeyButtonEvent(PX(2+i),PY(3),BW,BH,combo_3[i].title,combo_3[i].entry,combo_3[i].key);

	AddKeyButtonEvent(PX(0),PY(4),BW*2,BH,"SHIFT","lshift",KBD_leftshift);
	for (i=0;i<11;i++) AddKeyButtonEvent(PX(2+i),PY(4),BW,BH,combo_4[i].title,combo_4[i].entry,combo_4[i].key);
	AddKeyButtonEvent(PX(13),PY(4),BW*3,BH,"SHIFT","rshift",KBD_rightshift);

	/* Last Row */
	/* Last Row */
	AddKeyButtonEvent(PX(0) ,PY(5),BW*2,BH,"CTRL","lctrl",KBD_leftctrl);
	AddKeyButtonEvent(PX(2) ,PY(5),BW*1,BH,"WIN","lwindows",KBD_lwindows);
	AddKeyButtonEvent(PX(3) ,PY(5),BW*1,BH,"ALT","lalt",KBD_leftalt);
	AddKeyButtonEvent(PX(4) ,PY(5),BW*7,BH,"SPACE","space",KBD_space);
	AddKeyButtonEvent(PX(11),PY(5),BW*1,BH,"ALT","ralt",KBD_rightalt);
	AddKeyButtonEvent(PX(12),PY(5),BW*1,BH,"WIN","rwindows",KBD_rwindows);
	AddKeyButtonEvent(PX(13),PY(5),BW*1,BH,"WMN","rwinmenu",KBD_rwinmenu);
	AddKeyButtonEvent(PX(14),PY(5),BW*2,BH,"CTRL","rctrl",KBD_rightctrl);

	/* Arrow Keys */
#define XO 17
#define YO 0

	AddKeyButtonEvent(PX(XO+0),PY(YO),BW,BH,"PRT","printscreen",KBD_printscreen);
	AddKeyButtonEvent(PX(XO+1),PY(YO),BW,BH,"SCL","scrolllock",KBD_scrolllock);
	AddKeyButtonEvent(PX(XO+2),PY(YO),BW,BH,"PAU","pause",KBD_pause);
	AddKeyButtonEvent(PX(XO+0),PY(YO+1),BW,BH,"INS","insert",KBD_insert);
	AddKeyButtonEvent(PX(XO+1),PY(YO+1),BW,BH,"HOM","home",KBD_home);
	AddKeyButtonEvent(PX(XO+2),PY(YO+1),BW,BH,"PUP","pageup",KBD_pageup);
	AddKeyButtonEvent(PX(XO+0),PY(YO+2),BW,BH,"DEL","delete",KBD_delete);
	AddKeyButtonEvent(PX(XO+1),PY(YO+2),BW,BH,"END","end",KBD_end);
	AddKeyButtonEvent(PX(XO+2),PY(YO+2),BW,BH,"PDN","pagedown",KBD_pagedown);
	AddKeyButtonEvent(PX(XO+1),PY(YO+4),BW,BH,"\x18","up",KBD_up);
	AddKeyButtonEvent(PX(XO+0),PY(YO+5),BW,BH,"\x1B","left",KBD_left);
	AddKeyButtonEvent(PX(XO+1),PY(YO+5),BW,BH,"\x19","down",KBD_down);
	AddKeyButtonEvent(PX(XO+2),PY(YO+5),BW,BH,"\x1A","right",KBD_right);
#undef XO
#undef YO
#define XO 0
#define YO 7
	/* Numeric KeyPad */
	num_lock_event=AddKeyButtonEvent(PX(XO),PY(YO),BW,BH,"NUM","numlock",KBD_numlock);
	AddKeyButtonEvent(PX(XO+1),PY(YO),BW,BH,"/","kp_divide",KBD_kpdivide);
	AddKeyButtonEvent(PX(XO+2),PY(YO),BW,BH,"*","kp_multiply",KBD_kpmultiply);
	AddKeyButtonEvent(PX(XO+3),PY(YO),BW,BH,"-","kp_minus",KBD_kpminus);
	AddKeyButtonEvent(PX(XO+0),PY(YO+1),BW,BH,"7","kp_7",KBD_kp7);
	AddKeyButtonEvent(PX(XO+1),PY(YO+1),BW,BH,"8","kp_8",KBD_kp8);
	AddKeyButtonEvent(PX(XO+2),PY(YO+1),BW,BH,"9","kp_9",KBD_kp9);
	AddKeyButtonEvent(PX(XO+3),PY(YO+1),BW,BH*2,"+","kp_plus",KBD_kpplus);
	AddKeyButtonEvent(PX(XO),PY(YO+2),BW,BH,"4","kp_4",KBD_kp4);
	AddKeyButtonEvent(PX(XO+1),PY(YO+2),BW,BH,"5","kp_5",KBD_kp5);
	AddKeyButtonEvent(PX(XO+2),PY(YO+2),BW,BH,"6","kp_6",KBD_kp6);
	AddKeyButtonEvent(PX(XO+0),PY(YO+3),BW,BH,"1","kp_1",KBD_kp1);
	AddKeyButtonEvent(PX(XO+1),PY(YO+3),BW,BH,"2","kp_2",KBD_kp2);
	AddKeyButtonEvent(PX(XO+2),PY(YO+3),BW,BH,"3","kp_3",KBD_kp3);
	AddKeyButtonEvent(PX(XO+3),PY(YO+3),BW,BH*2,"ENT","kp_enter",KBD_kpenter);
	AddKeyButtonEvent(PX(XO),PY(YO+4),BW*2,BH,"0","kp_0",KBD_kp0);	
	AddKeyButtonEvent(PX(XO+2),PY(YO+4),BW,BH,".","kp_period",KBD_kpperiod);
#undef XO
#undef YO
#define XO 21
#define YO 0	
	AddKeyButtonEvent(PX(XO),PY(YO),BW+96,BH,"Media Vol. Mute","audiomute",KBD_audiomute);
	AddKeyButtonEvent(PX(XO),PY(YO+1),BW+96,BH,"Media Volume -","volumedown",KBD_volumedown);
	AddKeyButtonEvent(PX(XO),PY(YO+2),BW+96,BH,"Media Volume +","volumeup",KBD_volumeup);	
#undef XO
#undef YO
#define XO 21
#define YO 4
	/* Mouse Buttons */

	new CTextButton(PX(XO+0),PY(YO),BW+96,BH,"Mouse Button's");
	AddMouseButtonEvent(PX(XO+0),PY(YO+1),BW+7,BH,"Left","left",SDL_BUTTON_LEFT);
	AddMouseButtonEvent(PX(XO+1)+7,PY(YO+1),BW+18,BH," Mid","middle",SDL_BUTTON_MIDDLE);
	AddMouseButtonEvent(PX(XO+2)+25,PY(YO+1),BW+15,BH,"Right","right",SDL_BUTTON_RIGHT);
#undef XO
#undef YO
#define XO 5
#define YO 8

	CJAxisEvent * cjaxis;
	CTextButton * btn;
	const char * sJLabel1 = "  Disabled";
	const char * sJLabel2 = "  Disabled";	
	const char * sJLabel3 = "  Disabled";
	const char * sJLabel4 = "  Disabled";
	const char * sJLabel5 = "  Disabled";		
	const char * sJLabel6 = "  Disabled";		
	
	/* Labels */

	switch(joytype)
	{
		case JOY_2AXIS:	
		{
			sJLabel1 = "  Joystick 1";
			sJLabel2 = "D-Pad/Hat(J1)";			
			sJLabel3 = "  Joystick 2";
			sJLabel4 = "D-Pad/Hat(J2)";
			sJLabel5 = "Config: Supports and use two Joysticks with (Host ID #1 + #2).";			
		}
		break;
		
		case JOY_4AXIS:				
		{
			sJLabel1 = "  Joystick 1";
			sJLabel2 = "D-Pad/Hat(J1)";	
			sJLabel3 = " Axis 3/4";	
			sJLabel5 = "Config: The 1st joystick used (Host ID #1).";
		}		
		
		case JOY_WHEEL:				
		{
			sJLabel1 = "  Buttons";
			sJLabel2 = "Wheel";	
			sJLabel3 = " Pedal/Throttle";	
			sJLabel5 = "Config: The 1st joystick used as Wheel Pedal/Throttle (Host ID #1).";
		}	
		
		break;
		
		case JOY_4AXIS_2:				
		{
			sJLabel1 = "  Joystick 2";
			sJLabel2 = "D-Pad/Hat(J2)";	
			sJLabel3 = " Axis 3/4";	
			sJLabel5 = "Config: Second joystick on Host ID #2 used.";			
		}
		break;		
		case JOY_CH:
		{
			sJLabel1 = "Joystick Buttons";
			sJLabel2 = "Joy. Axis 1/2";	
			sJLabel3 = "Joy. Axis 3/4";			
			sJLabel4 = "  HAT System";
			sJLabel5 = "Config: CH Flightstick PRO";		
		}
		break;


		case JOY_FCSLW:
		{
			sJLabel1 = "Joystick Buttons";
			sJLabel2 = "Joy. Axis 1/2";	
			sJLabel3 = "Rudd./Thottle";	
			sJLabel4 = "Joy. Axis 3/4";				
			sJLabel6 = "  HAT System";
			
			/* ==================================================================================================== */
			switch(HostCtrlType)
			{			
				case HOSTJOY_DEFAULT:			
				{
					sJLabel5 = "Config: FLCSWL (Emulate with Default)";
				}
				break;
				case HOSTJOY_SAITEKX45:			
				{
					sJLabel5 = "Config: FLCSWL (Emulate with Saitek-X45)";
				}
				break;	
				case HOSTJOY_LEXT3DPRO:			
				{
					sJLabel5 = "Config: FLCSWL (Logitech Exptreme 3D Pro)";
				}
				break;		
				case HOSTJOY_TFLIGHTHO:			
				{
					sJLabel5 = "Config: FLCSWL (Thrustmaster Flight Hotas One)";
				}
				break;	
				case HOSTJOY_LDRIVINGF:			
				{
					sJLabel5 = "Config: FLCSWL (Logitech Driving Force /Pro)";
				}
				break;				
			}
			/* ==================================================================================================== */
			
		
		}
		break;
		case JOY_CHVP:	
		{
			sJLabel1 = "Joystick Buttons";
			sJLabel2 = "Joy. Axis 1/2";	
			sJLabel3 = "Rudd./Thottle";	
			sJLabel4 = "Joy. Axis 3/4";				
			sJLabel6 = "  HAT System";
			
			/* ==================================================================================================== */
			switch(HostCtrlType)
			{			
				case HOSTJOY_DEFAULT:			
				{
					sJLabel5 = "Config: CH Virtual Pilot (98)/Flightstick (98) (Emulate with Default)";
				}
				break;
				case HOSTJOY_SAITEKX45:			
				{
					sJLabel5 = "Config: CH Virtual Pilot (98)/Flightstick (98) (Emulate with Saitek-X45)";
				}
				break;	
				case HOSTJOY_LEXT3DPRO:			
				{
					sJLabel5 = "Config: CH Virtual Pilot (98)/Flightstick (98) (Logitech Exptreme 3D Pro)";
				}
				break;		
				case HOSTJOY_TFLIGHTHO:			
				{
					sJLabel5 = "Config: CH Virtual Pilot/ Flightstick (98) (Thrustmaster Flight Hotas One)";
				}
				break;	
				case HOSTJOY_LDRIVINGF:			
				{
					sJLabel5 = "Config: CH Virtual Pilot/ Flightstick (98) (Logitech Driving Force /Pro)";
				}
				break;				
			}
			/* ==================================================================================================== */
			
		
		}
		break;		

		case JOY_FCS:
		{
			sJLabel1 = "Joystick Buttons";
			sJLabel2 = "Joy. Axis 1/2";	
			sJLabel3 = "Joy. Axis 3";			
			sJLabel4 = "  HAT System";
			sJLabel5 = "Config: Thrustmaster FCS (Flight Control System)";		
		}
		break;		
		case JOY_SHOT6:
		{
			sJLabel1 = "Quickshot Pad";
			sJLabel5 = "Config: Quickshot 6 Button Pad";		
		}
		break;		
		case JOY_INTER6:
		{
			sJLabel1 = "Interact PCPad";
			sJLabel5 = "Config: Interact 6 Button Pc Pad (Pro)";		
		}		
		break;
		case JOY_CAPCOM:
		{
			sJLabel1 = "Directional Hat <--> Buttons";		
			sJLabel5 = "Config: Capcom 6 Button Pad";		
		}
		break;	
		case JOY_CHGS:
		{
			sJLabel1 = "Directional";
			sJLabel2 = "Joy. Axis 1/2";	
			sJLabel3 = "Rudd./Thottle";	
			sJLabel4 = "Joy. Axis 3/4";				
			sJLabel6 = "  HAT System";
			sJLabel5 = "Config: CH Gamestick v1.4 (Emulate with Xbox, F710)";	
		}		
		case JOY_NONE:
		{	
		}
		break;		
	}
	
	/* Setup Buttons Counter */
	if ( (joytype==JOY_2AXIS) && ( nJoyButtons == 0) ){
			nJoyButtons = 2;
	}
	
	if ((joytype==JOY_4AXIS) || (joytype==JOY_4AXIS_2)){
		if ( nJoyButtons == 0)
		{
			nJoyButtons = 4;
		}
	}
	
	if ( (joytype==JOY_SHOT6) || (joytype==JOY_CAPCOM) ){
		nJoyButtons = 6;
	}	
	
	/* Layout 1 =================================================================================*/
    if ( (joytype==JOY_2AXIS) || (joytype==JOY_4AXIS) || (joytype==JOY_4AXIS_2) )
	{	
		new CTextButton(PX(XO+1)-2,PY(YO-1),4*BW,20,sJLabel1);
		new CTextButton(PX(XO+6)-2,PY(YO-1),4*BW,20,sJLabel2);
		new CTextButton(PX(XO+12) ,PY(YO-1),4*BW,20,sJLabel3);
		
		if (joytype==JOY_2AXIS){
			new CTextButton(PX(XO+18),PY(YO-1),4*BW,20,sJLabel4);		
		}

		/* Buttons 1+8 of 1st Joystick */	
		for (int i=1;i<=nJoyButtons;i++){		
			switch(i)
			{
				case 5: {AddJButtonButton(PX(XO+2)+12,PY(YO+1)+22,BW   ,BH+6," 5" ,0,4); } break;
				case 3: {AddJButtonButton(PX(XO+1)   ,PY(YO)+5   ,BW   ,BH+7," 3" ,0,2); } break;	
				case 4: {AddJButtonButton(PX(XO+4)-4 ,PY(YO)+5   ,BW   ,BH+7," 4" ,0,3); } break;	//3
				case 1: {AddJButtonButton(PX(XO+1)   ,PY(YO+3)+18,BW   ,BH+7," 1" ,0,0); } break;	//4
				case 2: {AddJButtonButton(PX(XO+4)-4 ,PY(YO+3)+18,BW   ,BH+7," 2" ,0,1); } break;
				case 6: {AddJButtonButton(PX(XO+1)   ,PY(YO+5)+10,BW+8 ,BH+7," 6" ,0,5); } break;
				case 7: {AddJButtonButton(PX(XO+2)+10,PY(YO+5)+10,BW+5 ,BH+7," 7" ,0,6); } break;	
				case 8: {AddJButtonButton(PX(XO+4)-11,PY(YO+5)+10,BW+8 ,BH+7," 8" ,0,7); } break;			
			}					
		}	
		for (int i=nJoyButtons;i<13;i++) { AddJButtonButton_hidden(0,i);}	

	
	/* Axes 1+2 (X+Y) of 1st Joystick */
	cjaxis = AddJAxisButton (PX(XO+2)+12,PY(YO)  +7 ,BW,BH+10,"-Y-",0,1,false,NULL);
	         AddJAxisButton (PX(XO+2)+12,PY(YO+3)+13,BW,BH+10,"+Y+",0,1,true,cjaxis);
	cjaxis=  AddJAxisButton (PX(XO+1)+7 ,PY(YO+1)+20,BW,BH+10,"-X-",0,0,false,NULL);
	         AddJAxisButton (PX(XO+4)-11,PY(YO+1)+20,BW,BH+10,"+X+",0,0,true,cjaxis);	
	
	/* Hat directions up, left, down, right  or D-Pad*/
			AddJHatButton (PX(XO+7)+12  ,PY(YO)  + 5 ,BW ,BH+8 ," \x18", 0,0,0); // UP
			AddJHatButton (PX(XO+7)+12  ,PY(YO+3)+15 ,BW ,BH+8 ," \x19" ,0,0,2); // DOWN			
			AddJHatButton (PX(XO+6)+7   ,PY(YO+1)+20 ,BW ,BH+8 ,"\x1B"  ,0,0,3); // LEFT
			AddJHatButton (PX(XO+9)-11  ,PY(YO+1)+20 ,BW ,BH+8 ,"  \x1A",0,0,1); // RIGHT			

	/* Hide Buttons */	
    AddJButtonButton_hidden(0, 8);
    AddJButtonButton_hidden(0, 9);	
    AddJButtonButton_hidden(0,10);
    AddJButtonButton_hidden(0,11);	
    AddJButtonButton_hidden(0,12);	
	

    cjaxis= AddJAxisButton_hidden(0,4,false,NULL); AddJAxisButton_hidden(0,4,true,cjaxis);
    cjaxis= AddJAxisButton_hidden(0,5,false,NULL); AddJAxisButton_hidden(0,5,true,cjaxis);						
	
	
			
		if ( (joytype==JOY_4AXIS) || (joytype==JOY_4AXIS_2) )			
		{
			/* Axes 3+4 (X+Y) of 1st Joystick */
			cjaxis = AddJAxisButton (PX(XO+13)+12,PY(YO)  +7 ,BW,BH+10,"-Y-",0,2,false,NULL);
					 AddJAxisButton (PX(XO+13)+12,PY(YO+3)+13,BW,BH+10,"+Y+",0,2,true,cjaxis);
			cjaxis=  AddJAxisButton (PX(XO+12)+8 ,PY(YO+1)+20,BW,BH+10,"-X-",0,3,false,NULL);
					 AddJAxisButton (PX(XO+15)-12,PY(YO+1)+20,BW,BH+10,"+X+",0,3,true,cjaxis);	
					 
			/* Info Config */ 		 
			new CTextButton(PX(XO+1) ,PY(YO+7)+2,15*BW,18,sJLabel5);		

			/* Hide Buttons */	
			for (int i=0;i<13;i++) { AddJButtonButton_hidden(1,i);}
			for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			
			//AddJHatButton_hidden(1,0,0);
			// AddJHatButton_hidden(1,0,1);
			// AddJHatButton_hidden(1,0,2);
			// AddJHatButton_hidden(1,0,3);
			
		} else {
			
			cjaxis= AddJAxisButton_hidden(0,2,false,NULL); AddJAxisButton_hidden(0,2,true,cjaxis);
			cjaxis= AddJAxisButton_hidden(0,3,false,NULL); AddJAxisButton_hidden(0,3,true,cjaxis);				
		}
			
	}

/* Layout 1 =================================================================================*/
    if ( (joytype==JOY_WHEEL)  )
	{	
		new CTextButton(PX(XO+1)-2,PY(YO-1),4*BW,20,sJLabel1);
		new CTextButton(PX(XO+6)-2,PY(YO-1),4*BW,20,sJLabel2);
		new CTextButton(PX(XO+11) ,PY(YO-1),4*BW,20,sJLabel3);

		/* Buttons 1+8 of 1st Joystick */	
		for (int i=1;i<=nJoyButtons;i++){		
			switch(i)
			{
				case 1: {AddJButtonButton(PX(XO+1)+6 ,PY(YO)+6,BW   ,BH+7," 1" ,0,0); } break;	//4				
				case 2: {AddJButtonButton(PX(XO+4)-4 ,PY(YO)+6,BW   ,BH+7," 2" ,0,1); } break;				
				case 3: {AddJButtonButton(PX(XO+1)+6 ,PY(YO+1)+18   ,BW   ,BH+7," 3" ,0,2); } break;	
				case 4: {AddJButtonButton(PX(XO+4)-4 ,PY(YO+1)+18   ,BW   ,BH+7," 4" ,0,3); } break;	//3	
			}					
		}	
		for (int i=nJoyButtons;i<13;i++) { AddJButtonButton_hidden(0,i);}	

	
	/* Axes 1+2 (X+Y) of 1st Joystick */
	
	cjaxis= AddJAxisButton (PX(XO+6)+5   ,PY(YO)+6, BW+15, BH+15,"Left" ,0,0, false,NULL);
	        AddJAxisButton (PX(XO+8)+4   ,PY(YO)+6, BW+15, BH+15,"Right",0,0, true ,cjaxis);
			
	cjaxis= AddJAxisButton_hidden(0,1,false,NULL);
			AddJAxisButton (PX(XO+11)+5  ,PY(YO)+6 ,BW+15,BH+15,"Gas",0,1,true,cjaxis);
							
	cjaxis= AddJAxisButton_hidden(0,2,false,NULL);
			AddJAxisButton (PX(XO+13)+4  ,PY(YO)+6 ,BW+15,BH+15,"Brake",0,2,true,cjaxis);
			 		 
	/* Hide Buttons */	
    AddJButtonButton_hidden(0, 0);	
    AddJButtonButton_hidden(0, 1);	
    AddJButtonButton_hidden(0, 2);	
    AddJButtonButton_hidden(0, 3);	
    AddJButtonButton_hidden(0, 4);	
    AddJButtonButton_hidden(0, 5);	
    AddJButtonButton_hidden(0, 6);	
    AddJButtonButton_hidden(0, 7);	
    AddJButtonButton_hidden(0, 8);
    AddJButtonButton_hidden(0, 9);	
    AddJButtonButton_hidden(0,10);
    AddJButtonButton_hidden(0,11);	
    AddJButtonButton_hidden(0,12);	

    cjaxis= AddJAxisButton_hidden(0,3,false,NULL); AddJAxisButton_hidden(0,3,true,cjaxis);
    cjaxis= AddJAxisButton_hidden(0,4,false,NULL); AddJAxisButton_hidden(0,4,true,cjaxis);
    cjaxis= AddJAxisButton_hidden(0,5,false,NULL); AddJAxisButton_hidden(0,5,true,cjaxis);						
	
	
			
		// if ( (joytype==JOY_4AXIS) || (joytype==JOY_4AXIS_2) )			
		// {
			// /* Axes 3+4 (X+Y) of 1st Joystick */
			// cjaxis = AddJAxisButton (PX(XO+13)+12,PY(YO)  +7 ,BW,BH+10,"-Y-",0,2,false,NULL);
					 // AddJAxisButton (PX(XO+13)+12,PY(YO+3)+13,BW,BH+10,"+Y+",0,2,true,cjaxis);
			// cjaxis=  AddJAxisButton (PX(XO+12)+8 ,PY(YO+1)+20,BW,BH+10,"-X-",0,3,false,NULL);
					 // AddJAxisButton (PX(XO+15)-12,PY(YO+1)+20,BW,BH+10,"+X+",0,3,true,cjaxis);	
					 
			// /* Info Config */ 		 
			// new CTextButton(PX(XO+1) ,PY(YO+7)+2,15*BW,18,sJLabel5);		

			// /* Hide Buttons */	
			// for (int i=0;i<13;i++) { AddJButtonButton_hidden(1,i);}
			// for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			
			// //AddJHatButton_hidden(1,0,0);
			// // AddJHatButton_hidden(1,0,1);
			// // AddJHatButton_hidden(1,0,2);
			// // AddJHatButton_hidden(1,0,3);
			
		// } else {
			
			// cjaxis= AddJAxisButton_hidden(0,2,false,NULL); AddJAxisButton_hidden(0,2,true,cjaxis);
			// cjaxis= AddJAxisButton_hidden(0,3,false,NULL); AddJAxisButton_hidden(0,3,true,cjaxis);				
		// }
			
	}
	
    if (joytype==JOY_2AXIS) 	
	{
	/* ===============================================================  Joystick 2*/
	
		/* Buttons 1+8 of 1st Joystick */	
		for (int i=1;i<=nJoyButtons;i++){		
			switch(i)
			{
				case 5: {AddJButtonButton(PX(XO+13)+12	,PY(YO+1)+22,BW   ,BH+6," 5" ,1,4); } break;
				case 3: {AddJButtonButton(PX(XO+12) 	,PY(YO)+5   ,BW   ,BH+7," 3" ,1,2); } break;	
				case 4: {AddJButtonButton(PX(XO+15)-4 	,PY(YO)+5   ,BW   ,BH+7," 4" ,1,3); } break;	
				case 1: {AddJButtonButton(PX(XO+12) 	,PY(YO+3)+18,BW   ,BH+7," 1" ,1,0);} break;	
				case 2: {AddJButtonButton(PX(XO+15)-4 	,PY(YO+3)+18,BW   ,BH+7," 2" ,1,1);} break;
				case 6: {AddJButtonButton(PX(XO+12) 	,PY(YO+5)+10,BW+8 ,BH+7," 6" ,1,5);} break;
				case 7: {AddJButtonButton(PX(XO+13)+10	,PY(YO+5)+10,BW+5 ,BH+7," 7" ,1,6);} break;	
				case 8: {AddJButtonButton(PX(XO+15)-11	,PY(YO+5)+10,BW+8 ,BH+7," 8" ,1,7);} break;			
			}					
		}	
		for (int i=nJoyButtons;i<13;i++) { AddJButtonButton_hidden(1,i);}	

			
	/* Hide Buttons */	
    AddJButtonButton_hidden(1, 8);
    AddJButtonButton_hidden(1, 9);	
    AddJButtonButton_hidden(1,10);
    AddJButtonButton_hidden(1,11);	
    AddJButtonButton_hidden(1,12);
						
	cjaxis= AddJAxisButton_hidden(1,2,false,NULL); AddJAxisButton_hidden(1,2,true,cjaxis);
	cjaxis= AddJAxisButton_hidden(1,3,false,NULL); AddJAxisButton_hidden(1,3,true,cjaxis);							
    cjaxis= AddJAxisButton_hidden(1,4,false,NULL); AddJAxisButton_hidden(1,4,true,cjaxis);
    cjaxis= AddJAxisButton_hidden(1,5,false,NULL); AddJAxisButton_hidden(1,5,true,cjaxis);
	
	/* Axes 1+2 (X+Y) of 2st Joystick */
	cjaxis = AddJAxisButton (PX(XO+13)+12,PY(YO)  +7 ,BW,BH+10,"-Y-",1,1,false,NULL);
	         AddJAxisButton (PX(XO+13)+12,PY(YO+3)+13,BW,BH+10,"+Y+",1,1,true,cjaxis);
	cjaxis=  AddJAxisButton (PX(XO+12)+8 ,PY(YO+1)+20,BW,BH+10,"-X-",1,0,false,NULL);
	         AddJAxisButton (PX(XO+15)-12,PY(YO+1)+20,BW,BH+10,"+X+",1,0,true,cjaxis);	

	/* Hat directions up, left, down, right  or D-Pad*/
			AddJHatButton (PX(XO+19)+12   ,PY(YO)  +5,BW,BH+8," \x18",0,0,0); // UP
			AddJHatButton (PX(XO+19)+12  ,PY(YO+3)+15,BW,BH+8," \x19",0,0,2); // DOWN			
			AddJHatButton (PX(XO+18)+7   ,PY(YO+1)+20,BW,BH+8,"\x1B",0,0,3);  // LEFT
			AddJHatButton (PX(XO+21)-11   ,PY(YO+1)+20,BW,BH+8,"  \x1A",0,0,1);// RIGHT	
	/* Info Config */ 
	new CTextButton(PX(XO+1) ,PY(YO+7)+2,21*BW,18,sJLabel5);				
	}
	
    if (joytype==JOY_FCS) 	
	{
	
		new CTextButton(PX(XO+ 1) ,PY(YO-1)  , 4*BW+20 ,20,sJLabel1);
		new CTextButton(PX(XO+ 7) ,PY(YO-1)  , 4*BW    ,20,sJLabel2);
		new CTextButton(PX(XO+12) ,PY(YO-1)  , 4*BW    ,20,sJLabel3);
		new CTextButton(PX(XO+17) ,PY(YO-1)  , 4*BW    ,20,sJLabel4);	
		new CTextButton(PX(XO+ 1) ,PY(YO+7)+2,21*BW    ,18,sJLabel5);
		
		/* Buttons 1->4 of Thrustmaster Joystick */
		AddJButtonButton(PX(XO+1)   ,PY(YO),BW+5   ,BH+7," S1" ,0,0);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO),BW+5   ,BH+7," S2" ,0,1);
		AddJButtonButton(PX(XO+3)+10,PY(YO),BW+5   ,BH+7," S3" ,0,2);
		AddJButtonButton(PX(XO+4)+15,PY(YO),BW+5   ,BH+7," S4" ,0,3);		
	
	
		/* Buttons T1->T14 of Thrustmaster Joystick F16 Throttle System TQS
		AddJButtonButton(PX(XO+1)   ,PY(YO+1)+ 7,BW+5   ,BH+7," T1" ,0,4);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO+1)+ 7,BW+5   ,BH+7," T2" ,0,5);
		AddJButtonButton(PX(XO+3)+10,PY(YO+1)+ 7,BW+5   ,BH+7," T3" ,0,6);
		AddJButtonButton(PX(XO+4)+15,PY(YO+1)+ 7,BW+5   ,BH+7," T4" ,0,7);
		AddJButtonButton(PX(XO+1)   ,PY(YO+2)+14,BW+5   ,BH+7," T5" ,0,8);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO+2)+14,BW+5   ,BH+7," T6" ,0,9);
		AddJButtonButton(PX(XO+3)+10,PY(YO+2)+14,BW+5   ,BH+7," T7" ,0,10);
		AddJButtonButton(PX(XO+4)+15,PY(YO+2)+14,BW+5   ,BH+7," T8" ,0,11);	
		AddJButtonButton(PX(XO+1)   ,PY(YO+2)+21,BW+5   ,BH+7," T9" ,0,12);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO+2)+21,BW+5   ,BH+7," T10",0,13);
		AddJButtonButton(PX(XO+3)+10,PY(YO+2)+21,BW+5   ,BH+7," T11",0,14);
		AddJButtonButton(PX(XO+4)+15,PY(YO+2)+21,BW+5   ,BH+7," T12",0,15);			
		*/			
		
		/* Buttons T1->T8 of Thrustmaster JoystickWeapons Control System WCS Mark II Throttle		
		AddJButtonButton(PX(XO+1)   ,PY(YO+2)+ 7,BW+5   ,BH+7," T1" ,0,4);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO+2)+ 7,BW+5   ,BH+7," T2" ,0,5);
		AddJButtonButton(PX(XO+3)+10,PY(YO+2)+ 7,BW+5   ,BH+7," T3" ,0,6);
		AddJButtonButton(PX(XO+4)+15,PY(YO+2)+ 7,BW+5   ,BH+7," T4" ,0,7);
		AddJButtonButton(PX(XO+1)   ,PY(YO+3)+14,BW+5   ,BH+7," T5" ,0,8);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO+3)+14,BW+5   ,BH+7," T6" ,0,9);
		AddJButtonButton(PX(XO+3)+10,PY(YO+3)+14,BW+5   ,BH+7," T7" ,0,10);
		AddJButtonButton(PX(XO+4)+15,PY(YO+3)+14,BW+5   ,BH+7," T8" ,0,11);	
		AddJButtonButton(PX(XO+2)+20,PY(YO+4)+21,BW+5   ,BH+7," MT" ,0,12);	
		*/	
		
		/* Axes 1+2 (X+Y) of 1st Joystick */
		cjaxis = AddJAxisButton (PX(XO+8)+12 ,PY(YO)  + 5 ,BW,BH+10, "-Y-",0,1,false,NULL);
				 AddJAxisButton (PX(XO+8)+12 ,PY(YO+3)+15 ,BW,BH+10, "+Y+",0,1,true,cjaxis);
		cjaxis = AddJAxisButton (PX(XO+7)+7  ,PY(YO+1)+20 ,BW,BH+10, "-X-",0,0,false,NULL);
				 AddJAxisButton (PX(XO+10)-11,PY(YO+1)+20 ,BW,BH+10, "+X+",0,0,true,cjaxis);	

		/* Axes 3 (Y) of 1st Joystick */
		cjaxis = AddJAxisButton (PX(XO+13)+12,PY(YO)  + 5 ,BW,BH+25, "-Y-",0,2,false,NULL);
				 AddJAxisButton (PX(XO+13)+12,PY(YO+3)    ,BW,BH+25, "+Y+",0,2,true,cjaxis);				
				 
				 
		/* Hat directions up, left, down, right  or D-Pad*/
			AddJHatButton (PX(XO+18)+12  ,PY(YO)  + 5 ,BW ,BH+8 ," \x18", 0,0,0); // UP
			AddJHatButton (PX(XO+18)+12  ,PY(YO+3)+15 ,BW ,BH+8 ," \x19" ,0,0,2); // DOWN			
			AddJHatButton (PX(XO+17)+7   ,PY(YO+1)+20 ,BW ,BH+8 ,"\x1B"  ,0,0,3); // LEFT
			AddJHatButton (PX(XO+20)-11  ,PY(YO+1)+20 ,BW ,BH+8 ,"  \x1A",0,0,1); // RIGHT					 

		/*Hide Buttons From Joystick 1 */
		for (int i=4;i<16;i++) { AddJButtonButton_hidden(0,i);}		
		for (int i=3;i<6;i++) { cjaxis= AddJAxisButton_hidden(0,i,false,NULL); AddJAxisButton_hidden(0,i,true,cjaxis);}	
		
		/*Hide Buttons From Joystick 2 */		
		for (int i=0;i<16;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			
	}
	
    if (joytype==JOY_CH) 	
	{
	
		new CTextButton(PX(XO+ 1) ,PY(YO-1)  , 4*BW+20 ,20,sJLabel1);
		new CTextButton(PX(XO+ 7) ,PY(YO-1)  , 4*BW    ,20,sJLabel2);
		new CTextButton(PX(XO+12) ,PY(YO-1)  , 4*BW    ,20,sJLabel3);
		new CTextButton(PX(XO+17) ,PY(YO-1)  , 4*BW    ,20,sJLabel4);	
		new CTextButton(PX(XO+ 1) ,PY(YO+7)+2,21*BW    ,18,sJLabel5);
		
		/* Buttons 1->4 of CG Flightstick PRO */
		AddJButtonButton(PX(XO+1)   ,PY(YO),BW+5   ,BH+7," 1" ,0,0);
		AddJButtonButton(PX(XO+2)+5 ,PY(YO),BW+5   ,BH+7," 2" ,0,1);
		AddJButtonButton(PX(XO+3)+10,PY(YO),BW+5   ,BH+7," 3" ,0,2);
		AddJButtonButton(PX(XO+4)+15,PY(YO),BW+5   ,BH+7," 4" ,0,3);		
			
		/* Axes 1+2 (X+Y) of 1st Joystick */
		cjaxis = AddJAxisButton (PX(XO+8)+12 ,PY(YO)  + 5 ,BW,BH+10, "-Y-",0,1,false,NULL);
				 AddJAxisButton (PX(XO+8)+12 ,PY(YO+3)+15 ,BW,BH+10, "+Y+",0,1,true,cjaxis);
		cjaxis = AddJAxisButton (PX(XO+7)+7  ,PY(YO+1)+20 ,BW,BH+10, "-X-",0,0,false,NULL);
				 AddJAxisButton (PX(XO+10)-11,PY(YO+1)+20 ,BW,BH+10, "+X+",0,0,true,cjaxis);	

		/* Axes 3 (Y) of 1st Joystick */
		cjaxis = AddJAxisButton (PX(XO+13)+12,PY(YO)  + 5 ,BW,BH+10, "-Y-",0,2,false,NULL);
				 AddJAxisButton (PX(XO+13)+12,PY(YO+3)+15 ,BW,BH+10, "+Y+",0,2,true,cjaxis);				
		cjaxis = AddJAxisButton (PX(XO+12)+7  ,PY(YO+1)+20 ,BW,BH+10, "-X-",0,3,false,NULL);
				 AddJAxisButton (PX(XO+15)-11,PY(YO+1)+20 ,BW,BH+10, "+X+",0,3,true,cjaxis);				 
				 
		/* Hat directions up, left, down, right  or D-Pad*/
			AddJHatButton (PX(XO+18)+12  ,PY(YO)  + 5 ,BW ,BH+8 ," \x18", 0,0,0); // UP
			AddJHatButton (PX(XO+18)+12  ,PY(YO+3)+15 ,BW ,BH+8 ," \x19" ,0,0,2); // DOWN			
			AddJHatButton (PX(XO+17)+7   ,PY(YO+1)+20 ,BW ,BH+8 ,"\x1B"  ,0,0,3); // LEFT
			AddJHatButton (PX(XO+20)-11  ,PY(YO+1)+20 ,BW ,BH+8 ,"  \x1A",0,0,1); // RIGHT					 

		/*Hide Buttons From Joystick 1 */
		for (int i=4;i<16;i++) { AddJButtonButton_hidden(0,i);}		
		for (int i=4;i<6;i++) { cjaxis= AddJAxisButton_hidden(0,i,false,NULL); AddJAxisButton_hidden(0,i,true,cjaxis);}	
		
		/*Hide Buttons From Joystick 2 */		
		for (int i=0;i<16;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			
	}
	
    if ( (joytype==JOY_CHVP) || (joytype==JOY_FCSLW) )
	{	

		/* Adjusted with  Ch Virtual Pilot (98) and Flighstick PRO (98) in Windows98 */
		new CTextButton(PX(XO+ 0)   ,PY(YO-1)  , 4*BW+20 ,20,sJLabel1);
		new CTextButton(PX(XO+ 5)   ,PY(YO-1)  , 4*BW    ,20,sJLabel2);
		new CTextButton(PX(XO+ 9)+8 ,PY(YO-1)  , 4*BW    ,20,sJLabel3);
		new CTextButton(PX(XO+13)+16,PY(YO-1)  , 4*BW    ,20,sJLabel4);
		new CTextButton(PX(XO+18)   ,PY(YO-1)  , 4*BW    ,20,sJLabel6);			
		new CTextButton(PX(XO+ 0)   ,PY(YO+7)+2,22*BW    ,18,sJLabel5);
		
		/* Buttons 1->10 of Virtual Pilot / Flightsick 98*/
		AddJButtonButton(PX(XO+0)    ,PY(YO),	BW+16   ,BH+7,"Fire" ,0,0);
		AddJButtonButton(PX(XO+1)+16 ,PY(YO),	BW+16   ,BH+7,"Bt A" ,0,1);
		AddJButtonButton(PX(XO+2)+32 ,PY(YO),	BW+16   ,BH+7,"Bt B" ,0,2);
		
		AddJButtonButton(PX(XO+0)    ,PY(YO+1)+7,	BW+16   ,BH+7,"Lunch",0,3);
		AddJButtonButton(PX(XO+1)+16 ,PY(YO+1)+7,	BW+16   ,BH+7,"Bt D" ,0,4);
		AddJButtonButton(PX(XO+2)+32 ,PY(YO+1)+7,	BW+16   ,BH+7,"MFire",0,5);
		
		AddJButtonButton(PX(XO+0)    ,PY(YO+2)+14,	BW+16   ,BH+7,"Pnkle",0,6);
		AddJButtonButton(PX(XO+1)+16 ,PY(YO+2)+14,	BW+16   ,BH+7,"Bt C" ,0,7);
		AddJButtonButton(PX(XO+2)+32 ,PY(YO+2)+14,	BW+16   ,BH+7,"Mode2",0,8);
		
		AddJButtonButton(PX(XO+0)	 ,PY(YO+3)+21,	BW+104   ,BH+7,"Mode1",0,9);		


		/* ==================================================================================================== */
		switch(HostCtrlType)
		{			
			case HOSTJOY_DEFAULT:			
			case HOSTJOY_SAITEKX45:			
			{
				/* Axes 4/5 (X+Y) of Virtual Pilot / Flightsick 98 (Throttle/Rudder)*/
				cjaxis = AddJAxisButton (PX(XO+10)+20,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,4,false,NULL);
						 AddJAxisButton (PX(XO+10)+20,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,4,true,cjaxis);	
					
				cjaxis = AddJAxisButton (PX(XO+9)+15 ,PY(YO+1)+20 ,BW,BH+8, "-X-",0,5,false,NULL);
						 AddJAxisButton (PX(XO+12)-4 ,PY(YO+1)+20 ,BW,BH+8, "+X+",0,5,true,cjaxis);	

				/* Axes 3 (Y) of 1st Joystick */
				cjaxis = AddJAxisButton (PX(XO+14)+28,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,2,false,NULL);
						 AddJAxisButton (PX(XO+14)+28,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,2,true,cjaxis);				
				cjaxis = AddJAxisButton (PX(XO+13)+23,PY(YO+1)+20 ,BW,BH+8, "-X-",0,3,false,NULL);
						 AddJAxisButton (PX(XO+16)+ 5,PY(YO+1)+20 ,BW,BH+8, "+X+",0,3,true,cjaxis);	
			}
			break;	
			case HOSTJOY_LEXT3DPRO:			
			{
				/* Axes 4/5 (X+Y) of Virtual Pilot / Flightsick 98 (Throttle/Rudder)*/
				cjaxis = AddJAxisButton (PX(XO+10)+20,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,3,false,NULL);
						 AddJAxisButton (PX(XO+10)+20,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,3,true,cjaxis);	
					
				cjaxis = AddJAxisButton (PX(XO+9)+15 ,PY(YO+1)+20 ,BW,BH+8, "-X-",0,5,false,NULL);
						 AddJAxisButton (PX(XO+12)-4 ,PY(YO+1)+20 ,BW,BH+8, "+X+",0,5,true,cjaxis);	

				/* Axes 3 (Y) of 1st Joystick */
				cjaxis = AddJAxisButton (PX(XO+14)+28,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,4,false,NULL);
						 AddJAxisButton (PX(XO+14)+28,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,4,true,cjaxis);				
				cjaxis = AddJAxisButton (PX(XO+13)+23,PY(YO+1)+20 ,BW,BH+8, "-X-",0,2,false,NULL);
						 AddJAxisButton (PX(XO+16)+ 5,PY(YO+1)+20 ,BW,BH+8, "+X+",0,2,true,cjaxis);	
			}
			break;	
			case HOSTJOY_TFLIGHTHO:			
			{
				/* Axes 4/5 (X+Y) of Virtual Pilot / Flightsick 98 (Throttle/Rudder)*/
				cjaxis = AddJAxisButton (PX(XO+10)+20,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,2,false,NULL);
						 AddJAxisButton (PX(XO+10)+20,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,2,true,cjaxis);	
					
				cjaxis = AddJAxisButton (PX(XO+9)+15 ,PY(YO+1)+20 ,BW,BH+8, "-X-",0,3,false,NULL);
						 AddJAxisButton (PX(XO+12)-4 ,PY(YO+1)+20 ,BW,BH+8, "+X+",0,3,true,cjaxis);	

				/* Axes 3 (Y) of 1st Joystick */
				cjaxis = AddJAxisButton (PX(XO+14)+28,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,4,false,NULL);
						 AddJAxisButton (PX(XO+14)+28,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,4,true,cjaxis);				
				cjaxis = AddJAxisButton (PX(XO+13)+23,PY(YO+1)+20 ,BW,BH+8, "-X-",0,5,false,NULL);
						 AddJAxisButton (PX(XO+16)+ 5,PY(YO+1)+20 ,BW,BH+8, "+X+",0,5,true,cjaxis);	
			}
			break;				
			
		}
		/* ==================================================================================================== */
			
		/* Axes 1+2 (X+Y) of Virtual Pilot / Flightsick 98*/
		cjaxis = AddJAxisButton (PX(XO+6)+12 ,PY(YO)  + 5 ,BW,BH+8, "-Y-",0,1,false,NULL);
				 AddJAxisButton (PX(XO+6)+12 ,PY(YO+3)+15 ,BW,BH+8, "+Y+",0,1,true,cjaxis);
		cjaxis = AddJAxisButton (PX(XO+5)+7  ,PY(YO+1)+20 ,BW,BH+8, "-X-",0,0,false,NULL);
				 AddJAxisButton (PX(XO+8)-11 ,PY(YO+1)+20 ,BW,BH+8, "+X+",0,0,true,cjaxis);	
				 
				 
		/* Hat directions up, left, down, right  or D-Pad*/
			AddJHatButton (PX(XO+19)+12  ,PY(YO)  + 5 ,BW ,BH+8 ," \x18", 0,0,0); // UP
			AddJHatButton (PX(XO+19)+12  ,PY(YO+3)+15 ,BW ,BH+8 ," \x19" ,0,0,2); // DOWN			
			AddJHatButton (PX(XO+18)+7   ,PY(YO+1)+20 ,BW ,BH+8 ,"\x1B"  ,0,0,3); // LEFT
			AddJHatButton (PX(XO+21)-11  ,PY(YO+1)+20 ,BW ,BH+8 ,"  \x1A",0,0,1); // RIGHT						 
				
		/*Hide Buttons From Joystick 1 */
		for (int i=10;i<16;i++) { AddJButtonButton_hidden(0,i);}		
		
		/*Hide Buttons From Joystick 2 */		
		for (int i=0;i<16;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}	
		
	}
	
    if (joytype==JOY_CHGS) 	
	{	

		/* Adjusted with  Ch Virtual Pilot (98) and Flighstick PRO (98) in Windows98 */
		//new CTextButton(PX(XO+ 0)   ,PY(YO-1)  , 4*BW+20 ,20,sJLabel1);
		// new CTextButton(PX(XO+ 5)   ,PY(YO-1)  , 4*BW    ,20,sJLabel2);
		// new CTextButton(PX(XO+ 9)+8 ,PY(YO-1)  , 4*BW    ,20,sJLabel3);
		// new CTextButton(PX(XO+13)+16,PY(YO-1)  , 4*BW    ,20,sJLabel4);
		// new CTextButton(PX(XO+18)   ,PY(YO-1)  , 4*BW    ,20,sJLabel6);			
		new CTextButton(PX(XO+ 0)   ,PY(YO+7)+2,22*BW    ,18,sJLabel5);
		
		/* Hat directions up, left, down, right  or D-Pad*/
			AddJHatButton (PX(XO+3)+7  	,PY(YO-1)  + 7 ,BW ,BH+8 ," \x18", 0,0,0); // UP
			AddJHatButton (PX(XO+3)+7   ,PY(YO+2)+13 ,BW ,BH+8 ," \x19" ,0,0,2); // DOWN			
			AddJHatButton (PX(XO+2)+3   ,PY(YO)+20 ,BW ,BH+8 ,"\x1B"  ,0,0,3); // LEFT
			AddJHatButton (PX(XO+5)-18  ,PY(YO)+20 ,BW ,BH+8 ,"  \x1A",0,0,1); // RIGHT		
			
		// /* Axes 1+2 (X+Y) 
		cjaxis = AddJAxisButton (PX(XO+6)+12 ,PY(YO+1)+17 ,BW,BH+8, "-Y-",0,1,false,NULL);
				 AddJAxisButton (PX(XO+6)+12 ,PY(YO+4)+23 ,BW,BH+8, "+Y+",0,1,true,cjaxis);
		cjaxis = AddJAxisButton (PX(XO+5)+7  ,PY(YO+2)+30 ,BW,BH+8, "-X-",0,0,false,NULL);
				 AddJAxisButton (PX(XO+8)-13 ,PY(YO+2)+30 ,BW,BH+8, "+X+",0,0,true,cjaxis);	

		// /* Axes 3+4 (X+Y)
		cjaxis = AddJAxisButton (PX(XO+12)+12,PY(YO+1)+17 ,BW,BH+8, "-Y-",0,4,false,NULL);
				 AddJAxisButton (PX(XO+12)+12,PY(YO+4)+23 ,BW,BH+8, "+Y+",0,4,true,cjaxis);				
		cjaxis = AddJAxisButton (PX(XO+11)+7 ,PY(YO+2)+30 ,BW,BH+8, "-X-",0,3,false,NULL);
				 AddJAxisButton (PX(XO+14)-13,PY(YO+2)+30 ,BW,BH+8, "+X+",0,3,true,cjaxis);	

		cjaxis = AddJAxisButton (PX(XO+6) 	,PY(YO-1)-10,BW+36,BH ,"Z/LFT 7",0,2,false,NULL);
				 AddJAxisButton_hidden(0,2,true,cjaxis);
				 
		cjaxis = AddJAxisButton (PX(XO+12)-7,PY(YO-1)-10,BW+36,BH ,"Z/RGT 8",0,5,false,NULL);
				 AddJAxisButton_hidden(0,5,true,cjaxis);	
			
		/* Buttons 1->6 of 1st Joystick */	
		for (int i=1;i<=nJoyButtons;i++){		
			switch(i)
			{
				case 1:  {AddJButtonButton(PX(XO+15)+17	,PY(YO+2)+13,BW ,BH+8 ,"1/A" ,0,0); } break;	//1	
				case 2:  {AddJButtonButton(PX(XO+17)-8	,PY(YO)+20 	,BW ,BH+8 ,"2/B" ,0,1); } break;	//2
				case 3:  {AddJButtonButton(PX(XO+14)+13 ,PY(YO)+20 	,BW ,BH+8 ,"3/X" ,0,2); } break;	//3
				case 4:  {AddJButtonButton(PX(XO+15)+17 ,PY(YO-1)+7	,BW ,BH+8 ,"4/Y" ,0,3); } break;	//4	
				
				case 9:  {AddJButtonButton(PX(XO+8) 	,PY(YO+1)+10,BW+20,BH+6 ,"Back",0,9); } break;	//9						
				case 10: {AddJButtonButton(PX(XO+10)+4 	,PY(YO+1)+10,BW+20,BH+6 ,"Start",0,8); } break;	//10

				case 5:  {AddJButtonButton(PX(XO+6) 	,PY(YO),BW+36,BH ,"LEFT 5",0,4); } break;	//5				
				case 6:  {AddJButtonButton(PX(XO+12)-7 	,PY(YO),BW+36,BH ,"RIGHT 6",0,5); } break;	//6
				
				//case 7:  {AddJButtonButton(PX(XO+6) 	,PY(YO-1)-10,BW+36,BH ,"Z/LFT 7",0,6); } break;	//5				
				//case 8:  {AddJButtonButton(PX(XO+12)-7 	,PY(YO-1)-10,BW+36,BH ,"Z/RGT 8",0,7); } break;	//6				
				
				// case 2: {AddJButtonButton(PX(XO+7)+40	,PY(YO+4)+28,BW+18,BH+10,"A (1)" ,0,0); } break;				
				// case 3: {AddJButtonButton(PX(XO+8)+38	,PY(YO+2)+34,BW+18,BH+10,"B (2)" ,0,1); } break;	
				// case 4: {AddJButtonButton(PX(XO+7)+13	,PY(YO+2)+34,BW+18,BH+10,"X (3)" ,0,2); } break;	//3
				// case 5: {AddJButtonButton(PX(XO+2)+15 	,PY(YO)+8   ,3*BW+20 ,BH+5," Btn C (5)" ,0,4); } break;	
				// case 6: {AddJButtonButton(PX(XO+7)+13  	,PY(YO)+8   ,3*BW+20 ,BH+5," Btn Z (6)" ,0,5); } break;					
			}					
		}			
			
		// /* Buttons 1->10 of Virtual Pilot / Flightsick 98*/
		// AddJButtonButton(PX(XO+0)    ,PY(YO),	BW+16   ,BH+7,"BT 1" ,0,0);
		// AddJButtonButton(PX(XO+1)+16 ,PY(YO),	BW+16   ,BH+7,"BT 1" ,0,1);
		// AddJButtonButton(PX(XO+2)+32 ,PY(YO),	BW+16   ,BH+7,"BT 3" ,0,2);
		
		// AddJButtonButton(PX(XO+0)    ,PY(YO+1)+7,	BW+16   ,BH+7,"BT 4",0,3);
		// AddJButtonButton(PX(XO+1)+16 ,PY(YO+1)+7,	BW+16   ,BH+7,"BT 5" ,0,4);
		// AddJButtonButton(PX(XO+2)+32 ,PY(YO+1)+7,	BW+16   ,BH+7,"BT 6",0,5);
		
		// AddJButtonButton(PX(XO+0)    ,PY(YO+2)+14,	BW+16   ,BH+7,"BT 7",0,6);
		// AddJButtonButton(PX(XO+1)+16 ,PY(YO+2)+14,	BW+16   ,BH+7,"BT 8" ,0,7);
		// AddJButtonButton(PX(XO+2)+32 ,PY(YO+2)+14,	BW+16   ,BH+7,"BT 9",0,8);
		
		// AddJButtonButton(PX(XO+0)	 ,PY(YO+3)+21,	BW+104   ,BH+7,"BT10",0,9);		

		/*Hide Buttons From Joystick 1 */
		for (int i=10;i<16;i++) { AddJButtonButton_hidden(0,i);}		
		//for (int i=4;i<6;i++) { cjaxis= AddJAxisButton_hidden(0,i,false,NULL); AddJAxisButton_hidden(0,i,true,cjaxis);}			
		/*Hide Buttons From Joystick 2 */		
		for (int i=0;i<16;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}	
		
	}
	
	
	if ( (joytype==JOY_SHOT6) || (joytype==JOY_INTER6) )
	{	
		/* Quickshot 6 Button Gameport Pad
		   Adjusted via Windows 95 Driver. Curious? Button 5 and 6 are on 
		   Axis  2 X negative (Resolution -0.25000) and 6 on Axis 3 Y negative (Resolution -0.25000)		  
		*/
		new CTextButton(PX(XO+2)-2,PY(YO-1),3*BW+14,20,sJLabel1);
		
		/* Buttons 1->6 of 1st Joystick */	
		for (int i=1;i<=nJoyButtons;i++){		
			switch(i)
			{
				case 3: {AddJButtonButton(PX(XO+2)   ,PY(YO)+5   ,BW   ,BH+7," 3" ,0,2); } break;	
				case 4: {AddJButtonButton(PX(XO+5)-4 ,PY(YO)+5   ,BW   ,BH+7," 4" ,0,3); } break;	//3
				case 1: {AddJButtonButton(PX(XO+2)   ,PY(YO+3)+18,BW   ,BH+7," 1" ,0,0); } break;	//4
				case 2: {AddJButtonButton(PX(XO+5)-4 ,PY(YO+3)+18,BW   ,BH+7," 2" ,0,1); } break;
				case 6: {AddJButtonButton(PX(XO+2)   ,PY(YO+5)+10,BW+8 ,BH+7," 6" ,0,5); } break;	
				case 5: {AddJButtonButton(PX(XO+5)-11,PY(YO+5)+10,BW+8 ,BH+7," 5" ,0,4); } break;					
			}					
		}	
		for (int i=nJoyButtons;i<13;i++) { AddJButtonButton_hidden(0,i);}	
		
		/* Axes 1+2 (X+Y) of 1st Joystick */
		cjaxis = AddJAxisButton (PX(XO+3)+12,PY(YO)  +7 ,BW,BH+10,"-Y-",0,1,false,NULL);
				AddJAxisButton (PX(XO+3)+12,PY(YO+3)+13,BW,BH+10,"+Y+",0,1,true,cjaxis);
		cjaxis=  AddJAxisButton (PX(XO+2)+7 ,PY(YO+1)+20,BW,BH+10,"-X-",0,0,false,NULL);
				AddJAxisButton (PX(XO+5)-11,PY(YO+1)+20,BW,BH+10,"+X+",0,0,true,cjaxis);	

				
		
		for (int i=2;i<6;i++) { cjaxis= AddJAxisButton_hidden(0,i,false,NULL); AddJAxisButton_hidden(0,i,true,cjaxis);}			

		AddJHatButton_hidden(0,0,0);		
		AddJHatButton_hidden(0,0,1);			
		AddJHatButton_hidden(0,0,2);			
		AddJHatButton_hidden(0,0,3);	
		
		for (int i=0;i<13;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			

		new CTextButton(PX(XO+1) ,PY(YO+7)+2,21*BW,18,sJLabel5);	
		
	}
	
	if ( joytype==JOY_CAPCOM )
	{	
		/* Capcom 6 Button Gameport DOS Pad
		   Curious? Button 5 and 6 are on 
		   Axis  3 X negative (Resolution -0.50) and 6 on Axis 4 Y negative (Resolution -0.50)		  
		*/
		new CTextButton(PX(XO+2)+14,PY(YO-1),8*BW+18,20,sJLabel1);
		
		/* Buttons 1->6 of 1st Joystick */	
		for (int i=1;i<=nJoyButtons;i++){		
			switch(i)
			{
				case 1: {AddJButtonButton(PX(XO+7)+40	,PY(YO+1)+20,BW+18,BH+10,"Y (4)" ,0,3); } break;	//4		
				case 2: {AddJButtonButton(PX(XO+7)+40	,PY(YO+4)+28,BW+18,BH+10,"A (1)" ,0,0); } break;				
				case 3: {AddJButtonButton(PX(XO+8)+38	,PY(YO+2)+34,BW+18,BH+10,"B (2)" ,0,1); } break;	
				case 4: {AddJButtonButton(PX(XO+7)+13	,PY(YO+2)+34,BW+18,BH+10,"X (3)" ,0,2); } break;	//3
				case 5: {AddJButtonButton(PX(XO+2)+15 	,PY(YO)+8   ,3*BW+20 ,BH+5," Btn C (5)" ,0,4); } break;	
				case 6: {AddJButtonButton(PX(XO+7)+13  	,PY(YO)+8   ,3*BW+20 ,BH+5," Btn Z (6)" ,0,5); } break;					
			}					
		}	
	
		for (int i=nJoyButtons;i<13;i++) { AddJButtonButton_hidden(0,i);}	
		
		/* Axes 1+2 (X+Y) of 1st Joystick */
		cjaxis = AddJAxisButton (PX(XO+3)+20	,PY(YO+1)+20,	BW,BH+10," \x18",0,1,false,NULL);  // UP
				 AddJAxisButton (PX(XO+3)+20	,PY(YO+4)+26,	BW,BH+10," \x19",0,1,true,cjaxis); // DOWN
		cjaxis=  AddJAxisButton (PX(XO+2)+15	,PY(YO+2)+33,	BW,BH+10," \x1B",0,0,false,NULL);  // LEFT
				 AddJAxisButton (PX(XO+5)-3		,PY(YO+2)+33,	BW,BH+10," \x1A",0,0,true,cjaxis); // RIGHT

		
		for (int i=2;i<6;i++) { cjaxis= AddJAxisButton_hidden(0,i,false,NULL); AddJAxisButton_hidden(0,i,true,cjaxis);}			

		AddJHatButton_hidden(0,0,0);		
		AddJHatButton_hidden(0,0,1);			
		AddJHatButton_hidden(0,0,2);			
		AddJHatButton_hidden(0,0,3);	
		
		for (int i=0;i<13;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			

		new CTextButton(PX(XO+1) ,PY(YO+7)+2,21*BW,18,sJLabel5);	
		
	}	

    if (joytype==JOY_NONE) 	
	{	
		/*Hide Buttons From Joystick 2 */		
		for (int i=0;i<16;i++) { AddJButtonButton_hidden(0,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(0,i,false,NULL); AddJAxisButton_hidden(0,i,true,cjaxis);}	
		
		/*Hide Buttons From Joystick 2 */		
		for (int i=0;i<16;i++) { AddJButtonButton_hidden(1,i);}
		for (int i=0;i<6;i++) { cjaxis= AddJAxisButton_hidden(1,i,false,NULL); AddJAxisButton_hidden(1,i,true,cjaxis);}			
		
		AddJHatButton_hidden(0,0,0);		
		AddJHatButton_hidden(0,0,1);			
		AddJHatButton_hidden(0,0,2);			
		AddJHatButton_hidden(0,0,3);			
	}
   
	/* The modifier buttons */
	AddModButton(PX(0),PY(13),112,20,"Mod1",1);
	AddModButton(PX(0),PY(14),112,20,"Mod2",2);
	AddModButton(PX(0),PY(15),112,20,"Mod3",3);
	
		// AddKeyButtonEvent(PX(XO+2),PY(YO+1),BW,BH,"9","kp_9",KBD_kp9);
	/* Create Handler buttons */
	Bitu xpos=0;Bitu ypos=18;
	for (CHandlerEventVector_it hit=handlergroup.begin();hit!=handlergroup.end();hit++) {
		new CEventButton(PX(xpos*4),PY(ypos),BW*4,BH,(*hit)->ButtonName(),(*hit));
		xpos++;
		if (xpos>6) {
			xpos=0;ypos++;
		}
	}
	/* Create some text buttons */
//	new CTextButton(PX(6),0,124,20,"Keyboard Layout");
//	new CTextButton(PX(17),0,124,20,"Joystick Layout");

	bind_but.action=new CCaptionButton(200,445,0,0);

	bind_but.event_title=new CCaptionButton(200,465,0,0);
	bind_but.bind_title=new CCaptionButton(200,485,0,0);

	/* Create binding support buttons */
	
	bind_but.mod1=new CCheckButton(5,530,112,20, "Key Mod 1",BC_Mod1);
	bind_but.mod2=new CCheckButton(5,550,112,20, "Key Mod 2",BC_Mod2);
	bind_but.mod3=new CCheckButton(5,570,112,20, "Key Mod 3",BC_Mod3);
	bind_but.hold=new CCheckButton(120,570,60,20,"Hold",BC_Hold);

	bind_but.next=new CBindButton(340,530,135,20,"  Next Binding",BB_Next);

	bind_but.add=new CBindButton(340,508,65,20,"  Add",BB_Add);
	bind_but.del=new CBindButton(410,508,65,20," Delete",BB_Del);

	bind_but.save=new CBindButton(340,570,60,26," Save ",BB_Save);
	bind_but.exit=new CBindButton(415,570,60,26," Exit ",BB_Exit);

	bind_but.bind_title->Change("Bind Title");
}

static SDL_Color map_pal[CLR_LAST]={
	{0x00,0x00,0x00,0x00},			//0=black
	{0x7f,0x7f,0x7f,0x00},			//1=grey
	{0xff,0xff,0xff,0x00},			//2=white
	{0xff,0x00,0x00,0x00},			//3=red
	{0x10,0x30,0xff,0x00},			//4=blue
	{0x00,0xff,0x20,0x00}			//5=green
};

static void CreateStringBind(char * line) {
	line=trim(line);
	char * eventname=StripWord(line);
	CEvent * event;
	for (CEventVector_it ev_it=events.begin();ev_it!=events.end();ev_it++) {
		if (!_stricmp((*ev_it)->GetName(),eventname)) {
			event=*ev_it;
			goto foundevent;
		}
	}
	LOG_MSG("SDL MAPPER:\nCan't find matching event for %s\n(File %s:, Line: %d)\n\n",eventname,__FILE__,__LINE__);
	return ;
foundevent:
	CBind * bind;
	for (char * bindline=StripWord(line);*bindline;bindline=StripWord(line)) {
		for (CBindGroup_it it=bindgroups.begin();it!=bindgroups.end();it++) {
			bind=(*it)->CreateConfigBind(bindline);
			if (bind) {
				event->AddBind(bind);
				bind->SetFlags(bindline);
				break;
			}
		}
	}
}

static struct {
	const char * eventend;
	Bitu key;
} DefaultKeys[]={

	{"f1",SDL_SCANCODE_F1},		{"f2",SDL_SCANCODE_F2},		{"f3",SDL_SCANCODE_F3},		{"f4",SDL_SCANCODE_F4},
	{"f5",SDL_SCANCODE_F5},		{"f6",SDL_SCANCODE_F6},		{"f7",SDL_SCANCODE_F7},		{"f8",SDL_SCANCODE_F8},
	{"f9",SDL_SCANCODE_F9},		{"f10",SDL_SCANCODE_F10},	{"f11",SDL_SCANCODE_F11},	{"f12",SDL_SCANCODE_F12},

	{"1",SDL_SCANCODE_1},		{"2",SDL_SCANCODE_2},		{"3",SDL_SCANCODE_3},		{"4",SDL_SCANCODE_4},
	{"5",SDL_SCANCODE_5},		{"6",SDL_SCANCODE_6},		{"7",SDL_SCANCODE_7},		{"8",SDL_SCANCODE_8},
	{"9",SDL_SCANCODE_9},		{"0",SDL_SCANCODE_0},

	{"a",SDL_SCANCODE_A},		{"b",SDL_SCANCODE_B},		{"c",SDL_SCANCODE_C},		{"d",SDL_SCANCODE_D},
	{"e",SDL_SCANCODE_E},		{"f",SDL_SCANCODE_F},		{"g",SDL_SCANCODE_G},		{"h",SDL_SCANCODE_H},
	{"i",SDL_SCANCODE_I},		{"j",SDL_SCANCODE_J},		{"k",SDL_SCANCODE_K},		{"l",SDL_SCANCODE_L},
	{"m",SDL_SCANCODE_M},		{"n",SDL_SCANCODE_N},		{"o",SDL_SCANCODE_O},		{"p",SDL_SCANCODE_P},
	{"q",SDL_SCANCODE_Q},		{"r",SDL_SCANCODE_R},		{"s",SDL_SCANCODE_S},		{"t",SDL_SCANCODE_T},
	{"u",SDL_SCANCODE_U},		{"v",SDL_SCANCODE_V},		{"w",SDL_SCANCODE_W},		{"x",SDL_SCANCODE_X},
	{"y",SDL_SCANCODE_Y},		{"z",SDL_SCANCODE_Z},		{"space",SDL_SCANCODE_SPACE},
	{"esc",SDL_SCANCODE_ESCAPE},	{"equals",SDL_SCANCODE_EQUALS},		{"grave",SDL_SCANCODE_GRAVE},
	{"tab",SDL_SCANCODE_TAB},		{"enter",SDL_SCANCODE_RETURN},		{"bspace",SDL_SCANCODE_BACKSPACE},
	{"lbracket",SDL_SCANCODE_LEFTBRACKET},						{"rbracket",SDL_SCANCODE_RIGHTBRACKET},
	{"minus",SDL_SCANCODE_MINUS},	{"capslock",SDL_SCANCODE_CAPSLOCK},	{"semicolon",SDL_SCANCODE_SEMICOLON},
	{"quote", SDL_SCANCODE_APOSTROPHE},	{"backslash",SDL_SCANCODE_BACKSLASH},	{"lshift",SDL_SCANCODE_LSHIFT},
	{"rshift",SDL_SCANCODE_RSHIFT},	{"lalt",SDL_SCANCODE_LALT},			{"ralt",SDL_SCANCODE_RALT},
	{"lctrl",SDL_SCANCODE_LCTRL},	{"rctrl",SDL_SCANCODE_RCTRL},		{"comma",SDL_SCANCODE_COMMA},
	{"period",SDL_SCANCODE_PERIOD},	{"slash",SDL_SCANCODE_SLASH},		{"printscreen",SDL_SCANCODE_PRINTSCREEN},
	{"scrolllock",SDL_SCANCODE_SCROLLLOCK},	{"pause",SDL_SCANCODE_PAUSE},		{"pagedown",SDL_SCANCODE_PAGEDOWN},
	{"pageup",SDL_SCANCODE_PAGEUP},	{"insert",SDL_SCANCODE_INSERT},		{"home",SDL_SCANCODE_HOME},
	{"delete",SDL_SCANCODE_DELETE},	{"end",SDL_SCANCODE_END},			{"up",SDL_SCANCODE_UP},
	{"left",SDL_SCANCODE_LEFT},		{"down",SDL_SCANCODE_DOWN},			{"right",SDL_SCANCODE_RIGHT},
	{"kp_0",SDL_SCANCODE_KP_0},	{"kp_1",SDL_SCANCODE_KP_1},	{"kp_2",SDL_SCANCODE_KP_2},	{"kp_3",SDL_SCANCODE_KP_3},
	{"kp_4",SDL_SCANCODE_KP_4},	{"kp_5",SDL_SCANCODE_KP_5},	{"kp_6",SDL_SCANCODE_KP_6},	{"kp_7",SDL_SCANCODE_KP_7},
	{"kp_8",SDL_SCANCODE_KP_8},	{"kp_9",SDL_SCANCODE_KP_9},	{"numlock",SDL_SCANCODE_NUMLOCKCLEAR},
	{"kp_divide",SDL_SCANCODE_KP_DIVIDE},	{"kp_multiply",SDL_SCANCODE_KP_MULTIPLY},
	{"kp_minus",SDL_SCANCODE_KP_MINUS},		{"kp_plus",SDL_SCANCODE_KP_PLUS},
	{"kp_period",SDL_SCANCODE_KP_PERIOD},	{"kp_enter",SDL_SCANCODE_KP_ENTER},

	
	/* Is that the extra backslash key ("less than" key) */
	/* on some keyboards with the 102-keys layout??      */
	{"lessthan",SDL_SCANCODE_NONUSBACKSLASH},

	/* Windows 95 keyboard stuff */
	{"lwindows",SDL_SCANCODE_LGUI},
	{"rwindows",SDL_SCANCODE_RGUI},
	{"rwinmenu",SDL_SCANCODE_MENU},
	
	{"audiomute",SDL_SCANCODE_AUDIOMUTE},
	{"volumedown",SDL_SCANCODE_VOLUMEDOWN},	
	{"volumeup",SDL_SCANCODE_VOLUMEUP},	
	
	{"mouse_left",SDL_BUTTON_LEFT},
	{"mouse_middle",SDL_BUTTON_MIDDLE},
	{"mouse_middle",SDL_BUTTON_RIGHT},	
	
	{0,0}
};

static void CreateDefaultBinds(void) {
	char buffer[512];
	Bitu i = 0;
	while (DefaultKeys[i].eventend) {
		sprintf(buffer, "key_%s \"key %" sBitfs(d) "\"", DefaultKeys[i].eventend, DefaultKeys[i].key);
		CreateStringBind(buffer);
		i++;
	}
	sprintf(buffer, "mod_1 \"key %d\"", SDL_SCANCODE_RCTRL); CreateStringBind(buffer);
	sprintf(buffer, "mod_1 \"key %d\"", SDL_SCANCODE_LCTRL); CreateStringBind(buffer);
	sprintf(buffer, "mod_2 \"key %d\"", SDL_SCANCODE_RALT); CreateStringBind(buffer);
	sprintf(buffer, "mod_2 \"key %d\"", SDL_SCANCODE_LALT); CreateStringBind(buffer);
	for (CHandlerEventVector_it hit = handlergroup.begin(); hit != handlergroup.end(); hit++) {
		(*hit)->MakeDefaultBind(buffer);
		CreateStringBind(buffer);
	}

	/* joystick1, buttons 1-6 */
	sprintf(buffer, "jbutton_0_0 \"stick_0 button 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_1 \"stick_0 button 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_2 \"stick_0 button 2\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_3 \"stick_0 button 3\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_4 \"stick_0 button 4\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_5 \"stick_0 button 5\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_6 \"stick_0 button 6\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_7 \"stick_0 button 7\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_8 \"stick_0 button 8\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_9 \"stick_0 button 9\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_10 \"stick_0 button 10\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_11 \"stick_0 button 11\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_0_12 \"stick_0 button 12\" "); CreateStringBind(buffer);
	/* joystick2, buttons 1-2 */
	sprintf(buffer, "jbutton_1_0 \"stick_1 button 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_1 \"stick_1 button 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_2 \"stick_1 button 2\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_3 \"stick_1 button 3\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_4 \"stick_1 button 4\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_5 \"stick_1 button 5\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_6 \"stick_1 button 6\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_7 \"stick_1 button 7\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_8 \"stick_1 button 8\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_9 \"stick_1 button 9\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_10 \"stick_1 button 10\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_11 \"stick_1 button 11\" "); CreateStringBind(buffer);
	sprintf(buffer, "jbutton_1_12 \"stick_1 button 12\" "); CreateStringBind(buffer);

	/* joystick1, axes 1-4 */
	sprintf(buffer, "jaxis_0_0- \"stick_0 axis 0 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_0+ \"stick_0 axis 0 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_1- \"stick_0 axis 1 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_1+ \"stick_0 axis 1 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_2- \"stick_0 axis 2 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_2+ \"stick_0 axis 2 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_3- \"stick_0 axis 3 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_3+ \"stick_0 axis 3 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_4- \"stick_0 axis 4 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_4+ \"stick_0 axis 4 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_5- \"stick_0 axis 5 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_0_5+ \"stick_0 axis 5 1\" "); CreateStringBind(buffer);
	/* joystick2, axes 1-2 */
	sprintf(buffer, "jaxis_1_0- \"stick_1 axis 0 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_0+ \"stick_1 axis 0 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_1- \"stick_1 axis 1 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_1+ \"stick_1 axis 1 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_2- \"stick_1 axis 2 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_2+ \"stick_1 axis 2 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_3- \"stick_1 axis 3 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_3+ \"stick_1 axis 3 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_4- \"stick_1 axis 4 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_4+ \"stick_1 axis 4 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_5- \"stick_1 axis 5 0\" "); CreateStringBind(buffer);
	sprintf(buffer, "jaxis_1_5+ \"stick_1 axis 5 1\" "); CreateStringBind(buffer);

	/* joystick1, hat */
	sprintf(buffer, "jhat_0_0_0 \"stick_0 hat 0 1\" "); CreateStringBind(buffer);
	sprintf(buffer, "jhat_0_0_1 \"stick_0 hat 0 2\" "); CreateStringBind(buffer);
	sprintf(buffer, "jhat_0_0_2 \"stick_0 hat 0 4\" "); CreateStringBind(buffer);
	sprintf(buffer, "jhat_0_0_3 \"stick_0 hat 0 8\" "); CreateStringBind(buffer);
	/*
	sprintf(buffer,"jhat_1_0_0 \"stick_1 hat 1 1\" ");CreateStringBind(buffer);
	sprintf(buffer,"jhat_1_0_1 \"stick_1 hat 1 2\" ");CreateStringBind(buffer);
	sprintf(buffer,"jhat_1_0_2 \"stick_1 hat 1 4\" ");CreateStringBind(buffer);
	sprintf(buffer,"jhat_1_0_3 \"stick_1 hat 1 8\" ");CreateStringBind(buffer);
	*/
}

void MAPPER_AddHandler(MAPPER_Handler * handler,MapKeys key,Bitu mods,char const * const eventname,char const * const buttonname) {
	//Check if it already exists=> if so return.
	for(CHandlerEventVector_it it=handlergroup.begin();it!=handlergroup.end();it++)
		if(strcmp((*it)->buttonname,buttonname) == 0) return;

	char tempname[17];
	strcpy(tempname,"hand_");
	strcat(tempname,eventname);
	new CHandlerEvent(tempname,handler,key,mods,buttonname);
	return ;
}
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline bool ExistsMapperFile (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
std::string MapperSetFilePath(std::string sFile)
{
	
	if ( sFile.empty() != 0 ){
		return sFile;
	}

	std::string cTemp = "";
	            cTemp = sFile[0];
	
	if (cTemp.compare(0,1,".") == 0){
		sFile.erase (sFile.begin()); 
	}	
	if (cTemp.compare(0,1,"\\") == 0){
		sFile.erase (sFile.begin()); 
	}	
	
	return sFile;
}

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
static void MAPPER_SaveBinds(void) {
	
	std::string MapFile = "";
	MapFile = MapperSetFilePath(mapper.filename.c_str());	
	
	if (ExistsMapperFile( MapFile.c_str() )){
			LOG_MSG("SDL : Saving Mapper Settings ...\n%s\n", MapFile.c_str());						
	}	
	
	FILE * savefile=fopen(MapFile.c_str(),"wt+");
	if (!savefile) {
		LOG_MSG("SDL : Can't Save The mappings \n%s\n",MapFile.c_str());
		return;
	}
	char buf[128];
	for (CEventVector_it event_it=events.begin();event_it!=events.end();event_it++) {
		CEvent * event=*(event_it);
		fprintf(savefile,"%s ",event->GetName());
		for (CBindList_it bind_it=event->bindlist.begin();bind_it!=event->bindlist.end();bind_it++) {
			CBind * bind=*(bind_it);
			bind->ConfigName(buf);
			bind->AddFlags(buf);
			fprintf(savefile,"\"%s\" ",buf);
		}
		fprintf(savefile,"\n");
	}
	fclose(savefile);
	change_action_text("SDL : Ok, Mapper file saved ...",CLR_WHITE);
}
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static bool MAPPER_LoadBinds(void) {
			
	std::string MapFile = "";
	MapFile = MapperSetFilePath(mapper.filename.c_str());		
	
	if (ExistsMapperFile( MapFile.c_str() )){
			LOG_MSG("SDL : Loading Mapper Settings ...\n%s\n", MapFile.c_str());						
	}
	else{
			LOG_MSG("SDL : Mapper File not Found \n%s\n", MapFile.c_str());
			return false;
	}
	
	FILE * loadfile=fopen(MapFile.c_str(),"rt");
	if (!loadfile) return false;
	char linein[512];
	while (fgets(linein,512,loadfile)) {
		CreateStringBind(linein);
	}
	fclose(loadfile);
	return true;
}

void MAPPER_CheckEvent(SDL_Event * event) {
	for (CBindGroup_it it=bindgroups.begin();it!=bindgroups.end();it++) {
		if ((*it)->CheckEvent(event)) return;
	}
}

void BIND_MappingEvents(void) {
	SDL_Event event;
	static bool isButtonPressed = false;
	static CButton *lastHoveredButton = NULL;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {

		case SDL_MOUSEBUTTONDOWN:
			isButtonPressed = true;
			/* Further check where are we pointing at right now */

		case SDL_MOUSEMOTION:
			if (!isButtonPressed)
				break;
			/* Normalize position in case a scaled sub-window is used (say on Android) */
			event.button.x=(event.button.x-mapper.draw_rect.x)*mapper.draw_surface->w/mapper.draw_rect.w;
			if ((event.button.x<0) || (event.button.x>=mapper.draw_surface->w))
				break;
			event.button.y=(event.button.y-mapper.draw_rect.y)*mapper.draw_surface->h/mapper.draw_rect.h;
			if ((event.button.y<0) || (event.button.y>=mapper.draw_surface->h))
				break;
			/* Maybe we have been pointing at a specific button for a little while  */
			if (lastHoveredButton) {
				/* Check if there's any change */
				if (lastHoveredButton->OnTop(event.button.x,event.button.y))
					break;
				/* Not pointing at given button anymore */
				if (lastHoveredButton == last_clicked)
					lastHoveredButton->Click();
				else
					lastHoveredButton->BindColor();
				mapper.redraw=true;
				lastHoveredButton=NULL;
			}
			/* Check which button are we currently pointing at */
			for (CButton_it but_it = buttons.begin();but_it!=buttons.end();but_it++) {
				if (dynamic_cast<CClickableTextButton *>(*but_it) && (*but_it)->OnTop(event.button.x,event.button.y)) {
					(*but_it)->SetColor(CLR_RED);
					mapper.redraw=true;
					lastHoveredButton=*but_it;
					break;
				}
			}
			break;

		case SDL_MOUSEBUTTONUP:
			isButtonPressed = false;
			if (lastHoveredButton) {
				/* For most buttons the actual new color is going to be green; But not for a few others. */
				lastHoveredButton->BindColor();
				mapper.redraw=true;
				lastHoveredButton = NULL;
			}
			/* Normalize position in case a scaled sub-window is used (say on Android) */
			event.button.x=(event.button.x-mapper.draw_rect.x)*mapper.draw_surface->w/mapper.draw_rect.w;
			if ((event.button.x<0) || (event.button.x>=mapper.draw_surface->w))
				break;
			event.button.y=(event.button.y-mapper.draw_rect.y)*mapper.draw_surface->h/mapper.draw_rect.h;
			if ((event.button.y<0) || (event.button.y>=mapper.draw_surface->h))
				break;
			/* Check the press */
			for (CButton_it but_it = buttons.begin();but_it!=buttons.end();but_it++) {
				if (dynamic_cast<CClickableTextButton *>(*but_it) && (*but_it)->OnTop(event.button.x,event.button.y)) {
					(*but_it)->Click();
					break;
				}
			}
			break;

		case SDL_WINDOWEVENT:
			/* The resize event MAY arrive e.g. when the mapper is
			 * toggled, at least on X11. Furthermore, the restore
			 * event should be handled on Android.
			 */
			if ((event.window.event == SDL_WINDOWEVENT_RESIZED)
			    || (event.window.event == SDL_WINDOWEVENT_RESTORED)) {
				mapper.surface = SDL_GetWindowSurface(mapper.window);
				GFX_UpdateDisplayDimensions(event.window.data1, event.window.data2);
				mapper.draw_rect=GFX_GetSDLSurfaceSubwindowDims(800,600);
				DrawButtons();
			}
			break;

		case SDL_QUIT:
			isButtonPressed = false;
			lastHoveredButton = NULL;
			mapper.exit=true;
			break;
		default:
			if (mapper.addbind) for (CBindGroup_it it=bindgroups.begin();it!=bindgroups.end();it++) {
				CBind * newbind=(*it)->CreateEventBind(&event);
				if (!newbind) continue;
				mapper.aevent->AddBind(newbind);
				SetActiveEvent(mapper.aevent);
				mapper.addbind=false;
				break;
			}
		}
	}
}

static void InitializeJoysticks(void) {
	mapper.sticks.num=0;
	mapper.sticks.num_groups=0;
	if (joytype != JOY_NONE) {
		mapper.sticks.num=SDL_NumJoysticks();
		
		if (joytype==JOY_AUTO) {
			// try to figure out what joystick type to select
			// depending on the number of physically attached joysticks
			if (mapper.sticks.num>1) {
				// more than one joystick present; if all are acceptable use 2axis
				// to allow emulation of two joysticks
				bool first_usable=false;
				SDL_Joystick* tmp_stick1=SDL_JoystickOpen(0);
				if (tmp_stick1) {
					if ((SDL_JoystickNumAxes(tmp_stick1)>1) || (SDL_JoystickNumButtons(tmp_stick1)>0)) {
						first_usable=true;
					}
					SDL_JoystickClose(tmp_stick1);
				}
				bool second_usable=false;
				SDL_Joystick* tmp_stick2=SDL_JoystickOpen(1);
				if (tmp_stick2) {
					if ((SDL_JoystickNumAxes(tmp_stick2)>1) || (SDL_JoystickNumButtons(tmp_stick2)>0)) {
						second_usable=true;
					}
					SDL_JoystickClose(tmp_stick2);
				}
				// choose joystick type now that we know which physical joysticks are usable
				if (first_usable) {
					if (second_usable) {
						joytype=JOY_2AXIS;
						LOG_MSG("SDL : Two or more joysticks reported, initializing with 2axis");
					} else {
						joytype=JOY_4AXIS;
						LOG_MSG("SDL : One joystick reported, initializing with 4axis");
					}
				} else if (second_usable) {
					joytype=JOY_4AXIS_2;
					LOG_MSG("SDL : One joystick reported, initializing with 4axis_2");
				}
			} else if (mapper.sticks.num) {
				// one joystick present; if it is acceptable use 4axis
				joytype=JOY_NONE;
				SDL_Joystick* tmp_stick1=SDL_JoystickOpen(0);
				if (tmp_stick1) {
					if ((SDL_JoystickNumAxes(tmp_stick1)>0) || (SDL_JoystickNumButtons(tmp_stick1)>0)) {
						joytype=JOY_4AXIS;
						LOG_MSG("SDL : One joystick reported, initializing with 4axis");
					}
				}
			} else {
				joytype=JOY_NONE;
			}
		}
	}
}

static void CreateBindGroups(void) {
	bindgroups.clear();
	new CKeyBindGroup(SDL_NUM_SCANCODES);
	if (joytype != JOY_NONE) {
#if defined (REDUCE_JOYSTICK_POLLING)
		// direct access to the SDL joystick, thus removed from the event handling
		if (mapper.sticks.num) SDL_JoystickEventState(SDL_DISABLE);
#else
		// enable joystick event handling
		if (mapper.sticks.num) SDL_JoystickEventState(SDL_ENABLE);
		else return;
#endif
		Bit8u joyno=0;
		switch (joytype) {
		case JOY_NONE:
			break;
		case JOY_4AXIS:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new C4AxisBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;
		case JOY_4AXIS_2:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new C4AxisBindGroup(joyno+1U,joyno);
			new CStickBindGroup(joyno,joyno+1U,true);
			break;
		case JOY_FCS:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CFCSBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;
		case JOY_FCSLW:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CFCSLWBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;			
		case JOY_CH:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CCHBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;
		case JOY_CHVP:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CCHVPBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;
		case JOY_CHGS:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CCHGSBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;				
		case JOY_SHOT6:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CShot6BindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;
		case JOY_CAPCOM:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CCPADBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;	
		case JOY_INTER6:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CIntA6BindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;	
		case JOY_WHEEL:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CWheelBindGroup(joyno,joyno);
			new CStickBindGroup(joyno+1U,joyno+1U,true);
			break;			
		case JOY_2AXIS:
		default:
			mapper.sticks.stick[mapper.sticks.num_groups++]=new CStickBindGroup(joyno,joyno);
			if((joyno+1U) < mapper.sticks.num) {
				mapper.sticks.stick[mapper.sticks.num_groups++]=new CStickBindGroup(joyno+1U,joyno+1U);
			} else {
				new CStickBindGroup(joyno+1U,joyno+1U,true);
			}
			break;
		}
	}
}

#if defined (REDUCE_JOYSTICK_POLLING)
void MAPPER_UpdateJoysticks(void) {
	for (Bitu i=0; i<mapper.sticks.num_groups; i++) {
		mapper.sticks.stick[i]->UpdateJoystick();
	}
}
#endif

void MAPPER_LosingFocus(void) {
	for (CEventVector_it evit=events.begin();evit!=events.end();evit++) {
		if(*evit != caps_lock_event && *evit != num_lock_event)
			(*evit)->DeActivateAll();
	}
}

void MAPPER_RunEvent(Bitu /*val*/) {
	KEYBOARD_ClrBuffer();	//Clear buffer
	GFX_LosingFocus();		//Release any keys pressed (buffer gets filled again).
	MAPPER_RunInternal();
}

void MAPPER_Run(bool pressed) {
	if (pressed)
		return;
	PIC_AddEvent(MAPPER_RunEvent,0);	//In case mapper deletes the key object that ran it
}

SDL_Surface* SDL_SetVideoMode_Wrap(int width,int height,int bpp,Bit32u flags);

void MAPPER_RunInternal() {
	int cursor = SDL_ShowCursor(SDL_QUERY);
	SDL_ShowCursor(SDL_ENABLE);
	bool mousetoggle=false;
	if(mouselocked) {
		mousetoggle=true;
		GFX_CaptureMouse();
	}

	/* Be sure that there is no update in progress */
		GFX_EndUpdate( 0 );

	mapper.window=GFX_SetSDLSurfaceWindow(800,600);
	SDL_ShowWindow( mapper.window );
	if (mapper.window == NULL)
		E_Exit("Could not initialize video mode for mapper: %s [Line %d]",SDL_GetError(),__LINE__);

	mapper.surface=SDL_GetWindowSurface(mapper.window);
	if (mapper.surface == NULL)
		E_Exit("Could not initialize video mode for mapper: %s [Line %d]",SDL_GetError(), __LINE__);

	/* Set some palette entries */
	mapper.draw_surface=SDL_CreateRGBSurface(0,800,600,8,0,0,0,0);
	// Needed for SDL_BlitScaled
	mapper.draw_surface_nonpaletted=SDL_CreateRGBSurface(0,800,600,32,0x0000ff00,0x00ff0000,0xff000000,0);
	mapper.draw_rect=GFX_GetSDLSurfaceSubwindowDims(800,600);
	// Sorry, but SDL_SetSurfacePalette requires a full palette.
	SDL_Palette *sdl2_map_pal_ptr = SDL_AllocPalette(256);
	SDL_SetPaletteColors(sdl2_map_pal_ptr, map_pal, 0, CLR_LAST);
	SDL_SetSurfacePalette(mapper.draw_surface, sdl2_map_pal_ptr);
	if (last_clicked) {
		last_clicked->BindColor();
		last_clicked=NULL;
	}
	
	/* Go in the event loop */
	mapper.exit=false;	
	mapper.redraw=true;
	SetActiveEvent(0);
#if defined (REDUCE_JOYSTICK_POLLING)
	SDL_JoystickEventState(SDL_ENABLE);
#endif
	while (!mapper.exit) {
		if (mapper.redraw) {
			mapper.redraw=false;		
			DrawButtons();
		} else {
			SDL_UpdateWindowSurface(mapper.window);
		}
		BIND_MappingEvents();
		SDL_Delay(1);
	}
	/* ONE SHOULD NOT FORGET TO DO THIS!
	Unless a memory leak is desired... */
	SDL_FreeSurface(mapper.draw_surface);
	SDL_FreeSurface(mapper.draw_surface_nonpaletted);
	SDL_FreePalette(sdl2_map_pal_ptr);
#if defined (REDUCE_JOYSTICK_POLLING)
	SDL_JoystickEventState(SDL_DISABLE);
#endif
	if(mousetoggle) GFX_CaptureMouse();
	SDL_ShowCursor(cursor);
	//GFX_ResetScreen();
	GFX_ResetVoodoo();
}

void MAPPER_Init(void) {
	InitializeJoysticks();
	CreateLayout();
	CreateBindGroups();
	if (!MAPPER_LoadBinds()) CreateDefaultBinds();
	for (CButton_it but_it = buttons.begin();but_it!=buttons.end();but_it++) {
		(*but_it)->BindColor();
	}
	
	if (SDL_GetModState()&KMOD_CAPS) {
		for (CBindList_it bit=caps_lock_event->bindlist.begin();bit!=caps_lock_event->bindlist.end();bit++) {
			(*bit)->ActivateBind(32767,true,false);
			(*bit)->DeActivateBind(false);
		}
	}
	if (SDL_GetModState()&KMOD_NUM) {
		for (CBindList_it bit=num_lock_event->bindlist.begin();bit!=num_lock_event->bindlist.end();bit++) {
			(*bit)->ActivateBind(32767,true,false);
			(*bit)->DeActivateBind(false);
		}
	}
	
}
//Somehow including them at the top conflicts with something in setup.h
#ifdef SDL_VIDEO_DRIVER_X11
	#ifdef C_X11_XKB
		#include "SDL_syswm.h"
		#include <X11/XKBlib.h>
	#endif
#endif
void MAPPER_StartUp(Section * sec) {
	Section_prop * section=static_cast<Section_prop *>(sec);
	mapper.sticks.num=0;
	mapper.sticks.num_groups=0;
	memset(&virtual_joysticks,0,sizeof(virtual_joysticks));
	Prop_path* pp = section->Get_path("mapperfile");
	mapper.filename = pp->realpath;
	MAPPER_AddHandler(&MAPPER_Run,MK_f1,MMOD1,"mapper","Mapper");
}

