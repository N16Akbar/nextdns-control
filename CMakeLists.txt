cmake_minimum_required(VERSION 3.16)

project(nextdns-control LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_MAJOR_VERSION 6)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(CTest)
include(KDEInstallDirs6)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Test)
find_package(KF6 REQUIRED COMPONENTS Auth CoreAddons)
find_package(Plasma REQUIRED COMPONENTS Plasma)

find_program(NEXTDNS_EXECUTABLE NAMES nextdns REQUIRED)
find_program(NMCLI_EXECUTABLE NAMES nmcli REQUIRED)
find_program(IP_EXECUTABLE NAMES ip REQUIRED)
find_program(SYSTEMCTL_EXECUTABLE NAMES systemctl REQUIRED)
find_program(RESOLVECTL_EXECUTABLE NAMES resolvectl REQUIRED)

configure_file(src/paths.h.in generated/paths.h @ONLY)

qt_policy(SET QTP0001 NEW)

qt_add_qml_module(nextdnscontrol
    URI io.github.n16akbar.nextdnscontrol
    VERSION 1.0
    SHARED
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/io/github/n16akbar/nextdnscontrol
    SOURCES
    src/dnscontroller.cpp
    src/dnscontroller.h
    src/networkstate.cpp
    src/networkstate.h
)

target_include_directories(nextdnscontrol PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(nextdnscontrol PRIVATE Qt6::Core Qt6::Gui Qt6::Qml KF6::AuthCore KF6::CoreAddons)
set_target_properties(nextdnscontrolplugin PROPERTIES PREFIX "lib")

install(TARGETS nextdnscontrol LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR})
install(TARGETS nextdnscontrolplugin LIBRARY DESTINATION ${KDE_INSTALL_QMLDIR}/io/github/n16akbar/nextdnscontrol)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/io/github/n16akbar/nextdnscontrol/qmldir
    ${CMAKE_CURRENT_BINARY_DIR}/io/github/n16akbar/nextdnscontrol/nextdnscontrol.qmltypes
    DESTINATION ${KDE_INSTALL_QMLDIR}/io/github/n16akbar/nextdnscontrol
)

add_executable(nextdnscontrolhelper
    src/helper.cpp
    src/helper.h
    src/networkstate.cpp
    src/networkstate.h
)
target_include_directories(nextdnscontrolhelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(nextdnscontrolhelper PRIVATE Qt6::Core KF6::AuthCore)
install(TARGETS nextdnscontrolhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(nextdnscontrolhelper io.github.n16akbar.nextdnscontrol root)
kauth_install_actions(io.github.n16akbar.nextdnscontrol io.github.n16akbar.nextdnscontrol.actions)

if(BUILD_TESTING)
    add_executable(networkstatetest
        tests/networkstatetest.cpp
                        src/networkstate.cpp
        src/networkstate.h
    )
    target_include_directories(networkstatetest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/generated)
    target_link_libraries(networkstatetest PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME networkstatetest COMMAND networkstatetest)
endif()

plasma_install_package(package io.github.n16akbar.nextdnscontrol)
