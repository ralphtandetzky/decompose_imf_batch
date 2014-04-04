/** @file
  @author Ralph Tandetzky
  @date 4 Apr 2014
*/

#pragma once

#include <vector>
#include <iosfwd>

// forward declarations
namespace dimf { struct OptimizationParams; }

std::vector<dimf::OptimizationParams> parseBatch( std::istream & is );
