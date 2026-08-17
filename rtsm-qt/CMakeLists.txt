set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)
set(QT_FORCE_CMP0156_TO_VALUE NEW)

qt_add_executable(apprtsm 
    main.cpp
    ui/qt/system_monitor_view_qt.cpp
    ui/qt/system_monitor_view_qt.hpp
    ui/qt/system_monitor_backend_fwd.hpp
)

set_target_properties(apprtsm PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

qt_add_qml_module(apprtsm
    URI rtsm
    VERSION 1.0
    QML_FILES ui/qt/qml/Main.qml
)

target_link_libraries(apprtsm PRIVATE 
    Qt6::Quick 
    RTSM::Core
)

include(GNUInstallDirs)
install(TARGETS apprtsm
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
