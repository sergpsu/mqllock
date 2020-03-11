#pragma once
#include <windows.h>

class CCaptcha
{
private:
	HWND m_wnd;
	WNDPROC m_origProc;
	char* m_code;
	int m_drawn;
	HDC m_dc;
	HBITMAP m_bitmap;
private:
	static LRESULT WINAPI wndProc( HWND wnd, UINT msg, WPARAM wp, LPARAM lp );
	void cleanup();

public:
	void generate( int symbols );
	const char* getCode(){ return( m_code ); }

public:
	CCaptcha( HWND dst );
	~CCaptcha();
};
