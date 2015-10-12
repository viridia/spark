/* ================================================================== *
 * Unit test for spark::source::Location
 * ================================================================== */

#include "gtest/gtest.h"
#include "spark/source/location.h"

namespace spark {
namespace source {

TEST(LocationTest, UnionTest) {
  Location S0(NULL, 1, 10, 1, 15);
  Location S1(NULL, 1, 20, 1, 25);
  Location S3(NULL, 2, 10, 2, 11);
  Location S4(NULL, 1, 5, 3, 22);
  
  Location R0 = S0 | S1;
  EXPECT_EQ(1, R0.startLine);
  EXPECT_EQ(10, R0.startCol);
  EXPECT_EQ(1, R0.endLine);
  EXPECT_EQ(25, R0.endCol);

  R0 = S1 | S0;
  EXPECT_EQ(1, R0.startLine);
  EXPECT_EQ(10, R0.startCol);
  EXPECT_EQ(1, R0.endLine);
  EXPECT_EQ(25, R0.endCol);

  R0 = S0 | S3;
  EXPECT_EQ(1, R0.startLine);
  EXPECT_EQ(10, R0.startCol);
  EXPECT_EQ(2, R0.endLine);
  EXPECT_EQ(11, R0.endCol);

  R0 = S3 | S0;
  EXPECT_EQ(1, R0.startLine);
  EXPECT_EQ(10, R0.startCol);
  EXPECT_EQ(2, R0.endLine);
  EXPECT_EQ(11, R0.endCol);

  R0 = S0 | S4;
  EXPECT_EQ(1, R0.startLine);
  EXPECT_EQ(5, R0.startCol);
  EXPECT_EQ(3, R0.endLine);
  EXPECT_EQ(22, R0.endCol);

  R0 = S4 | S0;
  EXPECT_EQ(1, R0.startLine);
  EXPECT_EQ(5, R0.startCol);
  EXPECT_EQ(3, R0.endLine);
  EXPECT_EQ(22, R0.endCol);
}

}}

