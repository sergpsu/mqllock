#include "xmlparser.h"
#include "api.h"

CXMLNode::CXMLNode()
{
dbg( "CXMLNode() %p", this );
	name = 0;
	text = 0;
#ifdef UNICODE
	atext = 0;
#endif
}

CXMLNode::CXMLNode( const CXMLNode* xml )
{
dbg( "CXMLNode(const CXMLNode*) %p", this );
	int len;
	const TCHAR* str = xml->GetName();
	if( str )
	{
		len = ( _tcslen( str ) + 1 ) * sizeof( TCHAR );
		name = ( TCHAR* )mymalloc( len );
		memcpy( name, str, len );
	}
	else
	{
		name = 0;
	}
	str = xml->GetText();
	if( str )
	{
		len = ( _tcslen( str ) + 1 ) * sizeof( TCHAR );
		text = ( TCHAR* )mymalloc( len );
		memcpy( text, str, len );
	}
	else
	{
		text = 0;
	}
#ifdef UNICODE
	atext = 0;
#endif

	const CXMLNodes ch = xml->GetChildren();
	int n = ch.GetSize();
	int i;
	for( i = 0; i < n; i ++ )
	{
		children.Add( new CXMLNode( ch[ i ] ) );
	}
}

CXMLNode::~CXMLNode()
{
	Close<CXMLNode>();
}

// ============
// parseName
// ============
BOOL CXMLNode::parseName( IN OUT char**_p, OUT TCHAR**name, OUT BOOL *closed )
{
//dbg( "pn" );
	char *p = *_p;
	while( *p && !isspace( *p & 0xff ) && *p != '>' )
	{
		p ++;
	}
	BOOL res;
	char *end = p;
	if( isspace( *p & 0xff ) )
	{
		while( *p && *p != '>' )
		{
			p ++;
		}
	}
	if( *( p - 1 ) == '/' )
	{
		*closed = TRUE;
	}
	else
	{
		*closed = FALSE;
	}

	res = ( *p == '>' );
	if( res )
	{
		int len = ( int )( end - *_p );
#ifndef UNICODE
		*name = ( char* )mymalloc( len + 1 );
		memcpy( *name, *_p, len );
		( *name )[ len ] = 0;
#else
		char* _name = ( char* )mymalloc( len + 1 );
		memcpy( _name, *_p, len );
		_name[ len ] = 0;
//dbg( "parsed node name '%s'", _name );
		*name = ( wchar_t* )mymalloc( ( len + 1 ) * sizeof( wchar_t ) );
		utf8ToUcs2( _name, len + 1, *name );
		free( _name );
//dbg( "pn2" );
#endif
		*_p = p + 1;
	}
	return( res );
} // parseName

// =============
// parseText
// =============
BOOL CXMLNode::parseText( IN OUT char**_p, OUT TCHAR**text )
{
//dbg( "pt" );
	char* p = *_p;
	BOOL res = FALSE;
	while( *p && *p != '<' )
	{
		p ++;
	}
	if( *p == '<' )
	{
		int len = ( int )( p - *_p );
#ifndef UNICODE
		*text = ( char* )mymalloc( len + 1 );
		memcpy( *text, *_p, len );
		( *text )[ len ] = 0;
		unescape( text );
		utf8ToAscii( text );
#else
		char* _text= ( char* )mymalloc( len + 1 );
		memcpy( _text, *_p, len );
		_text[ len ] = 0;
//dbg( "parsed node name '%s'", _name );
		*text = ( wchar_t* )mymalloc( ( len + 1 ) * sizeof( wchar_t ) );
		utf8ToUcs2( _text, len + 1, *text );
		free( _text );
//dbg( "pt2" );
#endif

		
		*_p = p;
		res = TRUE;
	}
	return( res );
} // parseText


