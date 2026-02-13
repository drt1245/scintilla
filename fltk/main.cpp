#include "Fl_Scintilla.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

int main()
{
	Fl_Window win(640, 480, "FLTK Scintilla");
	printf("Fl_Window: %p\n", &win);
	Fl_Button button(10, 5, 100, 20, "Button!");
	Fl_Scintilla scintilla(10, 30, 620, 440);
	win.resizable(scintilla);
	printf("Fl_Scintilla: %p\n", &scintilla);
	win.show();
	return Fl::run();
}
