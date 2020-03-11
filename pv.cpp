#include "pv.h"
#include "api.h"

PVRECORD::PVRECORD( const TCHAR* key, PVRECORDTYPE type )
{
	init( key );
	this->type = type;
}

PVRECORD::PVRECORD( const TCHAR* key, const TCHAR* value )
{
	init( key );

	type = PV_STRING;
	this->value.s = new TCHAR[ lstrlen( value ) + 1 ];
	lstrcpy( this->value.s,  value );
}

PVRECORD::PVRECORD( const TCHAR* key, int value )
{
	init( key );

	type = PV_INTEGER;
	this->value.d = value;
}

PVRECORD::PVRECORD( const TCHAR* key, double value )
{
	init( key );

	type = PV_DOUBLE;
	this->value.f = value;
}

PVRECORD::~PVRECORD()
{
	free();
}

PVRECORD& PVRECORD::operator=( PVRECORD& pvr )
{
	free();

	init( pvr.key );
	type = pvr.type;
	if( type == PV_STRING )
	{
		if( pvr.value.s )
		{
			value.s = new TCHAR[ lstrlen( pvr.value.s ) + 1 ];
			lstrcpy( value.s, pvr.value.s );
		}
	}
	else if( type == PV_INTEGER )
	{
		value.d = pvr.value.d;
	}
	else if( type == PV_DOUBLE )
	{
		value.f = pvr.value.f;
	}

	return( *this );
}

void PVRECORD::init( const TCHAR* key )
{
	value.s = 0;
	this->key = new TCHAR[ lstrlen( key ) + 1 ];
	lstrcpy( this->key, key );
}

void PVRECORD::free()
{
	delete[] key;
	if( type == PV_STRING && value.s )
	{
		delete[] value.s;
	}
}


void PVDATA::init( BOOL memory )
{
	nLastError = 0;
	closing = FALSE;
	this->memory = memory;
	hstr = CreateMutex( 0, 0, 0 );
	hqueue = CreateMutex( 0, 0, 0 );
}

PVDATA::PVDATA()
{
	init( TRUE );
}

PVDATA::PVDATA( HKEY hkey )
{
	init( FALSE );

	this->hkey = hkey;
	hQueueThread = myCreateThread( ( LPTHREAD_START_ROUTINE )queueWriteProc, this, "PVD:q" );
}

PVDATA::~PVDATA()
{
	close();

	CloseHandle( hstr );
	CloseHandle( hqueue );
	if( !memory )
	{
		CloseHandle( hQueueThread );
	}
}

// =============
// release
// =============
void PVDATA::release()
{
	ECS( hstr );
	int n = strings.GetSize();
	for( int i = 0; i < n; i ++ )
	{
		delete[] strings[ i ];
	}
	strings.RemoveAll();
	LCS( hstr );
} // release

// ===============
// releaseOne
// ===============
int PVDATA::releaseOne( TCHAR *str )
{
	ECS( hstr );
	int i = strings.Find( str );
	if( i != -1 )
	{
		delete[] str;
		strings.RemoveAt( i );
	}
	LCS( hstr );
	return( i );
} // releaseOne

// ============
// close
// ============
int PVDATA::close()
{
	dbg( "PVDATA::close()" );

	release();
	closing = TRUE;
dbg( "waiting for %x", hQueueThread );
	WaitForSingleObject( hQueueThread, 500 );
dbg( "done" );
	return( RegCloseKey( hkey ) );
} // close

// ===============
// write( TCHAR* )
// ===============
int PVDATA::write( const TCHAR* key, const TCHAR* value )
{
	PVRECORD *pvr = new PVRECORD( key, value );
	return( write( pvr ) );
} // write

// ===============
// write( int )
// ===============
int PVDATA::write( const TCHAR* key, int value )
{
	PVRECORD *pvr = new PVRECORD( key, value );
	return( write( pvr ) );
} // write

// ===============
// write( double )
// ===============
int PVDATA::write( const TCHAR* key, double value )
{
	PVRECORD *pvr = new PVRECORD( key, value );
	return( write( pvr ) );
} // write

// ===========
// write( PVRECORD* )
// ===========
int PVDATA::write( PVRECORD* pvr )
{
	ECS( hqueue );
	
	queue.Add( pvr );

	LCS( hqueue );

	return( 0 );
} // write

// ===============
// queueWriteProc
// ===============
DWORD WINAPI PVDATA::queueWriteProc( PVDATA* owner )
{
	BOOL end = FALSE;
	do
	{
		ECS( owner->hqueue );

		if( owner->queue.GetCount() > 0 )
		{
			PVRECORD *pvr = owner->queue[ 0 ];

			switch( pvr->type )
			{
				case PVRECORD::PV_STRING:
					RegSetValueEx( owner->hkey, pvr->key, 0, REG_SZ, ( LPBYTE )pvr->value.s, ( lstrlen( pvr->value.s ) + 1 ) * sizeof( TCHAR ) );
					break;

				case PVRECORD::PV_INTEGER:
					RegSetValueEx( owner->hkey, pvr->key, 0, REG_DWORD, ( LPBYTE )&pvr->value.d, sizeof( pvr->value.d ) );
					break;

				case PVRECORD::PV_DOUBLE:
					RegSetValueEx( owner->hkey, pvr->key, 0, REG_BINARY, ( LPBYTE )&pvr->value.f, sizeof( pvr->value.f ) );
					break;

				case PVRECORD::PV_DELETE:
					RegDeleteValue( owner->hkey, pvr->key );
					break;
			}

			owner->queue.RemoveAt( 0 );
			delete pvr;
		}

		if( owner->closing && owner->queue.GetCount() == 0 )
		{
			end = TRUE;
		}

		LCS( owner->hqueue );
		
		Sleep( 100 );
	}
	while( !end );
	dbg( "queue thread finished" );
	return( 0 );
} // queueWriteProc

