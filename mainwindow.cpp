#include "mainwindow.h"
#include "ui_mainwindow.h"

struct AppState
{
    AppState() {
     firmware_file.selected = false;     
     device_connected.update_mode = false;
     device_connected.production_mode = false;
     in_progress = false;
    }

    struct {
        bool selected;        
    } firmware_file;
    union{
        struct {
            bool update_mode :1;
            bool production_mode :1;
        } device_connected;
        uint8_t device_connected_raw;
    };
    bool in_progress;
    inline bool ready_to_update(){
        return device_connected.update_mode && firmware_file.selected && !in_progress;
    }
} state;


QString WELCOME = "Copyright 2018 Nitrokey UG (https://www.nitrokey.com). \n"
                  "This software is licensed under GNU General Public License v3. \n"
                  "Source is available on https://github.com/Nitrokey/nitrokey-update-tool. \n"
                ;
QString WELCOME2 =  "This tool allows to update Nitrokey Storage's firmware. "
                    "<ol><li>- Please select "
                    "<I>Configure -> Enable Firmware Update</I> in the Nitrokey App (see <i>Links->Nitrokey App</i>), </li>"
                    "<li>- Download the .hex file of the latest firmware version (see <i>Links->Nitrokey Storage firmware</i>), </li>"
                    "<li>- Select the firmware file with 'Select firmware file',"
                    "<li>- Start the procedure by pressing 'Update firmware' button."
                    "</ol>"

//                    "<br />[1] https://github.com/Nitrokey/nitrokey-storage-firmware/releases/latest \n"
//                    "<br />[2] https://www.nitrokey.com/download \n"
//                    "<br />"
                  ;

#include <QTimer>
#include "windowscheckprivileges.h"
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->text_log->appendHtml(WELCOME2);
    logUI("");
    ui->btn_update->setEnabled(false);

    QTimer *timer_device_count = new QTimer(this);
    connect(timer_device_count, SIGNAL(timeout()), this, SLOT(timer_device_count()));
    timer_device_count->start(2000);

    #ifdef Q_OS_WIN
    if(!WindowsCheckPrivileges::IsElevated()){
        QMessageBox::warning(nullptr, "No Admin privileges",
                             "Application is run without Admin privileges, which are required to operate. Please run it again with Administrator privileges.");
    }
    #endif
}

#include "lib_implementation.h"
MainWindow::~MainWindow()
{
    delete ui;
    deinit();
}

#include <QTime>
void MainWindow::logUI(QString msg){
    static QTime t;
    if (!t.isValid()) t.start();
    if (!msg.isEmpty()){
        ui->text_log->appendPlainText( QString("[%1]: %2").arg(t.elapsed()/1000., 7).arg(msg));
    } else {
        ui->text_log->appendPlainText("");
    }
}

#include "usb_connection.h"
#include <QMutex>
#include <QMutexLocker>
void MainWindow::timer_device_count(){
    static USB_connection connection;
    static QMutex mtx;
    static int last_status = -1;

    ui->btn_quit->setEnabled(!state.in_progress);
    if (state.in_progress) return;

    state.device_connected.update_mode = connection.count_devices_in_update_mode() > 0;
    state.device_connected.production_mode = connection.count_devices_in_production_mode() > 0;

    ui->cb_device_connected->setChecked(state.device_connected.update_mode);
    if (last_status != state.device_connected_raw){
        if(state.device_connected.production_mode){
            logUI("Nitrokey Storage detected. Please enable firmware update mode in Nitrokey App first.");
        } else if(state.device_connected.update_mode){
            logUI("Nitrokey Storage detected in Update mode.");
        } else {
            logUI("No Storage device detected. Please insert Storage device in Update mode.");
        }
        last_status = state.device_connected_raw;
    }

    ui->btn_update->setEnabled(state.ready_to_update());
}

inline void delay(int millisecondsWait);

#include <QFileDialog>
#include <QFileInfo>
void MainWindow::on_btn_select_file_clicked()
{
    auto filename = QFileDialog::getOpenFileName(this,
        tr("Open Firmware"), "", tr("Firmware files (*.hex)"));
    if(filename.isEmpty()) return;
    ui->edit_firmware_file->setText(filename);

    //if file exist and has correct extension
    state.firmware_file.selected = launchInThread([&filename](){
        QFileInfo check_file(filename);
        return check_file.exists() && check_file.isFile();
    });
    ui->cb_file_selected->setChecked(state.firmware_file.selected);
    logUI("Set file " + filename);
}

