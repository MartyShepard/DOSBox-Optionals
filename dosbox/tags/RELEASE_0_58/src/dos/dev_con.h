/*
 *  Copyright (C) 2002-2003  The DOSBox Team
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

#include "dos_inc.h"
#include "../ints/int10.h"

#define NUMBER_ANSI_DATA 10

class device_CON : public DOS_Device {
public:
	device_CON();
	bool Read(Bit8u * data,Bit16u * size);
	bool Write(Bit8u * data,Bit16u * size);
	bool Seek(Bit32u * pos,Bit32u type);
	bool Close();
    void ClearAnsi(void);
	Bit16u GetInformation(void);
private:
	Bit8u cache;
    struct ansi { /* should create a constructor which fills them with the appriorate values */
        bool esc;
        bool sci;
        Bit8u attr;
        Bit8u data[NUMBER_ANSI_DATA];
        Bit8u numberofarg;
        Bit16u nrows;
        Bit16u ncols;
        Bit8s savecol;
        Bit8s saverow;
    } ansi;
    
};
void INT10_TeletypeOutput(Bit8u chr,Bit8u attr,bool showattr, Bit8u page);
void INT10_SetCursorPos(Bit8u row,Bit8u col,Bit8u page);

bool device_CON::Read(Bit8u * data,Bit16u * size) {
	Bit16u oldax=reg_ax;
	Bit16u count=0;
	if ((cache) && (*size)) {
		data[count++]=cache;
        if(dos.echo) {
            INT10_TeletypeOutput(cache,7,false,0);
        }
        cache=0;

	}
	while (*size>count) {
		reg_ah=0;
		CALLBACK_RunRealInt(0x16);
		switch(reg_al) {
		case 13:
			data[count++]=0x0D;
			if (*size>count) data[count++]=0x0A;    // it's only expanded if there is room for it. (NO cache)
  			*size=count;
			reg_ax=oldax;
            if(dos.echo) { 
                 INT10_TeletypeOutput(13,7,false,0); //maybe don't do this ( no need for it actually ) (but it's compatible)
                 INT10_TeletypeOutput(10,7,false,0);
            }
			return true;
            break;
        case 8:
            if(*size==1) data[count++]=reg_al;  //one char at the time so give back that BS
            else if(count) {                    //Remove data if it exists (extended keys don't go right)
                data[count--]=0;
                INT10_TeletypeOutput(8,7,false,0);
                INT10_TeletypeOutput(' ',7,false,0);
            } else {
                continue;                       //no data read yet so restart whileloop.
            }
                
            break;
		default:
			data[count++]=reg_al;
			break;
		case 0:
			data[count++]=reg_al;
			if (*size>count) data[count++]=reg_ah;
			else cache=reg_ah;
			break;
            
            }
        if(dos.echo) { //what to do if *size==1 and character is BS ?????
            INT10_TeletypeOutput(reg_al,7,false,0);
        }
	}
	*size=count;
	reg_ax=oldax;
	return true;
}


