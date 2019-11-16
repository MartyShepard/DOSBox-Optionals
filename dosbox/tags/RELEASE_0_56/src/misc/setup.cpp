/*
 *  Copyright (C) 2002  The DOSBox Team
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

#include "dosbox.h"
#include "cross.h"
#include "setup.h"
#include <fstream>
#include <string>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include "support.h"

using namespace std;


void Prop_int::SetValue(char* input){
	input=trim(input);
	__value._int= atoi(input);
}

void Prop_string::SetValue(char* input){
	input=trim(input);
	__value._string->assign(input);
}
	
void Prop_bool::SetValue(char* input){
	input=trim(input);
	if((input[0]=='0') ||(input[0]=='D')||( (input[0]=='O') && (input[0]=='F'))||(input[0]=='f')){
		__value._bool=false;
	}else{
		__value._bool=true;
	}
}
void Prop_hex::SetValue(char* input){
	input=trim(input);
	if(!sscanf(input,"%X",&(__value._hex))) __value._hex=0;
}

void Prop_int::GetValuestring(char* str){
        sprintf(str,"%d",__value._int);
}

void Prop_string::GetValuestring(char* str){
        sprintf(str,"%s",__value._string->c_str());
}

void Prop_bool::GetValuestring(char* str){
        sprintf(str,"%s",__value._bool?"true":"false");
}

void Prop_hex::GetValuestring(char* str){
        sprintf(str,"%X",__value._hex);
}

void Section_prop::Add_int(const char* _propname, int _value) {
	Property* test=new Prop_int(_propname,_value);
	properties.push_back(test);
}

void Section_prop::Add_string(const char* _propname, char* _value) {
	Property* test=new Prop_string(_propname,_value);
	properties.push_back(test);
}

void Section_prop::Add_bool(const char* _propname, bool _value) {
	Property* test=new Prop_bool(_propname,_value);
	properties.push_back(test);
}
void Section_prop::Add_hex(const char* _propname, int _value) {
	Property* test=new Prop_hex(_propname,_value);
	properties.push_back(test);
}
int Section_prop::Get_int(const char* _propname){
	for(it tel=properties.begin();tel!=properties.end();tel++){
		if((*tel)->propname==_propname){
			return ((*tel)->GetValue())._int;
		}
	}
	return 0;
}

bool Section_prop::Get_bool(const char* _propname){
	for(it tel=properties.begin();tel!=properties.end();tel++){
		if((*tel)->propname==_propname){
			return ((*tel)->GetValue())._bool;
		}
	}
	return false;
}


const char* Section_prop::Get_string(const char* _propname){
	for(it tel=properties.begin();tel!=properties.end();tel++){
		if((*tel)->propname==_propname){
			return ((*tel)->GetValue())._string->c_str();
		}
	}
	return NULL;
}
int Section_prop::Get_hex(const char* _propname){
	for(it tel=properties.begin();tel!=properties.end();tel++){
		if((*tel)->propname==_propname){
			return ((*tel)->GetValue())._hex;
		}
	}
	return 0;
}

void Section_prop::HandleInputline(char *gegevens){
	char * rest=strrchr(gegevens,'=');
	*rest=0;
	gegevens=trim(gegevens);
	for(it tel=properties.begin();tel!=properties.end();tel++){
		if(!strcasecmp((*tel)->propname.c_str(),gegevens)){
			(*tel)->SetValue(rest+1);
			return;
		}
	}
}


void Section_prop::PrintData(FILE* outfile){
	char temp[1000];		/* Should be enough for the properties */
	/* Now print out the individual section entries */
	for(it tel=properties.begin();tel!=properties.end();tel++){
		(*tel)->GetValuestring(temp);
		fprintf(outfile,"%s=%s\n",(*tel)->propname.c_str(),temp);
	}
}

   
void Section_line::HandleInputline(char* line){ 
	data+=line;
	data+="\n";
}

void Section_line::PrintData(FILE* outfile) {
	fprintf(outfile,"%s",data.c_str());
}

void Config::PrintConfig(const char* configfilename){
	char temp[50];
	FILE* outfile=fopen(configfilename,"w+b");
	if(outfile==NULL) return;
	for (it tel=sectionlist.begin(); tel!=sectionlist.end(); tel++){
		/* Print out the Section header */
		strcpy(temp,(*tel)->sectionname.c_str());
		lowcase(temp);
		fprintf(outfile,"[%s]\n",temp);
		upcase(temp);
		strcat(temp,"_CONFIGFILE_HELP");
		fprintf(outfile,"# %s",MSG_Get(temp));
		(*tel)->PrintData(outfile);
		fprintf(outfile,"\n");	/* Always an empty line between sections */
	}
	fclose(outfile);
}
   
Section* Config::AddSection(const char* _name,void (*_initfunction)(Section*)){
	Section* blah = new Section(_name);
	blah->AddInitFunction(_initfunction);
	sectionlist.push_back(blah);
	return blah;
}

