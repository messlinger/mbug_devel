// mbug_2820_guiDlg.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "mbug_2820_gui.h"
#include "mbug_2820_guiDlg.h"

#include "mbug_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMbug_2820_guiDlg Dialogfeld

CMbug_2820_guiDlg::CMbug_2820_guiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMbug_2820_guiDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMbug_2820_guiDlg)
	//}}AFX_DATA_INIT
	// Beachten Sie, dass LoadIcon unter Win32 keinen nachfolgenden DestroyIcon-Aufruf benötigt
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Initalization
	m_acq_timer = 0;
	m_temperature = -300;
	m_humidity = -300;
	m_device = 0;
	m_logfile = 0;
	m_simulator = 0;


	m_hIcon_open = (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ICON_OPEN),
								IMAGE_ICON, 16,16, LR_DEFAULTCOLOR );
	m_hBitmap_dnarrow = (HBITMAP)LoadImage( 0, MAKEINTRESOURCE(OBM_DNARROW), IMAGE_BITMAP,
								0,0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED );
	m_hBitmap_uparrow = (HBITMAP)LoadImage( 0, MAKEINTRESOURCE(OBM_UPARROW), IMAGE_BITMAP,
								0,0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED );

  }


void CMbug_2820_guiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMbug_2820_guiDlg)
	DDX_Control(pDX, IDC_BUTTON_MORE, m_button_more);
	DDX_Control(pDX, IDC_BUTTON_LOGFILE, m_button_logfile);
	DDX_Control(pDX, IDC_EDIT_LOGFILE, m_edit_logfile);
	DDX_Control(pDX, IDC_LABEL_VALUE_HUMIDITY, m_disp_hum);
	DDX_Control(pDX, IDC_HEARTBEAT_HUM, m_Heartbeat_hum);
	DDX_Control(pDX, IDC_HEARTBEAT_T, m_Heartbeat_t);
	DDX_Control(pDX, IDC_LABEL_VALUE_STATUS, m_disp_status);
	DDX_Control(pDX, IDC_LABEL_VALUE_TEMPERATURE, m_disp_temp);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_combo_device);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMbug_2820_guiDlg, CDialog)
	//{{AFX_MSG_MAP(CMbug_2820_guiDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_DROPDOWN(IDC_COMBO_DEVICE, OnDropdownComboDevice)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnSelchangeComboDevice)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_LOGFILE, OnButtonLogfile)
	ON_BN_CLICKED(IDC_BUTTON_MORE, OnButtonMore)
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStartRec)
	ON_WM_CANCELMODE()
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStopRec)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMbug_2820_guiDlg Nachrichten-Handler

BOOL CMbug_2820_guiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, 1);

	// ZU ERLEDIGEN: Hier zusätzliche Initialisierung einfügen

	// Temperature value display
	// Increase the font size
	m_big_font.CreatePointFont(260, "Arial" );
	m_disp_temp.SetFont( &m_big_font, 1 );
	m_disp_hum.SetFont( &m_big_font, 1 );
	// Write initial text
	m_disp_temp.SetWindowText(" ---.--- \xB0""C");
	m_disp_hum.SetWindowText(" ---.--- %");
	m_edit_logfile.SetWindowText("");
	

	m_button_logfile.SetIcon( m_hIcon_open );
	
	BITMAP bmp; // Need to create bitmap from handle to get the actual size :(
	GetObject(m_hBitmap_dnarrow, sizeof(BITMAP), &bmp); // Fit button size to icon
	m_button_more.SetWindowPos( 0, -1, -1, bmp.bmWidth, bmp.bmHeight, SWP_NOMOVE | SWP_NOREPOSITION );

	show_rec_control(0); // hide record controls at startup

	return TRUE;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}


// Wollen Sie Ihrem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch für Sie erledigt.

void CMbug_2820_guiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext für Zeichnen

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Symbol in Client-Rechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


// Die Systemaufrufe fragen den Cursorform ab, die angezeigt werden soll, während der Benutzer
//  das zum Symbol verkleinerte Fenster mit der Maus zieht.
HCURSOR CMbug_2820_guiDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



void CMbug_2820_guiDlg::OnClose()
{
	close_logfile();
	close_device(0);

	CDialog::OnClose();
}


void CMbug_2820_guiDlg::OnDestroy()
{
	close_logfile();
	close_device(0);

	CDialog::OnDestroy();

}


void CMbug_2820_guiDlg::OnDropdownComboDevice()
{
	m_combo_device.ResetContent();
	m_combo_device.AddString( "None" );

	if (m_simulator) 
	{
		m_combo_device.AddString( "MBUG-9999-Simula" );
	}
	else
	{
		mbug_device_list list = mbug_get_device_list(0);
		for (int i=0; list[i]!=0; i++ )
		{
			switch (mbug_type_from_id(list[i]))
			{
				case 2810:
				case 2811:
				case 2820:
					m_combo_device.AddString( list[i] );
					break;
			}
		}
	}
}


