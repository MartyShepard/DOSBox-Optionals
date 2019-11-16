/*
 *  Copyright (C) 2002-2007  The DOSBox Team
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

#include "dosbox.h"
#include "bios.h"
#include "setup.h"
#include "support.h"
#include "../ints/int10.h"
#include "regs.h"
#include "callback.h"
#include "mapper.h"
#include "drives.h"
#include "dos_inc.h"

#include "dos_codepages.h"
#include "dos_keyboard_layout_data.h"


static FILE* OpenDosboxFile(const char* name) {
	Bit8u drive;
	char fullname[DOS_PATHLENGTH];

	localDrive* ldp=0;
	// try to build dos name
	if (DOS_MakeName((char*)name,fullname,&drive)) {
		try {
			// try to open file on mounted drive first
			ldp=dynamic_cast<localDrive*>(Drives[drive]);
			if (ldp) {
				FILE *tmpfile=ldp->GetSystemFilePtr(fullname, "rb");
				if (tmpfile != NULL) return tmpfile;
			}
		}
		catch(...) {}
	}
	FILE *tmpfile=fopen(name, "rb");
	return tmpfile;
}


class keyboard_layout {
public:
	keyboard_layout() {
		this->reset();
		language_codes=NULL;
		use_foreign_layout=false;
		sprintf(current_keyboard_file_name, "none");
	};

	~keyboard_layout();

	// read in a codepage from a .cpi-file
	Bitu read_codepage_file(const char* codepage_file_name, Bit32s codepage_id);
	Bit16u extract_codepage(const char* keyboard_file_name);
	// read in a keyboard layout from a .kl-file
	Bitu read_keyboard_file(const char* keyboard_file_name, Bit32s req_cp);

	// call layout_key to apply the current language layout
	bool layout_key(Bitu key, Bit8u flags1, Bit8u flags2, Bit8u flags3);

	Bitu switch_keyboard_layout(const char* new_layout, keyboard_layout* &created_layout);
	void switch_foreign_layout();


private:
	static const Bit8u layout_pages=12;
	Bit16u current_layout[(MAX_SCAN_CODE+1)*layout_pages];
	struct {
		Bit16u required_flags,forbidden_flags;
		Bit16u required_userflags,forbidden_userflags;
	} current_layout_planes[layout_pages-4];
	Bit8u additional_planes,used_lock_modifiers;

	// diacritics table
	Bit8u diacritics[2048];
	Bit16u diacritics_entries;
	Bit16u diacritics_character;
	Bit16u user_keys;

	char current_keyboard_file_name[256];
	bool use_foreign_layout;

	// language code storage used when switching layouts
	char** language_codes;
	Bitu language_code_count;

	void reset();
	void read_keyboard_file(Bit32s specific_layout);
	Bitu read_keyboard_file(const char* keyboard_file_name, Bit32s specific_layout, Bit32s requested_codepage);
	bool map_key(Bitu key, Bit16u layouted_key, bool is_command, bool is_keypair);
};


keyboard_layout::~keyboard_layout() {
	if (language_codes) {
		for (Bitu i=0; i<language_code_count; i++)
			delete[] language_codes[i];
		delete[] language_codes;
		language_codes=NULL;
	}
}

void keyboard_layout::reset() {
	for (Bit32u i=0; i<(MAX_SCAN_CODE+1)*layout_pages; i++) current_layout[i]=0;
	for (Bit32u i=0; i<layout_pages-4; i++) {
		current_layout_planes[i].required_flags=0;
		current_layout_planes[i].forbidden_flags=0xffff;
		current_layout_planes[i].required_userflags=0;
		current_layout_planes[i].forbidden_userflags=0xffff;
	}
	used_lock_modifiers=0x0f;
	diacritics_entries=0;		// no diacritics loaded
	diacritics_character=0;
	user_keys=0;				// all userkeys off
	language_code_count=0;
}

Bitu keyboard_layout::read_keyboard_file(const char* keyboard_file_name, Bit32s req_cp) {
	return this->read_keyboard_file(keyboard_file_name, -1, req_cp);
}

// switch to a different layout
void keyboard_layout::read_keyboard_file(Bit32s specific_layout) {
	if (strcmp(current_keyboard_file_name,"none"))
		this->read_keyboard_file(current_keyboard_file_name, specific_layout, dos.loaded_codepage);
}

static Bit32u read_kcl_file(const char* kcl_file_name, const char* layout_id) {
	FILE* tempfile = OpenDosboxFile(kcl_file_name);
	if (tempfile==0) return 0;

	static Bit8u rbuf[8192];

	// check ID-bytes of file
	Bit32u dr=(Bit32u)fread(rbuf, sizeof(Bit8u), 7, tempfile);
	if ((dr<7) || (rbuf[0]!=0x4b) || (rbuf[1]!=0x43) || (rbuf[2]!=0x46)) {
		fclose(tempfile);
		return 0;
	}

	fseek(tempfile, 7+rbuf[6], SEEK_SET);

	for (;;) {
		Bit32u cur_pos=(Bit32u)(ftell(tempfile));
		dr=(Bit32u)fread(rbuf, sizeof(Bit8u), 5, tempfile);
		if (dr<5) break;
		Bit16u len=host_readw(&rbuf[0]);

		Bit8u data_len=rbuf[2];

		char lng_codes[256];
		// get all language codes for this layout
		for (Bitu i=0; i<data_len;) {
			i+=2;
			Bitu lcpos=0;
			for (;i<data_len;) {
				fread(rbuf, sizeof(Bit8u), 1, tempfile);
				i++;
				if (((char)rbuf[0])==',') break;
				lng_codes[lcpos++]=(char)rbuf[0];
			}
			lng_codes[lcpos]=0;
			if (strcasecmp(lng_codes, layout_id)==0) {
				// language ID found in file, return file position
				fclose(tempfile);
				return cur_pos;
			}
		}
		fseek(tempfile, cur_pos+3+len, SEEK_SET);
	}

	fclose(tempfile);
	return 0;
}

Bitu keyboard_layout::read_keyboard_file(const char* keyboard_file_name, Bit32s specific_layout, Bit32s requested_codepage) {
	this->reset();

	if (specific_layout==-1) strcpy(current_keyboard_file_name, keyboard_file_name);
	if (!strcmp(keyboard_file_name,"none")) return KEYB_NOERROR;

	static Bit8u read_buf[65535];
	Bit32u read_buf_size, read_buf_pos, bytes_read;
	Bit32u start_pos=5;

	char nbuf[512];
	sprintf(nbuf, "%s.kl", keyboard_file_name);
	FILE* tempfile = OpenDosboxFile(nbuf);
	if (tempfile==NULL) {
		// see if build-in keyboard layout is available, then copy it
		if (!strncasecmp(keyboard_file_name,"BG",2)) {
			read_buf_size=687;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_BG[i];
		} else if (!strncasecmp(keyboard_file_name,"CZ243",5)) {
			read_buf_size=1003;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_CZ243[i];
		} else if (!strncasecmp(keyboard_file_name,"FR",2)) {
			read_buf_size=581;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_FR[i];
		} else if (!strncasecmp(keyboard_file_name,"GK",2)) {
			read_buf_size=1117;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_GK[i];
		} else if (!strncasecmp(keyboard_file_name,"GR",2)) {
			read_buf_size=596;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_GR[i];
		} else if (!strncasecmp(keyboard_file_name,"HR",2)) {
			read_buf_size=993;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_HR[i];
		} else if (!strncasecmp(keyboard_file_name,"HU",2)) {
			read_buf_size=964;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_HU[i];
		} else if (!strncasecmp(keyboard_file_name,"IT",2)) {
			read_buf_size=236;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_IT[i];
		} else if (!strncasecmp(keyboard_file_name,"NL",2)) {
			read_buf_size=552;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_NL[i];
		} else if (!strncasecmp(keyboard_file_name,"NO",2)) {
			read_buf_size=477;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_NO[i];
		} else if (!strncasecmp(keyboard_file_name,"PL",2)) {
			read_buf_size=261;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_PL[i];
		} else if (!strncasecmp(keyboard_file_name,"RU",2)) {
			read_buf_size=1130;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_RU[i];
		} else if (!strncasecmp(keyboard_file_name,"SK",2)) {
			read_buf_size=993;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_SK[i];
		} else if (!strncasecmp(keyboard_file_name,"SP",2)) {
			read_buf_size=471;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_SP[i];
		} else if (!strncasecmp(keyboard_file_name,"SU",2)) {
			read_buf_size=683;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_SU[i];
		} else if (!strncasecmp(keyboard_file_name,"SV",2)) {
			read_buf_size=683;
			for (Bitu i=0; i<read_buf_size; i++) read_buf[i]=layout_SV[i];
		} else {
			// neither a matching .kl-file found, nor is the layout available internally
			// try keyboard layout libraries next
			if ((start_pos=read_kcl_file("keyboard.sys",keyboard_file_name))==0) {
				if ((start_pos=read_kcl_file("keybrd2.sys",keyboard_file_name))==0) {
					if ((start_pos=read_kcl_file("keybrd3.sys",keyboard_file_name))==0) {
						LOG(LOG_BIOS,LOG_ERROR)("Keyboard layout file %s not found",keyboard_file_name);
						return KEYB_FILENOTFOUND;
					} else tempfile = OpenDosboxFile("keybrd3.sys");
				} else tempfile = OpenDosboxFile("keybrd2.sys");
			} else tempfile = OpenDosboxFile("keyboard.sys");

			fseek(tempfile, start_pos+2, SEEK_SET);
			read_buf_size=(Bit32u)fread(read_buf, sizeof(Bit8u), 65535, tempfile);
			fclose(tempfile);
			start_pos=0;
		}
	} else {
		// check ID-bytes of file
		Bit32u dr=(Bit32u)fread(read_buf, sizeof(Bit8u), 4, tempfile);
		if ((dr<4) || (read_buf[0]!=0x4b) || (read_buf[1]!=0x4c) || (read_buf[2]!=0x46)) {
			LOG(LOG_BIOS,LOG_ERROR)("Invalid keyboard layout file %s",keyboard_file_name);
			return KEYB_INVALIDFILE;
		}
		
		fseek(tempfile, 0, SEEK_SET);
		read_buf_size=(Bit32u)fread(read_buf, sizeof(Bit8u), 65535, tempfile);
		fclose(tempfile);
	}

	Bit8u data_len,submappings;
	data_len=read_buf[start_pos++];

	language_codes=new char*[data_len];
	language_code_count=0;
	// get all language codes for this layout
	for (Bitu i=0; i<data_len;) {
		language_codes[language_code_count]=new char[256];
		i+=2;
		Bitu lcpos=0;
		for (;i<data_len;) {
			char lcode=char(read_buf[start_pos+i]);
			i++;
			if (lcode==',') break;
			language_codes[language_code_count][lcpos++]=lcode;
		}
		language_codes[language_code_count][lcpos]=0;
		language_code_count++;
	}

	start_pos+=data_len;		// start_pos==absolute position of KeybCB block

	submappings=read_buf[start_pos];
	additional_planes=read_buf[start_pos+1];

	// four pages always occupied by normal,shift,flags,commandbits
	if (additional_planes>(layout_pages-4)) additional_planes=(layout_pages-4);

	// seek to plane descriptor
	read_buf_pos=start_pos+0x14+submappings*8;
	for (Bit16u cplane=0; cplane<additional_planes; cplane++) {
		Bit16u plane_flags;

		// get required-flags (shift/alt/ctrl-states etc.)
		plane_flags=host_readw(&read_buf[read_buf_pos]);
		read_buf_pos+=2;
		current_layout_planes[cplane].required_flags=plane_flags;
		used_lock_modifiers |= (plane_flags&0x70);
		// get forbidden-flags
		plane_flags=host_readw(&read_buf[read_buf_pos]);
		read_buf_pos+=2;
		current_layout_planes[cplane].forbidden_flags=plane_flags;

		// get required-userflags
		plane_flags=host_readw(&read_buf[read_buf_pos]);
		read_buf_pos+=2;
		current_layout_planes[cplane].required_userflags=plane_flags;
		// get forbidden-userflags
		plane_flags=host_readw(&read_buf[read_buf_pos]);
		read_buf_pos+=2;
		current_layout_planes[cplane].forbidden_userflags=plane_flags;
	}

	bool found_matching_layout=false;
	
	// check all submappings and use them if general submapping or same codepage submapping
	for (Bit16u sub_map=0; (sub_map<submappings) && (!found_matching_layout); sub_map++) {
		Bit16u submap_cp, table_offset;

		if ((sub_map!=0) && (specific_layout!=-1)) sub_map=(Bit16u)(specific_layout&0xffff);

		// read codepage of submapping
		submap_cp=host_readw(&read_buf[start_pos+0x14+sub_map*8]);
		if ((submap_cp!=0) && (submap_cp!=requested_codepage) && (specific_layout==-1))
			continue;		// skip nonfitting submappings

		if (submap_cp==requested_codepage) found_matching_layout=true;

		// get table offset
		table_offset=host_readw(&read_buf[start_pos+0x18+sub_map*8]);
		diacritics_entries=0;
		if (table_offset!=0) {
			// process table
			Bit16u i,j;
			for (i=0; i<2048;) {
				if (read_buf[start_pos+table_offset+i]==0) break;	// end of table
				diacritics_entries++;
				i+=read_buf[start_pos+table_offset+i+1]*2+2;
			}
			// copy diacritics table
			for (j=0; j<=i; j++) diacritics[j]=read_buf[start_pos+table_offset+j];
		}


		// get table offset
		table_offset=host_readw(&read_buf[start_pos+0x16+sub_map*8]);
		if (table_offset==0) continue;	// non-present table

		read_buf_pos=start_pos+table_offset;

		bytes_read=read_buf_size-read_buf_pos;

		// process submapping table
		for (Bit32u i=0; i<bytes_read;) {
			Bit8u scan=read_buf[read_buf_pos++];
			if (scan==0) break;
			Bit8u scan_length=(read_buf[read_buf_pos]&7)+1;		// length of data struct
			read_buf_pos+=2;
			i+=3;
			if (((scan&0x7f)<=MAX_SCAN_CODE) && (scan_length>0)) {
				// add all available mappings
				for (Bit16u addmap=0; addmap<scan_length; addmap++) {
					if (addmap>additional_planes+2) break;
					Bitu charptr=read_buf_pos+addmap*((read_buf[read_buf_pos-2]&0x80)?2:1);
					Bit16u kchar=read_buf[charptr];
					if (read_buf[read_buf_pos-2]&0x80) kchar|=read_buf[charptr+1]<<8;	// scancode/char pair

					if (kchar!=0) {		// key remapped
						// overwrite mapping
						current_layout[scan*layout_pages+addmap]=kchar;
						// clear command bit
						current_layout[scan*layout_pages+layout_pages-2]&=~(1<<addmap);
						// add command bit
						current_layout[scan*layout_pages+layout_pages-2]|=(read_buf[read_buf_pos-1] & (1<<addmap));
					}
				}

				current_layout[scan*layout_pages+layout_pages-1]=read_buf[read_buf_pos-2];	// flags
				if (read_buf[read_buf_pos-2]&0x80) scan_length*=2;		// granularity flag (S)
			}
			i+=scan_length;		// advance pointer
			read_buf_pos+=scan_length;
		}
		if (specific_layout==sub_map) break;
	}

	if (found_matching_layout) {
		if (specific_layout==-1) LOG(LOG_BIOS,LOG_NORMAL)("Keyboard layout %s successfully loaded",keyboard_file_name);
		else LOG(LOG_BIOS,LOG_NORMAL)("Keyboard layout %s (%i) successfully loaded",keyboard_file_name,specific_layout);
		this->use_foreign_layout=true;
		return KEYB_NOERROR;
	}

	LOG(LOG_BIOS,LOG_ERROR)("No matching keyboard layout found in %s",keyboard_file_name);

	// reset layout data (might have been changed by general layout)
	this->reset();

	return KEYB_LAYOUTNOTFOUND;
}

bool keyboard_layout::layout_key(Bitu key, Bit8u flags1, Bit8u flags2, Bit8u flags3) {
	if (key>MAX_SCAN_CODE) return false;
	if (!this->use_foreign_layout) return false;

	bool is_special_pair=(current_layout[key*layout_pages+layout_pages-1] & 0x80)==0x80;

	if ((((flags1&used_lock_modifiers)&0x7c)==0) && ((flags3&2)==0)) {
		// check if shift/caps is active:
		// (left_shift OR right_shift) XOR (key_affected_by_caps AND caps_locked)
		if ((((flags1&2)>>1) | (flags1&1)) ^ (((current_layout[key*layout_pages+layout_pages-1] & 0x40) & (flags1 & 0x40))>>6)) {
			// shift plane
			if (current_layout[key*layout_pages+1]!=0) {
				// check if command-bit is set for shift plane
				bool is_command=(current_layout[key*layout_pages+layout_pages-2]&2)!=0;
				if (this->map_key(key, current_layout[key*layout_pages+1],
					is_command, is_special_pair)) return true;
			}
		} else {
			// normal plane
			if (current_layout[key*layout_pages]!=0) {
				// check if command-bit is set for normal plane
				bool is_command=(current_layout[key*layout_pages+layout_pages-2]&1)!=0;
				if (this->map_key(key, current_layout[key*layout_pages],
					is_command, is_special_pair)) return true;
			}
		}
	}

	// calculate current flags
	Bit16u current_flags=(flags1&0x7f) | (((flags2&3) | (flags3&0xc))<<8);
	if (flags1&3) current_flags|=0x4000;	// either shift key active
	if (flags3&2) current_flags|=0x1000;	// e0 prefixed

	// check all planes if flags fit
	for (Bit16u cplane=0; cplane<additional_planes; cplane++) {
		Bit16u req_flags=current_layout_planes[cplane].required_flags;
		Bit16u req_userflags=current_layout_planes[cplane].required_userflags;
		// test flags
		if (((current_flags & req_flags)==req_flags) &&
			((user_keys & req_userflags)==req_userflags) &&
			((current_flags & current_layout_planes[cplane].forbidden_flags)==0) &&
			((user_keys & current_layout_planes[cplane].forbidden_userflags)==0)) {
				// remap key
				if (current_layout[key*layout_pages+2+cplane]!=0) {
					// check if command-bit is set for this plane
					bool is_command=((current_layout[key*layout_pages+layout_pages-2]>>(cplane+2))&1)!=0;
					if (this->map_key(key, current_layout[key*layout_pages+2+cplane],
						is_command, is_special_pair)) return true;
				} else break;	// abort plane checking
			}
	}

	if (diacritics_character>0) {
		// ignore state-changing keys
		switch(key) {
			case 0x1d:			/* Ctrl Pressed */
			case 0x2a:			/* Left Shift Pressed */
			case 0x36:			/* Right Shift Pressed */
			case 0x38:			/* Alt Pressed */
			case 0x3a:			/* Caps Lock */
			case 0x45:			/* Num Lock */
			case 0x46:			/* Scroll Lock */
				break;
			default:
				if (diacritics_character-200>=diacritics_entries) {
					diacritics_character=0;
					return true;
				}
				Bit16u diacritics_start=0;
				// search start of subtable
				for (Bit16u i=0; i<diacritics_character-200; i++)
					diacritics_start+=diacritics[diacritics_start+1]*2+2;

				BIOS_AddKeyToBuffer((Bit16u)(key<<8) | diacritics[diacritics_start]);
				diacritics_character=0;
		}
	}

	return false;
}

