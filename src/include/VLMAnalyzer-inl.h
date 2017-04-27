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

inline constexpr bool IsVLMWeakDisproved(const VLMSearchValue value)
{
  return (kVLMWeakDisprovedLB <= value && value <= kVLMWeakDisprovedUB);;
}

inline constexpr VLMSearchDepth GetVLMDepth(const VLMSearchValue value){
  assert(IsVLMProved(value) || IsVLMWeakDisproved(value));

  if(IsVLMProved(value)){
    // value = kVLMProvedUB - (depth - 1) <=> depth = kVLMProvedUB - value + 1
    return kVLMProvedUB - value + 1;
  }else{
    // value = kVLMWeakDisprovedUB - (depth - 1) <=> depth = kVLMWeakDisprovedUB - value + 1
    return kVLMWeakDisprovedUB - value + 1;
  }
}

inline constexpr VLMSearchValue GetVLMProvedSearchValue(const VLMSearchDepth depth)
{
  assert(1 <= depth && depth <= kInBoardMoveNum);
  // value = kVLMProvedUB - (depth - 1)
  return kVLMProvedUB - (depth - 1);
}

inline constexpr VLMSearchValue GetVLMWeakDisprovedSearchValue(const VLMSearchDepth depth)
{
  assert(1 <= depth && depth <= kInBoardMoveNum);
  // value = kVLMWeakDisprovedUB - (depth - 1)
  return kVLMWeakDisprovedUB - (depth - 1);
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SolveOR(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  search_manager_.AddNode();

  if(search_manager_.IsTerminate()){
    return kVLMWeakDisprovedUB;
  }

  // 置換表をチェック
  const auto hash_value = CalcHashValue(board_move_sequence_);
  VLMSearchValue table_value = 0;
  const bool is_registered = vlm_table_->find(hash_value, bit_board_, &table_value);

  if(is_registered){
    if(IsVLMProved(table_value) || IsVLMDisproved(table_value)){
      return table_value;
    }

    const auto value_depth = GetVLMDepth(table_value);
    
    if(value_depth >= vlm_search.remain_depth){
      return table_value;
    }
  }

  if(!is_registered){
    // 初回訪問時のみ終端チェックを行う
    MovePosition terminating_move;
    const bool is_terminate = TerminateCheck<P>(&terminating_move);

    if(is_terminate){
      // 終端
      const auto depth = search_sequence_.size() + 1;
      const VLMSearchValue search_value = GetVLMProvedSearchValue(depth);
      vlm_table_->Upsert(hash_value, bit_board_, search_value);
      return search_value;
    }
  }

  if(vlm_search.remain_depth == 1){
    // 残り深さ１で終端していなければ弱意の不詰
    const auto search_value = GetVLMWeakDisprovedSearchValue(vlm_search.remain_depth);
    vlm_table_->Upsert(hash_value, bit_board_, search_value);
    return search_value;
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveOR<P>(&candidate_move);

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
    
    if(!vlm_search.detect_dual_solution && IsVLMProved(and_node_value)){
      break;
    }
  }

  VLMSearchValue search_value = (IsVLMProved(or_node_value) || IsVLMDisproved(or_node_value)) ? or_node_value : GetVLMWeakDisprovedSearchValue(vlm_search.remain_depth);
  vlm_table_->Upsert(hash_value, bit_board_, search_value);
  return or_node_value;
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SolveAND(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  search_manager_.AddNode();

  if(search_manager_.IsTerminate()){
    return kVLMWeakDisprovedUB;
  }

  // 置換表をチェック
  const auto hash_value = CalcHashValue(board_move_sequence_);
  VLMSearchValue table_value = 0;
  const bool is_registered = vlm_table_->find(hash_value, bit_board_, &table_value);

  if(is_registered){
    if(IsVLMProved(table_value) || IsVLMDisproved(table_value)){
      return table_value;
    }

    const auto value_depth = GetVLMDepth(table_value);

    if(value_depth >= vlm_search.remain_depth){
      return table_value;
    }
  }

  // 初回訪問時のみ終端チェックを行う
  if(!is_registered){
    MovePosition terminating_move;
    const bool is_terminate = TerminateCheck<P>(&terminating_move);

    if(is_terminate){
      // 終端
      vlm_table_->Upsert(hash_value, bit_board_, kVLMStrongDisproved);
      return kVLMStrongDisproved;
    }
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveAND<P>(&candidate_move);

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

  VLMSearchValue search_value = (IsVLMProved(and_node_value) || IsVLMDisproved(and_node_value)) ? and_node_value : GetVLMWeakDisprovedSearchValue(vlm_search.remain_depth);
  vlm_table_->Upsert(hash_value, bit_board_, search_value);
  return and_node_value;
}

template<PlayerTurn P>
void VLMAnalyzer::GetCandidateMoveOR(MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);
  assert(candidate_move->empty());

  MovePosition guard_move;
  
  if(IsOpponentFour(&guard_move)){
    // 相手に四がある
    *candidate_move = guard_move;
    return;
  }

  // 全空点を生成する
  MoveBitSet forbidden_bit;
  EnumerateForbiddenMoves(&forbidden_bit);

  board_move_sequence_.GetOpenMove(forbidden_bit, candidate_move);

  MoveOrderingOR<P>(candidate_move);
}

template<PlayerTurn P>
void VLMAnalyzer::MoveOrderingOR(MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);

  MoveBitSet four_move_bit;
  MoveList four_move_list;

  EnumerateFourMoves<P>(&four_move_bit);
  GetMoveList(four_move_bit, &four_move_list);

  std::vector<MoveValue> move_value;
  move_value.reserve(candidate_move->size());

  static constexpr std::int64_t kMultipleCloseFour = 1024; // ３路以内に2つ以上の四ノビ点がある
  // 相手の四ノビに先着する手の優先度 -> 効果なし

  for(const auto move : *candidate_move){
    size_t close_four_count = 0;
    
    for(const auto four_move : four_move_list){
      const auto distance = CalcBoardDistance(move, four_move);
      close_four_count += distance <= 3;
    }
    
    if(close_four_count >= 2){
      move_value.emplace_back(move, kMultipleCloseFour);
    }else{
      const auto last_move = board_move_sequence_.GetLastMove();
      const auto weight = kMaxBoardDistance - CalcBoardDistance(last_move, move);
      move_value.emplace_back(move, weight);
    }
  }

  DescendingSort(&move_value);
  *candidate_move = move_value;
}

template<PlayerTurn P>
void VLMAnalyzer::GetCandidateMoveAND(MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);
  assert(candidate_move->empty());

  MovePosition guard_move;
  
  if(IsOpponentFour(&guard_move)){
    // 相手に四がある
    *candidate_move = guard_move;
    return;
  }

  // 全空点 + Passを生成する
  MoveBitSet forbidden_bit;
  EnumerateForbiddenMoves(&forbidden_bit);
  
  board_move_sequence_.GetPossibleMove(forbidden_bit, candidate_move);

  MoveOrderingAND<P>(candidate_move);
}