// ============
// unescape
// ============
void CXMLNode::unescape( char** str )
{
	//char *from[ 5 ] = { "&apos;", "&quot;", "&lt;", "&gt;", "&amp;" };
	//char to[5] = { '\'', '"', '<', '>', '&' };
	char* p = *str;
	BOOL updated = 0;
	BOOL isAmp = 0;
	BOOL parsedAmp = 0;
	for( p = *str; *p; p ++ )
	{
		parsedAmp = 0;
		if( *p == '&' )
		{
			int plen = strlen( p + 1 );
			
			if( plen >= 5 && memcmp( p + 1, "apos;", 5 ) == 0 )
			{
				*p = '\'';
				memmove( p + 1, p + 5 + 1, plen - 5 + 1 );
				updated = 1;
			}
			else if( plen >= 5 && memcmp( p + 1, "quot;", 5 ) == 0 )
			{
				*p = '"';
				memmove( p + 1, p + 5 + 1, plen - 5 + 1 );
				updated = 1;
			}
			else if( plen >= 3 && memcmp( p + 1, "lt;", 3 ) == 0 )
			{
				*p = '<';
				memmove( p + 1, p + 3 + 1, plen - 3 + 1 );
				updated = 1;
			}
			else if( plen >= 3 && memcmp( p + 1, "gt;", 3 ) == 0 )
			{
				*p = '>';
				memmove( p + 1, p + 3 + 1, plen - 3 + 1 );
				updated = 1;
			}
			else if( plen >= 4 && memcmp( p + 1, "amp;", 4 ) == 0 )
			{
				//*p = '&';
				memmove( p + 1, p + 4 + 1, plen - 4 + 1 );
				updated = 1;

				parsedAmp = 1;
			}
		}
		else if( *p == '#' && isAmp )
		{
			char* p1 = p - 1;
			for( ++ p; isdigit( *p ); p ++ );
			if( *p == ';' )
			{
				int len;
				int code = atoi( p1 + 2 );
				if( code <= 255 )
				{
					*p1 = ( code & 255 );
					len = 1;
				}
				else
				{
					*p1 = ( code & 255 );
					*( p1 + 1 ) = ( code >> 8 ) & 255;
					len = 2;
				}
				int plen = strlen( p1 + 1 );
				memmove( p1 + len, p + 1, plen - ( p - p1 ) + 1 );

				p = p1 + len - 1;

				updated = 1;
/*#1234;avcder
012345678901
..........11
plen=12
*/
			}
		}
		isAmp = parsedAmp;
	}
	
	if( updated )//string length reduced
	{
		*str = ( char* )realloc( *str, strlen( *str ) + 1 );
	}
} // unescape

// ==========
// load
// ==========
template<class T>
BOOL CXMLNode::load( char**_p )
{
	char*p = *_p;
	int state = WAIT_OPEN_TAG_OPEN_BRAKET;
	BOOL res = FALSE;
	while( *p )
	{
/*char pp[2];
pp[0]=*p;
pp[1]=0;
dbg( "%d %s", ( DWORD )( p - m_str ), pp );
*/
		if( state == WAIT_OPEN_TAG_OPEN_BRAKET )
		{
			if( *p == '<' )
			{
				p ++;
				BOOL closed;
				if( parseName( &p, &name, &closed ) )
				{
					if( closed )
					{
						res = TRUE;
						break;
					}
				}
				else
				{
					break;
				}
				state = WAIT_CLOSE_TAG_OPEN_BRAKET;
			}
			else if( !isspace( *p & 0xff ) )
			{
				break;
			}
			else
			{
				p ++;
			}
		}
/*		else if( state == WAIT_OPEN_TAG_CLOSE_BRAKET )
		{
			if( *p == '>' )
			{
				end = p - 1;
				if( *( p - 1 ) == '/' )
				{
					end --;
				}
				int len = end - st;
				name = new char[ len + 1 ];
				memcpy( name, st, len );
				name[ len ] = 0;

				state = WAIT_CLOSE_TAG_OPEN_BRAKET;
			}
		}
*/
		else if( state == WAIT_CLOSE_TAG_OPEN_BRAKET )
		{
			if( !isspace( *p & 0xff ) )
			{
				if( *p == '<' )	
				{
					if( *( p + 1 ) != '/' )//child open tag
					{
						T *child = new T;
						if( child->load<T>( &p ) )
						{
							children.Add( child );
						}
						else
						{
							delete child;
							break;
						}
					}
					else // my close tag
					{
						TCHAR *closename;
						BOOL closed;
						p += 2;
						if( parseName( &p, &closename, &closed ) && !_tcsicmp( name, closename ) && !closed )
						{
							res = TRUE;
						}
						free( closename );
						break;
					}
				}
				else
				{
					if( !parseText( &p, &text ) )
					{
						break;
					}
					if( *p != '<' || *( p + 1 ) != '/' )
					{
						break;
					}
//dbg( "t0" );
					int len = _tcslen( name );
//dbg( "t1" );
					if( m_len - ( DWORD )( p + 2 - m_str ) <= len ||
						*( p + 2 + len ) != '>' )
					{
						//closing tag name length differs from opening -> invalid xml
						break;
					}
//dbg( "t2" );
#ifdef UNICODE
					int wlen = utf8ToUcs2( p + 2, len );
					wchar_t* wname = ( wchar_t* )malloc( wlen * sizeof( wchar_t ) );
					utf8ToUcs2( p + 2, len, wname );
					register int same = ( memcmp( name, wname, len * sizeof( wchar_t ) ) == 0 ); 
					free( wname );
#else
					register int same = ( memcmp( p + 2, name, len ) == 0 ); 
#endif
					
					if( !same )
					{
						break;
					}
					p += 3 + len;
					res = TRUE;
//dbg( "t3" );
					break;
				}
			}
			else
			{
				p ++;
			}
		}
		else
		{
			p ++;
		}
	}
	*_p = p;
	return( res );
} // load