bool keyboard_layout::map_key(Bitu key, Bit16u layouted_key, bool is_command, bool is_keypair) {
	if (is_command) {
		Bit8u key_command=(Bit8u)(layouted_key&0xff);
		// check if diacritics-command
		if ((key_command>=200) && (key_command<235)) {
			// diacritics command
			diacritics_character=key_command;
			if (diacritics_character-200>=diacritics_entries) diacritics_character=0;
			return true;
		} else if ((key_command>=120) && (key_command<129)) {
			// switch layout command
			this->read_keyboard_file(key_command-119);
			return true;
		} else if ((key_command>=180) && (key_command<188)) {
			// switch user key off
			user_keys&=~(1<<(key_command-180));
			return true;
		} else if ((key_command>=188) && (key_command<196)) {
			// switch user key on
			user_keys|=(1<<(key_command-188));
			return true;
		} else if (key_command==160) return true;	// nop command
	} else {
		// non-command
		if (diacritics_character>0) {
			if (diacritics_character-200>=diacritics_entries) diacritics_character = 0;
			else {
				Bit16u diacritics_start=0;
				// search start of subtable
				for (Bit16u i=0; i<diacritics_character-200; i++)
					diacritics_start+=diacritics[diacritics_start+1]*2+2;

				Bit8u diacritics_length=diacritics[diacritics_start+1];
				diacritics_start+=2;
				diacritics_character=0;	// reset

				// search scancode
				for (Bit16u i=0; i<diacritics_length; i++) {
					if (diacritics[diacritics_start+i*2]==(layouted_key&0xff)) {
						// add diacritics to keybuf
						BIOS_AddKeyToBuffer((Bit16u)(key<<8) | diacritics[diacritics_start+i*2+1]);
						return true;
					}
				}
				// add standard-diacritics to keybuf
				BIOS_AddKeyToBuffer((Bit16u)(key<<8) | diacritics[diacritics_start-2]);
			}
		}

		// add remapped key to keybuf
		if (is_keypair) BIOS_AddKeyToBuffer(layouted_key);
		else BIOS_AddKeyToBuffer((Bit16u)(key<<8) | (layouted_key&0xff));

		return true;
	}
	return false;
}

