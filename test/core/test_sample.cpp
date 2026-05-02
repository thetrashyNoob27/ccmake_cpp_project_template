#include <gtest/gtest.h>

// Sample test to verify gtest integration works
TEST(SampleTest, BasicAssertions) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST(SampleTest, StringComparison) {
    std::string hello = "hello";
    EXPECT_EQ(hello, "hello");
    EXPECT_NE(hello, "world");
}

// Example of how a core/ module test would look
TEST(CoreStrUtils, PlaceholderForFutureTests) {
    // TODO: add tests once core/str/ module is implemented
    EXPECT_EQ(1, 1);
}
