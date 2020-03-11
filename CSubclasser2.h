#pragma once
#include "commctrl.h"

class CSubclasser2
{
	private:
		static CSubclasser2* m_instance;
		int m_used;
		HMODULE m_hDll;
		HANDLE m_hHookMutex;

		//used by hook proc to hook chart
		static DWORD_PTR m_pData;
		static HWND m_hwnd;
		static SUBCLASSPROC m_wndProc;
		static BOOL m_subclassed;
		static HHOOK m_hook;
		static BOOL m_bSubclass;
		static HANDLE m_hApcCalled;

	private:
		static LRESULT CALLBACK m_hookCallWndProc( int nCode, WPARAM wParam, LPARAM lParam );
		static void CALLBACK apc( CSubclasser2* me );
		void setHook();
		void unsetHook();

	public:
		BOOL subclass( HWND hwndChart, SUBCLASSPROC proc, DWORD_PTR data );
		BOOL unsubclass( HWND hwndChart, SUBCLASSPROC proc );
		BOOL isSubclassed( HWND hwndChart, SUBCLASSPROC proc );

	protected:
		CSubclasser2( HMODULE hDll );
		~CSubclasser2();

	public:
		static CSubclasser2* getInstance( HMODULE hDll );
		static BOOL freeInstance();
};