inline void delay(int millisecondsWait)
{
    QApplication::processEvents();

    QEventLoop loop;
    QTimer t;
    t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(millisecondsWait);
    loop.exec();
}

#include <functional>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
int32_t MainWindow::launchInThread(std::function<int32_t(void)> func){    
    QEventLoop loop;
    QFutureWatcher<int32_t> watcher;
    connect(&watcher, &QFutureWatcher<int32_t>::finished, &loop, &QEventLoop::quit);

    QFuture<int32_t> future = QtConcurrent::run(func);
    watcher.setFuture(future);
    loop.exec();

    return future.result();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (state.in_progress)
        event->ignore();
}

#include "usbdriverinstaller.h"
#include <QStringList>
void MainWindow::on_btn_update_clicked()
{
    int32_t result;

    if (state.in_progress) return;

    state.in_progress = true; //FIXME use ScopeGuard/Exit later
    ui->btn_update->setEnabled(!state.in_progress);
    ui->btn_quit->setEnabled(!state.in_progress);


    ui->progressBar->setValue(0);

#ifdef Q_OS_WIN
    {
        static bool drivers_installed = false;
        ui->progressBar->setValue(5);
        if (!drivers_installed){
            logUI("Installing USB drivers ...");
            USBDriverInstaller installer;
            QStringList debug;
            int retcode = installer.install([&debug](QString s){ debug.append(s); });
            if (retcode != 0){
                for (auto d : debug){
                    logUI("USB setup: " + d);
                }
                logUI("Installing USB drivers has failed. Please run the application with Administrator privileges and try again.");
                logUI(QString("Error code %1").arg(retcode));
                logUI(QString("Output\n ```\n%1\n```").arg(QString::fromUtf8(installer.output.data())));
                logUI(QString("Output (err)\n```\n%1\n```").arg(QString::fromUtf8(installer.output_err.data())));
                ui->progressBar->setValue(0);
                state.in_progress = false;
                return;
            }
            drivers_installed = true;
            logUI("   USB drivers installed.");
        }
    }
#endif

    result = init();    
    if (result != 0){
        logUI(QString("WARNING: Device initialization has failed (code %1).").arg(result));
        logUI("Device could not be initialized. Cancelling the procedure.");
#ifdef Q_OS_LINUX
        logUI("Please run the tool with root privileges (e.g. using 'sudo') or install udev rules.");
#endif
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }
    ui->progressBar->setValue(10);

    logUI("Erasing device ...");

    result = launchInThread([]()->int32_t {
            return erase();
    });

    if (result != 0){
        logUI(QString("WARNING: Erasing device result: %1").arg(result));
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }

    ui->progressBar->setValue(20);
    delay(1*1000);
    ui->progressBar->setValue(21);

    logUI("Flashing device ...");

    std::string path = ui->edit_firmware_file->text().toStdString();
    result = launchInThread([path]()->int32_t {
        return flash(path.c_str());
    });

    if (result != 0){
        logUI(QString("WARNING: Flashing device result: %1").arg(result));
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }

    ui->progressBar->setValue(80);

    for (int i=0; i<5; i++){
        ui->progressBar->setValue(80+i+1);
        delay(1*1000);
    }

    logUI("Launching device to production mode");
    result = launchInThread([]()->int32_t {
         return launch();
    });
    if (result != 0){
        logUI(QString("WARNING: Launch device result: %1").arg(result));
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }

    ui->progressBar->setValue(100);
    state.in_progress = false;
}

#include "aboutdialog.h"
void MainWindow::on_actionAbout_triggered()
{
    AboutDialog a;
    a.exec();
}

#include <QDesktopServices>
void MainWindow::on_actionNitrokey_Update_Tool_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Nitrokey/nitrokey-update-tool/releases"));
}

void MainWindow::on_actionNitrokey_Storage_firmware_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Nitrokey/nitrokey-storage-firmware/releases"));
}

void MainWindow::on_actionNitrokey_App_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Nitrokey/nitrokey-app/releases"));
}

void MainWindow::on_actionNitrokey_com_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.nitrokey.com"));
}

void MainWindow::on_actionNitrokey_Support_Forum_triggered()
{
    QDesktopServices::openUrl(QUrl("https://support.nitrokey.com/c/nitrokey-support"));
}

void MainWindow::on_btn_quit_clicked()
{
    QApplication::quit();
}
