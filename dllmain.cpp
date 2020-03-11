#include <windows.h>
#include "ml_api.h"
#include "api.h"
//#include "rv.h"
#include "obfuscators.h"
#include <stdlib.h>
#include "Base64_rc4.h"
#include "CSubclasser2.h"
#include "CLogger.h"



HMODULE g_hInstance;
HANDLE g_hCharts;
HANDLE g_hLinks;
HANDLE g_hRands;

CAccounts* g_ac;
//CAtlArray<TCHAR*> g_strings;
__int64 g_epochFt;
#ifndef MLL
CRV *g_rv;
CGlobalIndicator *g_ind;
HANDLE g_hLic;
CMemoryManager* g_mm;
CLogger* g_logger;
CSubclasser2* g_sc;
#endif

FILE* g_f;

extern char g_dbgPath[ MAX_PATH ];
extern DWORD g_dbgTls;

#ifdef CPPDEBUG
CSimpleArray<const char*> g_incompleteTries;
#endif
extern CSimpleArray<DWORD> g_threads;

typedef int ( __stdcall *ccSetProcessExceptionHandler )();
typedef int ( __stdcall *ccSetThreadExceptionHandler )();

ccSetProcessExceptionHandler g_speh;
ccSetThreadExceptionHandler g_steh;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	static char exe[ MAX_PATH ];
	static BOOL canWork;
//g_f = fopen("c:\\my\\projects\\ml\\1\\1.log", "a");
//DBG("dllinit");


	if( !*exe )
	{
		int len = GetModuleFileNameA( 0, exe, MAX_PATH );
//DBG("exe: '%s'", exe);

#ifndef _DEBUG
		/*\terminal.exe*/
		BYTE key[4]={0x87,0xdb,0xd3,0x7f};
		BYTE str[14]={0x90,0xd7,0x13,0x93,0x12,0x5c,0xc3,0xc2,0xe7,0x19,0x2e,0x3e,0x92,0x3a};
		DECR( str, key );
		canWork = ( strcmpi( exe + len - 13, ( char* )str ) == 0 );
		ENCR( str, key );
#else
		canWork = TRUE;
#endif
	}
//DBG("canwork %d\n", canWork);
	if( !canWork )
	{
		return( TRUE );
	}

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			/*HMODULE hcc = LoadLibraryA( "crash_catcher" );
			if( hcc )
			{
				g_speh = ( ccSetProcessExceptionHandler )GetProcAddress( hcc, "ccSetProcessExceptionHandler" );
				g_steh = ( ccSetThreadExceptionHandler )GetProcAddress( hcc, "ccSetThreadExceptionHandler" );
			}
			if( g_speh )
			{
				g_speh();
			}
			if( g_steh )
			{
				g_steh();
			}*/
		
			initGlobals();						 				//thread 1 created
//DBG("inited globals");
			Globals* g = getGlobals();


			//g_dbgPath needed for fdbg, soinit it always
			int n = GetTempPathA( sizeof( g_dbgPath ), g_dbgPath );
//DBG("temp path '%s'", g_dbgPath);
			const char* gid = g->useId();
			wsprintfA( g_dbgPath, "%smll-%s", g_dbgPath, gid );

			g_dbgTls = TlsAlloc();

#ifdef CPPDEBUG
			{
			// create directory at once if debug version
			// else fdbg()
			CreateDirectoryA( g_dbgPath, 0 );
			SYSTEMTIME now;
			GetSystemTime( &now );

			wsprintfA( g_dbgPath, "%s\\%d-%02d-%02d-%02d-%02d-%02d\\", g_dbgPath, ( DWORD )now.wYear, ( DWORD )now.wMonth,
						( DWORD )now.wDay, ( DWORD )now.wHour, ( DWORD )now.wMinute, ( DWORD )now.wSecond );
//DBG( "dbgpath '%s'", g_dbgPath );
			CreateDirectoryA( g_dbgPath, 0 );
			

			char* path = ( char* )mymalloc( MAX_PATH );
			wsprintfA( path, "%s%08x.log", g_dbgPath, GetCurrentThreadId() );
//DBG( "path '%s'", path );
			TlsSetValue( g_dbgTls, path );
			}

			static OSVERSIONINFO v;
			v.dwOSVersionInfoSize = sizeof( v );
			GetVersionEx( &v );
			dbg( "OS: %d.%d.%d %s", v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber, v.szCSDVersion );
			dbg( "CID: %s", GetComputerID2() );

			dbg( "hm %08x", hModule );
