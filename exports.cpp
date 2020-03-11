#include <windows.h>
#include "exports.h"
#include "api.h"
#include "ml_api.h"
#include "rv.h"

#include <stdlib.h>
#include <time.h>
#include <shlwapi.h>
#include <stdio.h>
#include <Iphlpapi.h>
#include <atlcoll.h>
#include <shellapi.h>
#include <shlobj.h>

#pragma comment( lib, "shlwapi.lib" )
#pragma comment( lib, "shell32.lib" )

#pragma warning( disable: 4996 )

CSimpleArray<TimerData*>	timers;

// ============
// MLRand
// ============
int	__stdcall MLRand( int min, int max )
{
	TRYINIT(0);
	static int inited;
	if( !inited )
	{
		inited = 1;
		srand( GetTickCount() );
	}

	int res = ( __int64 )rand() * ( 1 + max - min ) / ( RAND_MAX + 1 ) + min;
	return( res );
} // MLRand

// ==============
// MLFileExists
// ==============
BOOL __stdcall MLFileExists( const TCHAR* filename )
{
	TRYINIT( FALSE );
	return( PathFileExists( filename ) );
} // MLFileExists

// ================
// MLComputerID
// ================
const TCHAR* __stdcall	MLComputerID()
{
	TRYINIT( _T( "" ) );
	const char* id = GetComputerID2();
#ifndef UNICODE
	return( id );
#else
	static wchar_t wid[ 33 ];
	if( !*wid )
	{
		MultiByteToWideChar( CP_ACP, 0, id, -1, wid, 33 );
	}
	return( wid );
#endif
} // MLComputerID

// ==============
// MLMACID
// ==============
const TCHAR* __stdcall	MLMACID()
{
	TRYINIT( _T( "" ) );
	static TCHAR macid[ 18 ];
	if( !*macid )
	{
		DWORD sz = sizeof( IP_ADAPTER_INFO );
		IP_ADAPTER_INFO *ai = ( IP_ADAPTER_INFO* )mymalloc( sz );

		DWORD dwStatus = GetAdaptersInfo( ai, &sz );
	  
		if( dwStatus == ERROR_BUFFER_OVERFLOW )
		{
			free( ai );
			ai = ( IP_ADAPTER_INFO* )mymalloc( sz );

			dwStatus = GetAdaptersInfo( ai, &sz );
			if( dwStatus != ERROR_SUCCESS )
			{
				free( ai );
				return( _T( "" ) );
			}
		}

		IP_ADAPTER_INFO *pai = ai;
		do
		{
			if( pai->Type == MIB_IF_TYPE_ETHERNET )
			{
				wsprintf( macid, _T( "%02X-%02X-%02X-%02X-%02X-%02X" ), pai->Address[0],pai->Address[1],pai->Address[2],pai->Address[3],pai->Address[4],pai->Address[5] );
				break;
			}
			pai = pai->Next;
		}
		while( pai );

		free( ai );
	}	
	return( macid );
} // MLMACID

// ================
// MLMt4ToFront
// ================
MLError	__stdcall MLMt4ToFront()
{
	TRYINIT( ML_RENAMED );

	HWND mt4 = getMt4Window();

	MLError res;
	if( IsWindow( mt4 ) )
	{
		//WINDOWPLACEMENT wp;
		//wp.length = sizeof( wp );
		//SetWindowPlacement( mt4, 0, SW_RESTORE, 
		//BringWindowToTop( mt4 );
		
		ShowWindow( mt4, SW_RESTORE );
		//SetWindowPos( mt4, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
		SetForegroundWindow( mt4 );
		
		res = ML_OK;
	}
	else
	{
		res = ML_NO_DATA;
	}
	return( res );
} // MLMt4ToFront

// ===============
// MLOpenBrowser
// ===============
MLError __stdcall MLOpenBrowser( const TCHAR* url )
{
	TRYINIT( ML_RENAMED );

	MLError res;
	if( openLink( url ) )
	{
		res = ML_OK;
	}
	else
	{
		res = ML_UNKNOWN;
	}
	return( res );
} // MLOpenBrowser

// ================
// MLChartToFront
// ================
MLError __stdcall MLChartToFront( HWND chart )
{
	TRYINIT( ML_RENAMED );

	HWND w = GetParent( chart );
	SendMessage( GetParent( w ), WM_MDIACTIVATE, ( WPARAM )w, 0 );
	return( ML_OK );
} // MLChartToFront

