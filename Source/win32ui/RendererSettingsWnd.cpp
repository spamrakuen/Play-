#include "PtrMacro.h"
#include "RendererSettingsWnd.h"
#include "layout/HorizontalLayout.h"
#include "layout/LayoutStretch.h"
#include "win32/LayoutWindow.h"
#include "win32/Static.h"
#include "../AppConfig.h"

#define CLSNAME			_T("RendererSettingsWnd")
#define WNDSTYLE		(WS_CAPTION | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU)
#define WNDSTYLEEX		(WS_EX_DLGMODALFRAME)

using namespace Framework;

CRendererSettingsWnd::CRendererSettingsWnd(HWND hParent, CGSH_OpenGL* pRenderer) :
CModalWindow(hParent)
{
	RECT rc;

	m_pRenderer = pRenderer;

	if(!DoesWindowClassExist(CLSNAME))
	{
		WNDCLASSEX w;
		memset(&w, 0, sizeof(WNDCLASSEX));
		w.cbSize		= sizeof(WNDCLASSEX);
		w.lpfnWndProc	= CWindow::WndProc;
		w.lpszClassName	= CLSNAME;
		w.hbrBackground	= (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
		w.hInstance		= GetModuleHandle(NULL);
		w.hCursor		= LoadCursor(NULL, IDC_ARROW);
		w.style			= CS_HREDRAW | CS_VREDRAW;
		RegisterClassEx(&w);
	}

	Create(WNDSTYLEEX, CLSNAME, _T("Renderer Settings"), WNDSTYLE, Framework::Win32::CRect(0, 0, 400, 350), hParent, NULL);
	SetClassPtr();

	m_pOk		= new Win32::CButton(_T("OK"), m_hWnd, Framework::Win32::CRect(0, 0, 1, 1));
	m_pCancel	= new Win32::CButton(_T("Cancel"), m_hWnd, Framework::Win32::CRect(0, 0, 1, 1));

	m_nLinesAsQuads				= CAppConfig::GetInstance().GetPreferenceBoolean(PREF_CGSH_OPENGL_LINEASQUADS);
	m_nForceBilinearTextures	= CAppConfig::GetInstance().GetPreferenceBoolean(PREF_CGSH_OPENGL_FORCEBILINEARTEXTURES);
	m_flipMode					= static_cast<CGSHandler::FLIP_MODE>(CAppConfig::GetInstance().GetPreferenceInteger(PREF_CGSHANDLER_FLIPMODE));

	m_pLineCheck = new Win32::CButton(_T("Render lines using quads"), m_hWnd, Framework::Win32::CRect(0, 0, 1, 1), BS_CHECKBOX);
	m_pLineCheck->SetCheck(m_nLinesAsQuads);

	m_pForceBilinearCheck = new Win32::CButton(_T("Force bilinear texture sampling"), m_hWnd, Framework::Win32::CRect(0, 0, 1, 1), BS_CHECKBOX);
	m_pForceBilinearCheck->SetCheck(m_nForceBilinearTextures);

	m_pFlipModeComboBox = new Win32::CComboBox(m_hWnd, Framework::Win32::CRect(0, 0, 1, 1), CBS_DROPDOWNLIST | WS_VSCROLL);
	m_pFlipModeComboBox->AddString(_T("#1 - SMODE2"));
	m_pFlipModeComboBox->AddString(_T("#2 - DISPFB2"));
	m_pFlipModeComboBox->AddString(_T("#3 - VBLANK"));
	m_pFlipModeComboBox->SetSelection(m_flipMode);

	m_pExtList = new Win32::CListView(m_hWnd, Framework::Win32::CRect(0, 0, 1, 1), LVS_REPORT | LVS_SORTASCENDING | LVS_NOSORTHEADER);
	m_pExtList->SetExtendedListViewStyle(m_pExtList->GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT);

	FlatLayoutPtr pSubLayout0 = CHorizontalLayout::Create();
	{
		pSubLayout0->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(150, 23, new Win32::CStatic(m_hWnd, _T("Swap frame buffer at:"), SS_CENTERIMAGE)));
		pSubLayout0->InsertObject(CLayoutStretch::Create());
		pSubLayout0->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(100, 23, m_pFlipModeComboBox));
		pSubLayout0->SetVerticalStretch(0);
	}

	FlatLayoutPtr pSubLayout1 = CHorizontalLayout::Create();
	{
		pSubLayout1->InsertObject(CLayoutStretch::Create());
		pSubLayout1->InsertObject(Win32::CLayoutWindow::CreateButtonBehavior(100, 23, m_pOk));
		pSubLayout1->InsertObject(Win32::CLayoutWindow::CreateButtonBehavior(100, 23, m_pCancel));
		pSubLayout1->SetVerticalStretch(0);
	}

	m_pLayout = CVerticalLayout::Create();
	m_pLayout->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(100, 15, m_pLineCheck));
	m_pLayout->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(100, 15, m_pForceBilinearCheck));
	m_pLayout->InsertObject(pSubLayout0);
	m_pLayout->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(100, 2, new Win32::CStatic(m_hWnd, rc, SS_ETCHEDHORZ)));
	m_pLayout->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(100, 15, new Win32::CStatic(m_hWnd, _T("OpenGL extension availability report:"))));
	m_pLayout->InsertObject(Win32::CLayoutWindow::CreateCustomBehavior(1, 1, 1, 1, m_pExtList));
	m_pLayout->InsertObject(Win32::CLayoutWindow::CreateTextBoxBehavior(100, 30, new Win32::CStatic(m_hWnd, _T("For more information about the consequences of the absence of an extension, please consult the documentation."), SS_LEFT)));
	m_pLayout->InsertObject(pSubLayout1);

	RefreshLayout();

	CreateExtListColumns();
	UpdateExtList();
}