template<PlayerTurn P>
void VLMAnalyzer::MoveOrderingAND(MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);

  constexpr PlayerTurn Q = GetOpponentTurn(P);
  MoveBitSet four_move_bit, opponent_four_move_bit;

  EnumerateFourMoves<P>(&four_move_bit);
  EnumerateFourMoves<Q>(&opponent_four_move_bit);
  
  std::vector<MoveValue> move_value;
  move_value.reserve(candidate_move->size());

  static constexpr std::int64_t kFourMoveWeight = 1024;    // 四ノビする手の優先度
  static constexpr std::int64_t kOpponentFourMoveWeight = kFourMoveWeight - 1;   // 相手の四ノビに先着する手の優先度

  for(const auto move : *candidate_move){
    if(four_move_bit[move]){
      move_value.emplace_back(move, kFourMoveWeight);
    }else if(opponent_four_move_bit[move]){
      move_value.emplace_back(move, kOpponentFourMoveWeight);
    }else{
      const auto last_move = board_move_sequence_.GetLastMove();
      const auto weight = kMaxBoardDistance - CalcBoardDistance(last_move, move);
      move_value.emplace_back(move, weight);
    }
  }

  DescendingSort(&move_value);
  *candidate_move = move_value;
}

inline const SearchManager& VLMAnalyzer::GetSearchManager() const
{
  return search_manager_;
}

template<PlayerTurn P>
const bool VLMAnalyzer::GetProofTreeOR(MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);
  
  MoveList candidate_move;
  GetCandidateMoveOR<P>(&candidate_move);

  const auto hash_value = CalcHashValue(board_move_sequence_);
  const bool is_black_turn = P == kBlackTurn;
  BitBoard child_bit_board = bit_board_;
  constexpr PositionState S = GetPlayerStone(P);
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  bool is_proof_tree_generated = false;

  // いずれかの候補手で詰みが登録されているかチェックする
  for(const auto move : candidate_move){
    // ほとんどの候補手は詰まないのでBitBoard, Hash値の更新のみで置換表をチェックする
    const auto child_hash_value = CalcHashValue(is_black_turn, move, hash_value); // OR nodeはPassがないため差分計算する
    child_bit_board.SetState<S>(move);

    VLMSearchValue search_value;
    const auto is_find = vlm_table_->find(child_hash_value, child_bit_board, &search_value);

    child_bit_board.SetState<kOpenPosition>(move);

    if(!is_find){
      continue;
    }

    if(!IsVLMProved(search_value)){
      continue;
    }

    // move: 登録済の詰む手
    proof_tree->AddChild(move);
    proof_tree->MoveChildNode(move);
    MakeMove(move);

    const auto is_child_generated = GetProofTreeAND<Q>(proof_tree);

    UndoMove();
    proof_tree->MoveParent();

    is_proof_tree_generated |= is_child_generated;
  }

  return is_proof_tree_generated;
}

template<PlayerTurn P>
const bool VLMAnalyzer::GetProofTreeAND(MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);
  
  MoveList candidate_move;
  GetCandidateMoveAND<P>(&candidate_move);

  constexpr PlayerTurn Q = GetOpponentTurn(P);

  // すべての候補手の詰みが登録されているかチェックする
  for(const auto move : candidate_move){
    // すべての候補手が登録済であることが期待されるのでBitBoardのみの更新ではなくMakeMove, Undoで更新する
    MakeMove(move);
    const auto child_hash_value = CalcHashValue(board_move_sequence_); // AND nodeはPassがあるため逐次計算する

    VLMSearchValue search_value;
    const auto is_find = vlm_table_->find(child_hash_value, bit_board_, &search_value);
    bool is_child_generated = false;
    
    if(is_find && IsVLMProved(search_value)){
      proof_tree->AddChild(move);
      proof_tree->MoveChildNode(move);
  
      is_child_generated = GetProofTreeOR<Q>(proof_tree);
  
      proof_tree->MoveParent();
    }

    UndoMove();

    if(!is_child_generated){
      // いずれかの候補手で証明木が生成できなければ生成失敗
      return false;
    }
  }

  return true;
}
}   // namespace realcore

#endif    // VLM_ANALYZER_INL_H