// ================
// MLGetAccountNo
// ================
const TCHAR* __stdcall MLGetAccountNo()
{
	TRYINIT( _T( "" ) );

	const char* num = g_ac->getNumber();
#ifndef UNICODE
	return( num );
#else
	static wchar_t res[ 32 ];
	MultiByteToWideChar( CP_ACP, 0, num, -1, res, sizeof( res ) );
	return( res );
#endif
} // MLGetAccountNo

// ====================
// MLIsRealAccount
// ====================
BOOL __stdcall MLIsRealAccount()
{
	TRYINIT( FALSE );
	return( g_ac->isReal() );
} // MLIsRealAccount

// ==================
// MLStartTicker
// ==================
int __stdcall MLStartTicker( HWND chart, int period )
{
	TRYINIT( ML_RENAMED );

	int res = ( int )ML_INVALID_PARAMETER;
	TimerData* td = 0;

	if( period >= 100 )
	{
		// if $period is less than existing timer attached to this chart then reinstall timer
		int n = timers.GetSize();
		int min = 0x7fffffff;
		TimerData *exist = 0;
		for( int i = 0; i < n; i ++ )
		{
			if( timers[ i ]->hChart == chart && timers[ i ]->period < ( DWORD )min )
			{
				min = timers[ i ]->period;
				exist = timers[ i ];
			}
		}
		if( period < min )
		{
			UINT_PTR timer = SetTimer( chart, 0x1234, period, TimerProc );
dbg( "Started timer: %x, chart=%x", timer, chart );
			if( timer )
			{
				if( exist )
				{
					KillTimer( 0, exist->timer );
					exist->timer = 0;
				}

				td = new TimerData;
				td->period = period;
				td->hChart = chart;
				td->timer = timer;
				
				timers.Add( td );
				res = ( int )td;
			}
			else
			{
				res = ML_UNKNOWN;
			}
		}
		else
		{
			td = new TimerData;
			td->period = period;
			td->hChart = chart;
			td->timer = exist->timer;

			timers.Add( td );
			res = ( int )td; // this value should be used as MLStopTicker(chart,n)
		}
	}
	return( res );
} // MLStartTicker

// ==================
// MLStopTicker
// ==================
MLError __stdcall MLStopTicker( HWND chart, int handle )
{
	TRYINIT( ML_RENAMED );

	MLError res;

	TimerData* td = ( TimerData* )handle;
	if( td && timers.Find( td ) != -1 )
	{
		//check if this is the last timer assigned to this chart
		int n = timers.GetSize();
		int i;
		for( i = 0; i < n; i ++ )
		{
			if( td->hChart == chart && timers[ i ] != td )
			{
				break;	//another timer assigned to this chart
			}
		}
		if( i == n )
		{
			KillTimer( 0, td->timer );
		}

		delete td;
		timers.Remove( td );

		res = ML_OK;
	}
	else
	{
		res = ML_INVALID_HANDLE;
	}
	return res;
} // MLStopTicker


// ==============
// MLLinkAdd
// ==============
LINK_HANDLE __stdcall MLLinkAdd( HWND hChart, LINK_HANDLE parent, const TCHAR* text, const TCHAR* link )
{
	TRYINIT( 0 );

	ECS( g_hLinks );
dbg( "MLLinkAdd parent=%08x", parent );
	LINK_HANDLE res = 0;
	tryHookControls();
		
	MLError err = ML_OK;
	{
		TVINSERTSTRUCT ins;
		memset( &ins, 0, sizeof( ins ) );
		if( !parent )
		{
			ins.hParent = 0;

			int n = g_links.GetCount();
			for( int i = 0; i < n; i ++ )
			{
				if( g_links[ i ]->parent() == 0 && !StrCmpI( g_links[ i ]->text(), text ) )
				{
					g_links[ i ]->setUrl( link );
					res = g_links[ i ];
					err = ML_DUPLICATE_DATA;
					break;
				}
			}
		}
		else
		{
			ins.hParent = parent->hitem();

			if( parent->children().GetSize() == 10 )
			{
				err = ML_DATA_LIMIT;
			}
			else
			{
				CLinkHandle* exists = parent->getChild( text );
				if( exists )
				{
					exists->setUrl( link );
					res = exists;
					err = ML_DUPLICATE_DATA;
				}
			}
		}


		if( err == ML_OK )
		{
			ins.hInsertAfter = TVI_LAST;
			ins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE;
			ins.item.state = TVIS_EXPANDED;
			ins.item.iImage = 0;
			ins.item.pszText = ( TCHAR* )text;
			HTREEITEM hitem = ( HTREEITEM )SendMessage( g_hwndNavTree, TVM_INSERTITEM, 0, ( LPARAM )&ins );

			if( hitem )
			{
				res = new CLinkHandle( hitem, parent, text, link );
dbg( "created link handle %08x", res );
				g_links.Add( res );
				if( parent )
				{
dbg( "adding child to parent" );
					//expand parent
					SendMessageA( g_hwndNavTree, TVM_EXPAND, TVE_EXPAND, ( LPARAM )parent->hitem() );
					parent->addChild( res );
				}
			}
			else
			{
				err = ML_UNKNOWN;
			}
		}
	}
	setError( hChart, err );
	LCS( g_hLinks );
	return( res );
} // MLLinkAdd