// ============
// Load
// ============
const char* CXMLNode::m_str;
int CXMLNode::m_len;

template<class T>
BOOL CXMLNode::Load( const char* str )
{
	m_str = str;
	m_len = strlen( str );

	char* p = ( char* )str;
	while( *p && isspace( *p & 0xff ) )
	{
		p ++;
	}
	BOOL res = FALSE;
	if( *p )
	{
		BOOL isValid = FALSE;
		if( *p == '<' )
		{
			if( *( p + 1 ) == '?' )		// <?xml...?> supposed
			{
				p += 2;
				if( !memcmp( p, "xml", 3 ) )
				{
					while( *p && *p != '?' && *p != '>' )
					{
						p ++;
					}
					if( *p == '?' && *( p + 1 ) == '>' )
					{
						isValid = TRUE;
						p += 2;
					}
				}
			}
			else
			{
				isValid = TRUE;
			}
		}
		if( isValid )
		{
			res = load<T>( &p );
		}
	}
	return( res );
} // Load

const CXMLNode* CXMLNode::GetChild( const TCHAR*name ) const 
{
	dbg( _T( "CXMLNode::GetChild '%s'" ), name );

	int n = children.GetSize();
	int i;
	for( i = 0; i < n; i ++ )
	{
		if( !_tcsicmp( name, children[ i ]->name ) )
		{
dbg( "found" );
			return( children[ i ] );
		}
	}
dbg( "not found" );
	return 0;
}
#ifdef UNICODE
const CXMLNode* CXMLNode::GetChildA( const char*name ) const 
{
	dbg( "CXMLNode::GetChildA '%s'", name );

	int len = utf8ToUcs2( name );
	wchar_t* wname = ( wchar_t* )malloc( ( len + 1 ) * 2 );
	utf8ToUcs2( name, len + 1, wname );
	
	const CXMLNode* res = GetChild( wname );

	free( wname );
	return( res );
}
#endif


CXMLNodes CXMLNode::GetChildren( const TCHAR *name ) const
{
	CXMLNodes res;
	if( name )
	{
		int n = children.GetSize();
		int i;
		for( i = 0; i < n; i ++ )
		{
			if( !_tcsicmp( name, children[ i ]->name ) )
			{
				res.Add( children[ i ] );
			}
		}
	}
	else
	{
		res = children;
	}
	return( res );
}
#ifdef UNICODE
CXMLNodes CXMLNode::GetChildrenA( const char* name ) const
{
	int len = utf8ToUcs2( name );
	wchar_t* wname = ( wchar_t* )malloc( ( len + 1 ) * 2 );
	utf8ToUcs2( name, len + 1, wname );

	CXMLNodes res = GetChildren( wname );

	free( wname );
	
	return( res );
}
#endif // #ifdef UNICODE

