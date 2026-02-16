#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <array>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <memory>

#include "ScintillaTypes.h"
#include "ScintillaMessages.h"
#include "ScintillaStructures.h"
#include "Scintilla.h"
#include "Debugging.h"
#include "Geometry.h"
#include "Platform.h"
#include "ILoader.h"
#include "ILexer.h"
#include "CharacterCategoryMap.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "AutoComplete.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "ScintillaBase.h"
#include "CaseConvert.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>

class Fl_Scintilla : public Fl_Group, public Scintilla::Internal::ScintillaBase
{
private:
	static void menu_cb(Fl_Widget*, void*);
	template<Scintilla::Internal::Editor::TickReason reason>
	static void timeout_cb(void*);
	std::array<double, 5> tick_period; //must match count of TickReason
	static void idle_cb(void*);
	static constexpr int scrollbar_width = 15;
	Fl_Scrollbar scrollbar_vertical, scrollbar_horizontal;
	void vscroll_cb();
	void hscroll_cb();
	Scintilla::Internal::Point get_mouse_position();
protected:
	void SetVerticalScrollPos() override;
	void SetHorizontalScrollPos() override;
	bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
	void Copy() override;
	void Paste() override;
	void ClaimSelection() override;
	void NotifyChange() override;
	void CopyToClipboard(const Scintilla::Internal::SelectionText& selectedText) override;
	bool HaveMouseCapture() override;
	void SetMouseCapture(bool on) override;
	std::string UTF8FromEncoded(std::string_view encoded) const override;
	std::string EncodedFromUTF8(std::string_view utf8) const override;
	Scintilla::sptr_t DefWndProc(Scintilla::Message iMessage, Scintilla::uptr_t wParam, Scintilla::sptr_t lParam) override;
	void CreateCallTipWindow(Scintilla::Internal::PRectangle rect) override;
	void AddToPopUp(const char* label, int cmd = 0, bool enabled = true) override;

	bool FineTickerRunning(Scintilla::Internal::Editor::TickReason reason) override;
	void FineTickerStart(Scintilla::Internal::Editor::TickReason reason, int millis, int tolerance) override;
	void FineTickerCancel(Scintilla::Internal::Editor::TickReason reason) override;
public:
	Fl_Scintilla(int X, int Y, int W, int H, const char *L = nullptr);
	~Fl_Scintilla() override;
	void draw() override;
	int handle(int) override;
	void resize(int X, int Y, int W, int H) override;
};