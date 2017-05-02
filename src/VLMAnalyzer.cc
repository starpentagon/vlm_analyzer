#include "VLMAnalyzer.h"

using namespace std;
using namespace realcore;

VLMAnalyzer::VLMAnalyzer(const MoveList &board_move_sequence)
: Board(board_move_sequence), search_manager_(kDefaultNoInterruptionException)
{
  vlm_table_ = make_shared<VLMTable>(kDefaultVLMTableSpace, kDefaultVLMTableLockFree);
}

VLMAnalyzer::VLMAnalyzer(const MoveList &board_move_sequence, const shared_ptr<VLMTable> &vlm_table)
: Board(board_move_sequence), search_manager_(kCatchInterruptException), vlm_table_(vlm_table)
{
}

void VLMAnalyzer::Solve(const VLMSearch &vlm_search, VLMResult * const vlm_result)
{
  assert(vlm_result != nullptr);

  const bool is_black_turn = board_move_sequence_.IsBlackTurn();
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

  if(vlm_result->solved){
    GetProofTree(&vlm_result->proof_tree);
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

const bool VLMAnalyzer::GetProofTree(MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);
  assert(proof_tree->empty());

  const bool is_black_turn = board_move_sequence_.IsBlackTurn();
  bool is_generated = false;

  if(is_black_turn){
    is_generated = GetProofTreeOR<kBlackTurn>(proof_tree);
  }else{
    is_generated = GetProofTreeOR<kWhiteTurn>(proof_tree);
  }

  if(!is_generated){
    proof_tree->clear();
  }

  return is_generated;
}

const std::string VLMAnalyzer::GetSettingInfo() const
{
  stringstream ss;
  
  // Build mode
  ss << "Build:";

  #ifdef NDEBUG
    ss << "Release" << endl;
  #else
    ss << "Debug" << endl;
  #endif

  // 置換表の定義情報
  ss << vlm_table_->GetDefinitionInfo();
  return ss.str();
}