bool device_CON::Write(Bit8u * data,Bit16u * size) {
	Bit16u count=0;
    Bitu i;
    Bit8s col,row;
    static bool ansiwarned=false;

    while (*size>count) {
        if (!ansi.esc){
            if(data[count]=='\033') {
                /*clear the datastructure */
                ClearAnsi();
                /* start the sequence */
                ansi.esc=true;
                count++;
                if(!ansiwarned) {
                    LOG(LOG_IOCTL,"ANSI sequences detected. enabling ansi support"); /* maybe LOG_MSG */
                    ansiwarned=true;
                }
                continue;

            } else { 
               INT10_TeletypeOutput(data[count],ansi.attr,true,0);
               count++;
               continue;
            };
        };
        /* ansi.esc=true */
        if(!ansi.sci){
            
            switch(data[count]){
            case '[': 
                ansi.sci=true;
                break;
            case '7': /* save cursor pos +attr */
            case '8': /* restore this  (Wonder if this is actually used) */
            case 'D':/* scrolling DOWN*/
            case 'M':/* scrolling UP*/ 
            default:
                LOG(LOG_IOCTL,"ANSI: unknown char %c after a esc",data[count]); /*prob () */
                ClearAnsi();
                break;
            }
            count++;
            continue;
       
        }
        /*ansi.esc and ansi.sci are true */
        switch(data[count]){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ansi.data[ansi.numberofarg]=10*ansi.data[ansi.numberofarg]+(data[count]-'0');
            break;
        case ';': /* till a max of NUMBER_ANSI_DATA */
            ansi.numberofarg++;
            break;
        case 'm':               /* SGR */
            for(i=0;i<=ansi.numberofarg;i++){ 

                switch(ansi.data[i]){
                case 0: /* normal */
                    ansi.attr=0x7;
                    break;
                case 1: /* bold mode on*/
                    ansi.attr|=0x8;
                    break;
                case 4: /* underline */
                    LOG(LOG_IOCTL,"ANSI:no support for underline yet");
                    break;
                case 5: /* blinking */
                    LOG(LOG_IOCTL,"ANSI:no support for blinking yet");
                    break;
                case 7: /* reverse */
                    LOG(LOG_IOCTL,"ANSI:no support for reverse yet");
                    break;
                case 30: /* fg color black */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x0;
                    break;
                case 31:  /* fg color red */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x4;
                    break;
                case 32:  /* fg color green */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x2;
                    break;
                case 33: /* fg color yellow */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x6;
                    break;
                case 34: /* fg color blue */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x1;
                    break;
                case 35: /* fg color magenta */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x5;
                    break;
                case 36: /* fg color cyan */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x3;
                    break;
                case 37: /* fg color white */
                    ansi.attr&=0xf8;
                    ansi.attr|=0x7;
                    break;
                case 40:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x0;
                    break;
                case 41:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x40;
                    break;
                case 42:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x20;
                    break;
                case 43:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x60;
                    break;
                case 44:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x10;
                    break;
                case 45:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x50;
                    break;
                case 46:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x30;
                    break;
                case 47:
                    ansi.attr&=0x8f;
                    ansi.attr|=0x70;
                    break;
                default:
                    break;

                }
            }
            ClearAnsi();
            break;
        case 'f':
        case 'H':/* Cursor Pos*/
            if(ansi.data[0]==0) ansi.data[0]=1;
            if(ansi.data[1]==0) ansi.data[1]=1;
            INT10_SetCursorPos(--(ansi.data[0]),--(ansi.data[1]),0); /*ansi=1 based, int10 is 0 based */
            ClearAnsi();
            break;
        case 'A': /* cursor up*/
            col=CURSOR_POS_COL(0) ;
            row=CURSOR_POS_ROW(0) - (ansi.data[0]? ansi.data[0] : 1);
            INT10_SetCursorPos(row,col,0);
            ClearAnsi();
            break;
        case 'C': /*cursor forward */
            col=CURSOR_POS_COL(0) + (ansi.data[0]? ansi.data[0] : 1);
            row=CURSOR_POS_ROW(0);
            while(col>=ansi.ncols) {
                row++;
                col = col - ansi.ncols;         // should depend on linebrake mode
            }
            INT10_SetCursorPos(row,col,0);
            ClearAnsi();
            break;
        case 'J': /*erase screen and move cursor home*/
            if(ansi.data[0]==0) ansi.data[0]=2;
            if(ansi.data[0]!=2) {/* only number 2 (the standard one supported) */ 
                LOG(LOG_IOCTL,"ANSI: esc[%dJ called : not supported",ansi.data[0]);
                break;
            }
            for(i=0;i<(Bitu)ansi.ncols*ansi.nrows;i++) INT10_TeletypeOutput(' ',ansi.attr,true,0);
            ClearAnsi();
            INT10_SetCursorPos(0,0,0);
            break;
        case 'h': /* set MODE (if code =7 enable linewrap) */
        case 'I': /*RESET MODE */
            LOG(LOG_IOCTL,"ANSI: set/reset mode called(not supported)");
            ClearAnsi();
            break;
        case 'D': /*Cursor Backward  */
            col=CURSOR_POS_COL(0) - (ansi.data[0]? ansi.data[0] : 1);
            row=CURSOR_POS_ROW(0);
            while(col<0) {
                row--;
                col = col + ansi.ncols ;        // should depend on linebrake mode
            }
            INT10_SetCursorPos(row,col,0);
            ClearAnsi();
            break;
        case 'u': /* Restore Cursor Pos */
            INT10_SetCursorPos(ansi.saverow,ansi.savecol,0);
            ClearAnsi();
            break;
        case 's': /* SAVE CURSOR POS */
            ansi.savecol=CURSOR_POS_COL(0);
            ansi.saverow=CURSOR_POS_ROW(0);
            ClearAnsi();
            break;
        case 'K':/* erase till end of line */
        case 'l':/* (if code =7) disable linewrap */
        case 'p':/* reassign keys (needs strings) */
        case 'i':/* printer stuff */
        default:
            LOG(LOG_IOCTL,"ANSI: unhandled char %c in esc[",data[count]);
            ClearAnsi();
            break;
        }
	count++;
	}
	*size=count;
	return true;
}

bool device_CON::Seek(Bit32u * pos,Bit32u type) {
	// seek is valid
	*pos = 0;
	return true;
}

bool device_CON::Close() {
	return false;
}

Bit16u device_CON::GetInformation(void) {
	Bit16u head=mem_readw(BIOS_KEYBOARD_BUFFER_HEAD);
	Bit16u tail=mem_readw(BIOS_KEYBOARD_BUFFER_TAIL);
	
	if ((head==tail) && !cache) return 0x80D3; /* No Key Available */
	return 0x8093;		/* Key Available */
};

device_CON::device_CON() {
	name="CON";
	cache=0;
    ansi.esc=false;
    ansi.sci=false;
    ansi.attr=0x7;
    ansi.numberofarg=0;
    for(Bit8u i=0; i<NUMBER_ANSI_DATA;i++) ansi.data[i]=0;
    ansi.ncols=real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS); //should be updated once set/reset mode is implemented
    ansi.nrows=real_readb(BIOSMEM_SEG,BIOSMEM_NB_ROWS);
    ansi.saverow=0;
    ansi.savecol=0;
}

void device_CON::ClearAnsi(void){
    for(Bit8u i=0; i<NUMBER_ANSI_DATA;i++) ansi.data[i]=0;
    ansi.esc=false;
    ansi.sci=false;
    ansi.numberofarg=0;
}
