#ifndef __INDICATOR_H__
#define __INDICATOR_H__
#include <atlcoll.h>
class CIndicator
{
	private:
		struct TimerData
		{
			UINT_PTR	idEvent;
			HWND		hwnd;
			DWORD		id;
		};
		static HANDLE m_cs;
		static BOOL m_inited;
		static CSimpleArray<TimerData> m_events;

	public:
		static BOOL start( HWND hDlg, DWORD id );
		static BOOL stop( HWND hDlg, DWORD id );
		static void CALLBACK CIndicator::onTimer( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime );
};
#endif