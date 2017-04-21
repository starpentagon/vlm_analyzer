#ifndef VLM_ANALYZER_INL_H
#define VLM_ANALYZER_INL_H

#include "VLMAnalyzer.h"

namespace realcore
{

inline constexpr bool IsVLMProved(const VLMSearchValue value){
  return (kVLMProvedLB <= value && value <= kVLMProvedUB);
}

inline constexpr bool IsVLMDisproved(const VLMSearchValue value){
  return (value == kVLMStrongDisproved);
}

inline constexpr VLMSearchDepth GetVLMDepth(const VLMSearchValue value){
  assert(IsVLMProved(value));
  // value = kVLMProvedUB - (depth - 1) <=> depth = kVLMProvedUB - value + 1
  return kVLMProvedUB - value + 1;
}

inline constexpr VLMSearchValue GetVLMSearchValue(const VLMSearchDepth depth)
{
  assert(1 <= depth && depth <= kInBoardMoveNum);
  // value = kVLMProvedUB - (depth - 1)
  return kVLMProvedUB - (depth - 1);
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SolveOR(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  search_manager_.AddNode();

  if(search_manager_.IsTerminate()){
    return kVLMWeakDisproved;
  }

  MovePosition terminating_move;
  const bool is_terminate = TerminateCheck<P>(&terminating_move);

  if(is_terminate){
    // 終端
    const auto depth = search_sequence_.size() + 1;
    const VLMSearchValue value = GetVLMSearchValue(depth);

    return value;
  }

  if(vlm_search.remain_depth == 1){
    // 残り深さ１で終端していなければ弱意の不詰
    return kVLMWeakDisproved;
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveOR(&candidate_move);

  // 展開
  VLMSearch child_vlm_search = vlm_search;
  child_vlm_search.remain_depth--;
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  VLMSearchValue or_node_value = kVLMStrongDisproved;

  for(const auto move : candidate_move){
    MakeMove(move);
    VLMSearchValue and_node_value = SolveAND<Q>(child_vlm_search, vlm_result);
    UndoMove();

    or_node_value = std::max(or_node_value, and_node_value);
    
    if(search_sequence_.empty() && IsVLMProved(and_node_value)){
      vlm_result->proof_tree.AddChild(move);
    }

    if(!vlm_search.detect_dual_solution && IsVLMProved(and_node_value)){
      break;
    }
  }

  return or_node_value;
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SolveAND(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  search_manager_.AddNode();

  if(search_manager_.IsTerminate()){
    return kVLMWeakDisproved;
  }

  if(vlm_search.remain_depth == 0){
    // 指定の深さまでに詰みがなければ弱意の不詰
    return kVLMWeakDisproved;
  }

  // 末端でのみ終端チェックを行う
  MovePosition terminating_move;
  const bool is_terminate = TerminateCheck<P>(&terminating_move);

  if(is_terminate){
    // 終端
    return kVLMStrongDisproved;
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveAND(&candidate_move);

  // 展開
  VLMSearch child_vlm_search = vlm_search;
  child_vlm_search.remain_depth--;
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  VLMSearchValue and_node_value = kVLMProvedUB;

  for(const auto move : candidate_move){
    MakeMove(move);
    VLMSearchValue or_node_value = SolveOR<Q>(child_vlm_search, vlm_result);
    UndoMove();

    and_node_value = std::min(and_node_value, or_node_value);

    if(!IsVLMProved(or_node_value)){
      break;
    }
  }

  return and_node_value;
}

inline const SearchManager& VLMAnalyzer::GetSearchManager() const
{
  return search_manager_;
}

}   // namespace realcore

#endif    // VLM_ANALYZER_INL_H
