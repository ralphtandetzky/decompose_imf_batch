/** @file
  @author Ralph Tandetzky
  @date 4 Apr 2014
*/

#pragma once

#include <vector>
#include <iosfwd>

// forward declarations
namespace dimf { struct OptimizationParams; }

/*
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec1.asc
seti swarmSize 200
setf angleDevDegs 120
setf amplitudeDev 0.5
setf crossOverProb 1
setf diffWeight 0.6
seti nParams 7
setf initSigmaUnits 64
setf initTauUnits 64
setf nodeDevUnits 0.5
setf sigmaDevUnits 8
setf tauDevUnits 8
add_preprocessing_step low_pass 2
add_preprocessing_step clip 512 768
add_preprocessing_step mul 0.02
add_interprocessing_step zero_moments 2
new_task
 */

std::vector<dimf::OptimizationParams> parseBatch( std::istream & is );
inline std::vector<dimf::OptimizationParams> parseBatch( std::istream && is )
{
    return parseBatch( is );
}
