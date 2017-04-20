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

inline constexpr VLMSearchValue GetVLMSearchValue(const VLMSearchDepth depth)
{
  assert(1 <= depth && depth <= kInBoardMoveNum);
  return kVLMProvedUB - (depth - 1);
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SolveOR(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  search_manager_.AddNode();
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SolveAND(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

}

}   // namespace realcore

#endif    // VLM_ANALYZER_INL_H
