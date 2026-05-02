#include <gtest/gtest.h>

// This test exists to verify that adding a new test file
// and registering it in test/CMakeLists.txt works as documented.
TEST(BuildVerification, DocInstructionsWork) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

TEST(BuildVerification, CanUseStandardLibrary) {
    std::vector<int> nums = {1, 2, 3};
    EXPECT_EQ(nums.size(), 3u);
    EXPECT_EQ(nums[0], 1);
    EXPECT_EQ(nums.back(), 3);
}
