#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <functional>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void timer_device_count();
    void on_btn_update_clicked();
    void on_btn_select_file_clicked();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    int32_t launchInThread(std::function<int32_t(void)> func);
};

#endif // MAINWINDOW_H