CRendererSettingsWnd::~CRendererSettingsWnd()
{

}

long CRendererSettingsWnd::OnCommand(unsigned short nID, unsigned short nCmd, HWND hSender)
{
	if(hSender == m_pOk->m_hWnd)
	{
		Save();
		Destroy();
		return TRUE;
	}
	else if(hSender == m_pCancel->m_hWnd)
	{
		Destroy();
		return TRUE;
	}
	else if(ProcessCheckBoxMessage(hSender, m_pLineCheck, &m_nLinesAsQuads)) return TRUE;
	else if(ProcessCheckBoxMessage(hSender, m_pForceBilinearCheck, &m_nForceBilinearTextures)) return TRUE;

	return FALSE;
}

void CRendererSettingsWnd::RefreshLayout()
{
	RECT rc = GetClientRect();

	SetRect(&rc, rc.left + 10, rc.top + 10, rc.right - 10, rc.bottom - 10);

	m_pLayout->SetRect(rc.left, rc.top, rc.right, rc.bottom);
	m_pLayout->RefreshGeometry();

	Redraw();
}

void CRendererSettingsWnd::CreateExtListColumns()
{
	LVCOLUMN col;
	RECT rc = m_pExtList->GetClientRect();

	memset(&col, 0, sizeof(LVCOLUMN));
	col.pszText = _T("Extension");
	col.mask	= LVCF_TEXT | LVCF_WIDTH;
	col.cx		= rc.right * 3 / 4;
	m_pExtList->InsertColumn(0, &col);

	memset(&col, 0, sizeof(LVCOLUMN));
	col.pszText = _T("Availability");
	col.mask	= LVCF_TEXT | LVCF_WIDTH;
	col.cx		= rc.right / 4;
	m_pExtList->InsertColumn(1, &col);
}

void CRendererSettingsWnd::UpdateExtList()
{
	unsigned int i;
	LVITEM itm;

	memset(&itm, 0, sizeof(LVITEM));
	itm.mask		= LVIF_TEXT;
	itm.pszText		= _T("glBlendColor function");
	i = m_pExtList->InsertItem(&itm);

	m_pExtList->SetItemText(i, 1, m_pRenderer->IsBlendColorExtSupported() ? _T("Present") : _T("Absent"));

	memset(&itm, 0, sizeof(LVITEM));
	itm.mask		= LVIF_TEXT;
	itm.pszText		= _T("GL_UNSIGNED_SHORT_1_5_5_5_REV texture format");
	i = m_pExtList->InsertItem(&itm);

	m_pExtList->SetItemText(i, 1, m_pRenderer->IsRGBA5551ExtSupported() ? _T("Present") : _T("Absent"));

	memset(&itm, 0, sizeof(LVITEM));
	itm.mask		= LVIF_TEXT;
	itm.pszText		= _T("glBlendEquation function");
	i = m_pExtList->InsertItem(&itm);

	m_pExtList->SetItemText(i, 1, m_pRenderer->IsBlendEquationExtSupported() ? _T("Present") : _T("Absent"));

	memset(&itm, 0, sizeof(LVITEM));
	itm.mask		= LVIF_TEXT;
	itm.pszText		= _T("glFogCoordf function");
	i = m_pExtList->InsertItem(&itm);

	m_pExtList->SetItemText(i, 1, m_pRenderer->IsFogCoordfExtSupported() ? _T("Present") : _T("Absent"));
}

void CRendererSettingsWnd::Save()
{
	m_flipMode = static_cast<CGSHandler::FLIP_MODE>(m_pFlipModeComboBox->GetSelection());

	CAppConfig::GetInstance().SetPreferenceBoolean(PREF_CGSH_OPENGL_LINEASQUADS, m_nLinesAsQuads);
	CAppConfig::GetInstance().SetPreferenceBoolean(PREF_CGSH_OPENGL_FORCEBILINEARTEXTURES, m_nForceBilinearTextures);
	CAppConfig::GetInstance().SetPreferenceInteger(PREF_CGSHANDLER_FLIPMODE, m_flipMode);
}

bool CRendererSettingsWnd::ProcessCheckBoxMessage(HWND hSender, Win32::CButton* pCheckBox, bool* pFlag)
{
	if(pCheckBox == NULL) return false;
	if(pFlag == NULL) return false;
	if(hSender != pCheckBox->m_hWnd) return false;

	(*pFlag) = !(*pFlag);
	pCheckBox->SetCheck(*pFlag);

	return true;
}
