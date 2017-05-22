#ifndef VLM_ANALYZER_INL_H
#define VLM_ANALYZER_INL_H

#include <numeric>

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
  assert(1 <= depth && depth <= static_cast<VLMSearchDepth>(kInBoardMoveNum));
  // value = kVLMProvedUB - (depth - 1)
  return kVLMProvedUB - (depth - 1);
}

inline constexpr VLMSearchValue GetVLMWeakDisprovedSearchValue(const VLMSearchDepth depth)
{
  assert(1 <= depth && depth <= static_cast<VLMSearchDepth>(kInBoardMoveNum));
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
    const bool is_terminate = TerminateCheck(&terminating_move);

    if(is_terminate){
      // 終端
      constexpr VLMSearchDepth depth = 1;
      constexpr VLMSearchValue search_value = GetVLMProvedSearchValue(depth);
      vlm_table_->Upsert(hash_value, bit_board_, search_value);
      return search_value;
    }
  }

  if(vlm_search.remain_depth == 1){
    // 残り深さ１で終端していなければ弱意の不詰
    constexpr VLMSearchDepth depth = 1;
    constexpr auto search_value = GetVLMWeakDisprovedSearchValue(depth);
    vlm_table_->Upsert(hash_value, bit_board_, search_value);
    return search_value;
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveOR<P>(vlm_search, &candidate_move);

  // 展開
  VLMSearch child_vlm_search = vlm_search;
  child_vlm_search.remain_depth--;
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  VLMSearchValue or_node_value = kVLMStrongDisproved;

  for(const auto move : candidate_move){
    MakeMove(child_vlm_search, move);
    VLMSearchValue and_node_value = SolveAND<Q>(child_vlm_search, vlm_result);
    UndoMove();

    or_node_value = std::max(or_node_value, and_node_value);
    
    if(!vlm_search.detect_dual_solution && IsVLMProved(and_node_value)){
      break;
    }
  }

  const VLMSearchValue search_value = GetSearchValue(or_node_value);
  vlm_table_->Upsert(hash_value, bit_board_, search_value);
  return search_value;
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
    const bool is_terminate = TerminateCheck(&terminating_move);

    if(is_terminate){
      // 終端
      vlm_table_->Upsert(hash_value, bit_board_, kVLMStrongDisproved);
      return kVLMStrongDisproved;
    }
  }

  // 候補手生成
  MoveList candidate_move;
  const auto is_terminate_guard = GetCandidateMoveAND<P>(vlm_search, &candidate_move);

  if(vlm_search.remain_depth == 2 && !is_terminate_guard){
    // 残り深さ２で相手に１手勝ちがない -> Passすると弱意の不詰になる
    static constexpr VLMSearchDepth depth = 2;
    constexpr VLMSearchValue search_value = GetVLMWeakDisprovedSearchValue(depth);

    vlm_table_->Upsert(hash_value, bit_board_, search_value);
    return search_value;
  }

  // 展開
  VLMSearch child_vlm_search = vlm_search;
  child_vlm_search.remain_depth--;
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  VLMSearchValue and_node_value = kVLMProvedUB;
  MoveTree proof_tree;

  for(const auto move : candidate_move){
    MakeMove(child_vlm_search, move);

    VLMSearchValue or_node_value = kVLMStrongDisproved;

    if(!proof_tree.empty()){
      // 証明木が存在する場合はSimulationを行う
      VLMSearch vlm_simulation = vlm_search;
      vlm_simulation.is_search = false;
      
      or_node_value = SimulationOR<Q>(vlm_simulation, &proof_tree);
      search_manager_.AddSimulationResult(IsVLMProved(or_node_value));
    }

    if(!IsVLMProved(or_node_value)){
      // Simulationをしなかった or 失敗した場合は通常探索を行う
      or_node_value = SolveOR<Q>(child_vlm_search, vlm_result);

      if(IsVLMProved(or_node_value) && GetVLMDepth(or_node_value) >= 3){
        const auto is_generated = GetProofTree(&proof_tree);
        search_manager_.AddGetProofTreeResult(is_generated);
      }
    }

    UndoMove();

    and_node_value = std::min(and_node_value, or_node_value);

    if(!IsVLMProved(or_node_value)){
      break;
    }
  }

  const VLMSearchValue search_value = GetSearchValue(and_node_value);
  vlm_table_->Upsert(hash_value, bit_board_, search_value);
  return search_value;
}

