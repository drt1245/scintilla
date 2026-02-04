#include <cstdint>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <stdarg.h>

#include <FL/fl_ask.H> 
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Window.H>

#include "../include/ScintillaTypes.h"
#include "../include/ScintillaMessages.h"

#include "../src/Debugging.h"
#include "../src/Geometry.h"
#include "../src/Platform.h"

namespace Scintilla::Internal {

Window::~Window() noexcept {}

void Window::Destroy() noexcept
{
	if (Fl_Widget* w = static_cast<Fl_Widget*>(wid); w)
	{
		if (ListBox* lb = dynamic_cast<ListBox*>(this); lb)
		{
			w->hide();
			lb->Clear();
			//TODO: resize window to smallest possible size for it to adapt to future content
		}
		else
		{
			delete w;
		}
		wid = nullptr;
	}
}

PRectangle Window::GetPosition() const
{
	if (const Fl_Widget* w = static_cast<const Fl_Widget*>(wid); w)
	{
		printf("GetPosition(%d, %d, %d, %d) %p\n", w->x(), w->y(), w->w(), w->h(), w);
		return PRectangle(0, 0, w->w() - 15, w->h() - 15);
		//return PRectangle(w->x(), w->y(), w->x() + w->w(), w->y() + w->h());
	}
	else
		return PRectangle{0, 0, 1000, 1000};
}

void Window::SetPosition(const PRectangle rc)
{
	printf("SetPosition(%lf, %lf, %lf, %lf)\n", rc.left, rc.top, rc.Width(), rc.Height());
	if (Fl_Widget* w = static_cast<Fl_Widget*>(wid); w)
		w->resize(lround(rc.left), lround(rc.top), lround(rc.Width()), lround(rc.Height()));
}

void Window::SetPositionRelative(const PRectangle rc, const Window* relativeTo)
{
	if (wid && relativeTo && relativeTo->wid) {
		Fl_Widget* w = (Fl_Widget*)wid,
		         * parent = (Fl_Widget*)relativeTo->wid;
		w->resize(lround(rc.left) + parent->x(), lround(rc.top) + parent->y(), lround(rc.Width()), lround(rc.Height()));
		printf("SetPositionRelative(%lf, %lf, %lf, %lf) %p\n", rc.left + parent->x(), rc.top + parent->y(), rc.Width(), rc.Height(), w);
	}
}

PRectangle Window::GetClientPosition() const
{
	return GetPosition();
}

void Window::Show(const bool show)
{
	if (Fl_Widget* w = static_cast<Fl_Widget*>(wid); w)
	{
		if (show)
			w->show();
		else
			w->hide();
	}
}

void Window::InvalidateAll()
{
	if (wid)
		((Fl_Widget*)wid)->redraw();
}

void Window::InvalidateRectangle(const PRectangle rc)
{
	if (wid)
		((Fl_Widget*)wid)->damage(FL_DAMAGE_ALL, lround(rc.left), lround(rc.top), lround(rc.Width()), lround(rc.Height()));
}

void Window::SetCursor(const Cursor curs)
{
	if (wid) {
		Fl_Window* win = ((Fl_Widget*)wid)->top_window();
		if (win)
		{
			switch (curs) {
			case Cursor::text:
				win->cursor(FL_CURSOR_INSERT);
				break;
			case Cursor::up:
				win->cursor(FL_CURSOR_N);
				break;
			case Cursor::wait:
				win->cursor(FL_CURSOR_WAIT);
				break;
			case Cursor::horizontal:
				win->cursor(FL_CURSOR_WE);
				break;
			case Cursor::vertical:
				win->cursor(FL_CURSOR_NS);
				break;
			case Cursor::hand:
				win->cursor(FL_CURSOR_HAND);
				break;
			default:
				win->cursor(FL_CURSOR_ARROW);
				break;
			}
			cursorLast = curs;
		}
	}
}

PRectangle Window::GetMonitorRect(const Point pt)
{
	int X = 0, Y = 0, W = 0, H = 0; //dimensions of screen
	if (Fl_Widget* w = (Fl_Widget*)wid; w)
		Fl::screen_xywh(X, Y, W, H, w->x() + lround(pt.x), w->y() + lround(pt.y));
	else
		Fl::screen_xywh(X, Y, W, H);
	printf("GetMonitorRect(%lf, %lf) = [%d, %d, %d, %d]\n", pt.x, pt.y, X, Y, W, H);
	return PRectangle::FromInts(X, Y, X + W, Y + H);
}

//--------------------------------------------

static std::map<std::string, Fl_Font> load_fonts()
{
	std::map<std::string, Fl_Font> retval;
	for (Fl_Font f = 0, fonts = Fl::set_fonts(); f < fonts; ++f)
	{
		int attributes;
		const char* const name = Fl::get_font_name(f, &attributes);
		if (attributes == 0 && name)
			retval.emplace(name, f);
	}
	return retval;
}

class FontFLTK : public Font
{
private:
	static const std::map<std::string, Fl_Font> fonts;
public:
	FontFLTK(const FontParameters& fp)
	{
		font = fonts.at(fp.faceName);
		size = lround(fp.size);
	}
	Fl_Font font;
	Fl_Fontsize size;
};


const std::map<std::string, Fl_Font> FontFLTK::fonts = load_fonts();

std::shared_ptr<Font> Font::Allocate(const FontParameters& fp)
{
	return std::make_shared<FontFLTK>(fp);
}

//----------------------------------------------------------

ListBox::ListBox() noexcept {}

ListBox::~ListBox() noexcept {}

class ListBoxFLTK : public ListBox {
public:
	ListBoxFLTK() noexcept = default;

