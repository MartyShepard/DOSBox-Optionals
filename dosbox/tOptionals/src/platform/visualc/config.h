/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */


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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Compiling on BSD */
/* #undef BSD */

/* Determines if the compiler supports always_inline attribute. */
#undef C_ATTRIBUTE_ALWAYS_INLINE

/* Determines if the compiler supports fastcall attribute. */
#define C_ATTRIBUTE_FASTCALL 1

/* Define to 1 to use inlined memory functions in cpu core */
#define C_CORE_INLINE 1

/* Define to 1 to enable output=ddraw (Win32) */
#define C_DDRAW 1

/* Define to 1 to enable internal debugger, requires libcurses */
#define C_DEBUG 1

/* Define to 1 if you want serial passthrough support (Win32, Posix and OS/2).
   */
#define C_DIRECTSERIAL 1

/* Define to 1 to use x86/x64 dynamic cpu core */
#define C_DYNAMIC_X86 1

/* Define to 1 to use recompiling cpu core. Can not be used together with the
   dynamic-x86 core */
/* #undef C_DYNREC */

/* Define to 1 to enable fluidsynth MIDI synthesis */
#define C_FLUIDSYNTH 1

/* Define to 1 to enable floating point emulation */
#define C_FPU 1

#ifdef _M_X64
//No support for inline asm with visual studio in x64 bit mode.
//This means that non-dynamic cores can't use the better fpu emulation.
#define C_FPU_X86 0
#else // _M_IX86
#define C_FPU_X86 1
#endif

/* Determines if the compiler supports attributes for structures. */
#undef C_HAS_ATTRIBUTE

/* Determines if the compilers supports __builtin_expect for branch
   prediction. */
#undef C_HAS_BUILTIN_EXPECT

/* Define to 1 if you have the mprotect function */
/* #undef C_HAVE_MPROTECT */

/* Define to 1 to enable heavy debugging, also have to enable C_DEBUG */
/* #undef C_HEAVY_DEBUG */

/* Define to 1 to enable IPX over Internet networking, requires SDL_net */
#define C_IPX 1

/* Define to 1 to enable internal modem support, requires SDL_net */
#define C_MODEM 1

/* Define to 1 to enable NE2000 ethernet passthrough, requires libwpcap */
#define C_NE2000 1

/* Define to 1 to use opengl display output support */
#define C_OPENGL 1

/* Define to 1 to use opengl display output support */
/* #undef C_RETINAFIX */

/* Define to 1 to enable SDL_sound support */
#define C_SDL_SOUND 1

/* Define to 1 if you have setpriority support */
/* #undef C_SET_PRIORITY */

/* Define to 1 to enable movie recording, requires zlib built without Z_SOLO
   */
#define C_SRECORD 1

/* Define to 1 to enable screenshots, requires libpng */
#define C_SSHOT 1

/* Compiler supports Core Audio headers */
/* #undef C_SUPPORTS_COREAUDIO */

/* Compiler supports Core MIDI headers */
/* #undef C_SUPPORTS_COREMIDI */

/* The type of cpu this target has */
#ifdef _M_X64
#define C_TARGETCPU X86_64
#else // _M_IX86
#define C_TARGETCPU X86
#endif

/* Define to 1 to use a unaligned memory access */
#define C_UNALIGNED_MEMORY 1

/* Define to 1 if you have wordexp.h */
/* #undef C_WORDEXP */

/* define to 1 if you have XKBlib.h and X11 lib */
/* #undef C_X11_XKB */

/* Determines if the function clock_gettime is available. */
#define DB_HAVE_CLOCK_GETTIME 1

/* libm doesn't include powf */
/* #undef DB_HAVE_NO_POWF */

/* struct dirent has d_type */
/* #undef DIRENT_HAS_D_TYPE */

/* environ can be included */
#define ENVIRON_INCLUDED 1

/* environ can be linked */
#define ENVIRON_LINKED 1

/* Define to 1 to use ALSA for MIDI */
/* #undef HAVE_ALSA */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `asound' library (-lasound). */
/* #undef HAVE_LIBASOUND */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H */

/* Define to 1 if you have the <pwd.h> header file. */
/* #undef HAVE_PWD_H */

/* Define to 1 if you have the `realpath' function. */
/* #undef HAVE_REALPATH */

/* Define to 1 if you have the "SDL.h" header file */
#define HAVE_SDL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Compiling on GNU/Linux */
/* #undef LINUX */

/* Compiling on Mac OS X */
/* #undef MACOSX */

/* Compiling on OS/2 EMX */
/* #undef OS2 */

/* Name of package */
#define PACKAGE "dosbox"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "dosbox"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "dosbox SVN"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "dosbox"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "SVN"

/* The size of `int *', as computed by sizeof. */
#define SIZEOF_INT_P 4

/* The size of `unsigned char', as computed by sizeof. */
#define SIZEOF_UNSIGNED_CHAR 1

/* The size of `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* The size of `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG 8

/* The size of `unsigned short', as computed by sizeof. */
#define SIZEOF_UNSIGNED_SHORT 2

/* Define to 1 if you have the ANSI C header files. */
/* #undef  STDC_HEADERS */

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Version number of package */
#define VERSION "SVN"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int` if you don't have socklen_t */
/* #undef socklen_t */

#if C_ATTRIBUTE_ALWAYS_INLINE
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE inline
#endif

#if C_ATTRIBUTE_FASTCALL
#define DB_FASTCALL __attribute__((fastcall))
#else
#define DB_FASTCALL
#endif

#if C_HAS_ATTRIBUTE
#define GCC_ATTRIBUTE(x) __attribute__ ((x))
#else
#define GCC_ATTRIBUTE(x) /* attribute not supported */
#endif

#if C_HAS_BUILTIN_EXPECT
#define GCC_UNLIKELY(x) __builtin_expect((x),0)
#define GCC_LIKELY(x) __builtin_expect((x),1)
#else
#define GCC_UNLIKELY(x) (x)
#define GCC_LIKELY(x) (x)
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400) 
#pragma warning(disable : 4996) 
#endif

typedef double            Real64;
/* The internal types */
typedef unsigned char     Bit8u;
typedef signed char       Bit8s;
typedef unsigned short    Bit16u;
typedef signed short      Bit16s;
typedef unsigned int      Bit32u;
typedef signed int        Bit32s;
typedef unsigned __int64  Bit64u;
typedef signed __int64    Bit64s;
#define sBit32t
#define sBit64t "I64"
#define sBit32fs(a) sBit32t #a
#define sBit64fs(a) sBit64t #a
#ifdef _M_X64
typedef Bit64u            Bitu;
typedef Bit64s            Bits;
#define sBitfs sBit64fs
#else // _M_IX86
typedef Bit32u            Bitu;
typedef Bit32s            Bits;
#define sBitfs sBit32fs
#endif
