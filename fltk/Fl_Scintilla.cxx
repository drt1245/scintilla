#include "Fl_Scintilla.h"

#include <FL/Fl_Menu_Item.H>

Fl_Scintilla::Fl_Scintilla(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L),
	scrollbar_vertical(X + W - scrollbar_width, Y, scrollbar_width, H - scrollbar_width),
	scrollbar_horizontal(X, Y + H - scrollbar_width, W - scrollbar_width, scrollbar_width)
{
	scrollbar_horizontal.type(FL_HORIZONTAL);
	scrollbar_horizontal.minimum(0);
	scrollbar_vertical.minimum(0);
	scrollbar_vertical.callback([](Fl_Widget*, void* v) { ((Fl_Scintilla*)v)->scroll_cb(); }, this);
	wMain = this;
	CaretSetPeriod(500);
}

void Fl_Scintilla::CreateCallTipWindow(Scintilla::Internal::PRectangle rect)
{
	//TODO
}

void Fl_Scintilla::menu_cb(Fl_Widget* w, void* v)
{
	//TODO: Fl_Widget* parameter is probably null here?
	((Fl_Scintilla*)w)->Command(reinterpret_cast<intptr_t>(v));
}

void Fl_Scintilla::AddToPopUp(const char* const label, const int cmd, const bool enabled)
{
	std::vector<Fl_Menu_Item>* menu = static_cast<std::vector<Fl_Menu_Item>*>(popup.GetID());
	if (menu)
		menu->emplace_back(label, 0, nullptr, (void*)cmd, enabled ? 0 : FL_MENU_INACTIVE);
}

void Fl_Scintilla::SetVerticalScrollPos()
{
	Editor::SetVerticalScrollPos();
	scrollbar_vertical.value(topLine);
	printf("SetVerticalScrollPos(%lld/%lf)\n", topLine, scrollbar_vertical.maximum());
}

void Fl_Scintilla::SetHorizontalScrollPos()
{
	scrollbar_horizontal.value(xOffset);
	printf("SetHorizontalScrollPos(%d)\n", xOffset);
}

bool Fl_Scintilla::ModifyScrollBars(const Sci::Line nMax, const Sci::Line nPage)
{
	bool modified = false;
	if (scrollbar_vertical.maximum() != nMax + 1)
	{
		printf("ModifyScrollBars(%lld, %lld, %d, %lld)\n", topLine, nPage, 0, nMax + 1);
		scrollbar_vertical.value(topLine, nPage, 0, nMax + 1);
		modified = true;
	}
	if (const int nWidth = GetTextRectangle().Width(); scrollbar_horizontal.maximum() != nWidth)
	{
		scrollbar_horizontal.maximum(nWidth);
		modified = true;
	}
	return modified;
}

void Fl_Scintilla::scroll_cb()
{
	ScrollTo(scrollbar_vertical.value());
}

void Fl_Scintilla::ClaimSelection()
{
	//TODO
}

void Fl_Scintilla::NotifyChange()
{
	//TODO
}

void Fl_Scintilla::NotifyParent(const Scintilla::NotificationData scn)
{
	//TODO
}

void Fl_Scintilla::Copy()
{
	if (!sel.Empty())
	{
		Scintilla::Internal::SelectionText st;
		CopySelectionRange(&st);
		CopyToClipboard(st);
	}
}

void Fl_Scintilla::Paste()
{
	Fl::paste(*this, 1);
}

void Fl_Scintilla::CopyToClipboard(const Scintilla::Internal::SelectionText& selectedText)
{
	Fl::copy(selectedText.Data(), selectedText.Length(), 1);
}

bool Fl_Scintilla::HaveMouseCapture()
{
	return Fl::grab() == top_window();
}

void Fl_Scintilla::SetMouseCapture(const bool grab)
{
	Fl::grab(grab ? top_window() : nullptr);
}

std::string Fl_Scintilla::UTF8FromEncoded(const std::string_view encoded) const
{
	return std::string{ encoded };
}

std::string Fl_Scintilla::EncodedFromUTF8(const std::string_view utf8) const
{
	return std::string{ utf8 };
}

Scintilla::sptr_t Fl_Scintilla::DefWndProc(Scintilla::Message, Scintilla::uptr_t, Scintilla::sptr_t)
{
	return 0;
}

template<Scintilla::Internal::Editor::TickReason reason>
void Fl_Scintilla::timeout_cb(void* v)
{
	((Fl_Scintilla*)v)->TickFor(reason);
	const double t = ((Fl_Scintilla*)v)->tick_period.at((int)reason);
	Fl::repeat_timeout(t, timeout_cb<reason>, v);
}

bool Fl_Scintilla::FineTickerRunning(const Scintilla::Internal::Editor::TickReason reason)
{
	switch (reason)
	{
	case Scintilla::Internal::Editor::TickReason::caret:
		return Fl::has_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::caret>, this);
	case Scintilla::Internal::Editor::TickReason::scroll:
		return Fl::has_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::scroll>, this);
	case Scintilla::Internal::Editor::TickReason::widen:
		return Fl::has_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::widen>, this);
	case Scintilla::Internal::Editor::TickReason::dwell:
		return Fl::has_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::dwell>, this);
	case Scintilla::Internal::Editor::TickReason::platform:
		return Fl::has_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::platform>, this);
	}
}