	void SetFont(const Font* font) override;
	void Create(Window& parent, int ctrlID, Point location, int lineHeight, bool unicodeMode_, Technology technology) override;
	void SetAverageCharWidth(int width) override;
	void SetVisibleRows(int rows) override;
	int GetVisibleRows() const override;
	PRectangle GetDesiredRect() override;
	int CaretFromEdge() override;
	void Clear() noexcept override;
	void Append(char* s, int type = -1) override;
	int Length() override;
	void Select(int n) override;
	int GetSelection() override;
	int Find(const char* prefix) override;
	std::string GetValue(int n) override;
	void RegisterImage(int type, const char* xpm_data) override;
	void RegisterRGBAImage(int type, int width, int height, const unsigned char* pixelsImage) override;
	void ClearRegisteredImages() override;
	void SetDelegate(IListBoxDelegate* lbDelegate) override;
	void SetList(const char* list, char separator, char typesep) override;
	void SetOptions(ListOptions options_) override;
private:
	std::map<int, std::unique_ptr<Fl_Image>> images;
	IListBoxDelegate* delegate = nullptr;
};

void ListBoxFLTK::SetFont(const Font* font) {
	if (const FontFLTK* f = dynamic_cast<const FontFLTK*>(font); f && wid) {
		Fl_Browser* browser = (Fl_Browser*)wid;
		browser->textfont(f->font);
		browser->textsize(f->size);
	}
}

void ListBoxFLTK::Create(Window& parent, int ctrlID, Point location, int lineHeight, bool, Technology)
{
	printf("ListBoxFLTK::Create()\n");
	if (Fl_Widget* w = (Fl_Widget*)parent.GetID(); w)
		Fl_Select_Browser* browser = new Fl_Select_Browser(lround(location.x), lround(location.y), 90, 90); //TODO: width/height
}

void ListBoxFLTK::SetAverageCharWidth(int) {}

void ListBoxFLTK::SetVisibleRows(int) {}

int ListBoxFLTK::GetVisibleRows() const { return 6; }

PRectangle ListBoxFLTK::GetDesiredRect() { return PRectangle{}; }

int ListBoxFLTK::CaretFromEdge() { return 0; }

void ListBoxFLTK::Clear() noexcept
{
	if (wid)
		((Fl_Browser*)wid)->clear();
}

void ListBoxFLTK::Append(char* s, const int type)
{
	if (wid && s) {
		Fl_Browser* browser = (Fl_Browser*)wid;
		const int i = browser->size();
		browser->insert(i, s);
		if (type >= 0)
			browser->icon(i, images.at(type).get());
	}
}

int ListBoxFLTK::Length()
{
	return wid ? ((Fl_Browser*)wid)->size() : 0;
}

void ListBoxFLTK::Select(const int n)
{
	if (wid)
		((Fl_Browser*)wid)->value(n);
}

int ListBoxFLTK::GetSelection()
{
	return wid ? ((Fl_Browser*)wid)->value() : 0;
}

int ListBoxFLTK::Find(const char* const prefix)
{
	if (wid) {
		Fl_Browser* browser = (Fl_Browser*)wid;
		const size_t len = strlen(prefix);
		for (int i = 0; i < browser->size(); ++i) //TODO: this is inefficient because Fl_Browser uses a linked list internally. but it doesn't publicly expose a good API
			if (strncmp(prefix, browser->text(i), len) == 0)
				return i;
	}
	return -1;
}

std::string ListBoxFLTK::GetValue(const int n)
{
	if (wid)
		return ((Fl_Browser*)wid)->text(n);
	return {};
}

void ListBoxFLTK::RegisterImage(int type, const char* xpm_data)
{
	images[type] = std::make_unique<Fl_Pixmap>(&xpm_data);
}

void ListBoxFLTK::RegisterRGBAImage(int type, int width, int height, const unsigned char* pixelsImage)
{
	images[type] = std::make_unique<Fl_RGB_Image>(pixelsImage, width, height, 4);
}

void ListBoxFLTK::ClearRegisteredImages()
{
	if (wid) {
		Fl_Browser* browser = (Fl_Browser*)wid;
		for (int i = 0; i < browser->size(); ++i) //TODO: this is inefficient because Fl_Browser uses a linked list internally. but it doesn't publicly expose a good API
			browser->icon(i, nullptr);
	}
	images.clear();
}

void ListBoxFLTK::SetDelegate(IListBoxDelegate* lbDelegate)
{
	delegate = lbDelegate;
}

void ListBoxFLTK::SetList(const char* const list, const char separator, const char typesep)
{
	// This method is *not* platform dependent.
	// It is borrowed from the GTK implementation.
	Clear();
	size_t count = strlen(list) + 1;
	std::vector<char> words(list, list + count);
	char* startword = &words[0];
	char* numword = nullptr;
	for (int i = 0; words[i]; i++)
	{
		if (words[i] == separator)
		{
			words[i] = '\0';
			if (numword)
				*numword = '\0';
			Append(startword, numword ? atoi(numword + 1) : -1);
			startword = &words[0] + i + 1;
			numword = nullptr;
		}
		else if (words[i] == typesep)
		{
			numword = &words[0] + i;
		}
	}
	if (startword)
	{
		if (numword)
			*numword = '\0';
		Append(startword, numword ? atoi(numword + 1) : -1);
	}
}

void ListBoxFLTK::SetOptions(ListOptions)
{
}

std::unique_ptr<ListBox> ListBox::Allocate()
{
	return std::make_unique<ListBoxFLTK>();
}

//-------------------------------------------------------------

class SurfaceFLTK : public Surface
{
public:
	std::unique_ptr<Surface> AllocatePixMap(const int width, const int height) override;
	void SetMode(SurfaceMode) override;
	void Release() noexcept override;
	int SupportsFeature(const Supports feature) noexcept override;
	int LogPixelsY() override;
	int PixelDivisions() override;
	int DeviceHeightFont(int points) override;

