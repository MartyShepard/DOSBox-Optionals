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

#if defined __MINGW32__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinNls.h>
#endif
#if defined __MINGW32__
	#include <windows.h>
#endif
#include "dosbox.h"
#include "cross.h"
#include "support.h"
#include "setup.h"
#include "control.h"
#include <list>
#include <string>
using namespace std;



#define LINE_IN_MAXLEN 2048

struct MessageBlock {
	string name;
	string val;
	MessageBlock(const char* _name, const char* _val):
	name(_name),val(_val){}
};

static list<MessageBlock> Lang;
typedef list<MessageBlock>::iterator itmb;

void MSG_Add(const char * _name, const char* _val) {
	/* Find the message */
	for(itmb tel=Lang.begin();tel!=Lang.end();tel++) {
		if((*tel).name==_name) { 
//			LOG_MSG("double entry for %s",_name); //Message file might be loaded before default text messages
			return;
		}
	}
	/* if the message doesn't exist add it */
	Lang.push_back(MessageBlock(_name,_val));
}

void MSG_Replace(const char * _name, const char* _val) {
	/* Find the message */
	for(itmb tel=Lang.begin();tel!=Lang.end();tel++) {
		if((*tel).name==_name) { 
			Lang.erase(tel);
			break;
		}
	}
	/* Even if the message doesn't exist add it */
	Lang.push_back(MessageBlock(_name,_val));
}

static void LoadMessageFile(const char * fname) {
	if (!fname) return;
	if(*fname=='\0') return;//empty string=no languagefile
	FILE * mfile=fopen(fname,"rt");
	/* This should never happen and since other modules depend on this use a normal printf */
	if (!mfile) {
		LOG_MSG("\nLanguage: Can't not open Language:\n"
		        "            %s",fname);
	}
	char linein[LINE_IN_MAXLEN];
	char name[LINE_IN_MAXLEN];
	char string[LINE_IN_MAXLEN*10];
	/* Start out with empty strings */
	name[0]=0;string[0]=0;
	while(fgets(linein, LINE_IN_MAXLEN, mfile)!=0) {
		/* Parse the read line */
		/* First remove characters 10 and 13 from the line */
		char * parser=linein;
		char * writer=linein;
		while (*parser) {
			if (*parser!=10 && *parser!=13) {
				*writer++=*parser;
			}
			parser++;
		}
		*writer=0;
		/* New string name */
		if (linein[0]==':') {
			string[0]=0;
			strcpy(name,linein+1);
		/* End of string marker */
		} else if (linein[0]=='.') {
			/* Replace/Add the string to the internal languagefile */
			/* Remove last newline (marker is \n.\n) */
			size_t ll = strlen(string);
			if(ll && string[ll - 1] == '\n') string[ll - 1] = 0; //Second if should not be needed, but better be safe.
			MSG_Replace(name,string);
		} else {
		/* Normal string to be added */
			strcat(string,linein);
			strcat(string,"\n");
		}
	}
	fclose(mfile);
}

const char * MSG_Get(char const * msg) {
	for(itmb tel=Lang.begin();tel!=Lang.end();tel++){	
		if((*tel).name==msg)
		{
			return  (*tel).val.c_str();
		}
	}
	return "Message not Found!\n";
}


bool MSG_Write(const char * location) {
	FILE* out=fopen(location,"w+t");
	if(out==NULL) return false;//maybe an error?
	for(itmb tel=Lang.begin();tel!=Lang.end();tel++){
		fprintf(out,":%s\n%s\n.\n",(*tel).name.c_str(),(*tel).val.c_str());
	}
	fclose(out);
	return true;
}

/* ------------------------------------------------------------------------------------------------------------- */
inline bool FileExists (const std::string& name) {
  struct stat buffer; 

  return (stat (name.c_str(), &buffer) == 0 ); 
}

/* ------------------------------------------------------------------------------------------------------------- */
static bool MSG_Load_Automatically(const std::string& name){
	
	bool bLoaded = FileExists( name.c_str() );
	if ( bLoaded == true )		
	{	
		LoadMessageFile(name.c_str());	
		LOG_MSG("Language: Loaded: %s ... \n\n",sGetFileName( name.c_str() ).c_str());				
		return true;
	}
	return false;	
	
}

/* ------------------------------------------------------------------------------------------------------------- */

