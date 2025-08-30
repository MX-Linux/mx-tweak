cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
include(build.cmake)

project(mx-tweak
	VERSION 3.0
	DESCRIPTION "Desktop configuration tool for MX Linux"
	LANGUAGES CXX
)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_SCAN_FOR_MODULES ON)

qt_standard_project_setup()

qt_add_executable(mx-tweak
	about.cpp about.h
	brightness_small.cpp brightness_small.h
	cmd.cpp cmd.h
	main.cpp
	remove_user_theme_set.cpp remove_user_theme_set.h
	theming_to_tweak.cpp theming_to_tweak.h
	tweak.cpp tweak.h
	tweak_compositor.cpp tweak_compositor.h
	tweak_display.cpp tweak_display.h
	tweak_fluxbox.cpp tweak_fluxbox.h
	tweak_misc.cpp tweak_misc.h
	tweak_plasma.cpp tweak_plasma.h
	tweak_superkey.cpp tweak_superkey.h
	tweak_theme.cpp tweak_theme.h
	tweak_thunar.cpp tweak_thunar.h
	tweak_xfce.cpp tweak_xfce.h
	tweak_xfce_panel.cpp tweak_xfce_panel.h
	window_buttons.cpp window_buttons.h
	xfwm_compositor_settings.cpp xfwm_compositor_settings.h
	images.qrc
)
qt_add_ui(mx-tweak SOURCES
	brightness_small.ui
	remove_user_theme_set.ui
	theming_to_tweak.ui
	tweak.ui
	window_buttons.ui
	xfwm_compositor_settings.ui
)
#target_sources(mx-tweak PRIVATE FILE_SET CXX_MODULES FILES
#	TODO: add files that are C++20 modules.
#)

qt_add_translations(mx-tweak TS_FILES
	"translations/mx-tweak_en.ts"
	LRELEASE_OPTIONS -compress -nounfinished -removeidentical -silent
)

target_link_libraries(mx-tweak PRIVATE
	Qt::Core
	Qt::Gui
	Qt::Widgets
)

build_compilation_setup(mx-tweak)
build_version_definition(mx-tweak VERSION)

target_compile_definitions(mx-tweak PRIVATE
	QT_DISABLE_DEPRECATED_UP_TO=0x060800
)

include(GNUInstallDirs)
install(TARGETS mx-tweak
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
