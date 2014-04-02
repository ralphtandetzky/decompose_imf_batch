#include "gui_main_window.h"
#include "ui_gui_main_window.h"

namespace gui {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

} // namespace gui
