#pragma once
#include <windows.h>
#include <atlcoll.h>
#include <commctrl.h>

typedef
class CLinkHandle
{
	private:
		TCHAR* m_text;
		TCHAR* m_url;
		HTREEITEM m_item;
		CSimpleArray<CLinkHandle*> m_children;
		CLinkHandle* m_parent;

	public:
		const TCHAR* url(){ return( m_url ); }
		const TCHAR* text(){ return( m_text ); }
		const HTREEITEM hitem() { return( m_item ); }
		const CLinkHandle* parent(){ return( m_parent ); }
		CSimpleArray<CLinkHandle*>& children(){ return( m_children ); }
		void addChild( CLinkHandle *child ){ m_children.Add( child ); }
		void setUrl( const TCHAR* url );
		CLinkHandle* getChild( const TCHAR* text );
		
		

	public:
		CLinkHandle( HTREEITEM item, CLinkHandle* parent, const TCHAR* text, const TCHAR* url );
		~CLinkHandle();
}
*LINK_HANDLE;


