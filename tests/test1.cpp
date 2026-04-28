#include "gtest/gtest.h"

int add (int a, int b) {
    return a + b;
}
TEST(MathTest, Addition) {
    EXPECT_EQ(add(2, 3), 5);
    ASSERT_TRUE(add(1, 2) == 3);
}