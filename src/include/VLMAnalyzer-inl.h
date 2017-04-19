#ifndef VLM_ANALYZER_INL_H
#define VLM_ANALYZER_INL_H

#include "VLMAnalyzer.h"

namespace realcore
{

inline constexpr bool IsVLMProved(const VLMSearchValue value){
  return (kVLMProvedLB <= value && value <= kVLMProvedUB);
}

inline constexpr VLMSearchDepth GetVLMDepth(const VLMSearchValue value){
  assert(IsVLMProved(value));
  return kVLMProvedUB - value + 1;
}

template<PlayerTurn P>
void VLMAnalyzer::SolveORBruteForce(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{

}

template<PlayerTurn P>
void VLMAnalyzer::SolveANDBruteForce(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{

}

}   // namespace realcore

#endif    // VLM_ANALYZER_INL_H