void Fl_Scintilla::FineTickerStart(const Scintilla::Internal::Editor::TickReason reason, const int millis, int)
{
	FineTickerCancel(reason);
	const double t = millis / 1000.0;
	tick_period.at((int)reason) = t;
	switch (reason)
	{
	case Scintilla::Internal::Editor::TickReason::caret:
		Fl::add_timeout(t, timeout_cb<Scintilla::Internal::Editor::TickReason::caret>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::scroll:
		Fl::add_timeout(t, timeout_cb<Scintilla::Internal::Editor::TickReason::scroll>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::widen:
		Fl::add_timeout(t, timeout_cb<Scintilla::Internal::Editor::TickReason::widen>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::dwell:
		Fl::add_timeout(t, timeout_cb<Scintilla::Internal::Editor::TickReason::dwell>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::platform:
		Fl::add_timeout(t, timeout_cb<Scintilla::Internal::Editor::TickReason::platform>, this);
		break;
	}
}

void Fl_Scintilla::FineTickerCancel(const Scintilla::Internal::Editor::TickReason reason)
{
	switch (reason)
	{
	case Scintilla::Internal::Editor::TickReason::caret:
		Fl::remove_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::caret>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::scroll:
		Fl::remove_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::scroll>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::widen:
		Fl::remove_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::widen>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::dwell:
		Fl::remove_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::dwell>, this);
		break;
	case Scintilla::Internal::Editor::TickReason::platform:
		Fl::remove_timeout(timeout_cb<Scintilla::Internal::Editor::TickReason::platform>, this);
		break;
	}
}

void Fl_Scintilla::draw()
{
	paintState = PaintState::painting;
	Scintilla::Internal::AutoSurface surf(this);
	Editor::Paint(surf, Scintilla::Internal::PRectangle{0, 0, (double)w() - scrollbar_width, (double)h() - scrollbar_width });
	paintState = PaintState::notPainting;
}

static Scintilla::Keys fl_keys_to_scintilla(const int key)
{
	switch (key)
	{
	case FL_Up: return Scintilla::Keys::Up;
	case FL_Down: return Scintilla::Keys::Down;
	case FL_Left: return Scintilla::Keys::Left;
	case FL_Right: return Scintilla::Keys::Right;
	case FL_Home: return Scintilla::Keys::Home;
	case FL_End: return Scintilla::Keys::End;
	case FL_Back: return Scintilla::Keys::Prior;
	case FL_Forward: return Scintilla::Keys::Next;
	case FL_Delete: return Scintilla::Keys::Delete;
	case FL_Insert: return Scintilla::Keys::Insert;
	case FL_Escape: return Scintilla::Keys::Escape;
	case FL_BackSpace: return Scintilla::Keys::Back;
	case FL_Tab: return Scintilla::Keys::Tab;
	case FL_Enter: return Scintilla::Keys::Return; //TODO: enter and return are different
	case FL_KP_Enter: return Scintilla::Keys::Return;
	//case ?: return Scintilla::Keys::Add;
	//case ?: return Scintilla::Keys::Subtract;
	//case ?: return Scintilla::Keys::Divide;
	case FL_Meta_L: return Scintilla::Keys::Win;
	case FL_Meta_R: return Scintilla::Keys::RWin;
	case FL_Menu: return Scintilla::Keys::Menu;
	default: return (Scintilla::Keys)0;
	}
}

static Scintilla::KeyMod get_modifiers()
{
	const Scintilla::KeyMod ctrl = Fl::event_ctrl() ? Scintilla::KeyMod::Ctrl : Scintilla::KeyMod::Norm;
	const Scintilla::KeyMod alt = Fl::event_alt() ? Scintilla::KeyMod::Alt : Scintilla::KeyMod::Norm;
	const Scintilla::KeyMod shift = Fl::event_shift() ? Scintilla::KeyMod::Shift : Scintilla::KeyMod::Norm;
	const Scintilla::KeyMod meta = Fl::event_state(FL_META) ? Scintilla::KeyMod::Meta : Scintilla::KeyMod::Norm;
	return ctrl | alt | shift | meta;
}

Scintilla::Internal::Point Fl_Scintilla::get_mouse_position()
{
	return Scintilla::Internal::Point(
		Fl::event_x() - x(),
		Fl::event_y() - y()
	);
}

int Fl_Scintilla::handle(int event)
{
	switch (Fl::event())
	{
	case FL_FOCUS:
		SetFocusState(true);
		return true;
	case FL_UNFOCUS:
		SetFocusState(false);
		return true;
	case FL_PUSH:
		switch (Fl::event_button())
		{
		case FL_LEFT_MOUSE:
			ButtonDownWithModifiers(get_mouse_position(), 0, get_modifiers());
			return true;
		case FL_RIGHT_MOUSE:
			RightButtonDownWithModifiers(get_mouse_position(), 0, get_modifiers());
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_LEFT_MOUSE)
		{
			ButtonUpWithModifiers(get_mouse_position(), 0, get_modifiers());
			return true;
		}
		break;
	case FL_DRAG:
		if (Fl::event_button() == FL_LEFT_MOUSE)
		{
			ButtonMoveWithModifiers(get_mouse_position(), 0, get_modifiers());
			return true;
		}
		break;
	case FL_KEYDOWN:
		bool consumed;
		int key = Fl::event_key();
		const bool added = KeyDownWithModifiers(fl_keys_to_scintilla(key), get_modifiers(), &consumed);
		if (added)
			printf("Added\n");
		if (consumed)
			printf("Consumed\n");
		if (key >= FL_KP && key < FL_KP_Last)
			key -= FL_KP;
		if (key >= ' ' && key <= '~') //printable ascii characters
		{
			if (Fl::event_shift())
				key = toupper(key);
			AddChar(key);
		}
		redraw();
		return true;
	}
	return Fl_Widget::handle(event);
}
