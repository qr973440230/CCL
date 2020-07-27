QT -= gui
QT += network serialport

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    include/client/serialportclient.h \
    include/client/tcpclient.h \
    include/client/udpclient.h \
    include/queue/abstractqueue.h \
    include/queue/dropqueue.h \
    include/queue/waitqueue.h \
    include/server/tcpserver.h

SOURCES += \
        main.cpp

LIBS += -L$$PWD/lib -lccl

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
    cclIncludeSrc = $$OUT_PWD/../CCL/include
    cclIncludeDst = $$PWD/include
    cclIncludeSrc = $$replace(cclIncludeSrc,/,\\)
    cclIncludeDst = $$replace(cclIncludeDst,/,\\)
    cclLibSrc = $$OUT_PWD/../CCL/lib
    cclLibDst = $$PWD/lib
    cclLibSrc = $$replace(cclLibSrc,/,\\)
    cclLibDst = $$replace(cclLibDst,/,\\)
    cclDllSrc = $$OUT_PWD/../CCL/share
    cclDllDst = $$PWD/share
    cclDllDst2 = $$DESTDIR
    cclDllSrc = $$replace(cclDllSrc,/,\\)
    cclDllDst = $$replace(cclDllDst,/,\\)
    cclDllDst2 = $$replace(cclDllDst2,/,\\)

    system("xcopy $$cclIncludeSrc $$cclIncludeDst /D /Y /S /C /I ")
    system("xcopy $$cclLibSrc $$cclLibDst /D /Y /S /C /I ")
    system("xcopy $$cclDllSrc $$cclDllDst /D /Y /S /C /I ")
    system("xcopy $$cclDllSrc $$cclDllDst2 /D /Y /S /C /I ")
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