#define NELEMS(a) (sizeof (a) / sizeof (a[0]))
static bool MSG_Load_LangMessage (LANGID langid)
{
  LCID lcid;
  char lng[10];
  char cty[10];
  char loc[26];
  char Lng[256];
  std::string LanguageFile;
  

  lcid = MAKELCID (langid, SORT_DEFAULT);
		
	if (GetLocaleInfo (lcid, LOCALE_SISO639LANGNAME, lng, NELEMS (lng)) && GetLocaleInfo (lcid, LOCALE_SISO3166CTRYNAME, cty, NELEMS (cty)))
	{			

		strcpy(Lng,"DosCodepage_");
		strcat(Lng,lng);
		strcat(Lng,"_");
		strcat(Lng,cty);			
		
    }
	
	else if (GetLocaleInfo (lcid, LOCALE_SISO639LANGNAME, loc, NELEMS (loc)))
    {		
		strcpy(Lng,"DosCodepage_");
		strcat(Lng,loc);	
	
    }	
	
	LanguageFile = sCurrentWorkingPath() + "\\data\\Language\\" + Lng + ".lng";
	if (!MSG_Load_Automatically(LanguageFile.c_str()))
	{
		
		LanguageFile = sCurrentWorkingPath() + "\\data\\Language\\" + Lng + ".lang";
		if (!MSG_Load_Automatically(LanguageFile.c_str()))
		{
			
			LanguageFile = sCurrentWorkingPath() + "\\data\\" + Lng + ".lng";			
			if (!MSG_Load_Automatically(LanguageFile.c_str()))
			{
				
				LanguageFile = sCurrentWorkingPath() + "\\data\\" + Lng + ".lang";	
				if (!MSG_Load_Automatically(LanguageFile.c_str()))
				{
					
					LanguageFile = sCurrentWorkingPath() + "\\Language\\" + Lng + ".lng";	
					if (!MSG_Load_Automatically(LanguageFile.c_str()))
					{						
					
						LanguageFile = sCurrentWorkingPath() + "\\Language\\" + Lng + ".lng";	
						if (!MSG_Load_Automatically(LanguageFile.c_str()))
						{						
							LanguageFile = sCurrentWorkingPath() + "\\" + Lng + ".lng";	
							if (!MSG_Load_Automatically(LanguageFile.c_str()))
							{	
			
								LanguageFile = sCurrentWorkingPath() + "\\" + Lng + ".lang";
								if (!MSG_Load_Automatically(LanguageFile.c_str()))
								{	
									LOG_MSG("Language: Nothing found, Searched -> \"%s(.lng/.lang)\"\n\n",Lng);
									return false;
								}
							}
						}
					}
				}								
			}		
		}
	}
	return true;
}
/* ------------------------------------------------------------------------------------------------------------- */

bool MSG_Load(void)
{
	/*
		Simple Automatically Load User Language File		
		...................Yep Windows User on baord
	*/
	
	/* 	Wird System wirklich gebraucht ?
		static LANGID nSysLangUIID = GetSystemDefaultUILanguage();
		
		if ( MSG_Load_LangMessage(nSysLangUIID) ){		
			return true;
		}		
	*/
	
	static LANGID nUsrLangUIID = GetUserDefaultUILanguage();
		
	LOG_MSG("\nLanguage: Automatically try to Load Language from System");
	
	if ( MSG_Load_LangMessage(nUsrLangUIID) ){
		return true;		
	}	
		
	LOG_MSG("Language: Nothing to do ... No files found\n\n");	
	return false;		
}
/* ------------------------------------------------------------------------------------------------------------- */

bool MSG_Load_Commandline(void)
{
	std::string file_name;
	
	if (control->cmdline->FindString("-lang",file_name,true))
	{	
		bool bLoaded = FileExists( file_name.c_str());
		if ( bLoaded == true )		
		{
			LoadMessageFile(file_name.c_str());
			LOG_MSG("\nLanguage: File loaded with argument [-lang]\n"
			        "          %s",file_name.c_str());	
			return true;
		}
		LOG_MSG("\nLanguage: Can't load with argument [-lang]\n"
		        "          %s",file_name.c_str());			
	}		
	return false;
}
/* ------------------------------------------------------------------------------------------------------------- */

bool MSG_Load_ConfigFile(Section_prop * section)
{

	Prop_path* pathprop = section->Get_path("language");
	if(pathprop)
	{	
		bool bLoaded = FileExists( pathprop->realpath.c_str());		
		if ( bLoaded == true )
		{
			LoadMessageFile(pathprop->realpath.c_str());
			LOG_MSG("\nLanguage: File loaded from Configfile\n"
			        "          %s",pathprop->realpath.c_str());	
			return true;			
		}
		LOG_MSG("\nLanguage: File from Configfile can not loaded\n"
		        "          %s",pathprop->realpath.c_str());			
	}		
	return false;			
}
/* ------------------------------------------------------------------------------------------------------------- */

void MSG_Init(Section_prop * section) {

	/*
		Scenario
		#1 Benutzer kann die Sprachdatei von der Kommandozeile aus laden.
		   Schlägt fehl oder wird es ohne Parameter angegeben
		#2 Es wird versucht die Sprachdatei aus der Konfiguration zu laden.
		   Schlägt dies auch fehl oder der Wert "Language =" bleibt leer
		#3 wird versucht eine Sprachdatei aus dem Dosbox Unter/verzeichnis
		   zu laden.		   	   
	*/

	bool bLoaded = MSG_Load_Commandline();
	
	if ( bLoaded == false )
	{	
		 bLoaded = MSG_Load_ConfigFile( section );
	}			
	
	if ( bLoaded == false )
	{	
		 bLoaded = MSG_Load();
	}
}
