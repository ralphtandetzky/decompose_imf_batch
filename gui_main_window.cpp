#include "gui_main_window.h"
#include "ui_gui_main_window.h"
#include "parse_batch.h"
#include "../decompose_imf_lib/optimization_task.h"
#include "../qt_utils/invoke_in_thread.h"
#include "../qt_utils/loop_thread.h"
#include "../cpp_utils/std_make_unique.h"
#include "../cpp_utils/exception_handling.h"

#include <list>
#include <QSettings>
#include <iostream>

static const char * tasksTextName = "tasksText";

namespace gui {

struct MainWindow::Impl
{
    std::list<BatchOptimizationParams> optParams;
    Ui::MainWindow ui;
//    dimf::OptimizationTask optTask;
    qu::LoopThread optimizationWorker;

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
    if ( m->optParams.empty() )
        return;

    const auto optParams = m->optParams;
    qu::invokeInThread( &m->optimizationWorker,
                        [this,optParams]() mutable
    {
        auto i = size_t{};
        for ( auto optParam : optParams )
        {
            ++i;
            const auto n = optParams.size();
            qu::invokeInGuiThread( [this,i,n]()
            {
                m->ui.statusbar->showMessage(
                            QString("Optimization %1 of %2 is running ...")
                            .arg(i)
                            .arg(n) );
            } );
            const auto stepLimit = optParam.stepLimit;
            optParam.howToContinue =
                    [this,stepLimit]( size_t nIter )
            {
                if ( nIter >= stepLimit )
                    return dimf::ContinueOption::Cancel;
                return dimf::ContinueOption::Continue;
            };
            dimf::runOptimization(optParam);
        }
        qu::invokeInGuiThread( [this]()
        {
            m->ui.statusbar->showMessage(
                QString("All optimization runs finished successfully.")
                , 5000 );
        } );
    } );
}

} // namespace gui
