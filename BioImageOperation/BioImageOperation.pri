unix {
HEADERS += /usr/include/opencv4 \
           $$files($$PWD/*.h, true)
}

win32 {
HEADERS += C:/opencv/build/include \
           $$files($$PWD/*.h, true)
}

SOURCES += $$files($$PWD/*.cpp, true)

FORMS += $$files($$PWD/*.ui, true)

RESOURCES += $$files($$PWD/*.qrc, true)
