#include "gtest/gtest.h"

#include "VLMAnalyzer.h"
using namespace realcore;

TEST(VLMAnalyzerTest, IsVLMMate){
  ASSERT_FALSE(IsVLMProved(kVLMStrongDisproved));
  ASSERT_FALSE(IsVLMProved(kVLMWeakDisproved));
  ASSERT_TRUE(IsVLMProved(kVLMProvedLB));
  ASSERT_TRUE(IsVLMProved(kVLMProvedUB));
}

TEST(VLMAnalyzerTest, GetVLMDepthTest){
  {
    constexpr VLMSearchValue value = kVLMProvedUB;
    ASSERT_EQ(1, GetVLMDepth(value));
  }
  {
    constexpr VLMSearchValue value = kVLMProvedUB;
    ASSERT_EQ(1, GetVLMDepth(value));
  }
}