#include "CLinkHandle.h"
#include "api.h"

CLinkHandle::CLinkHandle( HTREEITEM item, CLinkHandle* parent, const TCHAR* text, const TCHAR* url )
{
	int len = lstrlen( text ) + 1;
	m_text = ( TCHAR* )mymalloc( len * sizeof( TCHAR ) );
	memcpy( m_text, text, len );
	m_parent = parent;

	m_url = 0;
	setUrl( url );

	m_item = item;
}

CLinkHandle::~CLinkHandle()
{
	free( m_url );
	free( m_text );
}

void CLinkHandle::setUrl( const TCHAR* url )
{
	int len = lstrlen( url ) + 1;
	m_url = ( TCHAR* )realloc( m_url, len * sizeof( TCHAR ) );
	memcpy( m_url, url, len );
}

CLinkHandle* CLinkHandle::getChild( const TCHAR* text )
{
	CLinkHandle* res = 0;
	DWORD n = m_children.GetSize();
	for( DWORD i = 0; i < n; i ++ )
	{
		if( !StrCmpI( m_children[ i ]->m_text, text ) )
		{
			res = m_children[ i ];
			break;
		}
	}
	return( res );
}