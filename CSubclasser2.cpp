#include <windows.h>
#include "CSubclasser2.h"
#include "api.h"

#ifndef ECS
#define ECS( h ) WaitForSingleObject( h, INFINITE )
#define LCS( h ) ReleaseMutex( h )
#endif

#pragma comment( lib, "comctl32.lib" )

//statics
CSubclasser2* CSubclasser2::m_instance = 0;
BOOL CSubclasser2::m_subclassed = FALSE;
BOOL CSubclasser2::m_bSubclass = FALSE;
SUBCLASSPROC CSubclasser2::m_wndProc = 0;
HHOOK CSubclasser2::m_hook = 0;
DWORD_PTR CSubclasser2::m_pData = 0;
HWND CSubclasser2::m_hwnd = 0;
HANDLE CSubclasser2::m_hApcCalled = 0;


CSubclasser2::CSubclasser2( HMODULE hDll ):
	m_hDll( hDll )
{
	m_hHookMutex = CreateMutex( 0, 0, 0 );
}

CSubclasser2::~CSubclasser2()
{
	CloseHandle( m_hHookMutex );

	if( m_hook )
	{
		BOOL unh = UnhookWindowsHookEx( m_hook );
		dbg( "UnhookWindowsHookEx %08x = %d (%d)", m_hook, unh, GetLastError() );
	}
}


// ==============
// isSubclassed
// ==============
BOOL CSubclasser2::isSubclassed( HWND hwnd, SUBCLASSPROC proc )
{
	DWORD_PTR data;
	return( GetWindowSubclass( hwnd, proc, 0, &data ) );
} // isSubclassed

// ====================
// m_hookCallWndProc
// ====================
LRESULT CALLBACK CSubclasser2::m_hookCallWndProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if( nCode == HC_ACTION && m_hwnd && ( ( CWPSTRUCT* )lParam )->hwnd == m_hwnd )
	{
dbg( "hook" );
		if( m_bSubclass )
		{
			m_subclassed = SetWindowSubclass( m_hwnd, m_wndProc, 0, m_pData );
dbg( "subclassed %08x, %d", m_hwnd, m_subclassed );
		}
		else
		{
			m_subclassed = RemoveWindowSubclass( m_hwnd, m_wndProc, 0 );
dbg( "unsubclassed %08x, %d", m_hwnd, m_subclassed );
		}
		m_hwnd = 0;
	}
	return( CallNextHookEx( 0, nCode, wParam, lParam ) );
} // m_hookCallWndProc

void CSubclasser2::setHook()
{
	if( !m_hook )
	{
		// can't subclass chart across threads, so need to call subclass api inside charts' ( =mt4 ) thread
		m_hook = SetWindowsHookEx( WH_CALLWNDPROC, m_hookCallWndProc, 0, GetWindowThreadProcessId( m_hwnd, 0 ) );
		dbg( "set hook %08x", m_hook );
	}
}

void CSubclasser2::unsetHook()
{
	if( m_hook )
	{
		BOOL unh = UnhookWindowsHookEx( m_hook );
		dbg( "UnhookWindowsHookEx %08x = %d (%d)", m_hook, unh, GetLastError() );
		m_hook = 0;
	}
}

void CALLBACK CSubclasser2::apc( CSubclasser2* me )
{
dbg( ":apc" );
	if( m_bSubclass )
	{
		m_subclassed = SetWindowSubclass( m_hwnd, m_wndProc, 0, m_pData );
dbg( "subclassed %08x, %d", m_hwnd, m_subclassed );
	}
	else
	{
		m_subclassed = RemoveWindowSubclass( m_hwnd, m_wndProc, 0 );
dbg( "unsubclassed %08x, %d", m_hwnd, m_subclassed );
	}
	
	SetEvent( me->m_hApcCalled );
}

/*
HWND g_swnd;
SUBCLASSPROC g_sproc;
DWORD_PTR g_sdata;
void __declspec(naked) apc2()
{
dbg( "apc21 %08x %08x %08x", g_swnd, g_sproc, g_sdata );
//dbg( "apc2 %08x %08x %08x", m_hwnd, m_wndProc, m_pData );
CSubclasser2::m_subclassed = SetWindowSubclass( CSubclasser2::m_hwnd, CSubclasser2::m_wndProc, 0, CSubclasser2::m_pData );
dbg( "subclass res=%d", CSubclasser2::m_subclassed );
	SetEvent( CSubclasser2::m_hApcCalled );
	for(;;)Sleep(0);
}*/


