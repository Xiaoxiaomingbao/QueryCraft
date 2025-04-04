cmake_minimum_required(VERSION 3.30)
project(QueryCraft)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_PREFIX_PATH "D:/Download/Qt5.14.2/5.14.2/mingw73_64")

set(CMAKE_AUTOMOC ON)  # 启用 Qt MOC 自动生成
set(CMAKE_AUTOUIC ON)  # 如果有 UI 文件
set(CMAKE_AUTORCC ON)  # 如果有 Qt 资源文件

find_package(Qt5 COMPONENTS Core Gui Sql Widgets REQUIRED)

file(GLOB SOURCES "ui/*.cpp" "ui/*.h")

add_executable(QueryCraft
        main.cpp
        ${SOURCES})

target_link_libraries(QueryCraft
        Qt5::Core
        Qt5::Gui
        Qt5::Sql
        Qt5::Widgets
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    # 复制平台插件 qwindows.dll
    if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif()
    # 复制 Qt 依赖库
    foreach(QT_LIB Core Gui Sql Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(QT_LIB)
    # 复制 MySQL 所需的动态链接库
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/sqldrivers/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../res/qsqlmysql${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/sqldrivers/")
    foreach(DB_LIB libcrypto-3-x64 libssl-3-x64 libmysql)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../res/${DB_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/")
    endforeach (DB_LIB)
endif()