	void LineDraw(Point start, Point end, Stroke stroke) override;
	void PolyLine(const std::ranges::forward_range auto& pts, const Stroke stroke);
	void Polygon(const std::ranges::forward_range auto& pts, const FillStroke fillstroke);
	void RectangleDraw(PRectangle rect, FillStroke fillstroke) override;
	void RectangleFrame(const PRectangle rect, const Stroke stroke) override;
	void FillRectangle(const PRectangle rect, const Fill fill) override;
	void FillRectangle(PRectangle rc, Surface& surfacePattern) override;
	void FillRectangleAligned(const PRectangle rc, const Fill fill) override;
	void RoundedRectangle(const PRectangle rect, const FillStroke fillstroke) override;
	void AlphaRectangle(const PRectangle rect, const XYPOSITION cornerSize, const FillStroke fillstroke) override;
	void GradientRectangle(const PRectangle rect, const std::vector<ColourStop>& stops, const GradientOptions options) override;
	void DrawRGBAImage(const PRectangle rect, const int width, const int height, const unsigned char* pixelsImage) override;
	void Ellipse(const PRectangle rect, const FillStroke fillstroke) override;
	void Stadium(const PRectangle rect, const FillStroke fillstroke, const Ends ends) override;

	void Copy(PRectangle rc, Point from, Surface& surfaceSource) override;

	void DrawTextNoClip(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore, const ColourRGBA back) override;
	void DrawTextClipped(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore, const ColourRGBA back) override;
	void DrawTextTransparent(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore) override;

	void DrawTextNoClipUTF8(PRectangle rc, const Font* font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore, ColourRGBA back) override;
	void DrawTextClippedUTF8(PRectangle rc, const Font* font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore, ColourRGBA back) override;
	void DrawTextTransparentUTF8(PRectangle rc, const Font* font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore) override;

	void SetClip(const PRectangle rect) override;

	std::unique_ptr<IScreenLineLayout> Layout(const IScreenLine* screenLine) override;

	void MeasureWidths(const Font* font_, std::string_view text, XYPOSITION* positions) override;
	XYPOSITION WidthText(const Font* font_, std::string_view text) override;
	void MeasureWidthsUTF8(const Font* font_, std::string_view text, XYPOSITION* positions) override;
	XYPOSITION WidthTextUTF8(const Font* font_, std::string_view text) override;