Bit16u keyboard_layout::extract_codepage(const char* keyboard_file_name) {
	if (!strcmp(keyboard_file_name,"none")) return 437;

	Bit32u read_buf_size;
	static Bit8u read_buf[65535];
	Bit32u start_pos=5;

	char nbuf[512];
	sprintf(nbuf, "%s.kl", keyboard_file_name);
	FILE* tempfile = OpenDosboxFile(nbuf);
	if (tempfile==NULL) {
		// see if build-in keyboard layout is available, then copy it
		if (!strncasecmp(keyboard_file_name,"BG",2)) {
			for (Bitu i=0; i<687; i++) read_buf[i]=layout_BG[i];
		} else if (!strncasecmp(keyboard_file_name,"CZ243",5)) {
			for (Bitu i=0; i<1003; i++) read_buf[i]=layout_CZ243[i];
		} else if (!strncasecmp(keyboard_file_name,"FR",2)) {
			for (Bitu i=0; i<581; i++) read_buf[i]=layout_FR[i];
		} else if (!strncasecmp(keyboard_file_name,"GK",2)) {
			for (Bitu i=0; i<1117; i++) read_buf[i]=layout_GK[i];
		} else if (!strncasecmp(keyboard_file_name,"GR",2)) {
			for (Bitu i=0; i<596; i++) read_buf[i]=layout_GR[i];
		} else if (!strncasecmp(keyboard_file_name,"HR",2)) {
			for (Bitu i=0; i<993; i++) read_buf[i]=layout_HR[i];
		} else if (!strncasecmp(keyboard_file_name,"HU",2)) {
			for (Bitu i=0; i<964; i++) read_buf[i]=layout_HU[i];
		} else if (!strncasecmp(keyboard_file_name,"IT",2)) {
			for (Bitu i=0; i<236; i++) read_buf[i]=layout_IT[i];
		} else if (!strncasecmp(keyboard_file_name,"NL",2)) {
			for (Bitu i=0; i<552; i++) read_buf[i]=layout_NL[i];
		} else if (!strncasecmp(keyboard_file_name,"NO",2)) {
			for (Bitu i=0; i<477; i++) read_buf[i]=layout_NO[i];
		} else if (!strncasecmp(keyboard_file_name,"PL",2)) {
			for (Bitu i=0; i<261; i++) read_buf[i]=layout_PL[i];
		} else if (!strncasecmp(keyboard_file_name,"RU",2)) {
			for (Bitu i=0; i<1130; i++) read_buf[i]=layout_RU[i];
		} else if (!strncasecmp(keyboard_file_name,"SK",2)) {
			for (Bitu i=0; i<993; i++) read_buf[i]=layout_SK[i];
		} else if (!strncasecmp(keyboard_file_name,"SP",2)) {
			for (Bitu i=0; i<471; i++) read_buf[i]=layout_SP[i];
		} else if (!strncasecmp(keyboard_file_name,"SU",2)) {
			for (Bitu i=0; i<683; i++) read_buf[i]=layout_SU[i];
		} else if (!strncasecmp(keyboard_file_name,"SV",2)) {
			for (Bitu i=0; i<683; i++) read_buf[i]=layout_SV[i];
		} else {
			if ((start_pos=read_kcl_file("keyboard.sys",keyboard_file_name))==0) {
				if ((start_pos=read_kcl_file("keybrd2.sys",keyboard_file_name))==0) {
					if ((start_pos=read_kcl_file("keybrd3.sys",keyboard_file_name))==0) {
						LOG(LOG_BIOS,LOG_ERROR)("Keyboard layout file %s not found",keyboard_file_name);
						return 437;
					} else tempfile = OpenDosboxFile("keybrd3.sys");
				} else tempfile = OpenDosboxFile("keybrd2.sys");
			} else tempfile = OpenDosboxFile("keyboard.sys");

			fseek(tempfile, start_pos+2, SEEK_SET);
			read_buf_size=(Bit32u)fread(read_buf, sizeof(Bit8u), 65535, tempfile);
			fclose(tempfile);
			start_pos=0;
		}
	} else {
		// check ID-bytes of file
		Bit32u dr=(Bit32u)fread(read_buf, sizeof(Bit8u), 4, tempfile);
		if ((dr<4) || (read_buf[0]!=0x4b) || (read_buf[1]!=0x4c) || (read_buf[2]!=0x46)) {
			LOG(LOG_BIOS,LOG_ERROR)("Invalid keyboard layout file %s",keyboard_file_name);
			return 437;
		}

		fseek(tempfile, 0, SEEK_SET);
		read_buf_size=(Bit32u)fread(read_buf, sizeof(Bit8u), 65535, tempfile);
		fclose(tempfile);
	}

	Bit8u data_len,submappings;
	data_len=read_buf[start_pos++];

	start_pos+=data_len;		// start_pos==absolute position of KeybCB block

	submappings=read_buf[start_pos];

	// check all submappings and use them if general submapping or same codepage submapping
	for (Bit16u sub_map=0; (sub_map<submappings); sub_map++) {
		Bit16u submap_cp;

		// read codepage of submapping
		submap_cp=host_readw(&read_buf[start_pos+0x14+sub_map*8]);

		if (submap_cp!=0) return submap_cp;
	}
	return 437;
}

