#include "rv.h"
#include "api.h"
#include "CBaseServerManager.h"
#include "Base64_rc4.h"

CRV::CRV( const BYTE* xmlKey, DWORD keyLen )
{
	m_xml = 0;
	m_xmlKey = ( BYTE* )malloc( keyLen );
	memcpy( m_xmlKey, xmlKey, keyLen );
	m_keyLen = keyLen;
}

CRV::~CRV()
{
	if( m_xml )
	{
		delete m_xml;
	}
	delete m_xmlKey;
	m_err = ML_OK;
}

#ifdef MLL
// =============
// auth
// =============
BOOL CRV::auth( const TCHAR* TVAR( authkey ) )
{
	U2A( authkey );

dbg( "CRV::auth(%s)", authkey );
	BOOL res = FALSE;
	
	CBaseServerManager mgr;

	CRC4 rc4;
	/*/ml/rv.php?auth=*/
	BYTE key[4]={0xd6,0xda,0xd3,0x4f};
	BYTE str[17]={0x61,0x60,0xcd,0xaf,0x5d,0x37,0xc8,0x29,0x94,0x37,0xf3,0x3f,0xe3,0x0e,0x89,0xbf,0xbf};
	rc4.Decrypt( str, sizeof( str ), key, sizeof( key ) );

	char *uri = ( char* )mymalloc( MAX_URI_LENGTH );
	strcpy_s( uri, MAX_URI_LENGTH, ( char* )str );
	rc4.Encrypt( str, sizeof( str ), key, sizeof( key ) );
	strcat_s( uri, MAX_URI_LENGTH, authkey );

	/*mqllock.com*/
	BYTE key2[4]={0xd2,0x91,0xcb,0xae};
	BYTE str2[12]={0xf7,0x6a,0xbb,0x44,0xcf,0xcc,0x33,0x71,0xd4,0x88,0x89,0x8f};
	rc4.Decrypt( str2, sizeof( str2 ), key2, sizeof( key2 ) );
	CBaseServerManagerHandle *h = mgr.load( ( char* )str2, 443, uri, TRUE );
	rc4.Encrypt( str2, sizeof( str2 ), key2, sizeof( key2 ) );
dbg( "uri=%s", uri );
	free( uri );
	DWORD waited = h->wait( INFINITE );
dbg( "waited auth, waited=%d, h->error=%d, h->status=%d, h->cl=%d", waited, h->error, h->status, h->cl );
	if( waited == WAIT_OBJECT_0 && h->error == ML_OK && h->status == 200 )
	{
		//read xml, decrypt, parse
		char* raw = ( char* )mymalloc( h->cl + 1 );
		DWORD sz;
		if( InternetReadFile( h->hr, raw, h->cl, &sz ) && sz == h->cl )
		{	
dbg( "read %d bytes of rv xml", sz );
			if( raw[ 0 ] == '0' && raw[ 1 ] == '0' )
			{
				// --------------
				// decrypt xml
				// --------------
				CRC4 rc4;
dbg( "decrypting" );
				rc4.Decrypt( ( BYTE* )raw + 2, h->cl - 2, ( const BYTE* )( const char* )authkey, strlen( authkey ) );
dbg( "decrypted" );
				raw[ h->cl ] = 0;
dbg( "xml loaded: '%s'", raw );

				// -------------
				// parse xml
				// -------------
				m_xml = new CXMLNode;
				res = m_xml->Load( raw + 2 );
				free( raw );
				raw = 0;
dbg( "rv xml parsed %d", res );
				if( !res )
				{
					delete m_xml;
				}
			}
			else
			{
				switch( atoi( raw ) )
				{
					case -1:
						m_err = ML_INVALID_AUTH;
						break;

					case -2:
						m_err = ML_INVALID_PARAMETER;
						break;

					default:
						m_err = ML_UNKNOWN;
				}
			}
		}
		if( raw )
		{
			free( raw );
		}
	}
	else
	{
		m_err = h->error;
	}
	delete h;
	return( res );
} // auth

#else
// =============
// loadXml
// =============
BOOL CRV::loadXml( const CXMLNodeEnc* xml )
{
	BOOL res = FALSE;
	
	/*rv*/
	BYTE keyRv[4]={0xa1,0x25,0xa2,0xbe};
	BYTE strRv[3]={0x6c,0x5f,0x65};
	CRC4 rc4;
	rc4.Decrypt( strRv, sizeof( strRv ), keyRv, sizeof( keyRv ) );
	const CXMLNodeEnc* rv = xml->GetChildA( ( const char* )strRv, m_xmlKey, m_keyLen );
	rc4.Encrypt( strRv, sizeof( strRv ), keyRv, sizeof( keyRv ) );

	if( rv )
	{
dbg( "rv node exists" );

		/*vars*/
		BYTE keyVars[4]={0xf8,0xac,0xbc,0x76};
		BYTE strVars[5]={0xc3,0xd6,0xbb,0x53,0x14};

		rc4.Decrypt( strVars, sizeof( strVars ), keyVars, sizeof( keyVars ) );
		rv = rv->GetChildA( ( const char* )strVars, m_xmlKey, m_keyLen );
		rc4.Encrypt( strVars, sizeof( strVars ), keyVars, sizeof( keyVars ) );

		if( rv )
		{
dbg( "vars node exists" );
			if( m_xml )
			{
				delete m_xml;
			}
			m_xml = new CXMLNodeEnc( rv );

			res = TRUE;
		}
	}

	return( res );
} // loadXml
#endif