const TCHAR* CXMLNode::GetChildText( const TCHAR* name ) const
{
	const CXMLNode *node = GetChild( name );
	if( node )
	{
		return( node->text );
	}
	return 0;
}
#ifdef UNICODE
const char* CXMLNode::GetChildTextA( const char* name ) const
{
	int len = utf8ToUcs2( name );
	wchar_t* wname = ( wchar_t* )malloc( ( len + 1 ) * 2 );
	utf8ToUcs2( name, len + 1, wname );

	const CXMLNode *node = GetChild( wname );
	free( wname );

	if( node )
	{
		return( node->GetTextA() );
	}
	return 0;
}

const char* CXMLNode::GetTextA() const
{
	if( !atext && text )
	{
		int len = ucs2ToUtf8( text );
		( ( CXMLNode* )this )->atext = ( char* )malloc( len + 1 );
		ucs2ToUtf8( text, len + 1, atext );
	}
	return( atext );
}
#endif

template<class T>
void CXMLNode::Close()
{
	if( name )
	{
		free( name );
		name = 0;
	}
	if( text )
	{
		free( text );
		text = 0;
	}
#ifdef UNICODE
	if( atext )
	{
		free( atext );
		atext = 0;
	}
#endif
	int n = children.GetSize();
	if( n )
	{
		for( int i = 0; i < n; i ++ )
		{
			delete (T*)children[ i ];
		}
		children.RemoveAll();
	}
}

BOOL CXMLNode::RemoveChild( CXMLNode* child )
{
	int n = children.GetSize();
	for( int i = 0; i < n; i ++ )
	{
		if( children[ i ] == child )
		{
			delete child;
			children.RemoveAt( i );
			return( TRUE );
		}
	}
	return( FALSE );
}
/*
// ====================
// ucs2ToAscii
// ====================
int CXMLNode::ucs2ToAscii( const wchar_t* in, int inlen/* = 0* /, char* out/* = 0* / )
{
	if( inlen == 0 )
	{
		inlen = wcslen( in );
	}
	if( out )
	{
		int i;
		for( i = 0; i < inlen + 1; i ++ )
		{
			*out++ = static_cast<char>( *in ++ );
		}
	}
	return( inlen );
} // ucs2ToAscii*/

int CXMLNode::ucs2ToUtf8( const wchar_t* in, int inlen/*=0*/, char* out/* =0*/ )
{
	int res = 0;
	if( !inlen )
	{
		inlen = wcslen( in );
	}

	for( int i = 0; i < inlen; i ++ )
	{
		int ires;
		if( out )
		{
			ires = ucs2_to_utf8( in[ i ], ( BYTE* )out + res );
		}
		else
		{
			ires = ucs2_to_utf8( in[ i ], NULL );
		}
		if( ires != -1 )
		{
			res += ires;
		}
		else
		{
			res = -1;
			break;
		}
	}
	return( res );
}

int CXMLNode::ucs2_to_utf8( int ucs2, unsigned char * utf8 )
{
    if (ucs2 < 0x80)
	{
		if( utf8 )
		{
			utf8[0] = ucs2;
		}
        return 1;
    }
    if (ucs2 >= 0x80  && ucs2 < 0x800)
	{
		if( utf8 )
		{
			utf8[0] = (ucs2 >> 6)   | 0xC0;
			utf8[1] = (ucs2 & 0x3F) | 0x80;
		}
        return 2;
    }
    if (ucs2 >= 0x800 && ucs2 < 0xFFFF)
	{
		if (ucs2 >= 0xD800 && ucs2 <= 0xDFFF)
		{
		    /* Ill-formed. */
			return -1;
		}
		if( utf8 )
		{
			utf8[0] = ((ucs2 >> 12)       ) | 0xE0;
			utf8[1] = ((ucs2 >> 6 ) & 0x3F) | 0x80;
			utf8[2] = ((ucs2      ) & 0x3F) | 0x80;
		}
        return 3;
    }
    if (ucs2 >= 0x10000 && ucs2 < 0x10FFFF)
	{
		if( utf8 )
		{
			/* http://tidy.sourceforge.net/cgi-bin/lxr/source/src/utf8.c#L380 */
			utf8[0] = 0xF0 | (ucs2 >> 18);
			utf8[1] = 0x80 | ((ucs2 >> 12) & 0x3F);
			utf8[2] = 0x80 | ((ucs2 >> 6) & 0x3F);
			utf8[3] = 0x80 | ((ucs2 & 0x3F));
		}
        return 4;
    }
    return -1;
}

