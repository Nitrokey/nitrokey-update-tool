#-------------------------------------------------
#
# Project created by QtCreator 2018-05-29T11:13:41
#
#-------------------------------------------------

QT       += core gui concurrent
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nitrokey-update-tool
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    usb_connection.cpp \
    aboutdialog.cpp \
    usbdriverinstaller.cpp

HEADERS += \
        mainwindow.h \
    usb_connection.h \
    aboutdialog.h \
    usbdriverinstaller.h

FORMS += \
        mainwindow.ui \
    aboutdialog.ui

win32: LIBS += $$PWD/3rdparty/dfu-programmer/build-windows/libdfup.a
!win32: LIBS += $$PWD/3rdparty/dfu-programmer/build/libdfup.a
unix:!mac: LIBS += -lusb-1.0
win32: LIBS += -lusb-1.0
mac:LIBS += /usr/local/opt/libusb/lib//libusb-1.0.a -framework IOKit -framework CoreFoundation

INCLUDEPATH += 3rdparty/libusb/ \
        3rdparty/dfu-programmer/src/

win32: RESOURCES += resources.qrc