template<PlayerTurn P>
void VLMAnalyzer::GetCandidateMoveOR(const VLMSearch &vlm_search, MoveList * const candidate_move) const
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
  MoveBitSet forbidden_bit, candidate_move_bit;
  EnumerateForbiddenMoves(&forbidden_bit);

  board_move_sequence_.GetOpenMove(forbidden_bit, &candidate_move_bit);

  if(vlm_search.is_search){
    MoveOrderingOR<P>(vlm_search, &candidate_move_bit, candidate_move);
  }else{
    GetMoveList(candidate_move_bit, candidate_move);
  }
}

template<PlayerTurn P>
bool VLMAnalyzer::GetCandidateMoveAND(const VLMSearch &vlm_search, MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);
  assert(candidate_move->empty());

  MovePosition guard_move;
  
  if(IsOpponentFour(&guard_move)){
    // 相手に四がある
    *candidate_move = guard_move;
    return true;
  }

  MoveBitSet guard_move_bit;
  const bool terminate_threat = GetTerminateGuard(&guard_move_bit);
  MoveBitSet forbidden_bit;
  EnumerateForbiddenMoves(&forbidden_bit);
  
  if(terminate_threat)
  {
    guard_move_bit &= ~forbidden_bit;

    if(guard_move_bit.none()){
      // OR nodeで終端するように防手がない場合はPassをする
      guard_move_bit.set(kNullMove);
    }
  }else{
    // 全空点 + Passを生成する
    board_move_sequence_.GetPossibleMove(forbidden_bit, &guard_move_bit);
  }

  if(vlm_search.is_search && vlm_search.remain_depth >= 4){
    MoveOrderingAND<P>(&guard_move_bit, candidate_move);
  }else{
    // Passが先頭で生成され、Null move pruningを行うことに相当する
    GetMoveList(guard_move_bit, candidate_move);
  }

  return terminate_threat;
}

template<PlayerTurn P>
void VLMAnalyzer::MoveOrderingAND(MoveBitSet * const candidate_move_bit, MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);
  assert(candidate_move->empty());

  constexpr PlayerTurn Q = GetOpponentTurn(P);

  // @see doc/02_performance/vlm_analyzer_performance.xlsx, MoveOrderingAND sheet
  {
    // 優先度1: 三を作る手
    MoveBitSet semi_three_move_bit;
    EnumerateSemiThreeMoves<P>(&semi_three_move_bit);
    SelectMove(semi_three_move_bit, candidate_move_bit, candidate_move);
  }
  {
    // 優先度2: 相手の四ノビ点に先着する手
    MoveBitSet opponent_four_move_bit;
    EnumerateFourMoves<Q>(&opponent_four_move_bit);
    SelectMove(opponent_four_move_bit, candidate_move_bit, candidate_move);
  }

  GetMoveList(*candidate_move_bit, candidate_move);
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
  VLMSearch vlm_search;
  vlm_search.is_search = false;
  vlm_search.remain_depth = 225;

  GetCandidateMoveOR<P>(vlm_search, &candidate_move);

  const auto hash_value = CalcHashValue(board_move_sequence_);
  const bool is_black_turn = P == kBlackTurn;
  BitBoard child_bit_board = bit_board_;

  VLMSearchValue search_value;
  
  if(!vlm_table_->find(hash_value, bit_board_, &search_value) || !IsVLMProved(search_value))
  {
    return false;
  }

  const auto depth = GetVLMDepth(search_value);

  if(depth == 1){
    // 末端チェック
    MovePosition terminating_move;
    const bool is_terminate = TerminateCheck(&terminating_move);

    if(is_terminate){
      // 終端
      proof_tree->AddChild(terminating_move);
      return true;
    }
  }

  constexpr PositionState S = GetPlayerStone(P);
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  bool is_proof_tree_generated = false;

  // いずれかの候補手で詰みが登録されているかチェックする
  for(const auto move : candidate_move){
    // ほとんどの候補手は詰まないのでBitBoard, Hash値の更新のみで置換表をチェックする
    const auto child_hash_value = CalcHashValue(is_black_turn, move, hash_value); // OR nodeはPassがないため差分計算する
    child_bit_board.SetState<S>(move);

    VLMSearchValue child_search_value;
    const auto is_find = vlm_table_->find(child_hash_value, child_bit_board, &child_search_value);

    child_bit_board.SetState<kOpenPosition>(move);

    if(!is_find){
      continue;
    }

    if(!IsVLMProved(child_search_value)){
      continue;
    }

    // move: 登録済の詰む手
    const bool is_already_child = proof_tree->MoveChildNode(move);

    if(is_already_child){
      // すでに証明木に登録済
      proof_tree->MoveParent();
      continue;
    }

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

  VLMSearch vlm_search;
  vlm_search.is_search = false;
  vlm_search.remain_depth = 225;
  
  MoveList candidate_move;
  GetCandidateMoveAND<P>(vlm_search, &candidate_move);

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

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SimulationOR(const VLMSearch &vlm_search, MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);

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
    const bool is_terminate = TerminateCheck(&terminating_move);

    if(is_terminate){
      // 終端
      constexpr VLMSearchDepth depth = 1;
      constexpr VLMSearchValue search_value = GetVLMProvedSearchValue(depth);
      vlm_table_->Upsert(hash_value, bit_board_, search_value);
      return search_value;
    }
  }

  if(vlm_search.remain_depth == 1){
    // 残り深さ１で終端していなければ弱意の不詰
    constexpr VLMSearchDepth depth = 1;
    constexpr auto search_value = GetVLMWeakDisprovedSearchValue(depth);
    vlm_table_->Upsert(hash_value, bit_board_, search_value);
    return search_value;
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveOR<P>(vlm_search, &candidate_move);

  // 展開
  VLMSearch child_vlm_search = vlm_search;
  child_vlm_search.remain_depth--;
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  VLMSearchValue or_node_value = kVLMWeakDisprovedLB;   // 展開する手を証明木の手に制限するので詰まなくても弱意の不詰

  for(const auto move : candidate_move){
    // 証明木に存在する手のみ展開する
    if(!proof_tree->MoveChildNode(move)){
      continue;
    }

    MakeMove(move);
    VLMSearchValue and_node_value = SimulationAND<Q>(child_vlm_search, proof_tree);
    UndoMove();
    proof_tree->MoveParent();

    or_node_value = std::max(or_node_value, and_node_value);
    
    if(!vlm_search.detect_dual_solution && IsVLMProved(and_node_value)){
      break;
    }
  }

  const VLMSearchValue search_value = GetSearchValue(or_node_value);

  if(IsVLMProved(search_value)){
    // Simulaitonでは確定値のみ登録する
    vlm_table_->Upsert(hash_value, bit_board_, search_value);
  }

  return search_value;
}

