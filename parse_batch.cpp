#include "parse_batch.h"
#include "../decompose_imf_lib/calculations.h"
#include "../decompose_imf_lib/file_io.h"
#include "../decompose_imf_lib/optimization_task.h"
#include "../cpp_utils/exception.h"
#include "../cpp_utils/extract_by_line.h"

#include <cassert>
#include <iostream>


template <typename T>
std::pair<std::string,T> readKeyAndValue( std::istream & is )
{
    auto varName = std::string{};
    is >> varName;
    if ( varName.empty() )
        CU_THROW( "No variable name specified." );
    auto value = T{};
    is >> value;
    if ( is.fail() || is.bad() )
        CU_THROW( "The variable value could not be read." );
    is >> std::ws;
    if ( !is.eof() )
        CU_THROW( "I could successfully parse the variable name and "
                  "the value, but I don't know what to do with the rest "
                  "of the line." );
    return std::make_pair( std::move(varName), std::move(value) );
}


static void setFloatVar( dimf::OptimizationParams & params, std::istream & is )
{
    const auto keyAndValue = readKeyAndValue<double>( is );
    const auto & varName = keyAndValue.first;
    const auto & value   = keyAndValue.second;
    if      ( varName == "angleDevDegs"   ) { params.angleDevDegs   = value; }
    else if ( varName == "amplitudeDev"   ) { params.amplitudeDev   = value; }
    else if ( varName == "crossOverProb"  ) { params.crossOverProb  = value; }
    else if ( varName == "diffWeight"     ) { params.diffWeight     = value; }
    else if ( varName == "initSigmaUnits" ) { params.initSigmaUnits = value; }
    else if ( varName == "initTauUnits"   ) { params.initTauUnits   = value; }
    else if ( varName == "nodeDevUnits"   ) { params.nodeDevUnits   = value; }
    else if ( varName == "sigmaDevUnits"  ) { params.sigmaDevUnits  = value; }
    else if ( varName == "tauDevUnits"    ) { params.tauDevUnits    = value; }
    else CU_THROW( "Unknown variable name '" + varName + "'." );
}


static void setIntegerVar( dimf::OptimizationParams & params, std::istream & is )
{
    const auto keyAndValue = readKeyAndValue<size_t>( is );
    const auto & varName = keyAndValue.first;
    const auto & value   = keyAndValue.second;
    if      ( varName == "swarmSize" ) { params.swarmSize = value; }
    else if ( varName == "nParams"   ) { params.nParams   = value; }
    else CU_THROW( "Unknown variable name '" + varName + "'." );
}


static void loadSamplesFromFile( dimf::OptimizationParams & params, std::string & fileName )
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


static void addPreprocessingStep( dimf::OptimizationParams & params, std::istream & is )
{
    addProcessingStep( params.preprocessing, is );
}


static void addInterprocessingStep( dimf::OptimizationParams & params, std::istream & is )
{
    addProcessingStep( params.interprocessing, is );
}


static bool runLine( dimf::OptimizationParams & params, const std::string & line )
{
    std::istringstream lineStream{line};
    auto command = std::string{};
    lineStream >> command;
    if ( command == "setf" )
    {
        setFloatVar( params, lineStream );
        return false;
    }
    if ( command == "seti" )
    {
        setIntegerVar( params, lineStream );
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
    if ( command == "" ) // line is empty.
    {
        assert( lineStream.eof() );
        return false;
    }

    CU_THROW( "Unknown command '" + command + "'." );
    return false; // this line is never reached.
}


std::vector<dimf::OptimizationParams> parseBatch( std::istream & is )
{
    auto result = std::vector<dimf::OptimizationParams>{};
    auto params = dimf::OptimizationParams{};
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
    {
        try
        {
            if ( runLine( params, lines[lineNumber] ) )
                result.push_back( params );
        }
        catch (...)
        {
            CU_THROW( "Could evaluate line " +
                      std::to_string(lineNumber+1) +
                      ". The content of the line is '" +
                      lines[lineNumber] + "'." );
        }
    }
    return result;
}