	XYPOSITION Ascent(const Font* font_) override;
	XYPOSITION Descent(const Font* font_) override;
	XYPOSITION InternalLeading(const Font* font_) override;
	XYPOSITION Height(const Font* font_) override;
	XYPOSITION AverageCharWidth(const Font* font_) override;
};

class SurfaceOffscreen : public SurfaceFLTK {
public:
	SurfaceOffscreen(int w, int h);
	Fl_Image_Surface surf;

	void Init(WindowID wid) override;
	void Init(SurfaceID sid, WindowID wid) override;
	bool Initialised() override;

	void LineDraw(Point start, Point end, Stroke stroke) override;
	void PolyLine(const Point* pts, size_t npts, Stroke stroke) override;
	void Polygon(const Point* pts, size_t npts, FillStroke fillStroke) override;
	void RectangleFrame(PRectangle rc, Stroke stroke) override;
	void FillRectangle(PRectangle rc, Fill fill) override;
	void FillRectangle(PRectangle rc, Surface& surfacePattern) override;
	void RoundedRectangle(PRectangle rc, FillStroke fillStroke) override;

	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char* pixelsImage) override;
	void Ellipse(PRectangle rc, FillStroke fillStroke) override;
	void Copy(PRectangle rc, Point from, Surface& surfaceSource) override;

	void DrawTextTransparent(PRectangle rc, const Font* font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore) override;

	void SetClip(PRectangle rc) override;
	void PopClip() override;
	void FlushCachedState() override;
	void FlushDrawing() override;
};

void SurfaceFLTK::SetMode(SurfaceMode mode)
{
	printf("SetMode(%d)\n", mode.codePage);
}

void SurfaceFLTK::Release() noexcept
{
}

int SurfaceFLTK::SupportsFeature(const Supports feature) noexcept
{
	printf("SupportsFeature(%u)\n", (unsigned int)feature);
	switch (feature)
	{
	case Supports::LineDrawsFinal: return 0;
	case Supports::PixelDivisions: return 0;
	case Supports::FractionalStrokeWidth: return 0;
	case Supports::TranslucentStroke: return 0;
	case Supports::PixelModification: return 0;
	case Supports::ThreadSafeMeasureWidths: return 0;
	default: return 0;
	}
}

int SurfaceFLTK::LogPixelsY()
{
	float x, y;
	Fl::screen_dpi(x, y); //TODO: use correct screen number
	printf("LogPixelsY() = %f\n", y);
	return lroundf(y);
}

int SurfaceFLTK::PixelDivisions()
{
	return 1;
}

int SurfaceFLTK::DeviceHeightFont(const int points)
{
	printf("DeviceHeightFont(%d)\n", points);
	return points;
}