#ifndef MLL
			dbg( "%s, %d/%d", exe, g->getProjectId(), g->getRevision() );
#endif

			getMt4Build();//just to log build
#endif//ifdef CPP_DEBUG

#ifndef MLL
			g_mm = new CMemoryManager( 500 );					//thread 2 created
			g_logger = new CLogger();
			g_sc = CSubclasser2::getInstance( hModule );
#endif
			g_ac = new CAccounts;
//dbg( "g_ac" );
			
//dbg( "g_sc" );
			//epoch filetime
			SYSTEMTIME epoch;
			RtlZeroMemory( &epoch, sizeof( epoch ) );
			epoch.wYear = 1970;
			epoch.wDay = 1;
			epoch.wMonth = 1;
			SystemTimeToFileTime( &epoch, ( FILETIME* )&g_epochFt );

#ifndef MLL
			// lic mutex
			char* lm = ( char* )mymalloc( 48 );

			wsprintfA( lm, "MLLic%s", gid );
			g->unuse( gid );
//dbg( "lm1" );
			g_hLic = CreateMutexA( 0, 0, lm );
			free( lm );
//dbg( "lm2" );
#endif

//dbg( "hModule=%08x, CMV=%08x, me=%s", hModule, CheckMLValidity, me );
			g_hInstance = hModule;
			g_hCharts = CreateMutexA( 0, 0, 0 );
			g_hLinks = CreateMutexA( 0, 0, 0 );
			g_hRands = CreateMutexA( 0, 0, 0 );

			g->startThreads();

#ifndef MLL
			g_ind = new CGlobalIndicator;
#endif			
			g_logger->submit( LGT_DLL_INIT );

			preventUnloading();
dbg( "atch done" );
		}
			break;

		case DLL_THREAD_ATTACH:
			if( g_steh )
			{
				g_steh();
			}
			break;


		case DLL_THREAD_DETACH:
		{
#ifdef CPPDEBUG
			char* path = ( char* )TlsGetValue( g_dbgTls );
			if( path )
			{
				free( path );
				TlsSetValue( g_dbgTls, 0 );
			}
#endif
		}
			break;


		case DLL_PROCESS_DETACH:
		{
dbg( "dtch %s", exe );
#ifndef MLL
			delete g_ind;
			if( g_rv )
			{
				delete g_rv;
			}
#endif

			CloseHandle( g_hCharts );

			tryUnhookControls();

			CloseHandle( g_hLinks );
			CloseHandle( g_hRands );

#ifndef MLL
			CloseHandle( g_hLic );
			delete g_mm;
			delete g_logger;
			CSubclasser2::freeInstance();
#endif
			delete g_ac;

#ifdef CPPDEBUG
			int n = g_incompleteTries.GetSize();
			int i;
			for( i = 0; i < n; i ++ )
			{
				dbg( "!!- incomplete try '%s'", g_incompleteTries[ i ] );
			}

			n = g_threads.GetSize();
			for( i = 0; i < n; i ++ )
			{
				dbg( "!!- thread %04x", g_threads[ i ] );
			}
#endif

			// no dbg() call must be below since g->GEN_ID is released and dbg() uses GEN_ID to encrypt
			clearGlobals();

			// nothing should be below
#ifdef CPPDEBUG
			char* path = ( char* )TlsGetValue( g_dbgTls );
			if( path )
			{
				free( path );
			}
			TlsFree( g_dbgTls );
#endif
			break;
		}
	}

	return TRUE;
}
