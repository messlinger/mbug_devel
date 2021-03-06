// mbug_2810_guiDlg.h : Header-Datei
//

#if !defined(AFX_MBUG_2810_GUIDLG_H__A3145168_9AA8_4649_A126_D21A3A2B9888__INCLUDED_)
#define AFX_MBUG_2810_GUIDLG_H__A3145168_9AA8_4649_A126_D21A3A2B9888__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "mbug.h"
#include "mbug_2810.h"
#include "mbug_2811.h"
#include "mbug_2820.h"


/////////////////////////////////////////////////////////////////////////////
// CMbug_2810_guiDlg Dialogfeld

class CMbug_2810_guiDlg : public CDialog
{
// Konstruktion
public:
	CMbug_2810_guiDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CMbug_2810_guiDlg)
	enum { IDD = IDD_MBUG_2810_GUI_DIALOG };
	CStatic	m_Heartbeat;
	CStatic	m_disp_status;
	CStatic	m_disp_temp;
	CComboBox	m_combo_device;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CMbug_2810_guiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON m_hIcon;
	CFont m_big_font;
	UINT m_acq_timer;
	double m_temperature;
	mbug_device m_device;

	CRgn rgn;

	void print_status( const char* msg);
	void print_temperature( double temp );
	void open_device( const char* id );
	void close_device( int verbose = 1 );
	void update_temperature(void);

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CMbug_2810_guiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDropdownComboDevice();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeComboDevice();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_MBUG_2810_GUIDLG_H__A3145168_9AA8_4649_A126_D21A3A2B9888__INCLUDED_)
