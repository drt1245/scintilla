cmake_minimum_required (VERSION 3.12)

if (WIN32)
	add_compile_options(/W4 /Zi)
else()
	add_compile_options(-Wall -Wextra -O3 -g)
endif()

project ("scintilla_fltk")

add_library(Fl_Scintilla STATIC "PlatFLTK.cxx" "Fl_Scintilla.h" "Fl_Scintilla.cxx" "../src/XPM.cxx" "../src/ViewStyle.cxx" "../src/UniqueString.cxx" "../src/UniConversion.cxx" "../src/UndoHistory.cxx" "../src/Style.cxx" "../src/Selection.cxx" "../src/ScintillaBase.cxx" "../src/RunStyles.cxx" "../src/RESearch.cxx" "../src/PositionCache.cxx" "../src/PerLine.cxx" "../src/MarginView.cxx" "../src/LineMarker.cxx" "../src/KeyMap.cxx" "../src/Indicator.cxx" "../src/Geometry.cxx" "../src/EditView.cxx" "../src/Editor.cxx" "../src/EditModel.cxx" "../src/Document.cxx" "../src/Decoration.cxx" "../src/DBCS.cxx" "../src/ContractionState.cxx" "../src/CharClassify.cxx" "../src/CharacterType.cxx" "../src/CharacterCategoryMap.cxx" "../src/ChangeHistory.cxx" "../src/CellBuffer.cxx" "../src/CaseFolder.cxx" "../src/CaseConvert.cxx" "../src/CallTip.cxx" "../src/AutoComplete.cxx")
add_executable (scintilla_fltk "main.cpp")

find_package(FLTK REQUIRED NO_MODULE)
include_directories("../include" "../src" ${FLTK_INCLUDE_DIRS})
target_link_libraries(scintilla_fltk Fl_Scintilla ${FLTK_LIBRARIES})

set_property(TARGET Fl_Scintilla PROPERTY CXX_STANDARD 20)
set_property(TARGET scintilla_fltk PROPERTY CXX_STANDARD 20)
