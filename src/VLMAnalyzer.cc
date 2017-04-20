#include "VLMAnalyzer.h"

using namespace realcore;

VLMAnalyzer::VLMAnalyzer(const MoveList &move_list)
: Board(move_list), search_manager_(kCatchInterruptException)
{
}

void VLMAnalyzer::Solve(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  const bool is_black_turn = move_list_.IsBlackTurn();
  VLMSearch vlm_search_iterative = vlm_search;
  VLMSearchValue search_value = kVLMStrongDisproved;

  for(VLMSearchDepth depth=1; depth<=vlm_search.remain_depth; depth+=2){
    vlm_search_iterative.remain_depth = depth;

    search_value = SolveOR(is_black_turn, vlm_search_iterative, vlm_result);

    if(IsVLMProved(search_value) || IsVLMDisproved(search_value)){
      break;
    }

    if(search_manager_.IsTerminate()){
      break;
    }
  }

  // 探索結果
  vlm_result->solved = IsVLMProved(search_value);
  vlm_result->disproved = IsVLMDisproved(search_value);
  vlm_result->search_depth = vlm_search_iterative.remain_depth;

  if(search_manager_.IsTerminate() && vlm_result->search_depth >= 3){
    vlm_result->search_depth -= 2;
  }
}

VLMSearchValue VLMAnalyzer::SolveOR(const bool is_black_turn, const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  if(is_black_turn){
    return SolveOR<kBlackTurn>(vlm_search, vlm_result);
  }else{
    return SolveOR<kWhiteTurn>(vlm_search, vlm_result);
  }
}

void VLMAnalyzer::MakeMove(const MovePosition move)
{
  search_sequence_ += move;
  Board::MakeMove(move);
}

void VLMAnalyzer::UndoMove()
{
  --search_sequence_;
  Board::UndoMove();
}

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

  move_list_.GetOpenMove(forbidden_bit, candidate_move);
}

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
  
  move_list_.GetPossibleMove(forbidden_bit, candidate_move);
}
