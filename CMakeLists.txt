cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
project(mx-tweak VERSION 2.0 LANGUAGES CXX)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets)

if(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    set(CMAKE_DEFAULT_BUILD_TYPE "Release")
endif()
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_SCAN_FOR_MODULES ON)

qt_standard_project_setup()

qt_add_executable(mx-tweak
    about.cpp about.h
    brightness_small.cpp brightness_small.h brightness_small.ui
    cmd.h
    main.cpp
    remove_user_theme_set.cpp remove_user_theme_set.h remove_user_theme_set.ui
    theming_to_tweak.cpp theming_to_tweak.h theming_to_tweak.ui
    tweak.cpp tweak.h tweak.ui
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
    version.h
    window_buttons.cpp window_buttons.h window_buttons.ui
    xfwm_compositor_settings.cpp xfwm_compositor_settings.h xfwm_compositor_settings.ui
    images.qrc
)
#target_sources(mx-tweak PRIVATE FILE_SET CXX_MODULES FILES
#    TODO: add files that are C++20 modules.
#)

target_compile_definitions(mx-tweak PRIVATE
    QT_DISABLE_DEPRECATED_UP_TO=0x060800
)

target_link_libraries(mx-tweak PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")
target_compile_definitions(mx-tweak PRIVATE $<$<CONFIG:Release>: NDEBUG>)
target_compile_options(mx-tweak PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
    -gdwarf-4
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:Clang>:-flto=thin>
        $<$<CXX_COMPILER_ID:GNU>:-flto=auto>
        -Werror
    >
)
target_link_options(mx-tweak PRIVATE
    -O2
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:Clang>:-flto=thin>
        $<$<CXX_COMPILER_ID:GNU>:-flto=auto -fno-fat-lto-objects>
    >
)

include(GNUInstallDirs)

install(TARGETS mx-tweak
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET mx-tweak
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