Bitu keyboard_layout::read_codepage_file(const char* codepage_file_name, Bit32s codepage_id) {
	char cp_filename[512];
	strcpy(cp_filename, codepage_file_name);
	if (!strcmp(cp_filename,"none")) return KEYB_NOERROR;

	if (codepage_id==dos.loaded_codepage) return KEYB_NOERROR;

	if (!strcmp(cp_filename,"auto")) {
		// select matching .cpi-file for specified codepage
		switch (codepage_id) {
			case 437:	case 850:	case 852:	case 853:	case 857:	case 858:	
						sprintf(cp_filename, "EGA.CPI"); break;
			case 775:	case 859:	case 1116:	case 1117:
						sprintf(cp_filename, "EGA2.CPI"); break;
			case 771:	case 772:	case 808:	case 855:	case 866:	case 872:
						sprintf(cp_filename, "EGA3.CPI"); break;
			case 848:	case 849:	case 1125:	case 1131:	case 61282:
						sprintf(cp_filename, "EGA4.CPI"); break;
			case 737:	case 851:	case 869:
						sprintf(cp_filename, "EGA5.CPI"); break;
			case 113:	case 899:	case 59829:	case 60853:
						sprintf(cp_filename, "EGA6.CPI"); break;
			case 58152:	case 58210:	case 59234:	case 60258:	case 62306:
						sprintf(cp_filename, "EGA7.CPI"); break;
			case 770:	case 773:	case 774:	case 777:	case 778:
						sprintf(cp_filename, "EGA8.CPI"); break;
			case 860:	case 861:	case 863:	case 865:
						sprintf(cp_filename, "EGA9.CPI"); break;
			case 667:	case 668:	case 790:	case 867:	case 991:	case 57781:
						sprintf(cp_filename, "EGA10.CPI"); break;
			default:
				LOG_MSG("No matching cpi file for codepage %i",codepage_id);
				return KEYB_INVALIDCPFILE;
		}
	}

	Bit32u start_pos;
	Bit16u number_of_codepages;

	char nbuf[512];
	sprintf(nbuf, "%s", cp_filename);
	FILE* tempfile=OpenDosboxFile(nbuf);
	if (tempfile==NULL) {
		size_t strsz=strlen(nbuf);
		if (strsz) {
			char plc=(char)toupper(*reinterpret_cast<unsigned char*>(&nbuf[strsz-1]));
			if (plc=='I') {
				// try CPX-extension as well
				nbuf[strsz-1]='X';
				tempfile=OpenDosboxFile(nbuf);
			} else if (plc=='X') {
				// try CPI-extension as well
				nbuf[strsz-1]='I';
				tempfile=OpenDosboxFile(nbuf);
			}
		}
	}

	static Bit8u cpi_buf[65536];
	Bit32u cpi_buf_size=0,size_of_cpxdata=0;;
	bool upxfound=false;
	Bit16u found_at_pos=5;
	if (tempfile==NULL) {
		// check if build-in codepage is available
		switch (codepage_id) {
			case 437:	case 850:	case 852:	case 853:	case 857:	case 858:	
						for (Bitu bct=0; bct<6322; bct++) cpi_buf[bct]=font_ega_cpx[bct];
						cpi_buf_size=6322;
						break;
			case 771:	case 772:	case 808:	case 855:	case 866:	case 872:
						for (Bitu bct=0; bct<5455; bct++) cpi_buf[bct]=font_ega3_cpx[bct];
						cpi_buf_size=5455;
						break;
			case 737:	case 851:	case 869:
						for (Bitu bct=0; bct<5720; bct++) cpi_buf[bct]=font_ega5_cpx[bct];
						cpi_buf_size=5720;
						break;
			default: 
				return KEYB_INVALIDCPFILE;
				break;
		}
		upxfound=true;
		found_at_pos=0x29;
		size_of_cpxdata=cpi_buf_size;
	} else {
		Bit32u dr=(Bit32u)fread(cpi_buf, sizeof(Bit8u), 5, tempfile);
		// check if file is valid
		if (dr<5) {
			LOG(LOG_BIOS,LOG_ERROR)("Codepage file %s invalid",cp_filename);
			return KEYB_INVALIDCPFILE;
		}
		// check if non-compressed cpi file
		if ((cpi_buf[0]!=0xff) || (cpi_buf[1]!=0x46) || (cpi_buf[2]!=0x4f) || 
			(cpi_buf[3]!=0x4e) || (cpi_buf[4]!=0x54)) {
			// check if dr-dos custom cpi file
			if ((cpi_buf[0]==0x7f) && (cpi_buf[1]!=0x44) && (cpi_buf[2]!=0x52) && 
				(cpi_buf[3]!=0x46) && (cpi_buf[4]!=0x5f)) {
				LOG(LOG_BIOS,LOG_ERROR)("Codepage file %s has unsupported DR-DOS format",cp_filename);
				return KEYB_INVALIDCPFILE;
			}
			// check if compressed cpi file
			Bit8u next_byte=0;
			for (Bitu i=0; i<100; i++) {
				fread(&next_byte, sizeof(Bit8u), 1, tempfile);	found_at_pos++;
				while (next_byte==0x55) {
					fread(&next_byte, sizeof(Bit8u), 1, tempfile);	found_at_pos++;
					if (next_byte==0x50) {
						fread(&next_byte, sizeof(Bit8u), 1, tempfile);	found_at_pos++;
						if (next_byte==0x58) {
							fread(&next_byte, sizeof(Bit8u), 1, tempfile);	found_at_pos++;
							if (next_byte==0x21) {
								// read version ID
								fread(&next_byte, sizeof(Bit8u), 1, tempfile);
								found_at_pos++;
								upxfound=true;
								break;
							}
						}
					}
				}
				if (upxfound) break;
			}
			if (!upxfound) {
				LOG(LOG_BIOS,LOG_ERROR)("Codepage file %s invalid: %x",cp_filename,cpi_buf[0]);
				return KEYB_INVALIDCPFILE;
			} else {
				if (next_byte<10) E_Exit("UPX-compressed cpi file, but upx-version too old");

				// read in compressed CPX-file
				fseek(tempfile, 0, SEEK_SET);
				size_of_cpxdata=(Bitu)fread(cpi_buf, sizeof(Bit8u), 65536, tempfile);
			}
		} else {
			// standard uncompressed cpi-file
			fseek(tempfile, 0, SEEK_SET);
			cpi_buf_size=(Bit32u)fread(cpi_buf, sizeof(Bit8u), 65536, tempfile);
		}
	}

	if (upxfound) {
		if (size_of_cpxdata>0xfe00) E_Exit("Size of cpx-compressed data too big");

		found_at_pos+=19;
		// prepare for direct decompression
		cpi_buf[found_at_pos]=0xcb;

		Bit16u seg=0;
		Bit16u size=0x1500;
		if (!DOS_AllocateMemory(&seg,&size)) E_Exit("Not enough free low memory to unpack data");
		MEM_BlockWrite((seg<<4)+0x100,cpi_buf,size_of_cpxdata);

		// setup segments
		Bit16u save_ds=SegValue(ds);
		Bit16u save_es=SegValue(es);
		Bit16u save_ss=SegValue(ss);
		Bit32u save_esp=reg_esp;
		SegSet16(ds,seg);
		SegSet16(es,seg);
		SegSet16(ss,seg+0x1000);
		reg_esp=0xfffe;

		// let UPX unpack the file
		CALLBACK_RunRealFar(seg,0x100);

		SegSet16(ds,save_ds);
		SegSet16(es,save_es);
		SegSet16(ss,save_ss);
		reg_esp=save_esp;

		// get unpacked content
		MEM_BlockRead((seg<<4)+0x100,cpi_buf,65536);
		cpi_buf_size=65536;

		DOS_FreeMemory(seg);
	}


	start_pos=host_readd(&cpi_buf[0x13]);
	number_of_codepages=host_readw(&cpi_buf[start_pos]);
	start_pos+=4;

	// search if codepage is provided by file
	for (Bit16u test_codepage=0; test_codepage<number_of_codepages; test_codepage++) {
		Bit16u device_type, font_codepage, font_type;

		// device type can be display/printer (only the first is supported)
		device_type=host_readw(&cpi_buf[start_pos+0x04]);
		font_codepage=host_readw(&cpi_buf[start_pos+0x0e]);

		Bit32u font_data_header_pt;
		font_data_header_pt=host_readd(&cpi_buf[start_pos+0x16]);

		font_type=host_readw(&cpi_buf[font_data_header_pt]);

		if ((device_type==0x0001) && (font_type==0x0001) && (font_codepage==codepage_id)) {
			// valid/matching codepage found

			Bit16u number_of_fonts,font_data_length;
			number_of_fonts=host_readw(&cpi_buf[font_data_header_pt+0x02]);
			font_data_length=host_readw(&cpi_buf[font_data_header_pt+0x04]);

			bool font_changed=false;
			Bit32u font_data_start=font_data_header_pt+0x06;

			// load all fonts if possible
			for (Bit16u current_font=0; current_font<number_of_fonts; current_font++) {
				Bit8u font_height=cpi_buf[font_data_start];
				font_data_start+=6;
				if (font_height==0x10) {
					// 16x8 font
					PhysPt font16pt=Real2Phys(int10.rom.font_16);
					for (Bitu i=0;i<256*16;i++) {
						phys_writeb(font16pt+i,cpi_buf[font_data_start+i]);
					}
					font_changed=true;
				} else if (font_height==0x0e) {
					// 14x8 font
					PhysPt font14pt=Real2Phys(int10.rom.font_14);
					for (Bitu i=0;i<256*14;i++) {
						phys_writeb(font14pt+i,cpi_buf[font_data_start+i]);
					}
					font_changed=true;
				} else if (font_height==0x08) {
					// 8x8 fonts
					PhysPt font8pt=Real2Phys(int10.rom.font_8_first);
					for (Bitu i=0;i<128*8;i++) {
						phys_writeb(font8pt+i,cpi_buf[font_data_start+i]);
					}
					font8pt=Real2Phys(int10.rom.font_8_second);
					for (Bitu i=0;i<128*8;i++) {
						phys_writeb(font8pt+i,cpi_buf[font_data_start+i+128*8]);
					}
					font_changed=true;
				}
				font_data_start+=font_height*256;
			}

			LOG(LOG_BIOS,LOG_NORMAL)("Codepage %i successfully loaded",codepage_id);

			// set codepage entries
			dos.loaded_codepage=(Bit16u)(codepage_id&0xffff);

			// update font if necessary
			if (font_changed && (CurMode->type==M_TEXT) && (machine==MCH_VGA)) {
				INT10_LoadFont(Real2Phys(int10.rom.font_16),true,256,0,0,16);
			}
			INT10_SetupRomMemoryChecksum();

			return KEYB_NOERROR;
		}

		start_pos=host_readd(&cpi_buf[start_pos]);
		start_pos+=2;
	}

	LOG(LOG_BIOS,LOG_ERROR)("Codepage %i not found",codepage_id);

	return KEYB_INVALIDCPFILE;
}