void SurfaceFLTK::LineDraw(const Point start, const Point end, const Stroke stroke)
{
	printf("LineDraw(%lf, %lf, %lf, %lf)\n", start.x, start.y, end.x, end.y);
	fl_color(stroke.colour.GetRed(), stroke.colour.GetGreen(), stroke.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_line_style(0, lround(stroke.width));
	fl_line(lround(start.x), lround(start.y), lround(end.x), lround(end.y));
}

void SurfaceFLTK::PolyLine(const std::ranges::forward_range auto& pts, const Stroke stroke)
{
	printf("Polyline(");
	fl_color(stroke.colour.GetRed(), stroke.colour.GetGreen(), stroke.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_line_style(0, lround(stroke.width));
	// TODO: set line joins and caps
	for (size_t i = 1; i < pts.size(); ++i)
		fl_line(lround(pts[i - 1].x), lround(pts[i - 1].y), lround(pts[i].x), lround(pts[i].y));
	printf(")\n");
}

void SurfaceFLTK::Polygon(const std::ranges::forward_range auto& pts, const FillStroke fillstroke)
{
	printf("Polygon(");
	fl_color(fillstroke.fill.colour.GetRed(), fillstroke.fill.colour.GetGreen(), fillstroke.fill.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_begin_polygon(); //TODO: use fl_begin_complex_polygon?
	for (const auto& pt : pts)
		fl_vertex(lround(pt.x), lround(pt.y));
	fl_end_polygon();
	PolyLine(pts, fillstroke.stroke);
	printf(")\n");
}

void SurfaceFLTK::RectangleDraw(PRectangle rect, FillStroke fillstroke)
{
	FillRectangle(rect, fillstroke.fill);
	RectangleFrame(rect, fillstroke.stroke);
}

void SurfaceFLTK::RectangleFrame(const PRectangle rect, const Stroke stroke)
{
	fl_color(stroke.colour.GetRed(), stroke.colour.GetGreen(), stroke.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_line_style(0, lround(stroke.width));
	fl_rect(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()));
	printf("RectangleFrame(%lf, %lf, %lf, %lf)\n", rect.left, rect.top, rect.Width(), rect.Height());
}

void SurfaceFLTK::FillRectangle(const PRectangle rect, const Fill fill)
{
	fl_color(fill.colour.GetRed(), fill.colour.GetGreen(), fill.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_rectf(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()));
	printf("FillRectangle([%lf, %lf, %lf, %lf], [%d, %d, %d])\n", rect.left, rect.top, rect.Width(), rect.Height(), fill.colour.GetRed(), fill.colour.GetGreen(), fill.colour.GetBlue());
}

void SurfaceFLTK::FillRectangle(const PRectangle rect, Surface& surfacePattern)
{
	printf("PatternRectangle(%lf, %lf, %lf, %lf)\n", rect.left, rect.top, rect.Width(), rect.Height());
	if (SurfaceOffscreen* pattern = dynamic_cast<SurfaceOffscreen*>(&surfacePattern); pattern)
		for (double x = rect.left; x < rect.right; x += pattern->surf.image()->w())
			for (double y = rect.top; y < rect.bottom; y += pattern->surf.image()->h())
				pattern->surf.image()->draw(lround(x), lround(y)); //TODO: this will overflow outside the bounds of rect, if the dimensions of rect are not an even multiple of surfacepattern
	//TODO: else
}

void SurfaceFLTK::FillRectangleAligned(const PRectangle rc, const Fill fill)
{
	FillRectangle(PixelAlign(rc, 1), fill);
}

void SurfaceFLTK::RoundedRectangle(const PRectangle rect, const FillStroke fillstroke)
{
	AlphaRectangle(rect, 3.0, fillstroke);
}

void SurfaceFLTK::AlphaRectangle(const PRectangle rect, const XYPOSITION cornerSize, const FillStroke fillstroke)
{
	//TODO: fltk does not support transparency
	fl_color(fillstroke.fill.colour.GetRed(), fillstroke.fill.colour.GetGreen(), fillstroke.fill.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_rounded_rectf(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()), 3);

	fl_color(fillstroke.stroke.colour.GetRed(), fillstroke.stroke.colour.GetGreen(), fillstroke.stroke.colour.GetBlue()); //TODO: fltk does not support transparency
	fl_line_style(0, lround(fillstroke.stroke.width));
	fl_rounded_rect(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()), lround(cornerSize));
	printf("AlphaRectangle(%lf, %lf, %lf, %lf)\n", rect.left, rect.top, rect.Width(), rect.Height());
}

void SurfaceFLTK::GradientRectangle(const PRectangle rect, const std::vector<ColourStop>& stops, const GradientOptions)
{
	//TODO: fltk does not support gradients
	FillRectangle(rect, stops.at(0).colour);
}

void SurfaceFLTK::DrawRGBAImage(const PRectangle rect, const int width, const int height, const unsigned char* pixelsImage)
{
	//TODO: fltk does not support transparency
	printf("DrawRGBAImage(%lf, %lf, %lf, %lf, %d, %d)\n", rect.left, rect.top, rect.Width(), rect.Height(), width, height);
	fl_draw_image(pixelsImage, lround(rect.left), lround(rect.top), width, height, 4); //TODO: scale image to fit inside rectangle
}

void SurfaceFLTK::Ellipse(const PRectangle rect, const FillStroke fillstroke)
{
	printf("Ellipse(%lf, %lf, %lf, %lf)\n", rect.left, rect.top, rect.Width(), rect.Height());
	fl_color(fillstroke.fill.colour.GetRed(), fillstroke.fill.colour.GetGreen(), fillstroke.fill.colour.GetBlue()); //TODO: transparency
	fl_pie(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()), 0, 360);

	fl_color(fillstroke.stroke.colour.GetRed(), fillstroke.stroke.colour.GetGreen(), fillstroke.stroke.colour.GetBlue()); //TODO: transparency
	fl_line_style(0, lround(fillstroke.stroke.width));
	fl_arc(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()), 0, 360);
}

void SurfaceFLTK::Stadium(const PRectangle rect, const FillStroke fillstroke, const Ends)
{
	RectangleDraw(rect, fillstroke);
	//TODO: what the heck is a stadium?
}

void SurfaceFLTK::Copy(const PRectangle rect, const Point from, Surface& src_)
{
	printf("Copy([%lf, %lf, %lf, %lf], [%lf, %lf])\n", rect.left, rect.top, rect.Width(), rect.Height(), from.x, from.y);
	if (SurfaceOffscreen* src = dynamic_cast<SurfaceOffscreen*>(&src_); src)
		src->surf.image()->draw(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()), lround(from.x), lround(from.y));
	else
		return; //TODO
}

