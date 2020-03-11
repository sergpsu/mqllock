#include <windows.h>
#include "CReg.h"

CReg::CReg()
{
	hkey = 0;
	lastErr = 0;
}

CReg::~CReg()
{
	close();
}

// ===========
// getLastError
// ===========
LRESULT CReg::getLastError()
{
	return( lastErr );
} // getLastError


int CReg::open( const char* path, DWORD access )
{
	int lres = ( int )RegCreateKeyExA( HKEY_CURRENT_USER, path, 0, 0, 0, access, 0, &hkey, 0 );
	lastErr = lres;
	return( lres );
}

int CReg::close( BOOL flush )
{
	int res = -1;
	if( hkey )
	{
		if( flush )
		{
			RegFlushKey( hkey );
		}

		res = RegCloseKey( hkey );
		hkey = 0;
	}
	return( res );
}

int CReg::get( const char* key, void* value, IN OUT DWORD *max )
{
	if( !hkey )
	{
		return( -1 );
	}

	int lres = ( int )RegQueryValueExA( hkey, key, 0, 0, ( LPBYTE )value, max );
	return( lres );
}

int CReg::set( const char* key, void* value, DWORD len, DWORD type )
{
	if( !hkey )
	{
		return( -1 );
	}

	int lres = ( int )RegSetValueExA( hkey, key, 0, type, ( const BYTE*) value, len );
	return( lres );
}

int CReg::del( const char* key )
{
	return( ( int )RegDeleteValueA( hkey, key ) );
}
int CReg::delKey( const char* key )
{
	return( ( int )RegDeleteKeyA( hkey, key ) );
}

int CReg::enumKey( int i, OUT char* name, DWORD maxName )
{
	if( !hkey )
	{
		return( -1 );
	}
	return( RegEnumKeyA( hkey, i, name, maxName ) );
}

int CReg::enumValue( int i, OUT char* name, DWORD maxName, void* data, OUT DWORD *sz )
{
	if( !hkey )
	{
		return( -1 );
	}
	return( RegEnumValueA( hkey, i, name, &maxName, 0, 0, ( LPBYTE )data, sz ) );
}