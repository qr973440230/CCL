QT -= gui
QT += network

TEMPLATE = lib

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
contains(DEFINES,USE_SERIALPORT){
    QT += serialport
    SOURCES += client/serialportclient.cpp
    HEADERS += client/serialportclient.h
}

SOURCES += \
        client/udpclient.cpp \
        client/tcpclient.cpp \
        server/tcpserver.cpp


HEADERS += \
        queue/abstractqueue.h \
        queue/dropqueue.h \
        queue/waitqueue.h \
        client/udpclient.h \
        client/tcpclient.h \
        server/tcpserver.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target


OBJECTS_DIR = $$OUT_PWD/objects_dir
MOC_DIR = $$OUT_PWD/moc_dir
UI_DIR = $$OUT_PWD/ui_dir
RCC_DIR = $$OUT_PWD/rcc_dir

TARGET = ccl

COMPILER_PATH = msvc
win32-g++{
    COMPILER_PATH = g++
}

win32-msvc{
    COMPILER_PATH = msvc
}

BUILD_PATH = debug
CONFIG(debug,debug|release){
    DESTDIR = $$OUT_PWD/debug
    BUILD_PATH = debug
}

CONFIG(release,debug|release){
    DESTDIR = $$OUT_PWD/release
    BUILD_PATH = release
}

INCLUDE_DIR = $$PWD/../out/include/$$TARGET
LIB_DIR = $$PWD/../out/lib/$$QT_ARCH/$$COMPILER_PATH/$$BUILD_PATH
SHARE_DIR = $$PWD/../out/share/$$QT_ARCH/$$COMPILER_PATH/$$BUILD_PATH

win32{
    libSrc = $$DESTDIR/*.lib
    libSrc = $$replace(libSrc,/,\\)
    libDst = $$replace(LIB_DIR,/,\\)
    aSrc = $$DESTDIR/*.a
    aSrc = $$replace(aSrc,/,\\)
    aDst = $$replace(LIB_DIR,/,\\)
    dllSrc = $$DESTDIR/*.dll
    dllSrc = $$replace(dllSrc,/,\\)
    dllDst = $$replace(SHARE_DIR,/,\\)
    includeSrc = $$PWD/*.h
    includeSrc = $$replace(includeSrc,/,\\)
    includeDst = $$replace(INCLUDE_DIR,/,\\)
    QMAKE_POST_LINK += xcopy $$libSrc $$libDst /D /Y /S /C /I &
    QMAKE_POST_LINK += xcopy $$aSrc $$aDst /D /Y /S /C /I &
    QMAKE_POST_LINK += xcopy $$dllSrc $$dllDst /D /Y /S /C /I &
    QMAKE_POST_LINK += xcopy $$includeSrc $$includeDst /D /Y /S /C /I &
}
