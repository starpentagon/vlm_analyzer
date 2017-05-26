#include "VLMAnalyzer.h"

using namespace std;

namespace realcore{

VLMAnalyzer::VLMAnalyzer(const MoveList &board_move_sequence)
: Board(board_move_sequence, board_move_sequence.IsBlackTurn() ? kUpdateVLMAnalyzerBlack : kUpdateVLMAnalyzerWhite), 
  search_manager_(kDefaultNoInterruptionException)
{
  vlm_table_ = make_shared<VLMTable>(kDefaultVLMTableSpace, kDefaultVLMTableLockFree);
}

VLMAnalyzer::VLMAnalyzer(const MoveList &board_move_sequence, const shared_ptr<VLMTable> &vlm_table)
: Board(board_move_sequence, board_move_sequence.IsBlackTurn() ? kUpdateVLMAnalyzerBlack : kUpdateVLMAnalyzerWhite), 
  search_manager_(kCatchInterruptException), vlm_table_(vlm_table)
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
    GetSummarizedProofTree(&vlm_result->proof_tree);
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

void VLMAnalyzer::MakeMove(const VLMSearch &child_vlm_search, const MovePosition move)
{
  const bool is_black_turn = (board_move_sequence_.size() - search_sequence_.size()) % 2 == 0;

  if(child_vlm_search.remain_depth == 1){
    const auto update_flag = is_black_turn ? kUpdateVLMAnalyzerDepthOneBlack : kUpdateVLMAnalyzerDepthOneWhite;
    Board::MakeMove(move, update_flag);
  }else if(child_vlm_search.remain_depth == 2){
    const auto update_flag = is_black_turn ? kUpdateVLMAnalyzerDepthTwoBlack : kUpdateVLMAnalyzerDepthTwoWhite;
    Board::MakeMove(move, update_flag);
  }else if(child_vlm_search.remain_depth == 3){
    const auto update_flag = is_black_turn ? kUpdateVLMAnalyzerDepthThreeBlack : kUpdateVLMAnalyzerDepthThreeWhite;
    Board::MakeMove(move, update_flag);
  }else if(child_vlm_search.remain_depth == 4){
    const auto update_flag = is_black_turn ? kUpdateVLMAnalyzerDepthFourBlack : kUpdateVLMAnalyzerDepthFourWhite;
    Board::MakeMove(move, update_flag);
  }else{
    Board::MakeMove(move);
  }

  search_sequence_ += move;
}

void VLMAnalyzer::UndoMove()
{
  --search_sequence_;
  Board::UndoMove();
}

template<>
void VLMAnalyzer::MoveOrderingOR<kBlackTurn>(const VLMSearch &vlm_search, MoveBitSet * const candidate_move_bit, MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);
  assert(candidate_move->empty());

  constexpr PlayerTurn P = kBlackTurn;

  MoveBitSet four_bit, three_bit;
  EnumerateFourMoves<P>(&four_bit);
  EnumerateSemiThreeMoves<P>(&three_bit);

  MoveBitSet mise_bit, multi_mise_bit;

  // @see doc/02_performance/vlm_analyzer_performance.pptx, 「OR nodeの指し手」
  if(vlm_search.remain_depth >= 3){
    // 残り深さ３以上
    // 四三を作る手を優先する
    MoveBitSet four_three_bit(four_bit & three_bit);
    SelectMove(four_three_bit, candidate_move_bit, candidate_move);
  }
  
  if(vlm_search.remain_depth >= 5){
    // 残り深さ5以上
    // (四 or 三) & ミセ手
    EnumerateMiseMoves<P>(&mise_bit, &multi_mise_bit);

    {
      MoveBitSet threat_mise_bit((four_bit | three_bit) & mise_bit);
      SelectMove(threat_mise_bit, candidate_move_bit, candidate_move);
    }

    // 両ミセ
    SelectMove(multi_mise_bit, candidate_move_bit, candidate_move);
  }

  // 剣先
  if(vlm_search.remain_depth >= 5){
    MoveBitSet point_of_sword_bit;
    EnumeratePointOfSwordMoves<P>(&point_of_sword_bit);
    SelectMove(point_of_sword_bit, candidate_move_bit, candidate_move);
  }

  // 二
  if(vlm_search.remain_depth >= 5){
    MoveBitSet two_bit;
    EnumerateTwoMoves<P>(&two_bit);
    SelectMove(two_bit, candidate_move_bit, candidate_move);
  }

  // 四
  SelectMove(four_bit, candidate_move_bit, candidate_move);

  // 三
  SelectMove(three_bit, candidate_move_bit, candidate_move);

  // 残りの手をすべて生成
  GetMoveList(*candidate_move_bit, candidate_move);

  return;
}