void CMbug_2820_guiDlg::OnSelchangeComboDevice()
{
	int sel_index = m_combo_device.GetCurSel();
	if (sel_index != CB_ERR)
	{
		close_logfile();
		CString item;
		m_combo_device.GetLBText( sel_index, item );
		item.MakeUpper();
		if (item.Left(4) == "NONE")
			close_device();
		else 
			open_device( item );
	}
}


void CMbug_2820_guiDlg::OnTimer(UINT id_timer)
{
	update_measurement();

	CDialog::OnTimer(id_timer);
}



void CMbug_2820_guiDlg::print_status( const char* msg )
{
	m_disp_status.SetWindowText( msg );
}


void CMbug_2820_guiDlg::print_temperature( double temp )
{
	m_temperature = temp;
	CString s;
	if (temp <= NOT_A_TEMPERATURE ) s = " ---.--- \xB0""C";
	else if (temp<0.)    s.Format( "%-2.2f \xB0""C", temp );
	else if (temp<100.)  s.Format( "% 2.2f \xB0""C", temp );
	else                 s.Format( "%3.2f \xB0""C", temp );

	m_disp_temp.SetWindowText( s );

}


void CMbug_2820_guiDlg::print_humidity( double hum )
{
	m_humidity = hum;
	CString s;
	if (hum <= NOT_A_TEMPERATURE ) s = " ---.--- %";
	else if (hum<100.)  s.Format( "% 2.2f %%", hum );
	else                s.Format( "%3.2f %%", hum );

	m_disp_hum.SetWindowText( s );
}


void CMbug_2820_guiDlg::open_device( const char* id )
{
	close_device();

	int type = mbug_type_from_id( id );
	if (type<0)
	{
		print_status("Invalid device id");
		return;
	}
	switch (type)
	{
		case 2810:
		case 2811:
		case 2820:
		case 9999:
			break;
		default:
			print_status("Unsupported device");
			return;
	}

	if (m_simulator)
	{
		print_status("Device opened");
	}
	else
	{
		print_status("Opening device");	
		{
			m_device = mbug_open_str( id );
			if (m_device == 0 )
			{
				print_status("Error opening device");
				return;
			}
		}
		print_status("Device opened");
	}

	update_measurement();

	print_status("Start timer");
	m_acq_timer = SetTimer( 0, 1000, 0 );
	if (m_acq_timer == 0)
	{
		print_status("Error in SetTimer()");
		return;
	}

	print_status("Ok");

}


void CMbug_2820_guiDlg::close_device( int verbose )
{
	if (m_acq_timer != 0)
	{
		if (verbose) print_status("Killing timer");
		//KillTimer( m_acq_timer );
		KillTimer( 0 );
		m_acq_timer = 0;
	}

	if (m_simulator)
	{
		if (verbose) print_status("Device closed");
	}
	else if (m_device != 0)
	{
		if (verbose) print_status("Closing device");
		mbug_close( m_device );
		m_device = 0;
		if (verbose) print_status("Device closed");
	}
}


void CMbug_2820_guiDlg::update_measurement()
{
	double temperature = -300, humidity = -300, tim = 0;
	int type = -1;
	static int tog = 1;

	if (m_simulator) 
	{
		type = 9999;
	}
	else
	{
		if (m_device == 0) return;
		type = mbug_type_from_id( mbug_id( m_device) );
		if (type < 0)
		{
			print_status("Invalid device id");
			return;
		}
	}

	switch (type)
	{
		case 2810: temperature = mbug_2810_read( m_device ); break;
		case 2811: temperature = mbug_2811_read( m_device ); break;
		case 2820: mbug_2820_read( m_device, &temperature, &humidity ); break;
		case 9999: temperature = 99.99; humidity = 99.99; break;
		default: print_status("Unsupported device"); return;
	}
	tim = floattime();

    switch (type)
    {
		case 2810:  /* temperature only */
		case 2811:
			if (temperature <= NOT_A_TEMPERATURE ) {
				print_status("Read error"); 
				if (m_logfile) fputs("### Read error\n", m_logfile );
				return;
			}
			print_temperature( temperature );
			print_humidity( NOT_A_TEMPERATURE );
			m_Heartbeat_t.ShowWindow( tog ? SW_SHOW : SW_HIDE );
			m_Heartbeat_hum.ShowWindow( SW_HIDE );
			tog = !tog;
			
			if (m_logfile) {
				fprintf( m_logfile, "%.2f\t%.3f\n", tim, temperature); fflush( m_logfile );
			}
			break;

		case 2820:
		case 9999:
			if (temperature <= NOT_A_TEMPERATURE || humidity < 0 ) {
				print_status("Read error"); 
				if (m_logfile) fputs("### Read error\n", m_logfile );
				return;
			}
			print_temperature( temperature );
			print_humidity( humidity );
			m_Heartbeat_t.ShowWindow( tog ? SW_SHOW : SW_HIDE );
			m_Heartbeat_hum.ShowWindow( tog ? SW_SHOW : SW_HIDE );
			tog = !tog;

			if (m_logfile) {
				fprintf( m_logfile, "%.2f\t%.3f\t%.2f\n", tim, temperature, humidity); fflush( m_logfile );
			}
			break;
	}

}