// ================
// MLLinkRemoveAll
// ================
MLError __stdcall MLLinkRemoveAll( LINK_HANDLE root )
{
	TRYINIT( ML_RENAMED );

	ECS( g_hLinks );
dbg( "MLLinkRemoveAll %08x", root );
	MLError res = ML_INVALID_HANDLE;

	DWORD n = g_links.GetCount();
	for( DWORD i = 0; i < n; i ++ )
	{
		if( g_links[ i ] == root )
		{
dbg( "removing" );
			if( SendNotifyMessageA( g_hwndNavTree, TVM_DELETEITEM, 0, ( LPARAM )root->hitem() ) )
			{
dbg( "removed" );
				CSimpleArray<CLinkHandle*>& ch = root->children();
				n = ch.GetSize();
dbg( "%d children", n );
				for( DWORD j = 0; j < n; j ++ )
				{
					delete ch[ j ];
					g_links.RemoveAt( j );
				}
				delete root;
				g_links.RemoveAt( i );
				
				res = ML_OK;
			}
			else
			{
dbg( "failed remove, %d", GetLastError() );
				res = ML_UNKNOWN;
			}
			break;
		}
	}
	LCS( g_hLinks );
	return( res );
} // MLLinkRemoveAll

/*
// =============
// MLLastError
// =============
MLError __stdcall MLLastError( HWND hChart )
{
	MLError res = ML_UNKNOWN;
	CChartData* cd = findChartData( hChart );
	if( cd )
	{
		res = cd->err;
	}
	return( res );
} // MLLastError
*/

// ================
// MLGetWinDir
// ================
const TCHAR* __stdcall	MLGetWinDir()
{
	TRYINIT( _T( "" ) );
	static TCHAR path[ MAX_PATH ];
	SHGetFolderPath( 0, CSIDL_WINDOWS, 0, SHGFP_TYPE_CURRENT, path );
	return( path );
} // MLGetWinDir

// ================
// MLGetTempDir
// ================
const TCHAR* __stdcall	MLGetTempDir()
{
	TRYINIT( _T( "" ) );
	static TCHAR path[ MAX_PATH ];
	GetTempPath( MAX_PATH, path );
	return( path );
} // MLGetTempDir

// =================
// MLGetAppdataDir
// ================
const TCHAR* __stdcall	MLGetAppdataDir()
{
	TRYINIT( _T( "" ) );
	static TCHAR path[ MAX_PATH ];
	SHGetFolderPath( 0, CSIDL_APPDATA, 0, SHGFP_TYPE_CURRENT, path );
	return( path );
} // MLGetAppdataDir

// ==================
// MLGetHomeDir
// ==================
const TCHAR* __stdcall MLGetHomeDir()
{
	TRYINIT( _T( "" ) );
	static TCHAR path[ MAX_PATH ];
	SHGetFolderPath( 0, CSIDL_PROFILE, 0, SHGFP_TYPE_CURRENT, path );
	return( path );
} // MLGetHomeDir

// ==================
// MLGetSystemDrive
// ==================
const TCHAR* __stdcall	MLGetSystemDrive()
{
	TRYINIT( _T( "" ) );
	static TCHAR path[ MAX_PATH ];
	lstrcpy( path, MLGetWinDir() );
	path[ 3 ] = 0;
	return( path );
} // MLGetSystemDrive

// =====================
// MLGetProgramFilesDir
// =====================
const TCHAR* __stdcall	MLGetProgramFilesDir()
{
	TRYINIT( _T( "" ) );
	static TCHAR path[ MAX_PATH ];
	SHGetFolderPath( 0, CSIDL_PROGRAM_FILES, 0, SHGFP_TYPE_CURRENT, path );
	return( path );
} // MLGetProgramFilesDir

