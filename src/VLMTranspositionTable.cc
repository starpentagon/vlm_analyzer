#include "BitBoard.h"
#include "VLMTranspositionTable.h"

using namespace std;
using namespace realcore;

VLMTable::VLMTable(const size_t table_space, const bool lock_control)
: hash_table_(table_space, lock_control)
{
}

void VLMTable::Upsert(const HashValue hash_value, const BitBoard &bit_board, const VLMSearchValue search_value)
{
  VLMTableData table_data(hash_value, search_value);

#if kUseExactBoardInfo
  bit_board.GetBoardStateBit(&table_data.board);
#endif

  hash_table_.Upsert(hash_value, table_data);
}

const bool VLMTable::find(const HashValue hash_value, const BitBoard &bit_board, VLMSearchValue * const search_value) const
{
  VLMTableData table_data;
  const auto find_result = hash_table_.find(hash_value, &table_data);

  if(!find_result){
    return false;
  }

#if kUseExactBoardInfo
  // 盤面が完全一致するかチェック
  array<StateBit, 8> board_info;
  bit_board.GetBoardStateBit(&board_info);

  for(size_t i=0; i<8; i++){
    if(board_info[i] != table_data.board[i]){
      return false;
    }
  }
#endif

  *search_value = table_data.search_value;
  return true;
}

void VLMTable::Initialize(){
  hash_table_.Initialize();
}

std::string VLMTable::GetDefinitionInfo() const
{
  stringstream ss;

  ss << "VLM Transposition Info:" << endl;
  ss << "  Data type: " << (kUseExactBoardInfo == 1 ? "Exact Board(" : "Hash Board(") << sizeof(VLMTableData) << " bytes)" << endl;
  ss << "  Table size: " << hash_table_.size() << endl;

  const auto space = round(10 * hash_table_.space()) / 10;
  ss << "  Table space: " << space << " MB" << endl;

  return ss.str();
}
