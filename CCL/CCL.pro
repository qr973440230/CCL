QT -= gui
QT += serialport network

TEMPLATE = lib
DEFINES += CCL_LIBRARY

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

SOURCES += \
        client/serialportclient.cpp \
        client/udpclient.cpp \
        client/tcpclient.cpp \
        server/tcpserver.cpp


HEADERS += \
        queue/abstractqueue.h \
        queue/dropqueue.h \
        queue/waitqueue.h \
        client/serialportclient.h \
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

CONFIG(debug,debug|release){
    DESTDIR = $$OUT_PWD/debug
}

CONFIG(release,debug|release){
    DESTDIR = $$OUT_PWD/release
}

win32{
    libSrc = $$DESTDIR/*.lib
    libDst = $$OUT_PWD/lib
    libSrc = $$replace(libSrc,/,\\)
    libDst = $$replace(libDst,/,\\)
    aSrc = $$DESTDIR/*.a
    aDst = $$OUT_PWD/lib
    aSrc = $$replace(aSrc,/,\\)
    aDst = $$replace(aDst,/,\\)
    dllSrc = $$DESTDIR/*.dll
    dllDst = $$OUT_PWD/share
    dllSrc = $$replace(dllSrc,/,\\)
    dllDst = $$replace(dllDst,/,\\)
    includeSrc = $$PWD/*.h
    includeDst = $$OUT_PWD/include
    includeSrc = $$replace(includeSrc,/,\\)
    includeDst = $$replace(includeDst,/,\\)

    QMAKE_POST_LINK += xcopy $$libSrc $$libDst /D /Y /S /C /I &
    QMAKE_POST_LINK += xcopy $$aSrc $$aDst /D /Y /S /C /I &
    QMAKE_POST_LINK += xcopy $$dllSrc $$dllDst /D /Y /S /C /I &
    QMAKE_POST_LINK += xcopy $$includeSrc $$includeDst /D /Y /S /C /I &
}
