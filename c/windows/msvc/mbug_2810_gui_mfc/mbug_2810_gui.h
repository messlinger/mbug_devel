// mbug_2810_gui.h : Haupt-Header-Datei für die Anwendung MBUG_2810_GUI
//

#if !defined(AFX_MBUG_2810_GUI_H__3D9EBD00_49F9_47B0_9994_FB381DAA4169__INCLUDED_)
#define AFX_MBUG_2810_GUI_H__3D9EBD00_49F9_47B0_9994_FB381DAA4169__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CMbug_2810_guiApp:
// Siehe mbug_2810_gui.cpp für die Implementierung dieser Klasse
//

class CMbug_2810_guiApp : public CWinApp
{
public:
	CMbug_2810_guiApp();

// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CMbug_2810_guiApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CMbug_2810_guiApp)
		// HINWEIS - An dieser Stelle werden Member-Funktionen vom Klassen-Assistenten eingefügt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VERÄNDERN!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_MBUG_2810_GUI_H__3D9EBD00_49F9_47B0_9994_FB381DAA4169__INCLUDED_)
