cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
if(CMAKE_BINARY_DIR PATH_EQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR
        "Build is in the same directory as the sources. Source tree will become heavily polluted.\n"
        "Delete the CMakeCache.txt file and CMakeFiles folder and use one of the supplied presets."
    )
endif()

project(mx-tweak
    VERSION 3.0
    DESCRIPTION "Desktop configuration tool for MX Linux"
    LANGUAGES CXX
)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

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
#    TODO: add files that are C++20 modules.
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

# Get version from debian/changelog
execute_process(
    COMMAND dpkg-parsechangelog -SVersion
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE pkg_version
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE dpkg_result
)
if(NOT dpkg_result EQUAL 0)
    message(WARNING "Failed to get version from debian/changelog using dpkg-parsechangelog")
    set(pkg_version ${PROJECT_VERSION})
endif()

set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")
target_compile_definitions(mx-tweak PRIVATE
    QT_DISABLE_DEPRECATED_UP_TO=0x060800
    VERSION="${pkg_version}"
    $<$<CONFIG:Release>:NDEBUG>
)
target_compile_options(mx-tweak PRIVATE
    -Wall
    -Werror
    -Wextra
    -Wpedantic
    -O2
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-gdwarf-4>
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:Clang>:-flto=thin>
        $<$<CXX_COMPILER_ID:GNU>:-flto=auto -fno-fat-lto-objects>
    >
)
target_link_options(mx-tweak PRIVATE
    -O2
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:Clang>:-flto=thin>
        $<$<CXX_COMPILER_ID:GNU>:-flto=auto -fno-fat-lto-objects>
    >
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "/usr")
endif()
include(GNUInstallDirs)
install(TARGETS mx-tweak
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