// ============
// exists
// ============
BOOL CRV::exists( const TCHAR* name )
{
	return( findVar( name ) != 0 );
} // exists

// ==========
// getDouble
// ==========
double CRV::getDouble( const TCHAR* name )
{
	if( !m_xml )
	{
		m_err = ML_INVALID_HANDLE;
		return( 0.0 );
	}
	const CXMLNodeEnc* node = findVar( name );
	if( !node )
	{
		m_err = ML_NO_DATA;
		return( 0.0 );
	}
	const TCHAR* value = node->GetChildText( _T( "value" ), m_xmlKey, m_keyLen );
	if( !value )
	{
		m_err = ML_UNKNOWN;
		return( 0.0 );
	}

	return( _tstof( value ) );

} // getDouble

// ==========
// getInt
// ==========
int	CRV::getInt( const TCHAR* name )
{
	if( !m_xml )
	{
		m_err = ML_INVALID_HANDLE;
		return( 0 );
	}
	const CXMLNodeEnc* node = findVar( name );
	if( !node )
	{
		m_err = ML_NO_DATA;
		return( 0 );
	}
	const TCHAR* value = node->GetChildText( _T( "value" ), m_xmlKey, m_keyLen );
	if( !value )
	{
		m_err = ML_UNKNOWN;
		return( 0 );
	}
	return( _tstoi( value ) );
} // getInt

// ==============
// getString
// ==============
const TCHAR* CRV::getString( const TCHAR* name )
{
//	const char* s = "°¡´ïËþ";

	//return( s );
	//BYTE b[] = {0xb0, 0xa1, 0xb4, 0xaf, 0x8b, 0xbe, 0x00, 0x00 };
	//CBase64 b64;
	//b64.Decrypt(
//return( ( const char* )b );
dbg( L"CRV::getString '%s'", name );
	if( !m_xml )
	{
dbg( "norvxml" );
		m_err = ML_INVALID_HANDLE;
		return( _T( "" ) );
	}
dbg( "getting node" );
	const CXMLNodeEnc* node = findVar( name );
dbg( "got node %08x", node );
	if( !node )
	{
dbg( "norvnode" );
		m_err = ML_NO_DATA;
		return( _T( "" ) );
	}
	const TCHAR* value = node->GetChildText( _T( "value" ), m_xmlKey, m_keyLen );
	if( !value )
	{
dbg( "norvvalue" );
		m_err = ML_UNKNOWN;
		return( _T( "" ) );
	}
dbg( "getString res '%s'", value );
/*char str[ 100 ];
for( int i = 0; value[ i ]; i ++ )
{
	DWORD d = value[ i ];
	wsprintf( str + i * 2, "%02x", d & 0xff );
}
	MessageBox( 0, str, "", 0 );
*/

	return( value );
} // getString

// =============
// getDate
// =============
long CRV::getDate( const TCHAR* name )
{
	if( !m_xml )
	{
		m_err = ML_INVALID_HANDLE;
		return( 0 );
	}
	const CXMLNodeEnc* node = findVar( name );
	if( !node )
	{
		m_err = ML_NO_DATA;
		return( 0 );
	}
	const TCHAR* value = node->GetChildText( _T( "value" ), m_xmlKey, m_keyLen );
	if( !value )
	{
		m_err = ML_UNKNOWN;
		return( 0 );
	}

	return( _tstol( value ) );
} // getDate

// =============
// getBool
// =============
BOOL CRV::getBool( const TCHAR* name )
{
	if( !m_xml )
	{
		m_err = ML_INVALID_HANDLE;
		return( 0 );
	}
	const CXMLNodeEnc* node = findVar( name );
	if( !node )
	{
		m_err = ML_NO_DATA;
		return( 0 );
	}
	const TCHAR* value = node->GetChildText( _T( "value" ), m_xmlKey, m_keyLen );
	if( !value )
	{
		m_err = ML_UNKNOWN;
		return( 0 );
	}

	return( _tstol( value ) ? TRUE : FALSE );
} // getBool

// ===============
// findVar
// ===============
const CXMLNodeEnc* CRV::findVar( const TCHAR* name )
{
dbg( "CRV::findVar '%s'", name );
	CXMLNodesEnc children = m_xml->GetChildren( NULL, m_xmlKey, m_keyLen );

	int n = children.GetSize();
dbg( "got %d children", n );
	int i;
	const CXMLNodeEnc* res = 0;
	for( i = 0; i < n; i ++ )
	{
dbg( "i=%d", i );
		const CXMLNodeEnc *nodeName = children[ i ]->GetChild( _T( "name" ), m_xmlKey, m_keyLen );
dbg( "name node %08x", nodeName );
		if( nodeName && _tcsicmp( name, nodeName->GetText( m_xmlKey, m_keyLen ) ) == 0 )
		{
			res = children[ i ];
			break;
		}
	}
	return( res );
} // findVar