template<>
void VLMAnalyzer::MoveOrderingOR<kWhiteTurn>(const VLMSearch &vlm_search, MoveBitSet * const candidate_move_bit, MoveList * const candidate_move) const
{
  assert(candidate_move != nullptr);
  assert(candidate_move->empty());

  constexpr PlayerTurn P = kWhiteTurn;

  MoveBitSet four_bit, three_bit;
  EnumerateFourMoves<P>(&four_bit);
  EnumerateSemiThreeMoves<P>(&three_bit);

  MoveBitSet point_of_sword_bit;
  EnumeratePointOfSwordMoves<P>(&point_of_sword_bit);

  MoveBitSet mise_bit, multi_mise_bit;

  // @see doc/02_performance/vlm_analyzer_performance.pptx, 「OR nodeの指し手」
  if(vlm_search.remain_depth >= 3){
    // 残り深さ３以上
    // 四三, 三々, (三 or 四) & 剣先点(四々ミセ、極め手ミセの代替)を優先する
    {
      // 四三
      MoveBitSet four_three_move_bit(four_bit & three_bit);
      four_three_move_bit &= *candidate_move_bit;

      GetMoveList(four_three_move_bit, candidate_move);
      *candidate_move_bit ^= four_three_move_bit;
    }
    {
      // 三々
      MoveBitSet double_three_bit;
      EnumerateDoubleSemiThreeMoves<P>(&double_three_bit);
      double_three_bit &= *candidate_move_bit;

      GetMoveList(double_three_bit, candidate_move);
      *candidate_move_bit ^= double_three_bit;
    }
    {
      // (三 or 四) & 剣先点(四々ミセ、極め手ミセの代替)
      MoveBitSet threat_sword_move_bit((four_bit | three_bit) & point_of_sword_bit);
      threat_sword_move_bit &= *candidate_move_bit;

      GetMoveList(threat_sword_move_bit, candidate_move);
      *candidate_move_bit ^= threat_sword_move_bit;
    }
  }
  
  if(vlm_search.remain_depth >= 5){
    // 残り深さ5以上
    // (四 or 三) & ミセ手
    EnumerateMiseMoves<P>(&mise_bit, &multi_mise_bit);

    {
      MoveBitSet threat_mise_bit((four_bit | three_bit) & mise_bit);
      threat_mise_bit &= *candidate_move_bit;

      GetMoveList(threat_mise_bit, candidate_move);
      *candidate_move_bit ^= threat_mise_bit;
    }

    // 両ミセ
    {
      multi_mise_bit &= *candidate_move_bit;

      GetMoveList(multi_mise_bit, candidate_move);
      *candidate_move_bit ^= multi_mise_bit;
    }
    
    // ミセ手 & 剣先点(四々ミセ、極め手ミセの代替)
    {
      MoveBitSet threat_sword_move_bit(mise_bit & point_of_sword_bit);
      threat_sword_move_bit &= *candidate_move_bit;

      GetMoveList(threat_sword_move_bit, candidate_move);
      *candidate_move_bit ^= threat_sword_move_bit;
    }
  }

  // 剣先
  if(vlm_search.remain_depth >= 5){
    point_of_sword_bit &= *candidate_move_bit;

    GetMoveList(point_of_sword_bit, candidate_move);
    *candidate_move_bit ^= point_of_sword_bit;
  }

  // 二ノビ
  if(vlm_search.remain_depth >= 5){
    MoveBitSet two_bit;
    EnumerateTwoMoves<P>(&two_bit);
    two_bit &= *candidate_move_bit;

    GetMoveList(two_bit, candidate_move);
    *candidate_move_bit ^= two_bit;
  }

  // 四
  four_bit &= *candidate_move_bit;

  GetMoveList(four_bit, candidate_move);
  *candidate_move_bit ^= four_bit;

  // 三
  three_bit &= *candidate_move_bit;

  GetMoveList(three_bit, candidate_move);
  *candidate_move_bit ^= three_bit;

  // 残りの手をすべて生成
  GetMoveList(*candidate_move_bit, candidate_move);

  return;
}

const bool VLMAnalyzer::GetProofTree(MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);

  proof_tree->MoveRootNode();

  const bool is_black_turn = board_move_sequence_.IsBlackTurn();
  bool is_generated = false;

  if(is_black_turn){
    is_generated = GetProofTreeOR<kBlackTurn>(proof_tree, kGenerateFullTree);
  }else{
    is_generated = GetProofTreeOR<kWhiteTurn>(proof_tree, kGenerateFullTree);
  }

  if(!is_generated){
    proof_tree->clear();
  }

  return is_generated;
}

const bool VLMAnalyzer::GetSummarizedProofTree(MoveTree * const proof_tree)
{
  assert(proof_tree != nullptr);

  proof_tree->MoveRootNode();

  const bool is_black_turn = board_move_sequence_.IsBlackTurn();
  bool is_generated = false;

  if(is_black_turn){
    is_generated = GetProofTreeOR<kBlackTurn>(proof_tree, kGenerateSummarizedTree);
  }else{
    is_generated = GetProofTreeOR<kWhiteTurn>(proof_tree, kGenerateSummarizedTree);
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
}   // namespace realcore