Bitu keyboard_layout::switch_keyboard_layout(const char* new_layout, keyboard_layout*& created_layout) {
	if (strncasecmp(new_layout,"US",2)) {
		// switch to a foreign layout
		char tbuf[256];
		strcpy(tbuf, new_layout);
		size_t newlen=strlen(tbuf);

		bool language_code_found=false;
		// check if language code is present in loaded foreign layout
		for (Bitu i=0; i<language_code_count; i++) {
			if (!strncasecmp(tbuf,language_codes[i],newlen)) {
				language_code_found=true;
				break;
			}
		}

		if (language_code_found) {
			if (!this->use_foreign_layout) {
				// switch to foreign layout
				this->use_foreign_layout=true;
				diacritics_character=0;
				LOG(LOG_BIOS,LOG_NORMAL)("Switched to layout %s",tbuf);
			}
		} else {
			keyboard_layout * temp_layout=new keyboard_layout();
			Bitu req_codepage=temp_layout->extract_codepage(new_layout);
			Bitu kerrcode=temp_layout->read_keyboard_file(new_layout, req_codepage);
			if (kerrcode) {
				delete temp_layout;
				return kerrcode;
			}
			// ...else keyboard layout loaded successfully, change codepage accordingly
			kerrcode=temp_layout->read_codepage_file("auto", req_codepage);
			if (kerrcode) {
				delete temp_layout;
				return kerrcode;
			}
			// Everything went fine, switch to new layout
			created_layout=temp_layout;
		}
	} else if (this->use_foreign_layout) {
		// switch to the US layout
		this->use_foreign_layout=false;
		diacritics_character=0;
		LOG(LOG_BIOS,LOG_NORMAL)("Switched to US layout");
	}
	return KEYB_NOERROR;
}

