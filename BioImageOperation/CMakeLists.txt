# CMakeList.txt : CMake project for CMakeProjectTest, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.10)

string (TIMESTAMP NOW "%Y-%m-%d")

project (BioImageOperation VERSION "1.7.12" DESCRIPTION ${NOW} HOMEPAGE_URL "https://github.com/folterj/BioImageOperation")

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)

configure_file (
    "${PROJECT_SOURCE_DIR}/BioImageOperation.rc.in"
    "${PROJECT_SOURCE_DIR}/BioImageOperation.rc"
)

#set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019_64/lib/cmake" "C:/opencv/build/x64/vc15/lib")
    set(OpenCV_BIN "C:/opencv/build/x64/vc15/bin")
else()
    list(APPEND CMAKE_PREFIX_PATH "/usr/lib/qt5/bin" "/usr/lib/x86_64-linux-gnu")
    set(OpenCV_BIN "/usr/lib/x86_64-linux-gnu")
endif()

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB BIO_SRC
    "*.h"
    "*.cpp"
)

set(RESOURCE_FILES
  BioImageOperation.qrc
  BioImageOperation.rc
)

add_executable(${PROJECT_NAME}
    ${BIO_SRC}
    ${RESOURCE_FILES}
)

qt5_add_resources(RESOURCE_FILES ${RESOURCE})

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Gui)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})



get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
if(WIN32 AND NOT WINDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "windeployqt not found")
endif()

find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
if(APPLE AND NOT MACDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "macdeployqt not found")
endif()

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	    COMMAND "${CMAKE_COMMAND}" -E
		env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
		    "$<TARGET_FILE:${PROJECT_NAME}>"
	    COMMENT "Running windeployqt..."
	)

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	    COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${OpenCV_BIN}/opencv_world440.dll" ${CMAKE_CURRENT_BINARY_DIR}
	    COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${OpenCV_BIN}/opencv_world440d.dll" ${CMAKE_CURRENT_BINARY_DIR}
	    COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${OpenCV_BIN}/opencv_videoio_ffmpeg440_64.dll" ${CMAKE_CURRENT_BINARY_DIR}
	    COMMENT "Copying OpenCV dlls..."
	)
endif()

if(APPLE)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${MACDEPLOYQT_EXECUTABLE}"
            \"$<TARGET_FILE_DIR:${target}>/../..\"
            -always-overwrite
        COMMENT "Running windeployqt..."
    )
endif()



set(CPACK_GENERATOR WIX)
set(CPACK_WIX_ROOT "C:/Program Files (x86)/WiX Toolset v3.11" CACHE PATH "")
set(CPACK_WIX_UPGRADE_GUID "EDE41139-C8E6-4D98-AE85-E669F8CA6D48")

include(CPack)
include(InstallRequiredSystemLibraries)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT applications)
install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT applications)
install(DIRECTORY ${QT_INSTALLED_PATH}/plugins/platforms DESTINATION bin COMPONENT applications)
