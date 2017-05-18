//! @file
//! @brief VLM解図のパラメタを定義する
//! @author Koichi NABETANI
//! @date 2017/04/25

#ifndef VLM_PARAMETER_H
#define VLM_PARAMETER_H

#include <cstdint>

#include "OpenState.h"

namespace realcore{

//! @brief 置換表に盤面情報をすべて用いるかどうかのフラグ
//! @note (pros)正確に同一局面を判定できる
//! @note (cons)テーブル要素サイズが16byte->80byteに増え同一容量で保持できる要素数が約1/4になる
#define kUseExactBoardInfo 1

//! @brief 置換表のデフォルトサイズ(1GB)
static constexpr std::size_t kDefaultVLMTableSpace = 1024;

//! @brief 置換表ではデフォルトではロックをしない
static constexpr bool kDefaultVLMTableLockFree = false;

//! @brief 置換表ではデフォルトではInterruption ExceptionをCatchしない
static constexpr bool kDefaultNoInterruptionException = false;

// VLM Analyzer用(黒)の空点情報の更新(深さ5以上)
static constexpr uint64_t kUpdateFlagVLMAnalyzerBlack = 
  kUpdateFlagForbidden |          // OR node(黒)での禁手チェック用
  kUpdateFlagFourWhite |          // AND node(白)での終端チェック(四ノビ点), AND node(白)の終端手防手生成
  kUpdateFlagSemiThreeWhite |     // AND node ordering(白)で三を作る手を生成
  kUpdateFlagPointOfSwordBlack |  // OR node ordering(黒)でミセ手を生成(残り深さ5以上)
  kUpdateFlagTwoBlack;            // OR node ordering(黒)でミセ手を生成(残り深さ5以上)

// VLM Analyzer用(黒)の空点情報の更新(深さ5未満)
static constexpr std::int16_t kShallowDepth = 4;

static constexpr uint64_t kUpdateFlagVLMAnalyzerShallowBlack = 
  kUpdateFlagForbidden |          // OR node(黒)での禁手チェック用
  kUpdateFlagFourWhite |          // AND node(白)での終端チェック(四ノビ点), AND node(白)の終端手防手生成
  kUpdateFlagSemiThreeWhite;      // AND node ordering(白)で三を作る手を生成

constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerBlack(kUpdateFlagVLMAnalyzerBlack);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerShallowBlack(kUpdateFlagVLMAnalyzerShallowBlack);

// VLM Analyzer用(白)の空点情報の更新: 禁手チェック用 + OR nodeでの自分(白番)の終端チェック + AND nodeでの相手(白番)の終端手防手生成(達四点)
static constexpr uint64_t kUpdateFlagVLMAnalyzerWhite = 
  kUpdateFlagForbidden |          // AND node(黒)での禁手チェック用
  kUpdateFlagOpenFourWhite |      // AND node(黒)で相手(白)の終端手防手生成
  kUpdateFlagFourWhite |          // OR node(白)での終端チェック, 終端手防手生成, MoveOrdering OR/AND
  kUpdateFlagSemiThreeWhite |     // OR node ordering(白)での三々生成用
  kUpdateFlagPointOfSwordWhite |  // OR node ordering(白)での四々ミセ、極め手ミセの代替として生成
  kUpdateFlagTwoWhite;            // OR node ordering(白)でミセ手を生成(残り深さ5以上)

static constexpr uint64_t kUpdateFlagVLMAnalyzerShallowWhite = 
  kUpdateFlagForbidden |          // AND node(黒)での禁手チェック用
  kUpdateFlagOpenFourWhite |      // AND node(黒)で相手(白)の終端手防手生成
  kUpdateFlagFourWhite |          // OR node(白)での終端チェック, 終端手防手生成, MoveOrdering OR/AND
  kUpdateFlagSemiThreeWhite |     // OR node ordering(白)での三々生成用
  kUpdateFlagPointOfSwordWhite;   // OR node ordering(白)での四々ミセ、極め手ミセの代替として生成

constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerWhite(kUpdateFlagVLMAnalyzerWhite);    
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerShallowWhite(kUpdateFlagVLMAnalyzerShallowWhite);    
}   // namespace realcore

#endif    // VLM_PARAMETER_H
