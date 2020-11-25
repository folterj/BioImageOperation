# CMakeList.txt : CMake project for CMakeProjectTest, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.1.0)

string (TIMESTAMP NOW "%Y-%m-%d")

project (BioImageOperation VERSION "1.6.440.5" DESCRIPTION ${NOW} HOMEPAGE_URL "https://joostdefolter.info")

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)

configure_file (
    "${PROJECT_SOURCE_DIR}/BioImageOperation.rc.in"
    "${PROJECT_SOURCE_DIR}/BioImageOperation.rc"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/5.15.0/msvc2019_64/lib/cmake" "C:/opencv/build/x64/vc15/lib")

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

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Gui)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})


set(CPACK_GENERATOR WIX)
set(CPACK_WIX_ROOT "C:/Program Files (x86)/WiX Toolset v3.11" CACHE PATH "")
set(CPACK_WIX_UPGRADE_GUID "EDE41139-C8E6-4D98-AE85-E669F8CA6D48")

include(CPack)
include(InstallRequiredSystemLibraries)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT applications)
install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT applications)
install(DIRECTORY ${QT_INSTALLED_PATH}/plugins/platforms DESTINATION bin COMPONENT applications)