// =====================
// utf8ToUcs2
// =====================
int CXMLNode::utf8ToUcs2( const char* in, int inlen /*=0*/, wchar_t* out /*=0*/)
{
	int i;
	int j;
	for( i = j = 0;;)
	{
		int charcode = 0;
		BYTE t = in[ i ];//coded.front();
		i ++;//coded.pop_front();
		if( inlen == 0 && t == 0 ) //function detects end of input
		{
			if( out )
			{
				out[ j ] = 0;
			}
			break;
		}
		if( t < 128 )
		{
			if( out )
			{
				out[ j ] = t;
			}
			j ++;
			if( inlen && i == inlen )// inlen fixed, in bytes
			{
				break;
			}
			continue;
		}
		int high_bit_mask = (1 << 6) -1;
		int high_bit_shift = 0;
		int total_bits = 0;
		const int other_bits = 6;
		while((t & 0xC0) == 0xC0)
		{
			t <<= 1;
			t &= 0xff;
			total_bits += 6;
			high_bit_mask >>= 1; 
			high_bit_shift++;
			charcode <<= other_bits;
			charcode |= in[ i ]/*coded.front()*/ & ((1 << other_bits)-1);
			i ++;//coded.pop_front();
		} 
		charcode |= ((t >> high_bit_shift) & high_bit_mask) << total_bits;

		if( out )
		{
			out[ j ] = charcode;
		}
		j ++;
		if( inlen && i == inlen )// inlen fixed, in bytes
		{
			break;
		}
	}
	return( j );
} // utf8ToUcs2

/*
std::deque<int> CXMLNode::unicode_to_utf8(int charcode)
{
    std::deque<int> d;
    if (charcode < 128)
    {
        d.push_back(charcode);
    }
    else
    {
        int first_bits = 6; 
        const int other_bits = 6;
        int first_val = 0xC0;
        int t = 0;
        while (charcode >= (1 << first_bits))
        {
            {
                t = 128 | (charcode & ((1 << other_bits)-1));
                charcode >>= other_bits;
                first_val |= 1 << (first_bits);
                first_bits--;
            }
            d.push_front(t);
        }
        t = first_val | charcode;
        d.push_front(t);
    }
    return d;
}


int CXMLNode::utf8_to_unicode(std::deque<int> &coded)
{
    int charcode = 0;
    int t = coded.front();
    coded.pop_front();
    if (t < 128)
    {
        return t;
    }
    int high_bit_mask = (1 << 6) -1;
    int high_bit_shift = 0;
    int total_bits = 0;
    const int other_bits = 6;
    while((t & 0xC0) == 0xC0)
    {
        t <<= 1;
        t &= 0xff;
        total_bits += 6;
        high_bit_mask >>= 1; 
        high_bit_shift++;
        charcode <<= other_bits;
        charcode |= coded.front() & ((1 << other_bits)-1);
        coded.pop_front();
    } 
    charcode |= ((t >> high_bit_shift) & high_bit_mask) << total_bits;
    return charcode;
}*/

// *********************************************************************************
//					CXMLNodeEnc
// *********************************************************************************
CXMLNodeEnc::CXMLNodeEnc():
	CXMLNode()
{
dbg( "CXMLNodeEnc %p", this );
	decName = 0;
	decText = 0;
#ifdef UNICODE
	decAText = 0;
#endif
}
CXMLNodeEnc::CXMLNodeEnc( const CXMLNodeEnc* xml )
{
dbg( "CXMLNodeEnc(const CXMLNodeEnc*) %p", this );

	decName = 0;
	decText = 0;
#ifdef UNICODE
	decAText = 0;
	atext = 0;
#endif


	int len;

	// copy node NAME
	const TCHAR* str = xml->GetName();
	if( str )
	{
		len = ( _tcslen( str ) + 1 ) * sizeof( TCHAR );
		name = ( TCHAR* )mymalloc( len );
		memcpy( name, str, len );
	}
	else
	{
		name = 0;
	}

	// copy node VALUE
	str = xml->CXMLNode::GetText();
	if( str )
	{
		len = ( _tcslen( str ) + 1 ) * sizeof( TCHAR );
		text = ( TCHAR* )mymalloc( len );
		memcpy( text, str, len );
	}
	else
	{
		text = 0;
	}

	const CXMLNodes ch = xml->CXMLNode::GetChildren();
	int n = ch.GetSize();
	int i;
	for( i = 0; i < n; i ++ )
	{
		children.Add( new CXMLNodeEnc( static_cast<const CXMLNodeEnc*>( ch[ i ] ) ) );
	}
}
CXMLNodeEnc::~CXMLNodeEnc()
{
	if( decName )
	{
		free( decName );
		decName = 0;
	}
	if( decText )
	{
		free( decText );
		decText = 0;
	}
#ifdef UNICODE
	if( decAText )
	{
		free( decAText );
		decAText = 0;
	}
#endif
	Close<CXMLNodeEnc>();
}

