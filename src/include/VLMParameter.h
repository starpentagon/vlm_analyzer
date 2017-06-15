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
//! @note (cons)テーブル要素サイズが16byte->80byteに増え同一容量で保持できる要素数が約1/5になる
#define kUseExactBoardInfo 1

//! @brief 置換表のデフォルトサイズ(1GB)
static constexpr std::size_t kDefaultVLMTableSpace = 1024;

//! @brief 置換表ではデフォルトではロックをしない
static constexpr bool kDefaultVLMTableLockFree = false;

//! @brief 置換表ではデフォルトではInterruption ExceptionをCatchしない
static constexpr bool kDefaultNoInterruptionException = false;

// VLM Analyzer用(黒)の空点情報の更新(深さ1)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthOneBlack = 
  kUpdateFlagOpenFourBlack;   // OR node(黒)での終端チェック

// VLM Analyzer用(黒)の空点情報の更新(深さ2)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthTwoBlack = 
  kUpdateFlagVLMAnalyzerDepthOneBlack | 
  kUpdateFlagOpenFourBlack |      // AND node(白)での終端手防手
  kUpdateFlagFourWhite;           // AND node(白)での終端チェック, 終端手防手

// VLM Analyzer用(黒)の空点情報の更新(深さ3)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthThreeBlack = 
  kUpdateFlagVLMAnalyzerDepthTwoBlack | 
  kUpdateFlagForbidden |          // 禁手チェック
  kUpdateFlagFourBlack |          // OR node ordering(黒)
  kUpdateFlagSemiThreeBlack;      // OR node ordering(黒)

// VLM Analyzer用(黒)の空点情報の更新(深さ4)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthFourBlack = 
  kUpdateFlagVLMAnalyzerDepthThreeBlack | 
  kUpdateFlagFourBlack |          // AND node ordering(白)
  kUpdateFlagSemiThreeWhite;      // AND node ordering(白)

// VLM Analyzer用(黒)の空点情報の更新(深さ5以上)
static constexpr uint64_t kUpdateFlagVLMAnalyzerBlack = 
  kUpdateFlagVLMAnalyzerDepthFourBlack | 
  kUpdateFlagPointOfSwordBlack |  // OR node ordering(黒)でミセ手を生成(残り深さ5以上)
  kUpdateFlagTwoBlack;            // OR node ordering(黒)でミセ手を生成(残り深さ5以上)

constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthOneBlack(kUpdateFlagVLMAnalyzerDepthOneBlack);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthTwoBlack(kUpdateFlagVLMAnalyzerDepthTwoBlack);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthThreeBlack(kUpdateFlagVLMAnalyzerDepthThreeBlack);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthFourBlack(kUpdateFlagVLMAnalyzerDepthFourBlack);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerBlack(kUpdateFlagVLMAnalyzerBlack);

// VLM Analyzer用(白)の空点情報の更新(深さ1)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthOneWhite = 
  kUpdateFlagFourWhite;           // OR node(白)での終端チェック

// VLM Analyzer用(白)の空点情報の更新(深さ2)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthTwoWhite = 
  kUpdateFlagVLMAnalyzerDepthOneWhite | 
  kUpdateFlagForbidden |          // 禁手チェック
  kUpdateFlagOpenFourBlack |      // AND node(黒)での終端チェック
  kUpdateFlagOpenFourWhite |      // AND node(黒)での終端手防手
  kUpdateFlagFourBlack |          // AND node(黒)での終端手防手
  kUpdateFlagFourWhite;           // AND node(黒)での終端手防手
  
// VLM Analyzer用(白)の空点情報の更新(深さ3)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthThreeWhite = 
  kUpdateFlagVLMAnalyzerDepthTwoWhite | 
  kUpdateFlagFourWhite |          // OR node ordering(白)
  kUpdateFlagSemiThreeWhite |     // OR node ordering(白)
  kUpdateFlagPointOfSwordWhite;   // OR node ordering(白)

// VLM Analyzer用(白)の空点情報の更新(深さ4)
static constexpr uint64_t kUpdateFlagVLMAnalyzerDepthFourWhite = 
  kUpdateFlagVLMAnalyzerDepthThreeWhite | 
  kUpdateFlagFourWhite |          // AND node ordering(黒)
  kUpdateFlagSemiThreeBlack;      // AND node ordering(黒)

// VLM Analyzer用(白)の空点情報の更新: 禁手チェック用 + OR nodeでの自分(白番)の終端チェック + AND nodeでの相手(白番)の終端手防手生成(達四点)
static constexpr uint64_t kUpdateFlagVLMAnalyzerWhite = 
  kUpdateFlagVLMAnalyzerDepthFourWhite | 
  kUpdateFlagTwoWhite;            // OR node ordering(白)でミセ手を生成(残り深さ5以上)

constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthOneWhite(kUpdateFlagVLMAnalyzerDepthOneWhite);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthTwoWhite(kUpdateFlagVLMAnalyzerDepthTwoWhite);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthThreeWhite(kUpdateFlagVLMAnalyzerDepthThreeWhite);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerDepthFourWhite(kUpdateFlagVLMAnalyzerDepthFourWhite);
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerWhite(kUpdateFlagVLMAnalyzerWhite);
}   // namespace realcore

#endif    // VLM_PARAMETER_H
