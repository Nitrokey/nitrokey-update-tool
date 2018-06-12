#ifndef USBDRIVERINSTALLER_H
#define USBDRIVERINSTALLER_H

#include <QObject>
#include <QWidget>
#include <functional>
class USBDriverInstaller
{
public:
    USBDriverInstaller(){}
    int install(std::function<void(QString)> log);
    QByteArray output;
    QByteArray output_err;
};

#endif // USBDRIVERINSTALLER_H
