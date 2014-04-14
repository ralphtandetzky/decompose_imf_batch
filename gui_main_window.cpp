#include "gui_main_window.h"
#include "ui_gui_main_window.h"
#include "parse_batch.h"

#include "../decompose_imf_lib/optimization_task.h"

#include "../qt_utils/exception_handling.h"
#include "../qt_utils/invoke_in_thread.h"
#include "../qt_utils/loop_thread.h"

#include "../cpp_utils/locking.h"
#include "../cpp_utils/exception_handling.h"
#include "../cpp_utils/scope_guard.h"
#include "../cpp_utils/std_make_unique.h"

#include <QSettings>

#include <iostream>

static const char * tasksTextName = "tasksText";

namespace gui {

static void printImfs( const std::vector<std::vector<double> > & imfs )
{
    auto i = size_t(0);
    for ( const auto & imf : imfs )
    {
        std::cout << "IMF " << (i++) << ": ";
        std::copy( begin(imf), end(imf),
                   std::ostream_iterator<double>(std::cout," ") );
        std::cout << std::endl;
    }
}

struct MainWindow::Impl
{
    Ui::MainWindow ui;
    qu::LoopThread optimizationWorker;

    struct SharedData
    {
        bool cancelled{};
        bool isRunning{};
    };
    cu::Monitor<SharedData> shared;

};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , m{ std::make_unique<Impl>() }
{
    m->ui.setupUi(this);
    m->ui.textEditor->setPlainText(
                QSettings().value( tasksTextName ).toString() );
}

MainWindow::~MainWindow()
{
    CU_SWALLOW_ALL_EXCEPTIONS_FROM
    {
        cancelRun();
        QSettings().setValue(
                    tasksTextName,
                    m->ui.textEditor->toPlainText() );
    };
}

void MainWindow::cancelRun()
{
    m->shared( []( Impl::SharedData & shared )
    {
        shared.cancelled = true;
    });
}

void MainWindow::runBatch()
{
    const auto optParams = parseBatch( std::istringstream(
        m->ui.textEditor->toPlainText().toStdString()) );
    m->shared( [this]( Impl::SharedData & shared )
    {
        if ( shared.isRunning )
            CU_THROW( "Batch processing is already in progress." );
    });
    qu::invokeInThread( &m->optimizationWorker, [=]()
    { QU_HANDLE_ALL_EXCEPTIONS_FROM {
        m->shared( [this]( Impl::SharedData & shared )
        {
            shared.cancelled = false;
            shared.isRunning = true;
        });
        CU_SCOPE_EXIT {
            m->shared( [this]( Impl::SharedData & shared )
            { shared.isRunning = false; });
        };
        qu    ::invokeInGuiThread( [this](){ m->ui.cancelRunButton->setEnabled(true); } );
        CU_SCOPE_EXIT {
            qu::invokeInGuiThread( [this](){ m->ui.cancelRunButton->setEnabled(false); } );
        };
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
            auto imfIndexes = std::vector<size_t>{};
            auto imfPartSums = std::vector<size_t>{};
            auto totalImfOptSteps = size_t{0};
            for ( const auto & imfOptimization : optParam.imfOptimizations )
            {
                imfIndexes.push_back( imfOptimization.first );
                imfPartSums.push_back( totalImfOptSteps += imfOptimization.second );
            }
            optParam.howToContinue =
                    [this,imfIndexes,imfPartSums]( size_t nIter )
            {
                const auto isCancelled = m->shared(
                    []( Impl::SharedData & shared )
                {
                    return shared.cancelled;
                });
                if ( isCancelled )
                    return ~size_t{0};
                const auto it = std::lower_bound(
                        begin(imfPartSums),
                        end(imfPartSums),
                        nIter );
                if ( it == end(imfPartSums) )
                    return ~size_t{0};
                return imfIndexes.at( it - begin(imfPartSums) );
            };
            const auto imfs = dimf::runOptimization(optParam);
            printImfs( imfs );
            const auto isCancelled = m->shared(
                []( Impl::SharedData & shared )
            {
                return shared.cancelled;
            });
            if ( isCancelled )
            {
                qu::invokeInGuiThread( [this]()
                {
                    m->ui.statusbar->showMessage(
                        QString("Optimization run was cancelled.")
                        , 5000 );
                } );
                return;
            }
        }
        qu::invokeInGuiThread( [this]()
        {
            m->ui.statusbar->showMessage(
                QString("All optimization runs finished successfully.")
                , 5000 );
        } );
    }; } );
}

} // namespace gui
