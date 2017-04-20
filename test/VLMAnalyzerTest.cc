#include "gtest/gtest.h"

#include "VLMAnalyzer.h"

namespace realcore
{

class VLMAnalyzerTest
: public ::testing::Test
{
public:
  void MakeMoveUndoTest(){
    MoveList move_list("hh");
    VLMAnalyzer vlm_analyzer(move_list);

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
      VLMAnalyzer vlm_analyzer(move_list);

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
      VLMAnalyzer vlm_analyzer(move_list);

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
      VLMAnalyzer vlm_analyzer(move_list);

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
      VLMAnalyzer vlm_analyzer(move_list);

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
  ASSERT_FALSE(IsVLMProved(kVLMWeakDisproved));
  ASSERT_TRUE(IsVLMProved(kVLMProvedLB));
  ASSERT_TRUE(IsVLMProved(kVLMProvedUB));
}

TEST_F(VLMAnalyzerTest, IsVLMDisprovedTest){
  ASSERT_TRUE(IsVLMDisproved(kVLMStrongDisproved));
  ASSERT_FALSE(IsVLMDisproved(kVLMWeakDisproved));
  ASSERT_FALSE(IsVLMDisproved(kVLMProvedLB));
  ASSERT_FALSE(IsVLMDisproved(kVLMProvedUB));
}

TEST_F(VLMAnalyzerTest, GetVLMDepthTest){
  {
    constexpr VLMSearchValue value = kVLMProvedUB;
    ASSERT_EQ(1, GetVLMDepth(value));
  }
  {
    constexpr VLMSearchValue value = kVLMProvedUB;
    ASSERT_EQ(1, GetVLMDepth(value));
  }
}

TEST_F(VLMAnalyzerTest, GetVLMSearchValueTest)
{
  {
    constexpr VLMSearchDepth depth = 1;
    ASSERT_EQ(kVLMProvedUB, GetVLMSearchValue(depth));
  }
  {
    constexpr VLMSearchDepth depth = 225;
    ASSERT_EQ(kVLMProvedLB, GetVLMSearchValue(depth));
  }
}

}
