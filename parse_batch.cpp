#include "parse_batch.h"
#include "../decompose_imf_lib/calculations.h"
#include "../decompose_imf_lib/file_io.h"
#include "../decompose_imf_lib/optimization_task.h"
#include "../cpp_utils/exception.h"
#include "../cpp_utils/extract_by_line.h"
#include "../cpp_utils/more_algorithms.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <map>


namespace {

    using ValueReadersType =
        std::map<std::string,std::function<void(std::istream&)> >;

    class ValueReaderMaker
    {
    public:
        ValueReaderMaker( ValueReadersType & valueReaders )
            : valueReaders(valueReaders)
        {
        }

        template <typename  T>
        void operator()( T & x, const char * varName ) const
        {
            Impl<T,std::is_arithmetic<T>::value >()( x, varName, valueReaders );
        }

    private:
        template <typename T, bool IsArithmetic> struct Impl;
        template <typename T> struct Impl<T,false>
        {
            void operator()( T &, const char *, ValueReadersType & ) const
            {
                // NOOP //
            }
        };
        template <typename T> struct Impl<T,true>
        {
            void operator()( T & x, const char * varName
                             , ValueReadersType & valueReaders ) const
            try
            {
                const auto success = valueReaders.insert(
                    std::make_pair(varName,
                    [&x](std::istream & is)
                {
                    auto value = T{};
                    is >> value;
                    if ( is.fail() || is.bad() )
                        CU_THROW( "The variable value could not be read." );
                    is >> std::ws;
                    if ( !is.eof() )
                        CU_THROW( "I could successfully parse the variable "
                                  "name and the value, but I don't know "
                                  "what to do with the rest of the line." );
                    x = value;
                }) ).second;
                CU_ASSERT_THROW( success,
                                 "The variable name already exists." );
            }
            catch (...)
            {
                CU_THROW( std::string("The variable name '") + varName +
                          "' could not be inserted into the list of "
                          "valid variable names." );
            }
        };
        ValueReadersType & valueReaders;
    };

    ValueReadersType makeValueReaders( BatchOptimizationParams & params )
    {
        ValueReadersType valueReaders;
        iterateMembers( params, ValueReaderMaker(valueReaders) );
        const auto nErasedItems = valueReaders.erase( "xIntervalWidth" );
        assert( nErasedItems == 1 );
        return valueReaders;
    }

    class ParamParser
    {
    public:
        ParamParser( BatchOptimizationParams & params )
            : valueReaders(makeValueReaders(params))
        {
        }

        void setParam( std::istream & is ) const
        {
            auto varName = std::string{};
            is >> varName;
            if ( varName.empty() )
                CU_THROW( "No variable name specified." );
            if ( valueReaders.count(varName) == 0 )
            {
                const auto message =
                        "The variable '" + varName +
                        "' is unknown.";
                auto minDist = varName.size() / 2 + 1;
                auto minDistName = std::string{};
                for ( const auto & x : valueReaders )
                {
                    const auto dist = cu::levenshteinDistance(
                                varName, x.first );
                    if ( dist < minDist )
                    {
                        minDist = dist;
                        minDistName = x.first;
                    }
                }
                // no appropriate match?
                if ( minDistName.empty() )
                    CU_THROW( message );
                CU_THROW( message + " Did you mean '" +
                          minDistName + "'?" );
            }
            valueReaders.at(varName)( is );
        }

    private:
        ValueReadersType valueReaders;
    };

} // unnamed namespace


static void loadSamplesFromFile( BatchOptimizationParams & params, std::string & fileName )
{
    params.samples = dimf::readSamplesFromFile( fileName );
    params.xIntervalWidth = params.samples.size();
}


static void addProcessingStep( std::string & processing, std::istream & is )
{
    auto preprocessingStep = std::string{};
    std::getline( is, preprocessingStep );
    CU_ASSERT_THROW( is.eof(), "The stream should only contain one line, but contains more." );
    processing.append( std::move(preprocessingStep) );
    processing.push_back( '\n' );
}

static void addPreprocessingStep( BatchOptimizationParams & params, std::istream & is )
{
    addProcessingStep( params.preprocessing, is );
}


static void addInterprocessingStep( BatchOptimizationParams & params, std::istream & is )
{
    addProcessingStep( params.interprocessing, is );
}


static void clearProcessingSteps( std::string & processing, std::istream & is )
{
    is >> std::ws;
    if ( !is.eof() )
        CU_THROW( "This line has invalid content after the command." );
    processing.clear();
}

static void clearPreprocessingSteps( BatchOptimizationParams & params, std::istream & is )
{
    clearProcessingSteps( params.preprocessing, is );
}

static void clearInterprocessingSteps( BatchOptimizationParams & params, std::istream & is )
{
    clearProcessingSteps( params.interprocessing, is );
}


static bool runLine(
        BatchOptimizationParams & params,
        const std::string & line,
        const ParamParser & paramParser
        )
{
    std::istringstream lineStream{line};
    auto command = std::string{};
    lineStream >> command;
    if ( command == "set" )
    {
        paramParser.setParam( lineStream );
        return false;
    }
    if ( command == "new_task" )
    {
        return true;
    }
    if ( command == "load_samples" )
    {
        auto fileName = std::string{};
        lineStream >> fileName;
        loadSamplesFromFile( params, fileName );
        return false;
    }
    if ( command == "add_preprocessing_step" )
    {
        addPreprocessingStep( params, lineStream );
        return false;
    }
    if ( command == "add_interprocessing_step" )
    {
        addInterprocessingStep( params, lineStream );
        return false;
    }
    if ( command == "clear_preprocessing_steps" )
    {
        clearPreprocessingSteps( params, lineStream );
        return false;
    }
    if ( command == "clear_interprocessing_steps" )
    {
        clearInterprocessingSteps( params, lineStream );
        return false;
    }
    if ( command == "" ) // line is empty.
    {
        assert( lineStream.eof() );
        return false;
    }

    CU_THROW( "Unknown command '" + command + "'." );
    return false; // this line is never reached.
}


std::vector<BatchOptimizationParams> parseBatch( std::istream & is )
{
    auto result = std::vector<BatchOptimizationParams>{};
    auto params = BatchOptimizationParams{};
    const auto paramParser = ParamParser{params};
    params.initializer = &dimf::getInitialApproximationByInterpolatingZeros;
    params.receiveBestFit = [](
            const std::vector<double> & //bestParams
            , double cost
            , size_t //nSamples
            , size_t nIter
            , const std::vector<double> & //f
            )
    {
        std::cout << nIter << ' ' << cost << std::endl;
    };
    const auto lines = cu::extractByLine( is );
    for ( auto lineNumber = size_t{}; lineNumber < lines.size(); ++lineNumber )
        try
        {
            if ( runLine( params, lines[lineNumber], paramParser ) )
                result.push_back( params );
        }
        catch (...)
        {
            CU_THROW( "Could evaluate line " +
                      std::to_string(lineNumber+1) +
                      ". The content of the line is '" +
                      lines[lineNumber] + "'." );
        }
    return result;
}
