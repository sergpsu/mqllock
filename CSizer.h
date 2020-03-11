#ifndef __CSIZER_H__
#define __CSIZER_H__

#include <AtlColl.h>

#define SIZER_RESIZE_RIGHT	1
#define SIZER_RESIZE_DOWN	2
#define SIZER_MOVE_RIGHT	4
#define SIZER_MOVE_DOWN		8


struct CSizerItem
{
	HWND	hWnd;
	BOOL	resizeRight;
	BOOL	resizeDown;
	BOOL	moveRight;
	BOOL	moveDown;
	int		x;
	int		y;
	int		w;
	int		h;
	int		dx;
	int		dy;
};

class CSizer
{
	protected:
		HWND	m_hDlg;
		CSimpleArray<CSizerItem> m_items;
		int m_clientX, m_clientY;

	protected:
		int findItem( CSizerItem* item );

	public:
		BOOL AddItem( HWND item, DWORD flags );
		INT_PTR ProcessMessage( HWND hDlg, UINT msg, WPARAM wp, LPARAM lp );
		CSizerItem* Find( HWND obj );
		BOOL Remove( HWND obj );

	public:
		CSizer( HWND hDlg );
};

#endif