void SurfaceFLTK::DrawTextNoClip(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore, const ColourRGBA back)
{
	FillRectangle(rect, back);
	DrawTextTransparent(rect, font_, ybase, text, fore);
}

void SurfaceFLTK::DrawTextClipped(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore, const ColourRGBA back)
{
	SetClip(rect);
	DrawTextNoClip(rect, font_, ybase, text, fore, back);
	PopClip();
}

void SurfaceFLTK::DrawTextTransparent(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore)
{
	//TODO: what is ybase?
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
		fl_font(font->font, font->size);
	fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());
	fl_draw(text.data(), text.size(), lround(rect.left), lround(rect.bottom) - fl_descent()); //TODO: use rect width/height?
	printf("DrawText(%lf, %lf, %lf, %.*s)\n", rect.left, rect.bottom, ybase, (int)text.size(), text.data());
}

void SurfaceFLTK::DrawTextNoClipUTF8(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore, const ColourRGBA back)
{
	DrawTextNoClip(rect, font_, ybase, text, fore, back);
}

void SurfaceFLTK::DrawTextClippedUTF8(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore, const ColourRGBA back)
{
	DrawTextClipped(rect, font_, ybase, text, fore, back);
}

void SurfaceFLTK::DrawTextTransparentUTF8(const PRectangle rect, const Font* font_, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore)
{
	DrawTextTransparent(rect, font_, ybase, text, fore);
}

void SurfaceFLTK::SetClip(const PRectangle rect)
{
	fl_push_clip(lround(rect.left), lround(rect.top), lround(rect.Width()), lround(rect.Height()));
	printf("SetClip(%lf, %lf, %lf, %lf)\n", rect.left, rect.top, rect.Width(), rect.Height());
}

std::unique_ptr<IScreenLineLayout> SurfaceFLTK::Layout(const IScreenLine*)
{
	return {};
}

void SurfaceFLTK::MeasureWidths(const Font* font_, const std::string_view text, XYPOSITION* positions)
{
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
		fl_font(font->font, font->size);
	printf("MeasureWidths(%.*s) = [", (int)text.size(), text.data());
	for (const char* s = text.data(); s < text.data() + text.size();)
	{
		const int len = fl_utf8len1(*s);
		const int w = fl_width(text.data(), s + len - text.data());
		printf("%d ", w);
		for (size_t j = 0; j < len; ++j)
			positions[(s - text.data()) + j] = w;
		s += len;
	}
	printf("]\n");
}

XYPOSITION SurfaceFLTK::WidthText(const Font* font_, const std::string_view text)
{
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
		fl_font(font->font, font->size);
	const int w = fl_width(text.data(), text.size());
	printf("WidthText(%.*s) = %d\n", (int)text.size(), text.data(), w);
	return w;
}

void SurfaceFLTK::MeasureWidthsUTF8(const Font* font_, std::string_view text, XYPOSITION* positions)
{
	MeasureWidths(font_, text, positions);
}

XYPOSITION SurfaceFLTK::WidthTextUTF8(const Font* font_, std::string_view text)
{
	return WidthText(font_, text);
}

XYPOSITION SurfaceFLTK::Ascent(const Font* font_)
{
	//TODO: fltk has a fl_descent function, but not fl_ascent
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
		fl_font(font->font, font->size);
	return fl_height() - fl_descent();
}

XYPOSITION SurfaceFLTK::Descent(const Font* font_)
{
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
		fl_font(font->font, font->size);
	return fl_descent();
}

XYPOSITION SurfaceFLTK::InternalLeading(const Font*)
{
	return 0;
}

XYPOSITION SurfaceFLTK::Height(const Font* font_)
{
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
	{
		fl_font(font->font, font->size);
		printf("Height(%d, %d) = %d\n", font->font, font->size, fl_height());
	}
	return fl_height();
}

XYPOSITION SurfaceFLTK::AverageCharWidth(const Font* font_)
{
	if (const FontFLTK* font = dynamic_cast<const FontFLTK*>(font_); font)
		fl_font(font->font, font->size);
	return fl_width("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 -+=,.") / 68.0;
}

std::unique_ptr<Surface> SurfaceFLTK::AllocatePixMap(const int width, const int height)
{
	printf("AllocatePixMap(%d, %d)\n", width, height);
	return std::make_unique<SurfaceOffscreen>(width, height);
}

SurfaceOffscreen::SurfaceOffscreen(const int w, const int h) : surf(w, h)
{
}