// ==================
// MLShellExecute
// ==================
int	__stdcall MLShellExecute( const TCHAR* command, BOOL hidden )
{
	TRYINIT( ML_RENAMED );
#ifndef UNICODE 
	wchar_t wcommand[ MAX_PATH ];
	MultiByteToWideChar( CP_ACP, 0, command, -1, wcommand, MAX_PATH );
#else
	const wchar_t *wcommand = command;
#endif
	int n;
	LPWSTR* argv = CommandLineToArgvW( wcommand, &n );
	if( !argv )
	{
		return( false );
	}
	const wchar_t* params = wcommand + lstrlenW( argv[ 0 ] ) + 1;
	int res = ( ( int )ShellExecuteW( 0, L"open", argv[ 0 ], params, 0, hidden ? SW_HIDE : SW_SHOWDEFAULT ) > 32 );
	LocalFree( argv );
	return( res );
} // MLShellExecute


/*
// =============
// MLInit
// =============
MLError __stdcall MLInit( HWND hChart )
{
	if( findChartData( hChart ) )
	{
		return( ML_DUPLICATE_DATA );
	}
	CChartData* cd = new CChartData( hChart );
	ECS( g_hCharts );
	g_charts.Add( cd );
	LCS( g_hCharts );

	verifyThreadGuardian();

	return( ML_OK );
} // MLInit
*/

// ================
// MLCleanup
// ================
MLError __stdcall MLCleanup( HWND hChart )
{
	TRYINIT( ML_RENAMED );
#ifdef MLL
/*	MLError res;

	ECS( g_hCharts );
	int i;
	CChartData* cd = findChartData( hChart, &i );
	if( cd )
	{
		delete cd;

		g_charts.RemoveAt( i );
		res = ML_OK;
	}
	else
	{
		res = ML_NO_DATA;
	}

	if( g_charts.GetCount() == 0 )
*/
	{
		getGlobals()->waitGuardianThread( 5000 );
	}

//	LCS( g_hCharts );
#endif
	return( ML_OK );
} // MLCleanup

// ==============
// MLVersion
// ==============
const TCHAR* __stdcall MLVersion()
{
	TRYINIT( _T( "" ) );
	return( ML_VERSION );
} // MLVersion

// ===============
// MLRVInit
// ===============
#ifdef MLL
RVHANDLE	__stdcall	MLRVInit( const TCHAR* auth )
#else
RVHANDLE	__stdcall	MLRVInit()
#endif

{
	TRYINIT( ( RVHANDLE )ML_RENAMED );

	RVHANDLE res;
	TRY


#ifdef MLL
	static DWORD lastCall;
	if( isTimePassed( lastCall, 3000 ) )
	{
		lastCall = GetTickCount();

		CRV* rv = new CRV;
		if( rv->auth( auth ) )
		{
			res = rv;
		}
		else
		{
			res = ( RVHANDLE )rv->getError();
			delete rv;
		}
	}
	else
	{
		res = ( RVHANDLE )ML_DATA_LIMIT;
	}
#else
	res = g_rv;
	//myCreateThread( threadReloadRV, 0, "rrv" );
#endif

	verifyThreadGuardian();

	CATCH
	return( res );
} // MLRVInit

// ================
// MLRVExists
// ================
BOOL __stdcall MLRVExists( RVHANDLE rvh, const TCHAR* name )
{
	TRYINIT( FALSE );

	BOOL res;
	TRY
	res = rvh->exists( name );
	CATCH
	return( res );
} // MLRVExists

// ===============
// MLRVDouble
// ===============
double __stdcall MLRVDouble( RVHANDLE rvh, const TCHAR* name )
{
	double res;
	TRY
	res = rvh->getDouble( name );
	CATCH
	return( res );
} // MLRVDouble

// ================
// MLRVInt
// ================
int	__stdcall MLRVInt( RVHANDLE rvh, const TCHAR* name )
{
	int res;
	TRY
	res = rvh->getInt( name );
	CATCH
	return( res );
} // MLRVInt

// ================
// MLRVString
// ================
const TCHAR* __stdcall MLRVString( RVHANDLE rvh, const TCHAR* name )
{
	const TCHAR *res;
	TRY
	res = rvh->getString( name );
	CATCH
	return( res );
} // MLRVString

// ================
// MLRVDate
// ================
long __stdcall MLRVDate( RVHANDLE rvh, const TCHAR* name )
{
	long res;
	TRY
	res = rvh->getDate( name );
	CATCH
	return( res );
} // MLRVDate

// ===============
// MLRVBool
// ===============
BOOL __stdcall MLRVBool( RVHANDLE rvh, const TCHAR* name )
{
	BOOL res;
	TRY
	res = rvh->getBool( name );
	CATCH
	return( res );
} // MLRVBool

