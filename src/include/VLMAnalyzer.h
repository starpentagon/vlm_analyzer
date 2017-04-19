#ifndef VLM_ANALYZER_H
#define VLM_ANALYZER_H

#include <cstdint>
#include <climits>

#include "MoveTree.h"
#include "Board.h"
#include "SearchManager.h"

namespace realcore
{

typedef std::uint8_t VLMSearchDepth;   //!< 探索深さ
typedef std::int16_t VLMSearchValue;   //!< 探索結果を表す値

constexpr VLMSearchValue kVLMStrongDisproved = std::numeric_limits<VLMSearchValue>::min();    //!< 強意の不詰(受け側に勝ちがある)
constexpr VLMSearchValue kVLMWeakDisproved = kVLMStrongDisproved + 1;    //!< 弱意の不詰
constexpr VLMSearchValue kVLMProvedLB = kVLMWeakDisproved + 1;    //!< 詰む場合(最小値)
constexpr VLMSearchValue kVLMProvedUB = kVLMProvedLB + (static_cast<VLMSearchValue>(kInBoardMoveNum) - 1);    //!< 詰む場合(最大値)

//! @brief 詰む場合の値かどうかを判定する
constexpr bool IsVLMProved(const VLMSearchValue value);

//! @brief 終端局面までの手数を返す
//! @pre valueは詰む値であること
constexpr VLMSearchDepth GetVLMDepth(const VLMSearchValue value);

//! @brief 探索制御
typedef struct sturctVLMSearch
{
  bool is_detect_dual_solution;   //!< 余詰探索をするかどうかのフラグ
  VLMSearchDepth max_depth;    //!< 探索最大深さ
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

class VLMAnalyzer
: public Board
{
public:
  VLMAnalyzer(const MoveList &move_list);

  //! @brief 総当り探索で解図を行う
  void SolveBruteForce(const VLMSearch &vlm_search, VLMResult * const vlm_result);
private:
  
  //! @brief 総当り探索(OR node)
  template<PlayerTurn P>
  void SolveORBruteForce(const VLMSearch &vlm_search, VLMResult * const vlm_result);

  //! @brief 総当り探索(AND node)
  template<PlayerTurn P>
  void SolveANDBruteForce(const VLMSearch &vlm_search, VLMResult * const vlm_result);

  SearchManager search_manager_;    //!< 探索制御
};

}   // namespace realcore

#include "VLMAnalyzer-inl.h"

#endif    // VLM_ANALYZER_H