template<PlayerTurn P>
VLMSearchValue VLMAnalyzer::SimulationAND(const VLMSearch &vlm_search, MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);

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
    const bool is_terminate = TerminateCheck(&terminating_move);

    if(is_terminate){
      // 終端
      vlm_table_->Upsert(hash_value, bit_board_, kVLMStrongDisproved);
      return kVLMStrongDisproved;
    }
  }

  // 候補手生成
  MoveList candidate_move;
  GetCandidateMoveAND<P>(vlm_search, &candidate_move);

  // 候補手がすべて証明木に登録されているかチェックする
  MoveList proof_tree_move;
  proof_tree->GetChildMoveList(&proof_tree_move);

  MoveBitSet proof_tree_move_bit;

  for(const auto move : proof_tree_move){
    proof_tree_move_bit.set(move);
  }

  for(const auto move : candidate_move){
    if(!proof_tree_move_bit[move]){
      // 候補手が登録されていない場合はSimulation失敗
      return kVLMWeakDisprovedUB;
    }
  }

  // 展開
  VLMSearch child_vlm_search = vlm_search;
  child_vlm_search.remain_depth--;
  constexpr PlayerTurn Q = GetOpponentTurn(P);
  VLMSearchValue and_node_value = kVLMProvedUB;

  for(const auto move : candidate_move){
    proof_tree->MoveChildNode(move);
    MakeMove(move);
    VLMSearchValue or_node_value = SimulationOR<Q>(child_vlm_search, proof_tree);
    UndoMove();
    proof_tree->MoveParent();

    and_node_value = std::min(and_node_value, or_node_value);

    if(!IsVLMProved(or_node_value)){
      break;
    }
  }

  const VLMSearchValue search_value = GetSearchValue(and_node_value);

  if(IsVLMProved(search_value)){
    // Simulationでは確定値のみ登録する
    vlm_table_->Upsert(hash_value, bit_board_, search_value);
  }

  return search_value;
}

inline const VLMSearchValue VLMAnalyzer::GetSearchValue(const VLMSearchValue child_search_value) const
{
  if(IsVLMDisproved(child_search_value)){
    return kVLMStrongDisproved;
  }

  return child_search_value - 1;
}
}   // namespace realcore

#endif    // VLM_ANALYZER_INL_H
