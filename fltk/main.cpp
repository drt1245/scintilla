#include "Fl_Scintilla.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

int main()
{
	Fl_Window win(640, 480, "FLTK Scintilla");
	printf("Fl_Window: %p\n", &win);
	Fl_Scintilla scintilla(10, 10, 620, 460);
	printf("Fl_Scintilla: %p\n", &scintilla);
	win.show();
	return Fl::run();
}