void SurfaceOffscreen::Init(WindowID wid)
{
}

void SurfaceOffscreen::Init(SurfaceID sid, WindowID wid)
{
}

bool SurfaceOffscreen::Initialised()
{
	return true;
}

void SurfaceOffscreen::LineDraw(Point start, Point end, Stroke stroke)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::LineDraw(start, end, stroke);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::PolyLine(const Point* pts, size_t npts, Stroke stroke)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::PolyLine(std::span<const Point>{pts, npts}, stroke);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::Polygon(const Point* pts, size_t npts, FillStroke fillstroke)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::Polygon(std::span<const Point>{pts, npts}, fillstroke);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::RectangleFrame(PRectangle rect, Stroke stroke)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::RectangleFrame(rect, stroke);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::FillRectangle(PRectangle rect, Fill fill)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::FillRectangle(rect, fill);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::FillRectangle(PRectangle rect, Surface& pattern)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::FillRectangle(rect, pattern);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::RoundedRectangle(PRectangle rect, FillStroke fillstroke)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::RoundedRectangle(rect, fillstroke);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::DrawRGBAImage(PRectangle rect, int width, int height, const unsigned char* pixelsImage)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::DrawRGBAImage(rect, width, height, pixelsImage);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::Ellipse(const PRectangle rect, const FillStroke fillstroke)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::Ellipse(rect, fillstroke);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::Copy(PRectangle rect, Point from, Surface& src)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::Copy(rect, from, src);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::DrawTextTransparent(PRectangle rect, const Font* font, XYPOSITION ybase, std::string_view text, ColourRGBA fore)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::DrawTextTransparent(rect, font, ybase, text, fore);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::SetClip(const PRectangle rect)
{
	Fl_Surface_Device::push_current(&surf);
	SurfaceFLTK::SetClip(rect);
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::PopClip()
{
	Fl_Surface_Device::push_current(&surf);
	fl_pop_clip();
	printf("PopClip()\n");
	Fl_Surface_Device::pop_current();
}

void SurfaceOffscreen::FlushCachedState()
{
	//TODO
}

void SurfaceOffscreen::FlushDrawing()
{
	//TODO
}

//Draw directly/immediately in a widget
class SurfaceWidget : public SurfaceFLTK {
public:
	SurfaceWidget() = default;
	~SurfaceWidget() = default;

	void Init(WindowID wid) override;
	void Init(SurfaceID sid, WindowID wid) override;
	bool Initialised() override;

	void LineDraw(Point start, Point end, Stroke stroke) override;
	void PolyLine(const Point* pts, size_t npts, Stroke stroke) override;
	void Polygon(const Point* pts, size_t npts, FillStroke fillStroke) override;
	void RectangleFrame(PRectangle rc, Stroke stroke) override;
	void FillRectangle(PRectangle rc, Fill fill) override;
	void FillRectangle(PRectangle rc, Surface& surfacePattern) override;
	void RoundedRectangle(PRectangle rc, FillStroke fillStroke) override;

	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char* pixelsImage) override;
	void Ellipse(PRectangle rc, FillStroke fillStroke) override;
	void Copy(PRectangle rc, Point from, Surface& surfaceSource) override;

	void DrawTextTransparent(PRectangle rc, const Font* font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore) override;

	void SetClip(PRectangle rc) override;
	void PopClip() override;
	void FlushCachedState() override;
	void FlushDrawing() override;
private:
	Fl_Widget* widget = nullptr;
	Point Transform(Point);
	PRectangle Transform(PRectangle);
};

//transform from Scintilla coordinates (relative to the widget)
//to fltk coordinates (relative to the parent window)
Point SurfaceWidget::Transform(const Point pt)
{
	return Point(
		widget->x() + pt.x,
		widget->y() + pt.y
	);
}

PRectangle SurfaceWidget::Transform(const PRectangle rect)
{
	return PRectangle(
		widget->x() + rect.left,
		widget->y() + rect.top,
		widget->x() + rect.right,
		widget->y() + rect.bottom
	);
}

void SurfaceWidget::Init(const WindowID wid)
{
	widget = (Fl_Widget*)wid;
}

void SurfaceWidget::Init(const SurfaceID sid, const WindowID wid)
{
	widget = (Fl_Widget*)wid;
	//TODO: use SurfaceID?
}

bool SurfaceWidget::Initialised()
{
	return widget != nullptr;
}

void SurfaceWidget::LineDraw(const Point start, const Point end, const Stroke stroke)
{
	SurfaceFLTK::LineDraw(Transform(start), Transform(end), stroke);
}

