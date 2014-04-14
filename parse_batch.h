/** @file
  @author Ralph Tandetzky
  @date 4 Apr 2014
*/

#pragma once

#include "../decompose_imf_lib/optimization_task.h"

#include <vector>
#include <iosfwd>

/*
set swarmSize 200
set angleDevDegs 120
set amplitudeDev 0.5
set crossOverProb 1
set diffWeight 0.6
set nParams 7
set initSigmaUnits 64
set initTauUnits 64
set nodeDevUnits 0.5
set sigmaDevUnits 8
set tauDevUnits 8
set stepLimit 100000
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
