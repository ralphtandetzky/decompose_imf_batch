/** @file
  @author Ralph Tandetzky
  @date 4 Apr 2014
*/

#pragma once

#include "../decompose_imf_lib/optimization_task.h"

#include <vector>
#include <iosfwd>

/*
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
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec1.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec2.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec3.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec4.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec5.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec6.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec7.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec8.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec9.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec10.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec11.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec12.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec13.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec14.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec15.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec16.asc
new_task
load_samples /home/ralph/Documents/c++/decompose_imf/data/effler_fp1_10sec17.asc
new_task
 */

struct BatchOptimizationParams : dimf::OptimizationParams
{
    size_t stepLimit{};
};

template <typename F>
void iterateMembers( BatchOptimizationParams & params, F && f )
{
    iterateMembers(
                static_cast<dimf::OptimizationParams&>(params),
                std::forward<F>(f) );
    f( params.stepLimit      , "stepLimit"       );
}

std::vector<BatchOptimizationParams> parseBatch( std::istream & is );
inline std::vector<BatchOptimizationParams> parseBatch( std::istream && is )
{
    return parseBatch( is );
}
