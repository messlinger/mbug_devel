// mbug_2810_guiDlg.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "mbug_2810_gui.h"
#include "mbug_2810_guiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMbug_2810_guiDlg Dialogfeld

CMbug_2810_guiDlg::CMbug_2810_guiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMbug_2810_guiDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMbug_2810_guiDlg)
	//}}AFX_DATA_INIT
	// Beachten Sie, dass LoadIcon unter Win32 keinen nachfolgenden DestroyIcon-Aufruf benötigt
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Initalization
	m_acq_timer = 0;
	m_temperature = -300;
	m_device = 0;

}


void CMbug_2810_guiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMbug_2810_guiDlg)
	DDX_Control(pDX, IDC_HEARTBEAT, m_Heartbeat);
	DDX_Control(pDX, IDC_LABEL_VALUE_STATUS, m_disp_status);
	DDX_Control(pDX, IDC_LABEL_VALUE_TEMPERATURE, m_disp_temp);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_combo_device);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMbug_2810_guiDlg, CDialog)
	//{{AFX_MSG_MAP(CMbug_2810_guiDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_DROPDOWN(IDC_COMBO_DEVICE, OnDropdownComboDevice)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnSelchangeComboDevice)
	ON_WM_CANCELMODE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMbug_2810_guiDlg Nachrichten-Handler

BOOL CMbug_2810_guiDlg::OnInitDialog()
{	
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden
	
	// ZU ERLEDIGEN: Hier zusätzliche Initialisierung einfügen
	
	// Temperature value display
	// Increase the font size
	m_big_font.CreatePointFont(260, "Arial" );
	m_disp_temp.SetFont( &m_big_font, 1 );
	// Write initial text
	m_disp_temp.SetWindowText("  ---.--- \xB0""C");

	return FALSE;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}


// Wollen Sie Ihrem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch für Sie erledigt.

void CMbug_2810_guiDlg::OnPaint() 
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
HCURSOR CMbug_2810_guiDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



void CMbug_2810_guiDlg::OnClose() 
{
	close_device(0);
	
	CDialog::OnClose();
}


void CMbug_2810_guiDlg::OnDestroy() 
{
	close_device(0);

	CDialog::OnDestroy();
		
}

void CMbug_2810_guiDlg::OnCancel()
{
	CDialog::OnCancel();
}

void CMbug_2810_guiDlg::OnOK()
{
}


void CMbug_2810_guiDlg::OnDropdownComboDevice() 
{
	// Remember the current selection
	int sel_index = m_combo_device.GetCurSel();
	CString sel_str;
	if (sel_index != CB_ERR)
		m_combo_device.GetLBText( sel_index, sel_str );

	// Refill the list
	m_combo_device.ResetContent();

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

	m_combo_device.InsertString( 0, "None" );

	// Reselect the last selection if present
	if (sel_index != CB_ERR)
		m_combo_device.SelectString( -1, sel_str );


}


void CMbug_2810_guiDlg::OnSelchangeComboDevice() 
{
	int sel_index = m_combo_device.GetCurSel();
	if (sel_index != CB_ERR)
	{
		CString item;
		m_combo_device.GetLBText( sel_index, item );
		item.MakeUpper();
		if (item.Left(4) == "NONE")
			close_device();
		else
			open_device( item );
	}
	
}


void CMbug_2810_guiDlg::OnTimer(UINT id_timer) 
{
	update_temperature();
	
	CDialog::OnTimer(id_timer);
}



void CMbug_2810_guiDlg::print_status( const char* msg )
{
	m_disp_status.SetWindowText( msg );
}


void CMbug_2810_guiDlg::print_temperature( double temp )
{
	m_temperature = temp;
	CString s;
	if (temp<0) 
		s.Format( "%-2.2f \xB0""C", m_temperature);
	else if (temp<100) 
		s.Format( "% 2.2f \xB0""C", m_temperature);
	else 
		s.Format( "%3.2f \xB0""C", m_temperature);

	m_disp_temp.SetWindowText( s );
}


void CMbug_2810_guiDlg::open_device( const char* id )
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
			break;
		default:
			print_status("Unsupported device");
			return;
	}

	print_status("Opening device");
	m_device = mbug_open_str( id );
	if (m_device == 0)
	{
		print_status("Error opening device");
		return;
	}
	print_status("Device opened");

	update_temperature();
	
	print_status("Start timer");
	m_acq_timer = SetTimer( 0, 1000, 0 );
	if (m_acq_timer == 0)
	{
		print_status("Error in SetTimer()");
		return;
	}

	print_status("Ok");

}


void CMbug_2810_guiDlg::close_device( int verbose )
{
	if (m_acq_timer != 0)
	{
		if (verbose) print_status("Killing timer");
		KillTimer( m_acq_timer );
		m_acq_timer = 0;
	}	

	if (m_device != 0)
	{
		if (verbose) print_status("Cosing device");
		mbug_close( m_device );
		m_device = 0;
		if (verbose) print_status("Device closed");
	}	
}


void CMbug_2810_guiDlg::update_temperature()
{
	if (m_device == 0) return;
	double temperature = 0.0;

	int type = mbug_type_from_id( mbug_id( m_device) );
	if (type < 0)
	{
		print_status("Invalid device id");
		return;
	}
	switch (type)
	{
		case 2810:
			temperature = mbug_2810_read( m_device );
			break;
		case 2811:
			temperature = mbug_2811_read( m_device );
			break;
		case 2820:
			temperature = mbug_2820_read_temperature( m_device );
			break;
		default:
			print_status("Unsupported device");
			return;
	}

	if (temperature<=NOT_A_TEMPERATURE)
	{
		print_status("Read error");
		return;
	}

	print_temperature( temperature );

	static tog = 1;
	m_Heartbeat.ShowWindow( tog ? SW_SHOW : SW_HIDE );
	tog = !tog;
}

