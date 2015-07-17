
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE  // needed to use the predefined system icons
#include <windows.h>
#include <CommCtrl.h>
#include <Commdlg.h>

#include <stdio.h>

#include "mbug_2820_gui.rc.h"

#include "mbug.h"
#include "mbug_2810.h"
#include "mbug_2811.h"
#include "mbug_2820.h"
#include "mbug_utils.h"

//-------------------------------------------------------------------------------------------------------

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")

//-------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
HFONT create_point_font( int point_size, LPCTSTR face_name );
void center_window( hWnd );
void show_rec_control( int show );

HWND hMainDlg;
HICON hIcon, hIconOpen;
HBITMAP hBmpDnArrow, hBmpUpArrow;
HFONT hFontBig;

mbug_device instrument;
FILE* logfile;
UINT acq_timer;

//-------------------------------------------------------------------------------------------------------


int WINAPI
WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow )
{
	MSG msg;
	BOOL ret;

	InitCommonControls();
	hMainDlg = CreateDialog(hInst, MAKEINTRESOURCE( IDD_DIALOG ), 0, (DLGPROC) DialogProc);
	if (hMainDlg == 0) { MessageBox(0, "Error creating dialog.", "GUI", MB_OK ); return 1; }

	// Load icons from ressources
	hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_ICON ) );
	hIconOpen = (HICON)LoadImage( hInst, MAKEINTRESOURCE(IDI_ICON_OPEN), IMAGE_ICON, 16,16, LR_DEFAULTCOLOR );
	hBmpDnArrow = (HBITMAP)LoadImage( 0, MAKEINTRESOURCE(OBM_DNARROW), IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED );
	hBmpUpArrow = (HBITMAP)LoadImage( 0, MAKEINTRESOURCE(OBM_UPARROW), IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED );
	// Set icons
	SendMessage(hMainDlg, WM_SETICON, 1, (LPARAM)hIcon); // NOTE: Set main window icon BEFORE showing the window, else it will overlay the caption
	SendDlgItemMessage(hMainDlg, IDC_BUTTON_LOGFILE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconOpen );
	// Set display fonts
	hFontBig = create_point_font( 26, "Arial" );
	SendDlgItemMessage(hMainDlg, IDC_VALUE_TEMP, WM_SETFONT, (WPARAM)hFontBig, 1);
	SendDlgItemMessage(hMainDlg, IDC_VALUE_HUM, WM_SETFONT, (WPARAM)hFontBig, 1);
	SetDlgItemText( hMainDlg, IDC_VALUE_TEMP, " ---.--- \xB0""C");
	SetDlgItemText( hMainDlg, IDC_VALUE_HUM, " ---.--- %");

	center_window( hMainDlg );
	show_rec_control( 0 );
	ShowWindow(hMainDlg, nCmdShow);

	// Message loop
	while((ret = GetMessage(&msg, 0, 0, 0)) != 0)
	{
		if(ret == -1) return -1;
		if(!IsDialogMessage(hMainDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

  return 0;
}



HFONT create_point_font( int point_size, LPCTSTR face_name )
{
	HDC hdc = GetDC( 0 );
	long height = -MulDiv(point_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC( 0, hdc );
	return CreateFont( height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, face_name );
}


void center_window( HWND hWnd )
{
	RECT rc, rcWnd, rcScreen;
	GetWindowRect( GetDesktopWindow(), &rcScreen);
    GetWindowRect( hWnd, &rcWnd );
    CopyRect(&rc, &rcScreen);
	OffsetRect( &rcWnd, -rcWnd.left, -rcWnd.top );
    OffsetRect( &rc, -rc.left, -rc.top );
    OffsetRect( &rc, -rcWnd.right, -rcWnd.bottom );
	SetWindowPos( hWnd, 0, rcScreen.left+(rc.right/2), rcScreen.top+(rc.bottom/2), 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

//-------------------------------------------------------------------------------------------------------
// Event handling

void on_combo_device_dropdown( HWND hCombo );
void on_combo_device_selchange( HWND hCombo );
void on_button_show_rec( void );
void on_button_start_rec( void );
void on_button_stop_rec( void );
void on_button_logfile( void );
void open_device( const char* id );
void close_device( int verbose );
void close_logfile( void );
void print_status( const char* status );
void print_temp( double temp );
void print_hum( double hum );
void update_measurement( void );
void on_timer( void );


BOOL CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)  // Message type
	{
		case WM_CREATE:  // Before dialog is actually created
			break;

		case WM_INITDIALOG: // Dialog and all controls are now created
			return 1; // focus to default control item

		case WM_COMMAND:
			// HIWORD(wParam)  notification code
			// LOWORD(wParam)  item, control, or accelerator identifier
			// (HWND) lParam   handle of control
			switch (LOWORD(wParam))  // which item?
			{
				case IDC_COMBO_DEVICE:
					switch (HIWORD(wParam))  // which notification code?
					{
						case CBN_DROPDOWN:
							on_combo_device_dropdown( (HWND)lParam );
							return 1;
						case CBN_SELCHANGE:
							on_combo_device_selchange( (HWND)lParam );
							return 1;
					}
					break;

				case IDC_BUTTON_SHOW_REC:  on_button_show_rec();  return 1;
				case IDC_BUTTON_START_REC: on_button_start_rec(); return 1;
				case IDC_BUTTON_STOP_REC:  on_button_stop_rec();  return 1;
				case IDC_BUTTON_LOGFILE:   on_button_logfile();   return 1;
			}
			break;

		case WM_TIMER:
			on_timer();
			break;

		case WM_CLOSE:
			DestroyWindow( hDlg );
			close_logfile();
			close_device( 0 );
			return 1;

		case WM_DESTROY:
			PostQuitMessage(0);
			close_logfile();
			close_device( 0 );
			return 1;
	}

	return 0;
}


// For convenience: some combobox commands
#define  cb_get_selection( hCombo ) (SendMessage( (hCombo), CB_GETCURSEL, 0, 0))
#define  cb_set_selection( hCombo, index ) (SendMessage( (hCombo), CB_SETCURSEL, (WPARAM)(index), 0))
#define  cb_get_text_len( hCombo, index )  (SendMessage( (hCombo), CB_GETLBTEXTLEN, (WPARAM)(index), 0))
#define  cb_get_text( hCombo, index, str )  (SendMessage( (hCombo), CB_GETLBTEXT, (WPARAM)(index), (LPARAM)(LPCSTR)(str)))
#define  cb_reset_content( hCombo ) (SendMessage( (hCombo), CB_RESETCONTENT, 0, 0))
#define  cb_add_string( hCombo, str )  (SendMessage( (hCombo), CB_ADDSTRING, 0, (LPARAM)(LPCSTR)(str)))
#define  cb_insert_string( hCombo, index, str )  (SendMessage( (hCombo), CB_INSERTSTRING, (WPARAM)(index), (LPARAM)(LPCSTR)(str)))
#define  cb_select_string( hCombo, start_after, str ) (SendMessage( (hCombo), CB_SELECTSTRING, (WPARAM)(start_after), (LPARAM)(LPCSTR)(str)))


void on_combo_device_dropdown( HWND hCombo )
{
	mbug_device_list dev_list;
	char sel_str[40] = {0};
	int sel_index, sel_len;
	int i;

	// Remember the current selection
	sel_index = cb_get_selection( hCombo );
	sel_len = cb_get_text_len( hCombo, sel_index );
	if (sel_index != CB_ERR && sel_len < 40)
		cb_get_text( hCombo, sel_index, sel_str );

	// Refill the list
	cb_reset_content( hCombo );

	dev_list = mbug_get_device_list(0);
	for (i=0; dev_list[i]!=0; i++ )
	{
		int t = mbug_type_from_id( dev_list[i] );
		if (t==2810 || t==2811 || t==2820 )
			cb_add_string( hCombo, dev_list[i] );
	}

	cb_insert_string( hCombo, 0, "None" );

	// Reselect the last selection if present
	if (sel_str[0] != 0)
		cb_select_string( hCombo, -1, sel_str );
}


void on_combo_device_selchange( HWND hCombo )
{
	int sel_index = cb_get_selection( hCombo );
	int sel_len = cb_get_text_len( hCombo, sel_index );
	if (sel_index != CB_ERR && sel_len < 40)
	{
		char sel_str[40] = {0};
		cb_get_text( hCombo, sel_index, sel_str );
		if (!lstrcmpi( sel_str, "None" ))
			close_device( 1 );
		else
			open_device( sel_str );
	}
}


void show_rec_control( int show )
{
	// Show/hide dialog bottom where the record controls are located
	SetWindowPos( hMainDlg, 0, -1, -1, 216, show ? 335 : 236, SWP_NOMOVE | SWP_NOREPOSITION );
	SendDlgItemMessage(hMainDlg, IDC_BUTTON_SHOW_REC, BM_SETIMAGE, IMAGE_BITMAP,
	                     (LPARAM)( show ? hBmpUpArrow : hBmpDnArrow ) );
}


void on_button_show_rec( void )
{
	// Toggle rec control display
	RECT rect;
	GetWindowRect( hMainDlg, &rect );
	show_rec_control( rect.bottom - rect.top < 335 );
}


void on_button_logfile( void )
{
	char filename[1000] = "";
	OPENFILENAME ofn = { sizeof(OPENFILENAME), 0, 0, 0, 0, 0, 0, filename, sizeof(filename)-1,
		                  0, 0, 0, "Select logfile",  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
		                  0, 0, "dat", 0, 0, 0 };
	if (GetOpenFileName( &ofn ))
		SetDlgItemText( hMainDlg, IDC_EDIT_LOGFILE, filename );
}


void on_button_stop_rec( void )
{
	if (!logfile)  return;
	close_logfile();
	print_status("Stopped recording");
}


void on_button_start_rec( void )
{
	char filename[1000] = {0};
	double tim = 0;
	int type = 0;

	if (logfile) return;  // Already recording
	if (!instrument)  {
		print_status( "No device opened" );
		return;
	}

	// Get filename
	GetDlgItemText( hMainDlg, IDC_EDIT_LOGFILE, filename, sizeof(filename)-1 );
	if ( !strlen(filename) )  {
		print_status("No data file selected");
		return;
	}

	// Open logfile
	print_status("Opening data file...");
	logfile = fopen( filename, "ac" );  // mode "c" on windows disables additional stream buffer
	if (!logfile)  {
		print_status("Error opening data file");
		return;
	}

	// Write header
	fseek( logfile, 0, SEEK_END );
	if (ftell(logfile) > 0)  fprintf( logfile, "\n\n" );
	fprintf( logfile, "# %s\n", mbug_id( instrument ) );
	tim = floattime();
	fprintf( logfile, "# Start recording at %.2f (%s)\n", tim, strtime(tim) );
	type = mbug_type_from_id( mbug_id( instrument ) );
	switch (type) {
		case 2810:
		case 2811: fprintf( logfile, "# timestamp\ttemperature\n" ); break;
		case 2820: fprintf( logfile, "# timestamp\ttemp\thumidity\n" ); break;
	}
	fflush( logfile );
	print_status("Recording");
}


void open_device( const char* id )
{
	int type = 0;

	close_device( 1 );

	type = mbug_type_from_id( id );
	if (type < 0)  { print_status("Invalid device id"); return; }
	switch (type)
	{
		case 2810: case 2811: case 2820: break;
		default:
			print_status("Unsupported device");
			return;
	}

	print_status("Opening device");
	instrument = mbug_open_str( id );
	if (instrument == 0)  { print_status("Error opening device"); return;}
	print_status("Device opened");

	update_measurement();

	print_status("Start timer");
	acq_timer = SetTimer( hMainDlg, 0, 1000, 0 );
	if (acq_timer == 0) { print_status("Error in SetTimer()"); return; }

	print_status("Ok");

}


void close_device( int verbose )
{
	if (acq_timer != 0)
	{
		if (verbose) print_status("Killing timer");
		KillTimer( hMainDlg, acq_timer );
		acq_timer = 0;
	}

	if (instrument != 0)
	{
		if (verbose) print_status("Closing device");
		mbug_close( instrument );
		instrument = 0;
		if (verbose) print_status("Device closed");
	}
}


void close_logfile( void )
{
	if (logfile) fclose(logfile);
	logfile = 0;
}


void update_measurement( void )
{
	int type = 0;
	double tim = 0, temp = -300, hum = -300;
	static int tog = 1;

	if (instrument == 0) return;

	type = mbug_type_from_id( mbug_id( instrument ) );
	if (type < 0)
	{
		print_status("Invalid device id"); return;
	}
	switch (type)
	{
		case 2810: temp = mbug_2810_read( instrument ); break;
		case 2811: temp = mbug_2811_read( instrument ); break;
		case 2820: mbug_2820_read( instrument, &temp, &hum ); break;
		default:  print_status("Unsupported device"); return;
	}
	tim = floattime();

    switch (type)
    {
		case 2810:  /* temperature only */
		case 2811:
			if (temp <= NOT_A_TEMPERATURE ) {
				print_status("Read error"); return;
			}
			print_temp( temp );
			print_hum( NOT_A_TEMPERATURE );
			ShowWindow( GetDlgItem( hMainDlg, IDC_BEAT_TEMP), tog ? SW_SHOW : SW_HIDE );
			ShowWindow( GetDlgItem( hMainDlg, IDC_BEAT_HUM), SW_HIDE );
			tog = !tog;
			if (logfile) {
				fprintf( logfile, "%.2f\t%.3f\n", tim, temp); fflush( logfile );
			}
			break;

		case 2820:
			if (temp <= NOT_A_TEMPERATURE || hum < 0 ) {
				print_status("Read error"); return;
			}
			print_temp( temp );
			print_hum( hum );
			ShowWindow( GetDlgItem( hMainDlg, IDC_BEAT_TEMP), tog ? SW_SHOW : SW_HIDE );
			ShowWindow( GetDlgItem( hMainDlg, IDC_BEAT_HUM), tog ? SW_SHOW : SW_HIDE );
			tog = !tog;
			if (logfile) {
				fprintf( logfile, "%.2f\t%.3f\t%.2f\n", tim, temp, hum); fflush( logfile );
			}
			break;
	}
}


void on_timer( void )
{
	update_measurement();

}


void print_status( const char* status )
{
	SetDlgItemText( hMainDlg, IDC_VALUE_STATUS, status );
}


void print_temp( double temp )
{
	char str[50] = {0};
	if (temp <= NOT_A_TEMPERATURE)  strcpy( str, " ---.--- \xB0""C" );
	else if (temp<0.)    sprintf( str, "%-2.2f \xB0""C", temp );
	else if (temp<100.)  sprintf( str, "% 2.2f \xB0""C", temp );
	else                 sprintf( str, "%3.2f \xB0""C", temp );

	SetDlgItemText( hMainDlg, IDC_VALUE_TEMP, str );
}


void print_hum( double hum )
{
	char str[50] = {0};
	if (hum < 0)        sprintf( str, " ---.--- %" );
	else if (hum<100.)  sprintf( str, "% 2.2f %%", hum );
	else                sprintf( str, "%3.2f %%", hum );

	SetDlgItemText( hMainDlg, IDC_VALUE_HUM, str );
}

