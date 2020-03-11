#pragma once

//#include "defs.h"

class CReg
{
	private:
		HKEY hkey;
		LRESULT lastErr;

	public:
		int open( const char* path, DWORD access = KEY_READ | KEY_WRITE );
		int get( const char* key, void* value, IN OUT DWORD *max );
		int set( const char* key, void* value, DWORD max, DWORD type );
		int close( BOOL flush = FALSE );
		int del( const char* key );
		int delKey( const char* key );
		int enumKey( int i, OUT char* name, DWORD maxName );
		int enumValue( int i, OUT char* name, DWORD maxName, void* data, OUT DWORD *sz );
		HKEY getKey(){ return( hkey ); }

	public:
		LRESULT getLastError();

	public:
		CReg();
		~CReg();
};