#include "mainwindow.h"
#include "ui_mainwindow.h"

struct AppState
{
    AppState() {
     firmware_file.selected = false;     
     device_connected_update_mode = false;
     in_progress = false;
    }

    struct {
        bool selected;        
    } firmware_file;
    bool device_connected_update_mode;
    bool in_progress;
    inline bool ready_to_update(){
        return device_connected_update_mode && firmware_file.selected && !in_progress;
    }
} state;


QString WELCOME = "Copyright 2018 Nitrokey UG (https://www.nitrokey.com). \n"
                  "This software is licensed under GNU General Public License v3. \n"
                  "Source is available on https://github.com/Nitrokey/. \n"
                  ;

#include <QTimer>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->text_log->appendPlainText("Nitrokey Production Tool started");
    ui->text_log->appendPlainText(WELCOME);
    ui->btn_update->setEnabled(false);

    QTimer *timer_device_count = new QTimer(this);
    connect(timer_device_count, SIGNAL(timeout()), this, SLOT(timer_device_count()));
    timer_device_count->start(2000);
}

#include "lib_implementation.h"
MainWindow::~MainWindow()
{
    delete ui;
    deinit();
}

#include "usb_connection.h"
#include <QMutex>
#include <QMutexLocker>
void MainWindow::timer_device_count(){
    static USB_connection connection;
    static QMutex mtx;
    static int last_status = -1;

    if (state.in_progress) return;

    state.device_connected_update_mode = connection.count_devices_in_update_mode() > 0;

    ui->cb_device_connected->setChecked(state.device_connected_update_mode);
    if (last_status != state.device_connected_update_mode){
        ui->text_log->appendPlainText(QString("Device connected in update mode: %1").arg(
                                          state.device_connected_update_mode ? "true" : "false"));
        last_status = state.device_connected_update_mode;
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
    ui->text_log->appendPlainText("Set file " + filename);
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

void MainWindow::on_btn_update_clicked()
{
    int32_t result;

    if (state.in_progress) return;

    state.in_progress = true; //FIXME use ScopeGuard/Exit later
    ui->btn_update->setEnabled(!state.in_progress);


    ui->progressBar->setValue(0);
    result = init();    
    if (result != 0){
        ui->text_log->appendPlainText(QString("WARNING: Device initialization has failed (code %1).").arg(result));
        ui->text_log->appendPlainText("Device could not be initialized. Cancelling the procedure.");
#ifdef Q_OS_LINUX
        ui->text_log->appendPlainText("Please run the tool with root privileges (e.g. using 'sudo') or install udev rules.");
#endif
#ifdef Q_OS_WIN
        ui->text_log->appendPlainText("Please install USB drivers for the device. Follow instructions from the release site. See http://zadig.akeo.ie/.");
#endif
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }
    ui->progressBar->setValue(10);

    ui->text_log->appendPlainText("Erasing device ...");

    result = launchInThread([]()->int32_t {
            return erase();
    });

    if (result != 0){
        ui->text_log->appendPlainText(QString("WARNING: Erasing device result: %1").arg(result));
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }

    ui->progressBar->setValue(20);
    delay(1*1000);
    ui->progressBar->setValue(21);

    ui->text_log->appendPlainText("Flashing device ...");

    std::string path = ui->edit_firmware_file->text().toStdString();
    result = launchInThread([path]()->int32_t {
        return flash(path.c_str());
    });

    if (result != 0){
        ui->text_log->appendPlainText(QString("WARNING: Flashing device result: %1").arg(result));
        ui->progressBar->setValue(0);
        state.in_progress = false;
        return;
    }

    ui->progressBar->setValue(80);

    for (int i=0; i<5; i++){
        ui->progressBar->setValue(80+i+1);
        delay(1*1000);
    }

    ui->text_log->appendPlainText("Launching device to production mode");
    result = launchInThread([]()->int32_t {
         return launch();
    });
    if (result != 0){
        ui->text_log->appendPlainText(QString("WARNING: Launch device result: %1").arg(result));
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
