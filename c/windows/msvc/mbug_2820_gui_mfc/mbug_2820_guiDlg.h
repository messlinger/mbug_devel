// mbug_2820_guiDlg.h : Header-Datei
//

#if !defined(AFX_MBUG_2820_GUIDLG_H__A3145168_9AA8_4649_A126_D21A3A2B9888__INCLUDED_)
#define AFX_MBUG_2820_GUIDLG_H__A3145168_9AA8_4649_A126_D21A3A2B9888__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "mbug.h"
#include "mbug_2810.h"
#include "mbug_2811.h"
#include "mbug_2820.h"


/////////////////////////////////////////////////////////////////////////////
// CMbug_2820_guiDlg Dialogfeld

class CMbug_2820_guiDlg : public CDialog
{
// Konstruktion
public:
	CMbug_2820_guiDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CMbug_2820_guiDlg)
	enum { IDD = IDD_MBUG_2820_GUI_DIALOG };
	CButton	m_button_more;
	CButton	m_button_logfile;
	CEdit	m_edit_logfile;
	CStatic	m_disp_hum;
	CStatic	m_Heartbeat_hum;
	CStatic	m_Heartbeat_t;
	CStatic	m_disp_status;
	CStatic	m_disp_temp;
	CComboBox	m_combo_device;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CMbug_2820_guiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON m_hIcon, m_hIcon_open, m_hIcon_down;
	HBITMAP m_hBitmap_dnarrow, m_hBitmap_uparrow;
	CFont m_big_font;
	UINT m_acq_timer;
	double m_temperature;
	double m_humidity;
	mbug_device m_device;
	bool m_simulator;
	FILE* m_logfile;

	void print_status( const char* msg);
	void print_temperature( double temp );
	void print_humidity( double hum );
	void open_device( const char* id );
	void close_device( int verbose = 1 );
	void update_measurement(void);
	void show_rec_control( int show );
	void close_logfile( void );

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CMbug_2820_guiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDropdownComboDevice();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeComboDevice();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnButtonLogfile();
	afx_msg void OnButtonMore();
	afx_msg void OnButtonStartRec();
	afx_msg void OnButtonStopRec();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_MBUG_2820_GUIDLG_H__A3145168_9AA8_4649_A126_D21A3A2B9888__INCLUDED_)
