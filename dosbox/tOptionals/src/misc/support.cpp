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


#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cctype>
#include <string>
  
#include "dosbox.h"
#include "debug.h"
#include "support.h"
#include "video.h"
#if defined(_MSC_VER)
	#include "SDL2/include/SDL_mixer.h"
	#include "SDL2/include/SDL_syswm.h"
	#include "SDL2/include/SDL.h"
#else
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL.h"
#endif

void upcase(std::string &str) {
	int (*tf)(int) = std::toupper;
	std::transform(str.begin(), str.end(), str.begin(), tf);
}

void lowcase(std::string &str) {
	int (*tf)(int) = std::tolower;
	std::transform(str.begin(), str.end(), str.begin(), tf);
}

void trim(std::string &str) {
	std::string::size_type loc = str.find_first_not_of(" \r\t\f\n");
	if (loc != std::string::npos) str.erase(0,loc);
	loc = str.find_last_not_of(" \r\t\f\n");
	if (loc != std::string::npos) str.erase(loc+1);
}
/* 
	Ripped some source from freedos for this one.

*/


/*
 * replaces all instances of character o with character c
 */


void strreplace(char * str,char o,char n) {
	while (*str) {
		if (*str==o) *str=n;
		str++;
	}
}
char *ltrim(char *str) { 
	while (*str && isspace(*reinterpret_cast<unsigned char*>(str))) str++;
	return str;
}

char *rtrim(char *str) {
	char *p;
	p = strchr(str, '\0');
	while (--p >= str && isspace(*reinterpret_cast<unsigned char*>(p))) {};
	p[1] = '\0';
	return str;
}

char *trim(char *str) {
	return ltrim(rtrim(str));
}

char * upcase(char * str) {
    for (char* idx = str; *idx ; idx++) *idx = toupper(*reinterpret_cast<unsigned char*>(idx));
    return str;
}

char * lowcase(char * str) {
	for(char* idx = str; *idx ; idx++)  *idx = tolower(*reinterpret_cast<unsigned char*>(idx));
	return str;
}



bool ScanCMDBool(char * cmd,char const * const check) {
	char * scan=cmd;size_t c_len=strlen(check);
	while ((scan=strchr(scan,'/'))) {
		/* found a / now see behind it */
		scan++;
		if (strncasecmp(scan,check,c_len)==0 && (scan[c_len]==' ' || scan[c_len]=='\t' || scan[c_len]=='/' || scan[c_len]==0)) {
		/* Found a math now remove it from the string */
			memmove(scan-1,scan+c_len,strlen(scan+c_len)+1);
			trim(scan-1);
			return true;
		}
	}
	return false;
}

/* This scans the command line for a remaining switch and reports it else returns 0*/
char * ScanCMDRemain(char * cmd) {
	char * scan,*found;;
	if ((scan=found=strchr(cmd,'/'))) {
		while ( *scan && !isspace(*reinterpret_cast<unsigned char*>(scan)) ) scan++;
		*scan=0;
		return found;
	} else return 0; 
}


char * StripWord(char *&line) {
	char * scan=line;
	scan=ltrim(scan);
	if (*scan=='"') {
		char * end_quote=strchr(scan+1,'"');
		if (end_quote) {
			*end_quote=0;
			line=ltrim(++end_quote);
			return (scan+1);
		}
	}
	char * begin=scan;
	for (char c = *scan ;(c = *scan);scan++) {
		if (isspace(*reinterpret_cast<unsigned char*>(&c))) {
			*scan++=0;
			break;
		}
	}
	line=scan;
	return begin;
}

char * StripArg(char *&line) {
       char * scan=line;
       int q=0;
       scan=ltrim(scan);
       char * begin=scan;
       for (char c = *scan ;(c = *scan);scan++) {
               if (*scan=='"') {
                       q++;
               } else if (q/2*2==q && isspace(*reinterpret_cast<unsigned char*>(&c))) {
			*scan++=0;
			break;
		}
	}
	line=scan;
	return begin;
}

Bits ConvDecWord(char * word) {
	bool negative=false;Bitu ret=0;
	if (*word=='-') {
		negative=true;
		word++;
	}
	while (char c=*word) {
		ret*=10;
		ret+=c-'0';
		word++;
	}
	if (negative) return 0-ret;
	else return ret;
}

Bits ConvHexWord(char * word) {
	Bitu ret=0;
	while (char c=toupper(*reinterpret_cast<unsigned char*>(word))) {
		ret*=16;
		if (c>='0' && c<='9') ret+=c-'0';
		else if (c>='A' && c<='F') ret+=10+(c-'A');
		word++;
	}
	return ret;
}

double ConvDblWord(char * word) {
	return 0.0f;
}

static char buf[1024];           //greater scope as else it doesn't always gets thrown right (linux/gcc2.95)
/* Added from DOSBox-X ///////////////////////////////////////////////////// Teilsweise */
void E_Exit(const char * format,...) {
#if defined(C_DEBUG) && defined(C_HEAVY_DEBUG)
 	DEBUG_HeavyWriteLogInstruction();
#endif
	va_list msg;
	va_start(msg,format);
	vsnprintf(buf,sizeof(buf),format,msg);
	va_end(msg);

	buf[sizeof(buf) - 1] = '\0';
	//strcat(buf,"\n"); catcher should handle the end of line.. 
	LOG_MSG("E_Exit: %s\n",buf);
#if defined(WIN32) && !defined(C_SDL2)
	/* Most Windows users DON'T run DOSBox-X from the command line! */
	MessageBox(GetHWND(), buf, "E_Exit", MB_OK | MB_ICONEXCLAMATION);
#endif
#if defined(C_DEBUG)
	//endwin();
#endif
	fprintf(stderr, "E_Exit: %s\n", buf);
	SDL_Quit();
	exit(0);
}
/* Added from DOSBox-X ///////////////////////////////////////////////////// Teilsweise */

char * sReplace(
    char const * const original, 
    char const * const pattern, 
    char const * const replacement
) {
  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string, 
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement 
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

