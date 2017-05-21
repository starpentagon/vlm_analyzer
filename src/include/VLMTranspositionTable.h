//! @file
//! @brief VLM analyzerの置換表定義
//! @author Koichi NABETANI
//! @date 2017/04/25
#ifndef VLM_TRANSPOSITION_TABLE_H
#define VLM_TRANSPOSITION_TABLE_H

#include <array>
#include <vector>

#include "HashTable.h"
#include "VLMParameter.h"

namespace realcore{

typedef std::int16_t VLMSearchValue;   //!< 探索結果を表す値

// 盤面をHash値で持つ場合のデータ構造
typedef struct structVLMHashData{
  structVLMHashData()
  : hash_value(0), logic_counter(0), search_value(0)
  {
  }

  structVLMHashData(const HashValue hash, const VLMSearchValue search)
  : hash_value(hash), search_value(search)
  {
  }
  
  HashValue hash_value;               // Hash値
  TableLogicCounter logic_counter;    // 論理カウンタ
  VLMSearchValue search_value;        // 探索結果
}VLMHashData;

// 盤面情報をすべて持つ場合のデータ構造
typedef struct structVLMExactData{
  structVLMExactData()
  : hash_value(0), board{{0}}, logic_counter(0), search_value(0)
  {
  }

  structVLMExactData(const HashValue hash, const VLMSearchValue search)
  : hash_value(hash), board{{0}}, logic_counter(0), search_value(search)
  {
  }

  HashValue hash_value;               // Hash値
  std::array<StateBit,8> board;                  // 盤面情報
  TableLogicCounter logic_counter;    // 論理カウンタ
  VLMSearchValue search_value;        // 探索結果
}VLMExactData;

#if kUseExactBoardInfo
typedef VLMExactData VLMTableData;
#else
typedef VLMHashData VLMTableData;
#endif

// 前方宣言
class BitBoard;

class VLMTable
{
public:
  //! @brief 置換表を確保する
  //! @param table_mb_size HashTableのサイズ(MB)
  VLMTable(const size_t table_space, const bool lock_control);

  //! @brief 置換表へのupsertを行う
  //! @param hash_value 局面のハッシュ値
  //! @param bit_board 現局面のBitBoard
  //! @param search_value 登録する探索結果
  void Upsert(const HashValue hash_value, const BitBoard &bit_board, const VLMSearchValue search_value);

  //! @brief 置換表の検索を行う
  //! @param search_value 探索結果の格納先
  //! @retval true 置換表にデータがある
  const bool find(const HashValue hash_value, const BitBoard &bit_board, VLMSearchValue * const search_value) const;

  //! @brief 置換表の定義情報を取得する
  std::string GetDefinitionInfo() const;

  //! @brief 初期化を行う
  void Initialize();

private:
  HashTable<VLMTableData> hash_table_;
  std::vector<VLMTableData> proved_data_list_;    // 証明木を取得するため詰むデータを記録する
};

}   // namespace realcore



#endif    // VLM_TRANSPOSITION_TABLE_H
