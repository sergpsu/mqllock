#include <windows.h>
#include "CLogger.h"
#include "api.h"
#include "CBaseServerManager.h"

CLogger::CLogger()
{
return; //logger disabled on 14/04/2015
	m_mgr = new CBaseServerManager();

	m_threadCleanup = 0;
	m_hHandles = CreateMutex( 0, 0, 0 );
	m_hDone = CreateEvent( 0, 0, 0, 0 );

	SYSTEMTIME st;
	GetSystemTime( &st );
	srand( GetTickCount() );
	wsprintfA( m_sessId, "%04X%02X%02X%02X%02x%02x%04X%04X%04X%04X%02X", ( DWORD )st.wYear, ( DWORD )st.wMonth, ( DWORD )st.wDay, 
		( DWORD )st.wHour, ( DWORD )st.wMinute, ( DWORD )st.wSecond, ( DWORD )st.wMilliseconds, rand() & 0xffff, rand() & 0xffff, rand() & 0xffff, rand() & 0xff );

}

CLogger::~CLogger()
{
return; //logger disabled on 14/04/2015
	int n = m_handles.GetCount();
dbg( "~CL %d", n );
	int i;
	for( i = 0; i < n; i ++ )
	{
		TerminateThread( m_handles[ i ]->hThread, 0 );
		delete m_handles[ i ];
	}

	CloseHandle( m_hHandles );
	CloseHandle( m_hDone );
	delete m_mgr;
}

DWORD WINAPI CLogger::cleanupThread( CLogger* me )
{
	while( WaitForSingleObject( me->m_hDone, 1000 ) != 0 )
	{
		me->cleanup();
	}
	return( 0 );
}

void CLogger::cleanup()
{
dbg( "CL:cl" );
	ECS( m_hHandles );
	int n = m_handles.GetCount();
dbg( "n%d", n );
	int i;
	for( i = 0; i < n; i ++ )
	{
		if( m_handles[ i ]->wait( 0 ) == 0 )
		{
dbg( "rm %i" );
			delete m_handles[ i ];
			m_handles.RemoveAt( i );
			i --;
			n --;
		}
	}
dbg( "cldn" );
	LCS( m_hHandles );
}

int CLogger::start()
{
return 0; //logger disabled on 14/04/2015
	if( !m_threadCleanup )
	{
		m_threadCleanup = myCreateThread( ( LPTHREAD_START_ROUTINE )cleanupThread, this, "CL:cleanup" );
		return( 0 );
	}
	return( -1 );
}

int CLogger::stop()
{
return 0; //logger disabled on 14/04/2015
	if( m_threadCleanup )
	{
		SetEvent( m_hDone );
		WaitForSingleObject( m_threadCleanup, 5000 );
		CloseHandle( m_threadCleanup );
		m_threadCleanup = 0;
		return( 0 );
	}
	return( -1 );
}

// =============
// urlEncode
// =============
char *CLogger::urlEncode( const char *str )
{
	static char hex[] = "0123456789abcdef";

  char *pstr = ( char* )str;
  char* buf = ( char* )malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    /*else if (*pstr == ' ') 
      *pbuf++ = '+';*/
    else 
      *pbuf++ = '%', *pbuf++ = hex[ ( *pstr >> 4 ) & 15 ], *pbuf++ = hex[ *pstr & 15];
    pstr++;
  }
  *pbuf = '\0';
  return buf;
} // urlEncode

// =============
// submit
// =============
int CLogger::submit( LoggerType t, ... )
{
return 0; //logger disabled on 14/04/2015
	TRY 
dbg( "lgr submit %d", t );
	/*%s/log.php?lic=%s&rev=%d&t=%d&sid=%s*/
	BYTE urlKey[4]={0x45,0x40,0x3b,0x39};
	BYTE urlFmt[37]={0x37,0x58,0xa1,0x89,0x2c,0x9c,0x7f,0xbf,0xb7,0x1e,0x37,0x1a,0x87,0xf9,0x01,0x8c,0xdb,0x5d,0x65,0x26,0x9d,0x63,0xa6,0x22,0x7a,0x4a,0x76,0x9c,0x37,0x1e,0x24,0x48,0x92,0x77,0x03,0x71,0x24};

	Globals* g = getGlobals();
	const char* servers = g->useServers();
	
	for( const char* curServer = servers; *curServer; curServer += strlen( curServer ) + 1 )
	{
		char server[ 100 ];
		INTERNET_PORT port;
		char baseUri[ 100 ];
		char scheme[ 32 ];

dbg( "fd %s", curServer );
		parseUrl( curServer, server, &port, baseUri, scheme );

		char *uri = ( char* )mymalloc( 512 );

		CRC4 rc4;
		rc4.Decrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

		const char* lic = g->useLicId();
		wsprintfA( uri, ( char* )urlFmt, baseUri, lic, g->getRevision(), t, m_sessId );
		g->unuse( lic );

		rc4.Encrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

		va_list list;
		va_start( list, t );
		switch( t )
		{
			case LGT_AU_FAILED:
			{
				//one int param expected
				int d = va_arg( list, int );
				uri = ( char* )realloc( uri, strlen( uri ) + 64 );
				wsprintfA( uri, "%s&s1=%d", uri, d );
			}
				break;

			case LGT_AU_CANCELLED:	
			case LGT_DLL_INIT:
			case LGT_AU_OK:
				//no params expected
				break;

			default:
			{
				//variable params count
				for( int i = 1; ; i ++ )
				{
					const char* s = va_arg( list, char* );
					if( !s )
					{
						break;
					}
					char *safe = urlEncode( s );
					uri = ( char* )realloc( uri, strlen( uri ) + strlen( safe ) + 64 );
					wsprintfA( uri, "%s&s%d=%s", uri, i, s );
					free( safe );
				}
			}
		}
		va_end( list );

dbg( "u %s %s:%d", uri, server, port );

		CBaseServerManagerHandle *h = m_mgr->load( server, port, uri, TRUE );
		free( uri );

		ECS( m_hHandles );
		m_handles.Add( h );
		LCS( m_hHandles );

		break;	//TODO: we always take 1st server and dont check if it's available
	}
	g->unuse( servers );
	CATCH
	return( 0 );
} // submit