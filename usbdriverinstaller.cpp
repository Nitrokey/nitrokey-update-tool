#include "usbdriverinstaller.h"

#include <QDir>
#include <QTemporaryFile>
#include <QProcess>
#include <QFile>
#include <functional>
int USBDriverInstaller::install(std::function<void(QString)> log){
    QString originalPath = ":/usb_setup.exe";
    auto tempPath = QDir::temp().absoluteFilePath("usb_setup-XXXXXX.exe");
    QString toExecute;

    {
        QTemporaryFile f(tempPath);
        f.setAutoRemove(false);
        f.open(); //open to get temp name

        QFile src(originalPath);
        if (!src.open(QIODevice::ReadOnly)){
            log("Failed to open source file from the resource file.");
            return -1;
        }
        toExecute = f.fileName();

        log("Copying: " + toExecute);
        bool copy_success = f.write(src.readAll()) != -1;
        f.close();
        if (!copy_success) {
            log("Copying failed");
            return -2;
        }
    }

    log("Running: " + toExecute);
    QProcess *process = new QProcess(nullptr);
    process->start(toExecute);

    process->waitForStarted();
    process->waitForFinished();
    output = process->readAllStandardOutput();
    output_err = process->readAllStandardError();
    if(process->exitStatus() != QProcess::NormalExit){
        log("Invalid exit status");
        return -1;
    }
    int retcode = process->exitCode();
    delete process;
    return retcode;
}
