#include "gtest/gtest.h"

#include "VLMTranspositionTable.h"
#include "VLMAnalyzer.h"

using namespace std;

namespace realcore
{

shared_ptr<VLMTable> vlm_table = make_shared<VLMTable>(0, kLockFree);

class VLMAnalyzerTest
: public ::testing::Test
{
public:
  void MakeMoveUndoTest(){
    MoveList move_list("hh");
    VLMAnalyzer vlm_analyzer(move_list, vlm_table);

    ASSERT_EQ(0, vlm_analyzer.search_sequence_.size());

    vlm_analyzer.MakeMove(kMoveHG);
    ASSERT_EQ(1, vlm_analyzer.search_sequence_.size());
    ASSERT_EQ(kMoveHG, vlm_analyzer.search_sequence_[0]);

    vlm_analyzer.UndoMove();
    ASSERT_EQ(0, vlm_analyzer.search_sequence_.size());
  }

  void GetCandidateMoveORTest()
  {
    {
      // 相手に四がある場合
      //   A B C D E F G H I J K L M N O 
      // A + --------------------------+ A 
      // B | . . . . . . . . . . . . . | B 
      // C | . . . . . . . . . . . . . | C 
      // D | . . * . . . . . . . * . . | D 
      // E | . . . . . . . . . . . . . | E 
      // F | . . . . . . . . . . . . . | F 
      // G | . . . . . o o . . . . . . | G 
      // H | . . . . . o x x x x . . . | H 
      // I | . . . . . . . . . . . . . | I 
      // J | . . . . . . . . . . . . . | J 
      // K | . . . . . . . . . . . . . | K 
      // L | . . * . . . . . . . * . . | L 
      // M | . . . . . . . . . . . . . | M 
      // N | . . . . . . . . . . . . . | N 
      // O + --------------------------+ O 
      //   A B C D E F G H I J K L M N O 
      MoveList move_list("hhhgihghjhggkh");
      VLMAnalyzer vlm_analyzer(move_list, vlm_table);

      MoveList candidate_move;
      vlm_analyzer.GetCandidateMoveOR(&candidate_move);

      ASSERT_EQ(1, candidate_move.size());
      ASSERT_EQ(kMoveLH, candidate_move[0]);
    }
    {
      // 相手に四がない場合
      //   A B C D E F G H I J K L M N O 
      // A + --------------------------+ A 
      // B | . . . . . . . . . . . . . | B 
      // C | . . . . . . . . . . . . . | C 
      // D | . . * . . . . . . . * . . | D 
      // E | . . . . . . . . . . . . . | E 
      // F | . . . . . . . . . . . . . | F 
      // G | . . . . . o o . . . . . . | G 
      // H | . . . . . o x x x . . . . | H 
      // I | . . . . . . . . . . . . . | I 
      // J | . . . . . . . . . . . . . | J 
      // K | . . . . . . . . . . . . . | K 
      // L | . . * . . . . . . . * . . | L 
      // M | . . . . . . . . . . . . . | M 
      // N | . . . . . . . . . . . . . | N 
      // O + --------------------------+ O 
      //   A B C D E F G H I J K L M N O 
      MoveList move_list("hhhgihghjhgg");
      VLMAnalyzer vlm_analyzer(move_list, vlm_table);

      MoveList candidate_move;
      vlm_analyzer.GetCandidateMoveOR(&candidate_move);

      ASSERT_EQ(219, candidate_move.size());
    }
  }

  void GetCandidateMoveANDTest()
  {
    {
      // 相手に四がある場合
      //   A B C D E F G H I J K L M N O 
      // A + --------------------------+ A 
      // B | . . . . . . . . . . . . . | B 
      // C | . . . . . . . . . . . . . | C 
      // D | . . * . . . . . . . * . . | D 
      // E | . . . . . . . . . . . . . | E 
      // F | . . . . . . . . . . . . . | F 
      // G | . . . . . o o . . . . . . | G 
      // H | . . . . . o x x x x . . . | H 
      // I | . . . . . . . . . . . . . | I 
      // J | . . . . . . . . . . . . . | J 
      // K | . . . . . . . . . . . . . | K 
      // L | . . * . . . . . . . * . . | L 
      // M | . . . . . . . . . . . . . | M 
      // N | . . . . . . . . . . . . . | N 
      // O + --------------------------+ O 
      //   A B C D E F G H I J K L M N O 
      MoveList move_list("hhhgihghjhggkh");
      VLMAnalyzer vlm_analyzer(move_list, vlm_table);

      MoveList candidate_move;
      vlm_analyzer.GetCandidateMoveAND(&candidate_move);

      ASSERT_EQ(1, candidate_move.size());
      ASSERT_EQ(kMoveLH, candidate_move[0]);
    }
    {
      // 相手に四がない場合
      //   A B C D E F G H I J K L M N O 
      // A + --------------------------+ A 
      // B | . . . . . . . . . . . . . | B 
      // C | . . . . . . . . . . . . . | C 
      // D | . . * . . . . . . . * . . | D 
      // E | . . . . . . . . . . . . . | E 
      // F | . . . . . . . . . . . . . | F 
      // G | . . . . . o o . . . . . . | G 
      // H | . . . . . o x x x . . . . | H 
      // I | . . . . . . . . . . . . . | I 
      // J | . . . . . . . . . . . . . | J 
      // K | . . . . . . . . . . . . . | K 
      // L | . . * . . . . . . . * . . | L 
      // M | . . . . . . . . . . . . . | M 
      // N | . . . . . . . . . . . . . | N 
      // O + --------------------------+ O 
      //   A B C D E F G H I J K L M N O 
      MoveList move_list("hhhgihghjhgg");
      VLMAnalyzer vlm_analyzer(move_list, vlm_table);

      MoveList candidate_move;
      vlm_analyzer.GetCandidateMoveAND(&candidate_move);

      ASSERT_EQ(219 + 1, candidate_move.size());    // Passを含む
    }
  }
};

TEST_F(VLMAnalyzerTest, MakeMoveUndoTest){
  MakeMoveUndoTest();
}

TEST_F(VLMAnalyzerTest, GetCandidateMoveORTest){
  GetCandidateMoveORTest();
}

TEST_F(VLMAnalyzerTest, GetCandidateMoveANDTest){
  GetCandidateMoveANDTest();
}

TEST_F(VLMAnalyzerTest, IsVLMProvedTest){
  ASSERT_FALSE(IsVLMProved(kVLMStrongDisproved));
  ASSERT_FALSE(IsVLMProved(kVLMWeakDisprovedLB));
  ASSERT_FALSE(IsVLMProved(kVLMWeakDisprovedUB));
  ASSERT_TRUE( IsVLMProved(kVLMProvedLB));
  ASSERT_TRUE( IsVLMProved(kVLMProvedUB));
}

TEST_F(VLMAnalyzerTest, IsVLMDisprovedTest){
  ASSERT_TRUE( IsVLMDisproved(kVLMStrongDisproved));
  ASSERT_FALSE(IsVLMDisproved(kVLMWeakDisprovedLB));
  ASSERT_FALSE(IsVLMDisproved(kVLMWeakDisprovedUB));
  ASSERT_FALSE(IsVLMDisproved(kVLMProvedLB));
  ASSERT_FALSE(IsVLMDisproved(kVLMProvedUB));
}

TEST_F(VLMAnalyzerTest, IsVLMWeakDisprovedTest){
  ASSERT_FALSE(IsVLMWeakDisproved(kVLMStrongDisproved));
  ASSERT_TRUE( IsVLMWeakDisproved(kVLMWeakDisprovedLB));
  ASSERT_TRUE( IsVLMWeakDisproved(kVLMWeakDisprovedUB));
  ASSERT_FALSE(IsVLMWeakDisproved(kVLMProvedLB));
  ASSERT_FALSE(IsVLMWeakDisproved(kVLMProvedUB));
}

TEST_F(VLMAnalyzerTest, GetVLMDepthTest){
  {
    constexpr VLMSearchValue value = kVLMProvedUB;
    ASSERT_EQ(1, GetVLMDepth(value));
  }
  {
    constexpr VLMSearchValue value = kVLMProvedLB;
    ASSERT_EQ(225, GetVLMDepth(value));
  }
  {
    constexpr VLMSearchValue value = kVLMWeakDisprovedUB;
    ASSERT_EQ(1, GetVLMDepth(value));
  }
  {
    constexpr VLMSearchValue value = kVLMWeakDisprovedLB;
    ASSERT_EQ(225, GetVLMDepth(value));
  }
}

TEST_F(VLMAnalyzerTest, GetVLMProvedSearchValueTest)
{
  {
    constexpr VLMSearchDepth depth = 1;
    ASSERT_EQ(kVLMProvedUB, GetVLMProvedSearchValue(depth));
  }
  {
    constexpr VLMSearchDepth depth = 225;
    ASSERT_EQ(kVLMProvedLB, GetVLMProvedSearchValue(depth));
  }
}

TEST_F(VLMAnalyzerTest, GetVLMWeakDisprovedSearchValueTest)
{
  {
    constexpr VLMSearchDepth depth = 1;
    ASSERT_EQ(kVLMWeakDisprovedUB, GetVLMWeakDisprovedSearchValue(depth));
  }
  {
    constexpr VLMSearchDepth depth = 225;
    ASSERT_EQ(kVLMWeakDisprovedLB, GetVLMWeakDisprovedSearchValue(depth));
  }
}

TEST_F(VLMAnalyzerTest, TranspositionTableTest)
{
  constexpr size_t table_space = 1;
  VLMTable vlm_table(table_space, kLockFree);

  static_assert(kUseExactBoardInfo == 1, "This test assumes kUseExactBoardInfo == 1");

  // Hash値は等しいが盤面情報が異なるケースのテスト
  constexpr HashValue hash_value = 0;
  BitBoard bit_board_1, bit_board_2;
  constexpr VLMSearchValue search_value = 1;

  bit_board_1.SetState<kBlackStone>(kMoveHH);
  bit_board_2.SetState<kBlackStone>(kMoveHG);

  vlm_table.Upsert(hash_value, bit_board_1, search_value);

  {
    VLMSearchValue table_value = 0;
    const auto find_result = vlm_table.find(hash_value, bit_board_1, &table_value);
    ASSERT_TRUE(find_result);
    ASSERT_EQ(search_value, table_value);
  }
  {
    VLMSearchValue table_value = 0;
    const auto find_result = vlm_table.find(hash_value, bit_board_2, &table_value);
    ASSERT_FALSE(find_result);
  }
}

}
