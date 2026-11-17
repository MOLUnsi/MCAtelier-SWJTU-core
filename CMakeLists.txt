cmake_minimum_required(VERSION 3.19)
project(MCAtelier-core LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(MCAtelier-core
    WIN32 MACOSX_BUNDLE
    main.cpp
    widget.cpp
    widget.h
    widget.ui
    FileFunctions.h
    FileFunctions.cpp
    CreateLaunchScript.h
    CreateLaunchScript.cpp
    SystemFunctions.h
    SystemFunctions.cpp

    UI-FILES.LICENSE
    README.md
    res/icon.rc
)

if(WIN32)
    target_sources(MCAtelier-core PRIVATE res/icon.rc)
endif()

target_link_libraries(MCAtelier-core
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS MCAtelier-core
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET MCAtelier-core
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
