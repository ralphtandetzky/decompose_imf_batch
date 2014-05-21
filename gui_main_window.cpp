#include "gui_main_window.h"
#include "ui_gui_main_window.h"
#include "parse_batch.h"

#include "../decompose_imf_lib/optimization_task.h"

#include "../qt_utils/exception_handling.h"
#include "../qt_utils/gui_progress_manager.h"
#include "../qt_utils/invoke_in_thread.h"
#include "../qt_utils/loop_thread.h"

#include "../cpp_utils/exception_handling.h"
#include "../cpp_utils/locking.h"
#include "../cpp_utils/parallel_executor.h"
#include "../cpp_utils/progress.h"
#include "../cpp_utils/progress_interface.h"
#include "../cpp_utils/scope_guard.h"
#include "../cpp_utils/std_make_unique.h"

#include <QSettings>

#include <iostream>

static const char * tasksTextName = "tasksText";

namespace gui {

static void printSamples( const std::vector<double> & samples )
{
    std::copy( begin(samples), end(samples),
               std::ostream_iterator<double>(std::cout, " ") );
    std::cout << std::endl;
}

static void printPreprocessedSamples(
        const dimf::OptimizationParams & params )
{
    std::cout << "Preprocessed samples: ";
    printSamples( getPreprocessedSamples(params) );
}

static void printImfs( const std::vector<std::vector<double> > & imfs )
{
    auto i = size_t(0);
    for ( const auto & imf : imfs )
    {
        std::cout << "IMF " << (i++) << ": ";
        printSamples( imf );
    }
}

struct MainWindow::Impl
{
    void runBatchStep(
            BatchOptimizationParams optParam,
            cu::ProgressInterface * progress );

    Ui::MainWindow ui;

    struct SharedData
    {
        bool cancelled{};
        bool isRunning{};
    };
    cu::Monitor<SharedData> shared;
    qu::LoopThread optimizationWorker;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , m{ std::make_unique<Impl>() }
{
    auto progressManager = std::make_unique<qu::ProgressWidgetContainer>();
    qu::setGlobalProgressManager( &progressManager->getProgressManagerInterface() );
    m->ui.setupUi(this);
    m->ui.statusbar->addPermanentWidget( progressManager.get() );
    progressManager.release();
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


void MainWindow::Impl::runBatchStep(
        BatchOptimizationParams optParam,
        cu::ProgressInterface * progress )
{
    // contains the indexes of the imfs that shall be optimized
    // in order.
    auto imfIndexes = std::vector<size_t>{};
    // contains the step numbers when the index of the imf
    // to be optimized shall change.
    auto imfPartSums = std::vector<size_t>{};
    auto totalImfOptSteps = size_t{0};
    for ( const auto & imfOptimization : optParam.imfOptimizations )
    {
        imfIndexes.push_back( imfOptimization.first );
        imfPartSums.push_back( totalImfOptSteps += imfOptimization.second );
    }
    optParam.howToContinue =
            [this,imfIndexes,imfPartSums,&progress]( size_t nIter )
    {
        progress->setProgress( double(nIter)/imfPartSums.back() );
        const auto isCancelled = shared(
            []( SharedData & shared )
        {
            return shared.cancelled;
        });
        if ( isCancelled || progress->shallAbort() )
            return ~size_t{0};
        const auto it = std::upper_bound(
                begin(imfPartSums),
                end(imfPartSums),
                nIter );
        if ( it == end(imfPartSums) )
            return ~size_t{0};
        return imfIndexes.at( it - begin(imfPartSums) );
    };
    if ( optParam.howToContinue(0) == ~size_t{0} )
        return;
    const auto imfs = dimf::runOptimization(optParam);
    printPreprocessedSamples(optParam);
    printImfs( imfs );
}


void MainWindow::runBatch()
{
    // parse script, produce optimization parameters.
    const auto optParams = parseBatch( std::istringstream(
        m->ui.textEditor->toPlainText().toStdString()) );

    // check if an optimization is already running. Throw, if so.
    m->shared( [this]( Impl::SharedData & shared )
    {
        if ( shared.isRunning )
            CU_THROW( "Batch processing is already in progress." );
    });

    // start optimization in worker thread.
    qu::invokeInThread( &m->optimizationWorker, [=]()
    { QU_HANDLE_ALL_EXCEPTIONS_FROM {
        // set cancelled flag and running flag.
        m->shared( [this]( Impl::SharedData & shared )
        {
            shared.cancelled = false;
            shared.isRunning = true;
        });
        CU_SCOPE_EXIT {
            m->shared( [this]( Impl::SharedData & shared )
            { shared.isRunning = false; });
        };

        // run script in loop.
        const auto progress = qu::createProgress( "Batch Run" );
        const auto nOptParams = optParams.size();
        // parProgress must be declared before executor. This ensures, that
        // all tasks are completed before the destructor of parProgress is
        // called. parProgress must not be destroyed before the tasks finish
        // since the tasks access parProgress. This is the reason parProgress
        // is made a unique_ptr, so it can be initialized after executor
        // and destroyed after executor.
        std::unique_ptr<cu::ParallelProgress> parProgress;
        std::vector<std::future<void> > tasks;
        cu::ParallelExecutor executor;
        parProgress = std::make_unique<cu::ParallelProgress>(
                    *progress, nOptParams, executor.getNWorkers() );
        // Queue up the tasks.
        for ( size_t i = 0; i < nOptParams; ++i )
        {
            const auto & optParam = optParams[i];
            tasks.push_back( executor.addTask( [=,&parProgress](){
                m->runBatchStep( optParam,
                                 &parProgress->getTaskProgressInterface( i ) );
            }) );
        }
        // wait for the tasks to finish.
        for ( auto & task : tasks )
        {
            task.get();
            const auto isCancelled = m->shared(
                []( Impl::SharedData & shared )
            {
                return shared.cancelled;
            });
            if ( isCancelled || progress->shallAbort() )
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
