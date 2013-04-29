#pragma once

#include "Types.h"
#include "win32/CustomDrawn.h"
#include "win32/DeviceContext.h"
#include <boost/signals2.hpp>

class CMemoryView : public Framework::Win32::CCustomDrawn
{
public:
											CMemoryView(HWND, const RECT&);
	virtual									~CMemoryView();
	void									SetMemorySize(uint32);
	void									ScrollToAddress(uint32);
	uint32									GetSelection();

	boost::signals2::signal<void (uint32)>	OnSelectionChange;

protected:
	virtual uint8							GetByte(uint32) = 0;
	virtual HFONT							GetFont();

	void									Paint(HDC);
	long									OnSize(unsigned int, unsigned int, unsigned int);
	long									OnVScroll(unsigned int, unsigned int);
	long									OnSetFocus();
	long									OnKillFocus();
	long									OnMouseWheel(short);
	long									OnLeftButtonDown(int, int);
	long									OnLeftButtonUp(int, int);
	long									OnKeyDown(unsigned int);
	void									SetSelectionStart(unsigned int);

private:
	void									GetVisibleRowsCols(unsigned int*, unsigned int*);
	void									UpdateScrollRange();
	unsigned int							GetScrollOffset();
	unsigned int							GetScrollThumbPosition();
	void									UpdateCaretPosition();
	void									GetRenderParams(const SIZE&, unsigned int&, unsigned int&, uint32&);

	uint32									m_nSelectionStart;
	uint32									m_nSize;
	unsigned int							m_nPos;
};