BOOL CXMLNodeEnc::Load( const char* str )
{
	return( CXMLNode::Load<CXMLNodeEnc>( str ) );
}
char* CXMLNodeEnc::enc( const char* text, const BYTE* key, DWORD keyLen ) const
{
	//only node names are encrypted so always do '/' => '*' replacement
dbg( "enc '%s'", text );
	CRC4 rc4;
	CBase64 b64;

	int len = strlen( text );
dbg( "len %d", len );
	char* rc4text = ( char* )malloc( len + 1 );
	memcpy( rc4text, text, len + 1 );
	
	rc4.Encrypt( ( BYTE* )rc4text, len, key, keyLen );
dbg( "encr" );
	int b64len = b64.B64_length( len );
dbg( "b64len %d", b64len );
	char* etext = ( char* )malloc( b64len + 1 );
	
	b64.Encrypt( rc4text, len, etext );
	
	_ASSERT( strlen( etext ) <= b64len );

dbg( "etext '%s'", etext ); 
	free( rc4text );
dbg( "rc4text free" );
	len = strlen( etext );
dbg( "b64len real %d", len );
	for( int i = 0; i < len; i ++ )
	{
		if( etext[ i ] == '/' )
		{
			etext[ i ] = '*';
		}
	}
dbg( "etext fixed '%s'", etext );
	return( etext );
}

char* CXMLNodeEnc::dec( const char* text, const BYTE* key, DWORD keyLen ) const
{
	//only text is decrypted here so no need to add '*' => '/' replacement
	CRC4 rc4;
	CBase64 b64;
dbg( "dec: '%s'", text );
	int len = strlen( text );
dbg( "len %d", len );
	char* res = ( char* )malloc( b64.Ascii_length( len ) + 1 );
	b64.Decrypt( text, len, res );
dbg( "res '%s'", res );
	len = b64.ires;//strlen( res );
dbg( "len2 %d", len );
	rc4.Decrypt( ( BYTE* )res, len, key, keyLen );
dbg( "res2 '%s'", res );
	return( res );
}

const CXMLNodeEnc* CXMLNodeEnc::GetChild( const TCHAR *name, const BYTE* key, DWORD keyLen ) const
{
#ifdef UNICODE
	int len = ucs2ToUtf8( name );
	char* aname = ( char* )malloc( len + 1 );
	ucs2ToUtf8( name, len + 1, aname );

	char* ename = enc( aname, key, keyLen );
	len = strlen( ename );

	free( aname );
	wchar_t* ewname = ( wchar_t* )malloc( ( len + 1 ) * 2 );
	utf8ToUcs2( ename, len + 1, ewname );
	free( ename );
	const CXMLNodeEnc* res = ( const CXMLNodeEnc* )CXMLNode::GetChild( ewname );
	free( ewname );
#else
	int len = strlen( name );
	char* ename = enc( name, key, len, TRUE );
	const CXMLNodeEnc* res = ( const CXMLNodeEnc* )CXMLNode::GetChild( ename );
	free( ename );
#endif

	return( res );
}

#ifdef UNICODE
const CXMLNodeEnc* CXMLNodeEnc::GetChildA( const char* name, const BYTE* key, DWORD keyLen ) const
{
	char* ename = enc( name, key, keyLen );

	const CXMLNodeEnc* res = ( const CXMLNodeEnc* )CXMLNode::GetChildA( ename );

	free( ename );

	return( res );
}
#endif

