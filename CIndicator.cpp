#include <windows.h>
#include "CIndicator.h"
#include "commctrl.h"

#pragma comment(lib,"comctl32.lib")

BOOL CIndicator::m_inited = FALSE;
HANDLE CIndicator::m_cs;
CSimpleArray<CIndicator::TimerData> CIndicator::m_events;

BOOL CIndicator::start( HWND hDlg, DWORD id )
{
	if( !m_inited )
	{
		m_inited = TRUE;
		m_cs = CreateMutex( 0, 0, 0 );
	}

	BOOL res = FALSE;
	
	WaitForSingleObject( m_cs, INFINITE );

	UINT_PTR tid = SetTimer( hDlg, ( UINT )hDlg + id, 100, ( TIMERPROC )onTimer );
	res = ( tid > 0 );
	if( res )
	{
		TimerData td;
		td.idEvent = tid;
		td.hwnd = hDlg;
		td.id = id;
		m_events.Add( td );
	}
	
	ReleaseMutex( m_cs );
	return( res );
} // start

BOOL CIndicator::stop( HWND hDlg, DWORD id )
{
	BOOL res = FALSE;
	WaitForSingleObject( m_cs, INFINITE );
	
	res = KillTimer( hDlg, ( UINT )hDlg + id );
	SendDlgItemMessage( hDlg, id, PBM_SETPOS, 0, 0 );
	
	ReleaseMutex( m_cs );
	return( res );
} // stop

void CALLBACK CIndicator::onTimer( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	WaitForSingleObject( m_cs, INFINITE );

	for( int i = 0; i < m_events.GetSize(); i ++ )
	{
		if( m_events[ i ].idEvent == idEvent && m_events[ i ].hwnd == hwnd )
		{
			DWORD id = m_events[ i ].id;
			SendDlgItemMessage( hwnd, id, PBM_STEPIT, 0, 0 );
			break;
		}
	}

	ReleaseMutex( m_cs );
} //onTimer