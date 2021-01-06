HEADERS += C:/opencv/build/include \
           $$files($$PWD/*.h, true)

SOURCES += $$PWD/BioImageOperation.cpp \
           $$files($$PWD/*.cpp, true)

FORMS += $$files($$PWD/*.ui, true)

RESOURCES += $$files($$PWD/*.qrc, true)