// ==============
// read( TCHAR** )
// ==============
int PVDATA::read( const TCHAR* key, OUT TCHAR** value )
{
	PVRECORD pvr( key, PVRECORD::PV_STRING );
	int res = read( &pvr );
	if( res == 0 )
	{
		*value = new TCHAR[ lstrlen( pvr.value.s ) + 1 ];
		lstrcpy( *value, pvr.value.s );

		ECS( hstr );
		strings.Add( *value );
		LCS( hstr );
	}
	return( res );
} // read

// ==========
// read( int* )
// ============
int PVDATA::read( const TCHAR* key, OUT int* value )
{
	PVRECORD pvr( key, PVRECORD::PV_INTEGER );
	int res = read( &pvr );
	if( res == 0 )
	{
		*value = pvr.value.d;
	}
	return( res );
} // read

// ===============
// read( double* )
// ===============
int PVDATA::read( const TCHAR* key, OUT double* value )
{
	PVRECORD pvr( key, PVRECORD::PV_DOUBLE );
	int res = read( &pvr );
	if( res == 0 )
	{
		*value = pvr.value.f;
	}
	return( res );
} // read

// ================
// read( PVRECORD )
// ================
int PVDATA::read( PVRECORD* pvr )
{
dbg( "PVDATA::read( PVRECORD* pvr )" );
	int res = ERROR_NOT_FOUND;
	ECS( hqueue );

	//scan queue first
	int n = ( int )queue.GetCount();
dbg( "queue	size=%d", n );
	int i;
	for( i = n - 1; i >= 0; i -- )
	{
		if( queue[ i ]->type == pvr->type &&
			!lstrcmp( queue[ i ]->key, pvr->key )
			)
		{
			res = 0;
			*pvr = *queue[ i ];
			break;
		}
	}
dbg( "i=%d, memory=%d", i, memory );	
dbg( "pvr->type=%d", pvr->type );
	if( i < 0 && !memory )
	{
		DWORD sz;
		BYTE *out;
		switch( pvr->type )
		{
			case PVRECORD::PV_STRING:
				RegQueryValueEx( hkey, pvr->key, 0, 0, 0, &sz );
				pvr->value.s = new TCHAR[ sz / sizeof( TCHAR ) ];
				if( pvr->value.s )
				{
					out = ( LPBYTE )pvr->value.s;
				}
				else
				{
					res = ERROR_NOT_ENOUGH_MEMORY;
					sz = 0;
				}
				break;
			
			case PVRECORD::PV_INTEGER:
				sz = sizeof( int );
				out = ( LPBYTE )&pvr->value.d;
				break;

			case PVRECORD::PV_DOUBLE:
				sz = sizeof( double );
				out = ( LPBYTE )&pvr->value.f;
				break;
		}

		if( sz )
		{
dbg( "RegQueryValueEx( %x, '%s', 0, 0, %x, %x )", hkey, pvr->key, out, &sz );
			res = RegQueryValueEx( hkey, pvr->key, 0, 0, out, &sz );
dbg( "res=%d", res );
			if( pvr->type == PVRECORD::PV_STRING && res != 0 )
			{
				delete[] pvr->value.s;
			}
		}
	}
	LCS( hqueue );
	return( res );
} // read

// =========
// del
// =========
int PVDATA::del( const TCHAR* key )
{
	PVRECORD *pvr = new PVRECORD( key, PVRECORD::PV_DELETE );
	return( write( pvr ) );
} // del

// ============
// isMemory
// ============
BOOL PVDATA::isMemory()
{
	return( memory );
} // isMemory

// ================
// enumerate
// ===============
LONG PVDATA::enumerate( DWORD i, OUT TCHAR** name )
{
	DWORD maxlen;
	LONG lres = RegQueryInfoKey( hkey, 0, 0, 0, 0, 0, 0, 0, &maxlen, 0, 0, 0 );
	if( lres == ERROR_SUCCESS )
	{
		maxlen ++;
		*name = new TCHAR[ maxlen ];
		
		lres = RegEnumValue( hkey, i, *name, &maxlen, 0, 0, 0, 0 );
		if( lres == ERROR_SUCCESS )
		{
			ECS( hstr );
			strings.Add( *name );
			LCS( hstr );
		}
		else
		{
			delete[] *name;
		}
	}
	return( lres );
} // enumerate

// ===========
// count
// ===========
int PVDATA::count()
{
	int res;
	LONG lres = RegQueryInfoKey( hkey, 0, 0, 0, 0, 0, 0, ( DWORD* )&res, 0, 0, 0, 0 );
	if( lres != ERROR_SUCCESS )
	{
		res = - lres;
	}
	return( res );
} // count