// ==============
// subclass
// ==============
BOOL CSubclasser2::subclass( HWND hwnd, SUBCLASSPROC proc, DWORD_PTR data )
{
dbg( "CSubclasser2::subclass %08x %08x %08x", hwnd, proc, data );
	BOOL res;
	ECS( m_hHookMutex );

	//check if window created in current thread
	DWORD wtid = GetWindowThreadProcessId( hwnd, 0 );
	if( GetCurrentThreadId() == wtid )
	{
dbg( "in window thread" );
		res = SetWindowSubclass( hwnd, proc, 0, data );
	}
	else
	{
		m_hwnd = hwnd;
		m_wndProc = proc;
		m_pData = data;
//g_swnd = hwnd;
//g_sproc = proc;
//g_sdata = data;

		m_bSubclass = TRUE;
//		m_hApcCalled = CreateEvent( 0, 0, 0, 0 );
		
		
		setHook();

		SendMessage( hwnd, WM_NULL, 0, 0 );	//waiting for hook proc called
dbg( "sent WM_NULL" );		

		// unhooking at once: for some reason on ea recompilation m_hook becomes invalid so next mt4 subclassing does not work
		unsetHook();	
/*		
		HANDLE ht = OpenThread( THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, 0, wtid );
		if( ht )
		{
			SuspendThread( ht );
			CONTEXT ctx = {0};
			ctx.ContextFlags = CONTEXT_ALL;
			GetThreadContext( ht, &ctx );
			DWORD eip = ctx.Eip;
			ctx.Eip = ( DWORD )apc2;
			SetThreadContext( ht, &ctx );
			if( ResumeThread( ht ) != 1 )
			{
				dbg( "!! frt%d (%d)", 1, GetLastError() );
			}
			if( WaitForSingleObject( m_hApcCalled, 5000 ) != 0 )
			{
				dbg( "!! fwapc" );
			}
			SuspendThread( ht );
			ctx.Eip = eip;
			SetThreadContext( ht, &ctx );
			if( ResumeThread( ht ) != 1 )
			{
				dbg( "!! frt%d (%d)", 2, GetLastError() );
			}

			CloseHandle( ht );
		}
		else
		{
			dbg( "!! fopt %d", GetLastError() );
		}
		CloseHandle( m_hApcCalled );
*/
		res = m_subclassed;
	}
	
	
dbg( "res %d", res );
	LCS( m_hHookMutex );

	return( res );
} // subclass

// ===============
// unsubclass
// ===============
BOOL CSubclasser2::unsubclass( HWND hwnd, SUBCLASSPROC proc )
{
	BOOL res;
dbg( "CSubclasser2::unsubclass %08x", hwnd );
	ECS( m_hHookMutex );
	if( GetCurrentThreadId() == GetWindowThreadProcessId( hwnd, 0 ) )
	{
dbg( "direct RemoveWindowSubclass" );
		res = RemoveWindowSubclass( hwnd, proc, 0 );
	}
	else
	{
		m_hwnd = hwnd;
		m_wndProc = proc;
		m_bSubclass = FALSE;

		setHook();

		SendMessage( hwnd, WM_NULL, 0, 0 );	//waiting for hook proc called
dbg( "sent WM_NULL" );		


		unsetHook();

		res = !m_subclassed;
	}
	LCS( m_hHookMutex );
dbg( "res=%d", res );
	return( res );
} // unsubclass

// =================
// getInstance
// =================
CSubclasser2* CSubclasser2::getInstance( HMODULE hDll )
{
	if( !m_instance )
	{
		m_instance = new CSubclasser2( hDll );
		m_instance->m_used = 1;
	}
	else
	{
		m_instance->m_used ++;
	}
	return( m_instance );
} // getInstance

// ===============
// freeInstance
// ===============
BOOL CSubclasser2::freeInstance()
{
	BOOL res = FALSE;
	if( m_instance )
	{
		m_instance->m_used --;
		if( !m_instance->m_used )
		{
			delete m_instance;
			m_instance = 0;
		}
		res = TRUE;
	}
	return( res );
} // freeInstance;