// =========================
// GetChildren
// =========================
CXMLNodesEnc CXMLNodeEnc::GetChildren( const TCHAR *name/* = NULL*/, const BYTE* key, DWORD keyLen ) const
{
#ifdef UNICODE
	wchar_t* ewname = NULL;
	if( name )
	{
dbg( L"GetChildren '%s'", name );
		int len = ucs2ToUtf8( name );
dbg( "len %d", len );
		char* aname = ( char* )malloc( len + 1 );
		ucs2ToUtf8( name, len + 1, aname );
dbg( "aname '%s'", aname );	
		char* ename = enc( aname, key, keyLen );
dbg( "ename '%s'", ename );
		len = strlen( ename );
dbg( "len %d", len );
		free( aname );
		ewname = ( wchar_t* )malloc( ( len + 1 ) * 2 );
		utf8ToUcs2( ename, len + 1, ewname );
dbg( L"ewname '%s'", ewname );
		free( ename );
	}
	CXMLNodes res = CXMLNode::GetChildren( ewname );
	if( ewname )
	{
		free( ewname );
	}

	int n = res.GetSize();
dbg( "res size %d", n );
	CXMLNodesEnc eres;
	for( int i = 0; i < n; i ++ )
	{
		eres.Add( static_cast<CXMLNodeEnc*>( res[ i ] ) );
	}
dbg( "done" );
#else
	//todo
#endif
	return( eres );
} // GetChildren

#ifdef UNICODE
// ====================
// GetChildrenA
// ====================
CXMLNodesEnc CXMLNodeEnc::GetChildrenA( const char*name/* = NULL*/, const BYTE* key, DWORD keyLen ) const
{
dbg( "GetChildrenA '%s'", name );
	int len = utf8ToUcs2( name );
dbg( "len %d", len );
	wchar_t* wname = ( wchar_t* )malloc( ( len + 1 ) * 2 );
	utf8ToUcs2( name, len + 1, wname );
dbg( L"wname '%s'", wname );

	CXMLNodesEnc res = GetChildren( wname, key, keyLen );
dbg( "res size %d", res.GetSize() );
	free( wname );
dbg( "done" );
	return( res );
} // GetChildrenA
#endif

// ======================
// GetChildText
// ======================
const TCHAR* CXMLNodeEnc::GetChildText( const TCHAR* name, const BYTE* key, DWORD keyLen ) const
{
#ifdef UNICODE
	const wchar_t* res = NULL;
	const CXMLNodeEnc* child = GetChild( name, key, keyLen );
	if( child )
	{
		res = child->GetText( key, keyLen );
	}
#else
	//todo
#endif
	return( res );
} // GetChildText

#ifdef UNICODE 
// ==================
// GetChildTextA
// ==================
const char* CXMLNodeEnc::GetChildTextA( const char* name, const BYTE* key, DWORD keyLen ) const
{
	dbg( "CXMLNodeEnc::GetChildTextA '%s'", name );
	const CXMLNodeEnc *node = GetChildA( name, key, keyLen );
	if( node )
	{
		dbg( "node found" );
		return( node->GetTextA( key, keyLen ) );
	}
	dbg( "!node not found");
	return( NULL );
} // GetChildTextA
#endif

// ===============
// GetText
// ===============
const TCHAR* CXMLNodeEnc::GetText( const BYTE* key, DWORD keyLen ) const
{
#ifdef UNICODE
dbg( L"CXMLNodeEnc::GetText %p", this );
	if( !decText )
	{
dbg( L"GetText '%s'", this, text );
		CXMLNode::GetTextA();//atext is filled
		if( atext )
		{
	dbg( "atext: '%s'", atext );
			( ( CXMLNodeEnc* )this )->decAText = dec( atext, key, keyLen );
			int len = utf8ToUcs2( this->decAText );
			( ( CXMLNodeEnc* )this )->decText = ( wchar_t* )malloc( ( len + 1 ) * 2 );
			utf8ToUcs2( this->decAText, len + 1, this->decText );
		}
	}
	else
	{
dbg( L"decText exist %08x", decText );
dbg( L"decText '%s'", decText );
	}
#endif
	return( decText );
} // GetText

#ifdef UNICODE
// ================
// GetTextA
// ================
const char* CXMLNodeEnc::GetTextA( const BYTE* key, DWORD keyLen ) const
{
	if( !decAText )
	{
		GetText( key, keyLen ); //decAText is filled here
	}
	return( decAText );
} // GetTextA
#endif
