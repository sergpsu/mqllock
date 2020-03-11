#include <windows.h>
#include "CSizer.h"

// ========================
// CSizer
// ========================
CSizer::CSizer( HWND hDlg ):
	m_hDlg( hDlg )
{
	RECT r;
	RECT cr;
	GetWindowRect( hDlg, &r );
	GetClientRect( hDlg, &cr );

	int borderWidth = ( r.right - r.left - cr.right ) / 2;
	m_clientX = borderWidth;
	m_clientY = r.bottom - r.top - borderWidth - cr.bottom;
} // CSizer

// ==========================================
// AddItem
// ==========================================
BOOL CSizer::AddItem( HWND hWnd, DWORD flags )
{
	CSizerItem item;
	item.hWnd = hWnd;

	RECT r;
	GetWindowRect( item.hWnd, &r );

	RECT wr;
	GetWindowRect( m_hDlg, &wr );
	int w = wr.right - wr.left;
	int h = wr.bottom - wr.top;

	item.resizeRight = ( flags & SIZER_RESIZE_RIGHT );
	item.resizeDown = ( flags & SIZER_RESIZE_DOWN );
	item.moveRight = ( flags & SIZER_MOVE_RIGHT );
	item.moveDown = ( flags & SIZER_MOVE_DOWN );
	item.dx = wr.right - r.right;
	item.dy = wr.bottom - r.bottom;
	item.w = r.right - r.left;
	item.h = r.bottom - r.top;
	item.x = wr.right - wr.left - item.dx - item.w - m_clientX;
	item.y = wr.bottom - wr.top - item.dy - item.h - m_clientY;

	int i;
	i = findItem( &item );
	if( i == -1 )
	{
		m_items.Add( item );
	}
	else
	{
		m_items.SetAtIndex( i, item );
	}
	return( TRUE );
} // AddItem

// =========================================
// ProcessMessage
// =========================================
INT_PTR CSizer::ProcessMessage( HWND hDlg, UINT msg, WPARAM wp, LPARAM lp )
{
	if( hDlg != m_hDlg )
	{
		return( FALSE );
	}
	int i;
	int n;
	int x, y, w, h;
	RECT r;
	switch( msg )
	{
		case WM_SIZE:
			n = m_items.GetSize();
			GetWindowRect( m_hDlg, &r );
			for( i = 0; i < n; i ++ )
			{
				CSizerItem &item = m_items[ i ];
				x = item.x;
				y = item.y;
				w = item.w;
				h = item.h;
				if( item.moveRight )
				{
					x = r.right - r.left - item.dx - item.w - m_clientX;
				}
				if( item.moveDown )
				{
					y = r.bottom - r.top - item.dy - item.h - m_clientY;
				}
				if( item.resizeRight )
				{
					w = r.right - r.left - item.dx - item.x - m_clientX;
				}
				if( item.resizeDown )
				{
					h = r.bottom - r.top - item.dy - item.y - m_clientY;
				}
				SetWindowPos( item.hWnd, 0, x, y, w, h, SWP_NOZORDER );
			}
			InvalidateRect( m_hDlg, 0, FALSE );
			break;

		default:
			return( FALSE );

	}
	return( TRUE );
} // ProcessMessage

// ========================================
// findItem
// ========================================
int CSizer::findItem( CSizerItem* item )
{
	int i;
	int n = m_items.GetSize();
	for( i = 0; i < n; i ++ )
	{
		if( m_items[ i ].hWnd == item->hWnd )
		{
			return( i );
		}
	}
	return( -1 );
} // findItem

// ===========
// Find
// ===========
CSizerItem* CSizer::Find( HWND obj )
{
	CSizerItem item;
	item.hWnd = obj;
	int i = findItem( &item );
	if( i != -1 )
	{
		return( &m_items[ i ] );
	}
	return( NULL );
} // Find

// ============
// Remove
// ============
BOOL CSizer::Remove( HWND obj )
{
	CSizerItem item;
	item.hWnd = obj;
	int i = findItem( &item );
	if( i != -1 )
	{
		m_items.RemoveAt( i );
		return( TRUE );
	}
	return( FALSE );

} // Remove