void keyboard_layout::switch_foreign_layout() {
	this->use_foreign_layout=!this->use_foreign_layout;
	diacritics_character=0;
	if (this->use_foreign_layout) LOG(LOG_BIOS,LOG_NORMAL)("Switched to foreign layout");
	else LOG(LOG_BIOS,LOG_NORMAL)("Switched to US layout");
}


static keyboard_layout* loaded_layout=NULL;

// CTRL-ALT-F2 switches between foreign and US-layout using this function
static void switch_keyboard_layout(bool pressed) {
	if (!pressed)
		return;
	if (loaded_layout) loaded_layout->switch_foreign_layout();
}

// called by int9-handler
bool DOS_LayoutKey(Bitu key, Bit8u flags1, Bit8u flags2, Bit8u flags3) {
	if (loaded_layout) return loaded_layout->layout_key(key, flags1, flags2, flags3);
	else return false;
}

Bitu DOS_LoadKeyboardLayout(const char * layoutname, Bit32s codepage, const char * codepagefile) {
	keyboard_layout * temp_layout=new keyboard_layout();
	// try to read the layout for the specified codepage
	Bitu kerrcode=temp_layout->read_keyboard_file(layoutname, codepage);
	if (kerrcode) {
		delete temp_layout;
		return kerrcode;
	}
	// ...else keyboard layout loaded successfully, change codepage accordingly
	kerrcode=temp_layout->read_codepage_file(codepagefile, codepage);
	if (kerrcode) {
		delete temp_layout;
		return kerrcode;
	}
	// Everything went fine, switch to new layout
	loaded_layout=temp_layout;
	return KEYB_NOERROR;
}

