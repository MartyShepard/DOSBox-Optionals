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

#include "dosbox.h"
#if defined(C_DEBUG)
#ifdef WIN32

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

HWND GetConsoleHwnd(void)
   {
	 /*Danke MS
	 * Das Console Window wurde immer Wahlweise woanders dargestellt.
	 * Benötigt Fixed Grösse und Weite für den ersten Start
	 */
       #define MY_BUFSIZE 1024 // Buffer size for console window titles.
       HWND hwndFound;         // This is what is returned to the caller.
       char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
                                           // WindowTitle.
       char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
                                           // WindowTitle.

       // Fetch current window title.

       GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

       // Format a "unique" NewWindowTitle.

       wsprintf(pszNewWindowTitle,"%d/%d",
                   GetTickCount(),
                   GetCurrentProcessId());

       // Change current window title.

       SetConsoleTitle(pszNewWindowTitle);

       // Ensure window title has been updated.

       Sleep(40);

       // Look for NewWindowTitle.

       hwndFound=FindWindow(NULL, pszNewWindowTitle);

       // Restore original window title.

       SetConsoleTitle(pszOldWindowTitle);

       return(hwndFound);
   }
   
/* 
	Have to remember where i ripped this code sometime ago.

*/
static void ResizeConsole( HANDLE hConsole, SHORT x, SHORT y, SHORT xSize, SHORT ySize ) {

	CONSOLE_SCREEN_BUFFER_INFO csbi; // Hold Current Console Buffer Info 
	BOOL bSuccess;   
	SMALL_RECT srWindowRect;         // Hold the New Console Size 
	COORD coordScreen;    
	
	bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
	
	// Get the Largest Size we can size the Console Window to 
	coordScreen = GetLargestConsoleWindowSize( hConsole );
	
	// Define the New Console Window Size and Scroll Position 
	srWindowRect.Right  = (SHORT)(min(xSize, coordScreen.X) - 1);
	srWindowRect.Bottom = (SHORT)(min(ySize, coordScreen.Y) - 1);
	srWindowRect.Left   = srWindowRect.Top = (SHORT)0;	
	// srWindowRect.Right = x;
	// srWindowRect.Top = y;	

	
	// Define the New Console Buffer Size    
	coordScreen.X = xSize;
	coordScreen.Y = ySize;
	
	
	// If the Current Buffer is Larger than what we want, Resize the 
	// Console Window First, then the Buffer 
	if( (DWORD)csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
	{
		bSuccess = SetConsoleWindowInfo( hConsole, TRUE, &srWindowRect );
		bSuccess = SetConsoleScreenBufferSize( hConsole, coordScreen );
	}
	
	// If the Current Buffer is Smaller than what we want, Resize the 
	// Buffer First, then the Console Window 
	if( (DWORD)csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize )
	{
		bSuccess = SetConsoleScreenBufferSize( hConsole, coordScreen );
		bSuccess = SetConsoleWindowInfo( hConsole, TRUE, &srWindowRect );
	}
	
	// If the Current Buffer *is* the Size we want, Don't do anything! 
	
	
	return;
   }


void WIN32_Console() {
	AllocConsole();
	SetConsoleTitle("DOSBox Debugger");
	ResizeConsole(GetStdHandle(STD_OUTPUT_HANDLE),  0,  0,  80, 150);
	SetWindowPos(GetConsoleHwnd(), HWND_TOPMOST, 1, 1, 0, 0, SWP_NOZORDER);
	HWND hWnd = GetConsoleWindow();
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) &
		~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_VSCROLL | WS_HSCROLL));
}
#endif
#endif