void SurfaceWidget::PolyLine(const Point* pts, const size_t npts, const Stroke stroke)
{
	SurfaceFLTK::PolyLine(std::span<const Point>{pts, npts} | std::ranges::views::transform([this](Point pt) { return Transform(pt); }), stroke);
}

void SurfaceWidget::Polygon(const Point* pts, const size_t npts, const FillStroke fillstroke)
{
	SurfaceFLTK::Polygon(std::span<const Point>{pts, npts} | std::ranges::views::transform([this](Point pt) { return Transform(pt); }), fillstroke);
}

void SurfaceWidget::RectangleFrame(const PRectangle rect, const Stroke stroke)
{
	SurfaceFLTK::RectangleFrame(Transform(rect), stroke);
}

void SurfaceWidget::FillRectangle(const PRectangle rect, const Fill fill)
{
	SurfaceFLTK::FillRectangle(Transform(rect), fill);
}

void SurfaceWidget::FillRectangle(const PRectangle rect, Surface& surfacePattern)
{
	SurfaceFLTK::FillRectangle(Transform(rect), surfacePattern);
}

void SurfaceWidget::RoundedRectangle(const PRectangle rect, const FillStroke fillstroke)
{
	SurfaceFLTK::RoundedRectangle(Transform(rect), fillstroke);
}

void SurfaceWidget::DrawRGBAImage(const PRectangle rect, const int width, const int height, const unsigned char* pixelsImage)
{
	SurfaceFLTK::DrawRGBAImage(Transform(rect), width, height, pixelsImage);
}

void SurfaceWidget::Ellipse(const PRectangle rect, const FillStroke fillstroke)
{
	SurfaceFLTK::Ellipse(Transform(rect), fillstroke);
}

void SurfaceWidget::Copy(const PRectangle rect, const Point from, Surface& src)
{
	SurfaceFLTK::Copy(Transform(rect), from, src);
}

void SurfaceWidget::DrawTextTransparent(const PRectangle rect, const Font* font, const XYPOSITION ybase, const std::string_view text, const ColourRGBA fore)
{
	SurfaceFLTK::DrawTextTransparent(Transform(rect), font, ybase, text, fore);
}

void SurfaceWidget::SetClip(const PRectangle rect)
{
	SurfaceFLTK::SetClip(Transform(rect));
}

void SurfaceWidget::PopClip()
{
	fl_pop_clip();
	printf("PopClip()\n");
}

void SurfaceWidget::FlushCachedState()
{
	//TODO
}

void SurfaceWidget::FlushDrawing()
{
	widget->redraw();
}

std::unique_ptr<Surface> Surface::Allocate(Technology) {
	return std::make_unique<SurfaceWidget>();
}

//------------------------------------

Menu::Menu() noexcept : mid(nullptr)
{
}

void Menu::CreatePopUp()
{
	Destroy();
	mid = new std::vector<Fl_Menu_Item>;
}

void Menu::Destroy() noexcept
{
	if (mid)
	{
		delete static_cast<std::vector<Fl_Menu_Item>*>(mid);
		mid = nullptr;
	}
}
void Menu::Show(Point pt, const Window& /*w*/)
{
	std::vector<Fl_Menu_Item>* menu = static_cast<std::vector<Fl_Menu_Item>*>(mid);
	if (!menu->empty())
	{
		menu->emplace_back(nullptr);
		menu->at(0).popup(lround(pt.x), lround(pt.y));
	}
	Destroy();
}

//-------------------------------------

ColourRGBA Platform::Chrome()
{
	return ColourRGBA::FromRGB(Fl::get_color(FL_BACKGROUND_COLOR));
}

ColourRGBA Platform::ChromeHighlight()
{
	return ColourRGBA::FromRGB(Fl::get_color(FL_SELECTION_COLOR));
}

const char* Platform::DefaultFont()
{
	return Fl::get_font_name(FL_HELVETICA); //TODO: is FL_HELVETICA correct?
}

int Platform::DefaultFontSize()
{
	return FL_NORMAL_SIZE;
}

unsigned int Platform::DoubleClickTime()
{
	return 1; //TODO
}

void Platform::DebugDisplay(const char* s) noexcept
{
	fl_alert("Scintilla: %s", s);
}

void Platform::DebugPrintf(const char* format, ...) noexcept
{
	char buffer[2048];
	va_list args{};
	va_start(args, format);
	vsnprintf(buffer, std::size(buffer), format, args);
	va_end(args);
	Platform::DebugDisplay(buffer);
}

bool Platform::ShowAssertionPopUps(bool /*assertionPopUps*/) noexcept
{
	return true;
}

void Platform::Assert(const char* c, const char* file, int line) noexcept
{
	fl_alert("Assertion [%s] failed at %s %d", c, file, line);
}

}
