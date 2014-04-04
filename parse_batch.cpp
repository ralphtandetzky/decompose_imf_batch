#include "parse_batch.h"
#include "../decompose_imf_lib/file_io.h"
#include "../decompose_imf_lib/optimization_task.h"
#include "../cpp_utils/exception.h"
#include "../cpp_utils/extract_by_line.h"

#include <cassert>


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


std::vector<dimf::OptimizationParams> parseBatch( std::istream & is )
{
    auto result = std::vector<dimf::OptimizationParams>{};
    auto params = dimf::OptimizationParams{};
    const auto lines = cu::extractByLine( is );
    for ( auto lineNumber = size_t{}; lineNumber < lines.size(); ++lineNumber )
    {
        const auto & line = lines[lineNumber];
        std::istringstream lineStream{line};
        auto command = std::string{};
        lineStream >> command;
        if ( command == "setf" )
        {
            setFloatVar( params, lineStream );
        }
        else if ( command == "seti" )
        {
            setIntegerVar( params, lineStream );
        }
        else if ( command == "new_task" )
        {
            result.push_back( params );
        }
        else if ( command == "load_samples" )
        {
            auto fileName = std::string{};
            lineStream >> fileName;
            loadSamplesFromFile( params, fileName );
        }
        else if ( command == "" ) // line is empty.
        {
            assert( lineStream.eof() );
            continue; // skip line
        }
        else
        {
            CU_THROW( "Unknown command '" + command + "' in line " +
                      std::to_string(lineNumber) + "." );
        }
    }
    return result;
}