Section_prop* Config::AddSection_prop(const char* _name,void (*_initfunction)(Section*)){
	Section_prop* blah = new Section_prop(_name);
	blah->AddInitFunction(_initfunction);
	sectionlist.push_back(blah);
	return blah;
}

Section_line* Config::AddSection_line(const char* _name,void (*_initfunction)(Section*)){
	Section_line* blah = new Section_line(_name);
	blah->AddInitFunction(_initfunction);
	sectionlist.push_back(blah);
	return blah;
}


void Config::Init(){
	for (it tel=sectionlist.begin(); tel!=sectionlist.end(); tel++){ 
		(*tel)->ExecuteInit();
	}
}

Config::~Config() {
	reverse_it cnt=sectionlist.rbegin();
	while (cnt!=sectionlist.rend()) {
		delete (*cnt);
		cnt++;
	}
}

Section* Config::GetSection(const char* _sectionname){
	for (it tel=sectionlist.begin(); tel!=sectionlist.end(); tel++){
		if (!strcasecmp((*tel)->sectionname.c_str(),_sectionname)) return (*tel);
	}
	return NULL;
}

void Config::ParseConfigFile(const char* configfilename){
	ifstream in(configfilename);
	if (!in) {
		LOG_MSG("CONFIG:Can't find config file %s, using default settings",configfilename);
		return;
	}
	char gegevens[150];
	Section* currentsection;
	while (in) {
		in.getline(gegevens,150);
		char* temp;
		switch(gegevens[0]){
		case '%':
		case '\0':
		case '\n':
        case '#':
		case ' ':
			continue;
			break;
		case '[':
			temp = strrchr(gegevens,']');
			*temp=0;
			currentsection=GetSection(&gegevens[1]);
			break;
		default:
			try{
				currentsection->HandleInputline(gegevens);
			}catch(const char* message){
				message=0;
				//EXIT with message
			}
			break;
		}
	}
}

void Config::SetStartUp(void (*_function)(void)) { 
	_start_function=_function;
}


void Config::StartUp(void) {
	(*_start_function)();
}

bool CommandLine::FindExist(char * name,bool remove) {
	cmd_it it;
	if (!(FindEntry(name,it,false))) return false;
	if (remove) cmds.erase(it);
	return true;
}

bool CommandLine::FindHex(char * name,int & value,bool remove) {
	cmd_it it,it_next;
	if (!(FindEntry(name,it,true))) return false;
	it_next=it;it_next++;
	sscanf((*it_next).c_str(),"%X",&value);
	if (remove) cmds.erase(it,++it_next);
	return true;
}

bool CommandLine::FindInt(char * name,int & value,bool remove) {
	cmd_it it,it_next;
	if (!(FindEntry(name,it,true))) return false;
	it_next=it;it_next++;
	value=atoi((*it_next).c_str());
	if (remove) cmds.erase(it,++it_next);
	return true;
}

bool CommandLine::FindString(char * name,std::string & value,bool remove) {
	cmd_it it,it_next;
	if (!(FindEntry(name,it,true))) return false;
	it_next=it;it_next++;
	value=*it_next;
	if (remove) cmds.erase(it,++it_next);
	return true;
}

bool CommandLine::FindCommand(int which,std::string & value) {
	if (which<1) return false;
	if (which>cmds.size()) return false;
	cmd_it it=cmds.begin();
	for (;which>1;which--) it++;
	value=(*it);
	return true;
}

bool CommandLine::FindEntry(char * name,cmd_it & it,bool neednext) {
	for (it=cmds.begin();it!=cmds.end();it++) {
		if (!strcasecmp((*it).c_str(),name)) {
			cmd_it itnext=it;itnext++;
			if (neednext && (itnext==cmds.end())) return false;
			return true;
		}
	}
	return false;
}

bool CommandLine::FindStringBegin(char * begin,std::string & value, bool remove) {
	cmd_it it;
	for (it=cmds.begin();it!=cmds.end();it++) {
		if (strncmp(begin,(*it).c_str(),strlen(begin))==0) {
			value=((*it).c_str()+strlen(begin));
			return true;
		}
	}
	return false;
}

int CommandLine::GetCount(void) {
	return cmds.size();
}

CommandLine::CommandLine(int argc,char * argv[]) {
	if (argc>0) {
		file_name=argv[0];
	}
	int i=1;
	while (i<argc) {
		cmds.push_back(argv[i]);
		i++;
	}
}


CommandLine::CommandLine(char * name,char * cmdline) {
	if (name) file_name=name;
	/* Parse the cmds and put them in the list */
	bool inword,inquote;char c;
	inword=false;inquote=false;
	std::string str;
	while ((c=*cmdline)!=0) {
		if (inquote) {
			if (c!='"') str+=c;
			else {
				inquote=false;
				cmds.push_back(str);
				str.erase();
			}
		}else if (inword) {
			if (c!=' ') str+=c;
			else {
				inword=false;
				cmds.push_back(str);
				str.erase();
			}
		} 
		else if (c=='"') { inquote=true;}
		else if (c!=' ') { str+=c;inword=true;}
		cmdline++;
	}
	if (inword || inquote) cmds.push_back(str);
}