Bitu DOS_SwitchKeyboardLayout(const char* new_layout) {
	if (loaded_layout) {
		keyboard_layout* changed_layout=NULL;
		Bitu ret_code=loaded_layout->switch_keyboard_layout(new_layout, changed_layout);
		if (changed_layout) {
			// Remove old layout, activate new layout
			delete loaded_layout;
			loaded_layout=changed_layout;
		}
		return ret_code;
	} else return 0xff;
}


class DOS_KeyboardLayout: public Module_base {
public:
	DOS_KeyboardLayout(Section* configuration):Module_base(configuration){
		Section_prop * section=static_cast<Section_prop *>(configuration);
		dos.loaded_codepage=437;	// US codepage already initialized
		loaded_layout=new keyboard_layout();

		const char * layoutname=section->Get_string("keyboardlayout");
		// try to find a good codepage for the requested layout
		Bitu req_codepage=loaded_layout->extract_codepage(layoutname);

		loaded_layout->read_codepage_file("auto", req_codepage);
		if (loaded_layout->read_keyboard_file(layoutname, dos.loaded_codepage))
			LOG_MSG("Error loading keyboard layout %s",layoutname);
	}

	~DOS_KeyboardLayout(){
		if ((dos.loaded_codepage!=437) && (CurMode->type==M_TEXT)) {
			INT10_ReloadRomFonts();
			dos.loaded_codepage=437;	// US codepage
		}
		if (loaded_layout) {
			delete loaded_layout;
			loaded_layout=NULL;
		}
	}
};

static DOS_KeyboardLayout* test;

void DOS_KeyboardLayout_ShutDown(Section* /*sec*/) {
	delete test;	
}

void DOS_KeyboardLayout_Init(Section* sec) {
	test = new DOS_KeyboardLayout(sec);
	sec->AddDestroyFunction(&DOS_KeyboardLayout_ShutDown,true);
//	MAPPER_AddHandler(switch_keyboard_layout,MK_f2,MMOD1|MMOD2,"sw_layout","Switch Layout");
}
