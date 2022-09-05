cmake_minimum_required(VERSION 3.5)

project(DualTeacher VERSION 0.1 LANGUAGES CXX)

set(QT_DIR "C:/Qt/6.3.1/msvc2019_64/lib/cmake/Qt6")
set(QTBinPath "${QT_DIR}/../../../bin")
set(Qt6_DIR ${QT_DIR})

set(DepsPath "${CMAKE_SOURCE_DIR}/deps/win")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets LinguistTools REQUIRED)

find_package(Qt6 COMPONENTS CoreTools GuiTools WidgetsTools LinguistTools REQUIRED)

find_package(Qt6 COMPONENTS OpenGL REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)

set(TS_FILES DualTeacher_en_001.ts)

if(DepsPath)
	# Dependencies path set by user or env var
else()
	set(DepsPath "" CACHE PATH "Path to compiled dependencies (e.g. D:/dependencies/win64)")
	message(WARNING "DepsPath variable is missing.  Please set this variable to specify path to compiled dependencies (e.g. D:/dependencies/win64)")
endif()

if(QTBinPath)
	# QTBinPath path set by user or env var
else()
	set(QTBinPath "" CACHE PATH "Path to qt bin (e.g. D:/msvc2019_64/bin)")
	message(WARNING "QTBinPath variable is missing.  Please set this variable to specify path to compiled dependencies (e.g. D:/msvc2019_64/bin)")
endif()


if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
  set(ARCH "x86")
else()
  set(ARCH "x64")
endif()

set(DualTeacher_HEADERS
         src/config.h
         src/AgoraRtcEngine.h
         src/agoracourse.h
         src/DlgSettings.h
         src/DlgVersion.h
         src/DlgInfo.h
         src/DlgVideoRoom.h
         src/DlgSettingAudio.h
         src/DlgSettingVideo.h
         src/DlgSettingSelect.h
          src/DlgExtend.h
         src/QRoundCornerDialog.h
         src/resource.h
         src/SettingsData.h
         src/VideoWidget.h
         src/video_render_opengl.h
)

set(DualTeacher_SOURCES
         src/main.cpp
         src/AgoraRtcEngine.cpp
         src/agoracourse.cpp
         src/DlgSettings.cpp
         src/DlgVersion.cpp
         src/DlgInfo.cpp
         src/DlgVideoRoom.cpp
         src/DlgSettingAudio.cpp
         src/DlgSettingVideo.cpp
         src/DlgSettingSelect.cpp
         src/QRoundCornerDialog.cpp
         src/SettingsData.cpp
         src/UILayout.cpp
         src/VideoWidget.cpp
         src/video_render_opengl.cpp
         src/DlgExtend.cpp
         src/agoracourse.ui
         src/DlgSettings.ui
         src/DlgVersion.ui
         src/DlgInfo.ui
         src/DlgVideoRoom.ui
         src/DlgSettingAudio.ui
         src/DlgSettingVideo.ui
         src/DlgSettingSelect.ui
         src/VideoWidget.ui
         src/DlgExtend.ui
)

qt_add_resources(DualTeacher_QRC_SOURCES src/AgoraCourse.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DualTeacher WIN32
        MANUAL_FINALIZATION
        ${DualTeacher_HEADERS}
        ${DualTeacher_SOURCES}
        ${DualTeacher_QRC_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET DualTeacher APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(DualTeacher SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(DualTeacher WIN32
            ${PROJECT_SOURCES}
            ${DualTeacher_QRC_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if(APPLE)
else()
    target_link_libraries(DualTeacher PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        Qt${QT_VERSION_MAJOR}::Gui
        ${DepsPath}/x64/agora_rtc_sdk.dll.lib
    )
  
endif()

include_directories(
    ${DepsPath}/high_level_api/include
    src/
)

set_target_properties(DualTeacher PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DualTeacher)
endif()

if(WIN32) 
    add_custom_command(TARGET DualTeacher POST_BUILD
        COMMAND("${CMAKE_COMMAND}" -E copy_directory "${DepsPath}/${ARCH}/" "${PROJECT_BINARY_DIR}/$<CONFIG>/")
        COMMAND("${CMAKE_COMMAND}" -E copy "${DepsPath}/ID_TEST_AUDIO.wav" "${PROJECT_BINARY_DIR}/$<CONFIG>/")
        COMMAND("${QTBinPath}/windeployqt.exe" "${PROJECT_BINARY_DIR}/$<CONFIG>/DualTeacher.exe")
    )
endif()
