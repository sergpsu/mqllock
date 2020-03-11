#pragma once
#include "ml_api.h"
#include "CLinkHandle.h"
#include "rv.h"

#define WIDE_HELPER( x, y ) x ## y
#define WIDE(x) WIDE_HELPER( L, x )

#define ML_VERSIONA	"2.1.012"
#define ML_VERSIONW WIDE( ML_VERSIONA )

#ifdef UNICODE
#define ML_VERSION ML_VERSIONW
#else
#define ML_VERSION ML_VERSIONA
#endif

#ifdef MLL
//#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
//MLError		__stdcall MLInit( HWND chart );
#define TRYINIT( res ) if( !tryInit() ) return( res )
#else
#define TRYINIT( res )
#endif

const TCHAR*	__stdcall	MLVersion();
MLError		__stdcall MLCleanup( HWND chart );
int			__stdcall	MLRand( int from, int to );
BOOL		__stdcall	MLFileExists( const TCHAR* filename );
const TCHAR* __stdcall	MLComputerID();
const TCHAR* __stdcall	MLMACID();
MLError		__stdcall	MLMt4ToFront();
MLError		__stdcall	MLOpenBrowser( const TCHAR* url );
MLError		__stdcall	MLChartToFront( HWND chart );
int			__stdcall	MLStartTicker( HWND chart, int period );
MLError		__stdcall	MLStopTicker( HWND chart, int handle );
const TCHAR*	__stdcall	MLGetAccountNo();
BOOL		__stdcall	MLIsRealAccount();
LINK_HANDLE __stdcall	MLLinkAdd( HWND hChart, LINK_HANDLE parent, const TCHAR* text, const TCHAR* link );
MLError		__stdcall	MLLinkRemoveAll( LINK_HANDLE root );
//MLError		__stdcall	MLLastError( HWND hChart );

const TCHAR* __stdcall	MLGetWinDir();
const TCHAR* __stdcall	MLGetTempDir();
const TCHAR* __stdcall	MLGetAppdataDir();
const TCHAR*	__stdcall	MLGetHomeDir();
const TCHAR* __stdcall	MLGetSystemDrive();
const TCHAR* __stdcall	MLGetProgramFilesDir();
int			__stdcall	MLShellExecute( const TCHAR* command, BOOL hidden );

#ifdef MLL
RVHANDLE	__stdcall	MLRVInit( const TCHAR* auth );
#else
RVHANDLE	__stdcall	MLRVInit();
#endif
BOOL		__stdcall	MLRVExists( RVHANDLE rvh, const TCHAR* name );
double		__stdcall	MLRVDouble( RVHANDLE rvh, const TCHAR* name );
int			__stdcall	MLRVInt( RVHANDLE rvh, const TCHAR* name );
const TCHAR*	__stdcall	MLRVString( RVHANDLE rvh, const TCHAR* name );
long		__stdcall	MLRVDate( RVHANDLE rvh, const TCHAR* name );
BOOL		__stdcall	MLRVBool( RVHANDLE rvh, const TCHAR* name );
MLError		__stdcall	MLRVClose( RVHANDLE rvh );

MLError		__stdcall	MLChartMouseClick( HWND chart, int x, int y, int key );
MLError		__stdcall	MLChartMouseDown( HWND chart, int x, int y, int key );
MLError		__stdcall	MLChartMouseUp( HWND chart, int x, int y, int key );
MLError		__stdcall	MLChartMouseMove( HWND chart, int x, int y );
MLError		__stdcall	MLChartKey( HWND chart, int c );