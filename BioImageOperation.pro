TARGET = BioImageOperation
TEMPLATE = app

QT += core widgets gui network

#Added for linux build
QT += printsupport

include(BioImageOperation/BioImageOperation.pri)

CONFIG += c++17
CONFIG += app_bundle
CONFIG += console

RC_FILE = BioImageOperation/BioImageOperation.rc
RC_ICONS = BioImageOperation/BioImageOperation.ico

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    QMAKE_CXXFLAGS += -stdlib=libc++

    INCLUDEPATH += /usr/local/include

    LIBS += -L/usr/local/lib \
            -lopencv_core

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}

unix {
    QMAKE_CXXFLAGS += -std=c++17

    INCLUDEPATH += /usr/include

    LIBS += -L/usr/lib \
            -lopencv_core

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}

win32 {
    INCLUDEPATH += C:/opencv/build/include

    CONFIG(debug,debug|release){
        LIBS += C:/opencv/build/x64/vc15/lib/opencv_world440d.lib

    }

    CONFIG(release,debug|release){
        LIBS += C:/opencv/build/x64/vc15/lib/opencv_world440.lib
    }
}