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

// VLM Analyzer用(黒)の空点情報の更新: 禁手チェック用 + AND nodeでの終端チェック(四ノビ点)
//constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerBlack(0b000111011);    
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerBlack(0b001111011);    // 三を作る手用
//constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerBlack(0b100111011);    // 剣先を作る手用

// VLM Analyzer用(白)の空点情報の更新: 禁手チェック用 + OR nodeでの自分(白番)の終端チェック + AND nodeでの相手(白番)の終端手防手生成(達四点)
constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerWhite(0b000111111);    
//constexpr UpdateOpenStateFlag kUpdateVLMAnalyzerWhite(0b010111111);    // 剣先を作る手用
}   // namespace realcore

#endif    // VLM_PARAMETER_H
