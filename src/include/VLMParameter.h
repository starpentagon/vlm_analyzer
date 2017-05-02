//! @file
//! @brief VLM解図のパラメタを定義する
//! @author Koichi NABETANI
//! @date 2017/04/25

#ifndef VLM_PARAMETER_H
#define VLM_PARAMETER_H

#include <cstdint>

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

}   // namespace realcore

#endif    // VLM_PARAMETER_H