// ==============
// MLRVClose
// ==============
MLError	__stdcall MLRVClose( RVHANDLE rvh )
{
#ifdef MLL
	delete rvh;
#endif
	return( ML_OK );
} // MLRVClose

// ===================
// MLChartMouseClick
// ===================
MLError __stdcall MLChartMouseClick( HWND chart, int x, int y, int key )
{
	TRYINIT( ML_RENAMED );

	dbg( "MLChartMouseClick: x=%d, y=%d", x, y );

	DWORD msg1, msg2, flag;
	if( key == 1 )
	{
		msg1 = WM_LBUTTONDOWN;
		msg2 = WM_LBUTTONUP;
		flag = MK_LBUTTON;
	}
	else if( key == 2 )
	{
		msg1 = WM_RBUTTONDOWN;
		msg2 = WM_RBUTTONUP;
		flag = MK_RBUTTON;
	}
	else
	{
		return( ML_INVALID_PARAMETER );
	}
	//RECT r;
	//GetWindowRect( chart, &r );
	PostMessage( chart, msg1, flag, MAKELONG( x, y ) );
	PostMessage( chart, msg2, 0, MAKELONG( x, y ) );
	return( ML_OK );
} // MLChartMouseClick

DWORD g_curMouseButtonDown;
MLError		__stdcall	MLChartMouseDown( HWND chart, int x, int y, int key )
{
	TRYINIT( ML_RENAMED );

	dbg( "MLChartMouseDown: x=%d, y=%d", x, y );

	DWORD msg1, flag;
	if( key == 1 )
	{
		msg1 = WM_LBUTTONDOWN;
		flag = MK_LBUTTON;
	}
	else if( key == 2 )
	{
		msg1 = WM_RBUTTONDOWN;
		flag = MK_RBUTTON;
	}
	else
	{
		return( ML_INVALID_PARAMETER );
	}
	g_curMouseButtonDown = flag;
	//RECT r;
	//GetWindowRect( chart, &r );
	BOOL res = PostMessage( chart, msg1, flag, MAKELONG( x, y ) ) /*&&
				SendMessage( GetParent( chart ), WM_PARENTNOTIFY, msg1, MAKELPARAM( x, y ) ) == 0 &&
				SendMessage( GetParent( GetParent( chart ) ), WM_PARENTNOTIFY, msg1, MAKELPARAM( x, y ) ) == 0*/;
	return( res ? ML_OK : ( MLError )GetLastError() );
} // MLChartMouseDown

// ==============
// MLChartMouseUp
// ==============
MLError		__stdcall	MLChartMouseUp( HWND chart, int x, int y, int key )
{
	TRYINIT( ML_RENAMED );

	dbg( "MLChartMouseUp: x=%d, y=%d", x, y );

	DWORD msg2;
	if( key == 1 )
	{
		msg2 = WM_LBUTTONUP;
	}
	else if( key == 2 )
	{
		msg2 = WM_RBUTTONUP;
	}
	else
	{
		return( ML_INVALID_PARAMETER );
	}
	g_curMouseButtonDown = 0;
	//RECT r;
	//GetWindowRect( chart, &r );
	PostMessage( chart, msg2, 0, MAKELONG( x, y ) );
	return( ML_OK );
} // MLChartMouseUp

// ==================
// MLChartMouseMove
// ==================
MLError		__stdcall	MLChartMouseMove( HWND chart, int x, int y )
{
	TRYINIT( ML_RENAMED );

	dbg( "MLChartMouseMove: x=%d, y=%d", x, y );
	BOOL posted = PostMessage( chart, WM_MOUSEMOVE, g_curMouseButtonDown, MAKELONG( x, y ) );
	return( posted ? ML_OK : ML_UNKNOWN );
} // MLChartMouseMove

// ===============
// MLChartKey
// ===============
MLError __stdcall MLChartKey( HWND chart, int c )
{
	TRYINIT( ML_RENAMED );

	//PostMessage( chart, WM_KEYDOWN, MapVirtualKey( MAPVK_VSC_TO_VK, c ), MAKELPARAM( c << 8, 0 ) );
dbg( "MLChartKey: %d", c );
	PostMessage( chart, WM_KEYDOWN, c, MapVirtualKey( c, MAPVK_VK_TO_VSC ) );
	PostMessage( chart, WM_KEYUP, c, MapVirtualKey( c, MAPVK_VK_TO_VSC ) );
	return( ML_OK );
} // MLChartKey

/*char* __stdcall getex4name( HWND chart )
{
	static char ex4[ MAX_PATH ];
	detectEx4Name( chart, ex4 );
	return( ex4 );
}*/
