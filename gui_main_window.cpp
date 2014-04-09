#include "gui_main_window.h"
#include "ui_gui_main_window.h"
#include "parse_batch.h"
#include "../decompose_imf_lib/optimization_task.h"
#include "../cpp_utils/std_make_unique.h"
#include "../cpp_utils/exception_handling.h"

#include <list>
#include <QSettings>

static const char * tasksTextName = "tasksText";

namespace gui {

struct MainWindow::Impl
{
    std::list<dimf::OptimizationParams> optParams;
    Ui::MainWindow ui;
    dimf::OptimizationTask optTask;

    void updateState()
    {
        ui.runNextOptimizationPushButton->setEnabled( !optParams.empty() );
        ui.statusbar->showMessage(
                    QString("%1 optimization runs left.")
                    .arg(optParams.size()) );
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , m{ std::make_unique<Impl>() }
{
    m->ui.setupUi(this);
    m->ui.textEditor->setPlainText(
                QSettings().value( tasksTextName ).toString() );
    m->updateState();
}

MainWindow::~MainWindow()
{
    CU_SWALLOW_ALL_EXCEPTIONS_FROM
    {
        QSettings().setValue(
                    tasksTextName,
                    m->ui.textEditor->toPlainText() );
    };
}

void MainWindow::parse()
{
    auto optParams = parseBatch(
                std::istringstream(
                    m->ui.textEditor->toPlainText().toStdString()) );
    decltype(m->optParams)(
                make_move_iterator(begin(optParams)),
                make_move_iterator(end(optParams)) ).swap(m->optParams);
    m->updateState();
}

void MainWindow::runNextOptimization()
{
    m->optTask.restart( m->optParams.front() );
    m->optParams.pop_front();
    m->updateState();
}

} // namespace gui
