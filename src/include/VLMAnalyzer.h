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
  : is_search(true), detect_dual_solution(true), remain_depth(kInBoardMoveNum)
  {
  }

  bool is_search;                 //!< 探索(true), 証明木取得, Simulaiton(false): MoveOrdering制御に使用
  bool detect_dual_solution;      //!< 余詰探索をするかどうかのフラグ
  VLMSearchDepth remain_depth;    //!< 探索残り深さ
}VLMSearch;

//! @brief 解図結果
typedef struct structVLMResult
{
  structVLMResult()
  : solved(false), disproved(false), search_depth(0), detect_dual_solution(false)
  {
  }

  bool solved;          // 解図できかどうか
  bool disproved;       // 反証できたかどうか
  MoveTree proof_tree;  // 解図できた時の証明木
  VLMSearchDepth search_depth;     // 探索済の深さ
  bool detect_dual_solution;       // 余詰の有無
  MoveTree dual_solution_tree;     // 余詰の変化
  MoveList best_response;          // 最善応手
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
  void MakeMove(const VLMSearch &child_vlm_search, const MovePosition move);

  //! @brief 指し手を１手戻す
  void UndoMove();

  //! @brief 現局面をroot nodeとする証明木を取得する
  const bool GetProofTree(MoveTree * const proof_tree);

  //! @brief 現局面をroot nodeとする集約した証明木を取得する
  //! @note AND nodeでPassして詰む手順と同一手順で詰む手はPassに集約する
  const bool GetSummarizedProofTree(MoveTree * const proof_tree);

  //! @brief 探索制御オブジェクトを返す
  const SearchManager& GetSearchManager() const;

  //! @brief パラメタ設定情報を返す
  const std::string GetSettingInfo() const;

private:
  //! @brief 余詰判定を行う
  //! @param proof_tree 証明木
  //! @param best_response 最善応手
  //! @param dual_solution_tree 余詰解
  //! @retval true 余詰が存在する
  //! @note 証明木はすべての変化を生成してあること(kGenerateFullTreeであること)
  const bool DetectDualSolution(MoveTree * const proof_tree, MoveList * const best_response, MoveTree * const dual_solution_tree);

  //! @brief 余詰判定(OR node)
  template<PlayerTurn P>
  const bool DetectDualSolutionOR(MoveTree * const proof_tree, MoveList * const best_response, MoveTree * const dual_solution_tree);
  
  //! @brief 余詰判定(AND node)
  template<PlayerTurn P>
  const bool DetectDualSolutionAND(MoveTree * const proof_tree, MoveList * const best_response, MoveTree * const dual_solution_tree);

  //! @brief 余詰となる手を管理する
  //! @param move 詰む手
  //! @param move_proof_tree 詰む手 -> 証明木のmap
  template<PlayerTurn P>
  void UpdateDualSolution(const MovePosition move, std::map<MovePosition, MoveTree> * const move_proof_tree);

  //! @brief 手順前後を検知するために終端局面直前の局面までのOR node手順のHash値を求める
  void GetPreTerminateHash(MoveTree * const proof_tree, std::set<HashValue> * const pre_terminate_hash_set) const;

  //! @brief OR nodeの探索
  template<PlayerTurn P>
  VLMSearchValue SolveOR(const VLMSearch &vlm_search, VLMResult * const vlm_result);
  
  VLMSearchValue SolveOR(const bool is_black_turn, const VLMSearch &vlm_search, VLMResult * const vlm_result);

  //! @brief AND nodeの探索
  template<PlayerTurn P>
  VLMSearchValue SolveAND(const VLMSearch &vlm_search, VLMResult * const vlm_result);

  //! @brief OR nodeの指し手生成
  template<PlayerTurn P>
  void GetCandidateMoveOR(const VLMSearch &vlm_search, MoveList * const candidate_move) const;

  //! @brief AND nodeの指し手生成
  //! @retval true 相手に四ノビ or １手勝ちが発生, false 相手に四ノビ and １手勝ちがない
  template<PlayerTurn P>
  bool GetCandidateMoveAND(const VLMSearch &vlm_search, MoveList * const candidate_move) const;

  //! @brief OR nodeのMoveOrdering
  //! @pre 相手に四ノビが生じていない
  //! @param candidate_move_bit 候補手の位置
  //! @param candidate_move ソート済の指し手リストの格納先
  template<PlayerTurn P>
  void MoveOrderingOR(const VLMSearch &vlm_search, MoveBitSet * const candidate_move_bit, MoveList * const candidate_move) const;

  //! @brief AND nodeのMoveOrdering
  //! @pre 相手に四ノビが生じていない
  //! @param candidate_move_bit 候補手の位置
  //! @param ソート済の指し手リストの格納先
  template<PlayerTurn P>
  void MoveOrderingAND(MoveBitSet * const candidate_move_bit, MoveList * const candidate_move) const;

  //! @brief 証明木の生成モード
  static constexpr bool kGenerateFullTree = true;         // すべての変化を生成する
  static constexpr bool kGenerateSummarizedTree = false;  // Passして詰む手順と同一手順で詰む変化はPassに集約する

  //! @brief 証明木の取得(OR node)
  //! @param generate_full_tree 証明木の生成モード
  template<PlayerTurn P>
  const bool GetProofTreeOR(MoveTree * const proof_tree, const bool generate_full_tree);

  //! @brief 証明木の取得(AND node)
  //! @param generate_full_tree 証明木の生成モード
  template<PlayerTurn P>
  const bool GetProofTreeAND(MoveTree * const proof_tree, const bool generate_full_tree);

  static constexpr bool kCheckVLMTable = true;    // Transposition tableのチェックを行う
  static constexpr bool kScanProofTree = false;   // Transposition tableのチェックを行わず証明木の走査のみ行う

  //! @brief 証明木によるSimulation(OR node)
  //! @param vlm_search 探索設定
  //! @param check_vlm_table Simulation中にTransposition tableをチェックするか
  //! @param proof_tree 証明木
  template<PlayerTurn P>
  VLMSearchValue SimulationOR(const VLMSearch &vlm_search, const bool check_vlm_table, MoveTree * const proof_tree);

  //! @brief 証明木によるSimulation(AND node)
  //! @param vlm_search 探索設定
  //! @param check_vlm_table Simulation中にTransposition tableをチェックするか
  //! @param proof_tree 証明木
  template<PlayerTurn P>
  VLMSearchValue SimulationAND(const VLMSearch &vlm_search, const bool check_vlm_table, MoveTree * const proof_tree);

  //! @brief 終端チェック(OR node)
  const bool IsTerminate(VLMResult * const vlm_result);

  //! @brief 子局面の探索結果値から現局面の探索結果値を算出する
  const VLMSearchValue GetSearchValue(const VLMSearchValue child_search_value) const;

  //! @brief root nodeかどうかを返す
  const bool IsRootNode() const;

  SearchManager search_manager_;    //!< 探索制御
  MoveList search_sequence_;        //!< 探索手順
  std::shared_ptr<VLMTable> vlm_table_;   //!< 置換表
};

}   // namespace realcore

#include "VLMAnalyzer-inl.h"

#endif    // VLM_ANALYZER_H
