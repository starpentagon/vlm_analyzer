//! @file
//! @brief VLM解図を行うクラス
//! @author Koichi NABETANI
//! @date 2017/04/21
#ifndef VLM_ANALYZER_H
#define VLM_ANALYZER_H

#include <cstdint>
#include <climits>
#include <memory>

#include "MoveTree.h"
#include "Board.h"
#include "SearchManager.h"
#include "VLMTranspositionTable.h"

namespace realcore
{

typedef std::int16_t VLMSearchDepth;   //!< 探索深さ

constexpr VLMSearchValue kVLMStrongDisproved = std::numeric_limits<VLMSearchValue>::min();    //!< 強意の不詰(受け側に勝ちがある)
constexpr VLMSearchValue kVLMWeakDisprovedLB = kVLMStrongDisproved + 1;    //!< 弱意の不詰(最小値)
constexpr VLMSearchValue kVLMWeakDisprovedUB = kVLMWeakDisprovedLB + (static_cast<VLMSearchValue>(kInBoardMoveNum) - 1);    //!< 弱意の不詰(最大値)
constexpr VLMSearchValue kVLMProvedLB = kVLMWeakDisprovedUB + 1;    //!< 詰む場合(最小値)
constexpr VLMSearchValue kVLMProvedUB = kVLMProvedLB + (static_cast<VLMSearchValue>(kInBoardMoveNum) - 1);    //!< 詰む場合(最大値)

//! @brief 詰む場合の値かどうかを判定する
constexpr bool IsVLMProved(const VLMSearchValue value);

//! @brief 強意の不詰かどうかを判定する
constexpr bool IsVLMDisproved(const VLMSearchValue value);

//! @brief 弱意の不詰かどうかを判定する
constexpr bool IsVLMWeakDisproved(const VLMSearchValue value);

//! @brief 終端局面までの手数を返す
//! @pre valueは詰む値 or 弱意の不詰であること
constexpr VLMSearchDepth GetVLMDepth(const VLMSearchValue value);

//! @brief 終端局面までの手数から探索結果値を返す
constexpr VLMSearchValue GetVLMProvedSearchValue(const VLMSearchDepth depth);

//! @brief 弱意の不詰の探索結果値を返す
constexpr VLMSearchValue GetVLMWeakDisprovedSearchValue(const VLMSearchDepth depth);

//! @brief 探索制御
typedef struct sturctVLMSearch
{
  sturctVLMSearch()
  : detect_dual_solution(true), remain_depth(kInBoardMoveNum)
  {
  }

  bool detect_dual_solution;   //!< 余詰探索をするかどうかのフラグ
  VLMSearchDepth remain_depth;    //!< 探索残り深さ
}VLMSearch;

//! @brief 解図結果
typedef struct structVLMResult
{
  structVLMResult()
  : solved(false), disproved(false), search_depth(0)
  {
  }

  bool solved;          // 解図できかどうか
  bool disproved;       // 反証できたかどうか
  MoveTree proof_tree;  // 解図できた時の証明木
  VLMSearchDepth search_depth;     // 探索済の深さ
}VLMResult;

// 前方宣言
class VLMAnalyzerTest;

class VLMAnalyzer
: public Board
{
  friend class VLMAnalyzerTest;

public:
  //! @pre 対象局面の指し手リストは終端ではない正規手順であること
  VLMAnalyzer(const MoveList &board_move_sequence);
  VLMAnalyzer(const MoveList &board_move_sequence, const std::shared_ptr<VLMTable> &vlm_table);

  //! @brief 解図を行う
  void Solve(const VLMSearch &vlm_search, VLMResult * const vlm_result);

  //! @brief 指し手を設定する
  void MakeMove(const MovePosition move);

  //! @brief 指し手を１手戻す
  void UndoMove();

  //! @brief 現局面をroot nodeとする証明木を取得する
  const bool GetProofTree(MoveTree * const proof_tree);

  //! @brief 探索制御オブジェクトを返す
  const SearchManager& GetSearchManager() const;

  //! @brief パラメタ設定情報を返す
  const std::string GetSettingInfo() const;

private:
  
  //! @brief OR nodeの探索
  template<PlayerTurn P>
  VLMSearchValue SolveOR(const VLMSearch &vlm_search, VLMResult * const vlm_result);
  
  VLMSearchValue SolveOR(const bool is_black_turn, const VLMSearch &vlm_search, VLMResult * const vlm_result);

  //! @brief AND nodeの探索
  template<PlayerTurn P>
  VLMSearchValue SolveAND(const VLMSearch &vlm_search, VLMResult * const vlm_result);

  //! @brief OR nodeの指し手生成
  template<PlayerTurn P>
  void GetCandidateMoveOR(MoveList * const candidate_move) const;

  //! @brief AND nodeの指し手生成
  //! @retval true 相手に四ノビ or １手勝ちが発生, false 相手に四ノビ and １手勝ちがない
  template<PlayerTurn P>
  bool GetCandidateMoveAND(MoveList * const candidate_move) const;

  //! @brief OR nodeのMoveOrdering
  //! @pre 相手に四ノビが生じていない
  template<PlayerTurn P>
  void MoveOrderingOR(MoveList * const candidate_move) const;

  //! @brief AND nodeのMoveOrdering
  //! @pre 相手に四ノビが生じていない
  //! @param candidate_move_bit 候補手の位置
  //! @param ソート済の指し手リストの格納先
  template<PlayerTurn P>
  void MoveOrderingAND(MoveBitSet * const candidate_move_bit, MoveList * const candidate_move) const;

  //! @brief 証明木の取得(OR node)
  template<PlayerTurn P>
  const bool GetProofTreeOR(MoveTree * const proof_tree);

  //! @brief 証明木の取得(AND node)
  template<PlayerTurn P>
  const bool GetProofTreeAND(MoveTree * const proof_tree);

  //! @brief 証明木によるSimulation(OR node)
  template<PlayerTurn P>
  VLMSearchValue SimulationOR(const VLMSearch &vlm_search, MoveTree * const proof_tree);

  //! @brief 証明木によるSimulation(AND node)
  template<PlayerTurn P>
  VLMSearchValue SimulationAND(const VLMSearch &vlm_search, MoveTree * const proof_tree);

  //! @brief 終端チェック(OR node)
  const bool IsTerminate(VLMResult * const vlm_result);

  //! @brief 子局面の探索結果値から現局面の探索結果値を算出する
  const VLMSearchValue GetSearchValue(const VLMSearchValue child_search_value) const;

  SearchManager search_manager_;    //!< 探索制御
  MoveList search_sequence_;        //!< 探索手順
  std::shared_ptr<VLMTable> vlm_table_;   //!< 置換表
};

}   // namespace realcore

#include "VLMAnalyzer-inl.h"

#endif    // VLM_ANALYZER_H