void CMbug_2820_guiDlg::show_rec_control( int show )
{
	this->SetWindowPos( 0, -1, -1, 216, show ? 335 : 236, SWP_NOMOVE | SWP_NOREPOSITION );
	m_button_more.SetBitmap( show ? m_hBitmap_uparrow : m_hBitmap_dnarrow );
}


void CMbug_2820_guiDlg::OnButtonMore() 
{
	RECT rect;
	this->GetWindowRect( &rect );
	show_rec_control( rect.bottom - rect.top < 335 );
}


void CMbug_2820_guiDlg::OnButtonLogfile() 
{
	char filename[1000] = "";
	OPENFILENAME ofn = {
		sizeof(OPENFILENAME),	// DWORD         lStructSize; 
		0,						// HWND          hwndOwner; 
		0,						// HINSTANCE     hInstance;     
		0,						// LPCTSTR       lpstrFilter; 
		0,						// LPTSTR        lpstrCustomFilter; 
		0,						// DWORD         nMaxCustFilter; 
		0,						// DWORD         nFilterIndex; 
		filename,				// LPTSTR        lpstrFile; 
		sizeof(filename)-1,		// DWORD         nMaxFile; 
		0,						// LPTSTR        lpstrFileTitle; 
		0,						// DWORD         nMaxFileTitle; 
		0,						// LPCTSTR       lpstrInitialDir; 
		"Select logfile",		// LPCTSTR       lpstrTitle; 
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, // DWORD         Flags; 
		0,						// WORD          nFileOffset; 
		0,						// WORD          nFileExtension; 
		"dat",					// LPCTSTR       lpstrDefExt; 
		0,						// DWORD         lCustData; 
		0,						// LPOFNHOOKPROC lpfnHook; 
		0						// LPCTSTR       lpTemplateName; 
	};
	
	BOOL res = GetOpenFileName( &ofn );
	if (res) {
		m_edit_logfile.SetWindowText(filename);
	}
}


void CMbug_2820_guiDlg::close_logfile( void )
{
	if (m_logfile)  fclose( m_logfile );
	m_logfile = 0;
}


void CMbug_2820_guiDlg::OnButtonStartRec() 
{
	if (!m_simulator && !m_device)  {
		print_status( "No device opened" );
		return;
	}
	if (m_logfile)  close_logfile();
	
	// Get logfile name 
	char filename[1000] = {0};
	m_edit_logfile.GetWindowText( filename, sizeof(filename)-1 );
	if ( !strlen(filename) )  {
		print_status("No data file selected");
		return;
	}

	// Open file
	print_status("Opening data file...");
	m_logfile = fopen( filename, "ac" );  // mode "c" on windows turns off additional write buffer
	if (!m_logfile)  {
		print_status("Error opening data file");
		return;
	}

	// Write header
	fseek(m_logfile, 0, SEEK_END);
	if (ftell(m_logfile) > 0)  fprintf( m_logfile, "\n\n" );
	int type = -1;
	if (m_simulator) type = 9999;
	else type = mbug_type_from_id( mbug_id( m_device ) );
	if (type == 9999)
		fprintf( m_logfile, "# %s\n", "MBUG Simulator" );
	else
		fprintf( m_logfile, "# %s\n", mbug_id( m_device) );
	
	double tim = floattime();
	fprintf( m_logfile, "# Start recording at %.2f (%s)\n", tim, strtime(tim) );
	
	switch (type) {
		case 2810:
		case 2811: fputs( "# timestamp\ttemperature\n", m_logfile); break;
		case 2820:
		case 9999: fputs( "# timestamp\ttemp\thumidity\n", m_logfile ); break;
	}

	fflush( m_logfile );
	print_status("Recording");
}


void CMbug_2820_guiDlg::OnButtonStopRec() 
{
	if (m_logfile)  fclose(m_logfile);
	print_status("Stopped recording");
}
