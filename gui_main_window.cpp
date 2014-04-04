#include "gui_main_window.h"
#include "ui_gui_main_window.h"
#include "../cpp_utils/std_make_unique.h"

namespace gui {

struct MainWindow::Impl
{
    Ui::MainWindow ui;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , m{ std::make_unique<Impl>() }
{
    m->ui.setupUi(this);
}

MainWindow::~MainWindow()
{
}

} // namespace gui
