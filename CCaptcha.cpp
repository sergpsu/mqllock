#include "CCaptcha.h"
#define RANGE( a, b ) ( rand() % ( b - ( a ) + 1 ) + a )

CCaptcha::CCaptcha( HWND dst )
{
	m_wnd = dst;
	m_code = 0;
	m_drawn = 0;
	m_dc = 0;
	m_bitmap = 0;

	SetWindowLongPtr( dst, GWLP_USERDATA, ( LONG_PTR )this );
	m_origProc = ( WNDPROC )SetWindowLongPtr( dst, GWLP_WNDPROC, ( LONG_PTR )wndProc );
}

CCaptcha::~CCaptcha()
{
	cleanup();
}

// =================
// generate
// =================
void CCaptcha::generate( int symbols )
{
	static char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int n = strlen( chars );
	char *res = ( char* )malloc( symbols + 1 );
	int i;
	srand( GetTickCount() );
	for( i = 0; i < symbols; i ++ )
	{
		res[ i ] = chars[ rand() % n ];
	}
	res[ i ] = 0;
	cleanup();

	m_drawn = 0;
	m_code = res;
	InvalidateRect( m_wnd, 0, 0 );
} // generate

void CCaptcha::cleanup()
{
	if( m_code )
	{
		free( m_code );
		m_code = 0;
	}
	if( m_dc )
	{
		DeleteDC( m_dc );
		m_dc = 0;
	}
	if( m_bitmap )
	{
		DeleteObject( m_bitmap );
		m_bitmap = 0;
	}
}

// ==================
// wndProc
// ==================
LRESULT WINAPI CCaptcha::wndProc( HWND wnd, UINT msg, WPARAM wp, LPARAM lp )
{
	static CCaptcha *me;
	if( !me )
	{
		me = ( CCaptcha* )GetWindowLongPtr( wnd, GWLP_USERDATA );
	}
	switch( msg )
	{
		case WM_PAINT:
			if( me->m_code )
			{
				RECT r;
				GetClientRect( wnd, &r );

				PAINTSTRUCT ps;
				HDC dc = BeginPaint( wnd, &ps );
				if( !me->m_drawn )
				{
					//drawing here
					int n = strlen( me->m_code );
					int w = r.right / n;

					HPEN bp = ( HPEN )GetStockObject( BLACK_PEN );
					SelectObject( dc, bp );
					
					HBRUSH hbr = CreateHatchBrush( RANGE( 0, 5 ), RANGE( 0x73CEFB, 0x067DB8 ) );
					SelectObject( dc, hbr );
					Rectangle( dc, 0, 0, r.right, r.bottom );
					DeleteObject( hbr );


					SetBkMode( dc, TRANSPARENT );
					for( int i = 0; i < n; i ++ )
					{	
						HFONT hf = CreateFontA( RANGE( r.bottom - 10, r.bottom - 5 ), w - 4, RANGE( 0, 45 ), RANGE( 0, 45 ), 0, 0, 0, 0, ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, "Times New Roman" );
						SelectObject( dc, hf );
						RECT r2;
						r2.left = i * w;
						r2.right = i * w + w;
						r2.top = 0;
						r2.bottom = r.bottom;
						
						DrawTextA( dc, me->m_code + i, 1, &r2, DT_CENTER );
						
						DeleteObject( hf );
					}

					//copying to memory
					me->m_dc = CreateCompatibleDC( dc );
					me->m_bitmap = CreateCompatibleBitmap ( dc, r.right, r.bottom );
					SelectObject( me->m_dc, me->m_bitmap );
					BitBlt( me->m_dc, 0, 0, r.right, r.bottom, dc, 0, 0, SRCCOPY );

					me->m_drawn = 1;
				}
				else
				{
					BitBlt( dc, 0, 0, r.right, r.bottom, me->m_dc, 0, 0, SRCCOPY );
				}
				EndPaint( wnd, &ps );
				return( 0 );
			}
			break;
	}
	return( CallWindowProc( me->m_origProc, wnd, msg, wp, lp ) );
} // wndProc