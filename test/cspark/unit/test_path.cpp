/* ================================================================== *
 * Unit test for spark::support::Path
 * ================================================================== */

#include "gtest/gtest.h"
#include "spark/support/path.h"

namespace spark {
namespace support {

TEST(PathTest, Concat) {
  Path p0("foo");
  Path p1(p0, "bar");
  
  EXPECT_EQ("foo/bar", p1.str());
}

TEST(PathTest, IsDir) {
  Path p("..");
  EXPECT_TRUE(p.exists());
  EXPECT_TRUE(p.isDir());
  EXPECT_FALSE(p.isFile());
}

TEST(PathTest, IsFile) {
  Path p("unittest");
  EXPECT_TRUE(p.exists());
  EXPECT_FALSE(p.isDir());
  EXPECT_TRUE(p.isFile());
}

TEST(PathTest, NoExist) {
  Path p("bad");
  EXPECT_FALSE(p.exists());
  EXPECT_FALSE(p.isDir());
  EXPECT_FALSE(p.isFile());
}

TEST(PathTest, Iterate) {
  Path p("..");
  PathIterator pi = p.iterate();
  StringRef entry;
  bool hasUnit = false;
  while (pi.next(entry)) {
    if (entry == "unit") {
      hasUnit = true;
    }
  }
  ASSERT_TRUE(hasUnit);
}

TEST(PathTest, NextComponent) {
  Path pa("a/test/path");
  Path pb("/another/path/");
  size_t index = 0;
  size_t size = 0;
  
  EXPECT_TRUE(pa.nextComponent(index, size));
  ASSERT_EQ(0, index);
  ASSERT_EQ(1, size);

  EXPECT_TRUE(pa.nextComponent(index, size));
  ASSERT_EQ(2, index);
  ASSERT_EQ(4, size);

  EXPECT_TRUE(pa.nextComponent(index, size));
  ASSERT_EQ(7, index);
  ASSERT_EQ(4, size);

  EXPECT_FALSE(pa.nextComponent(index, size));
  ASSERT_EQ(11, index);
  ASSERT_EQ(0, size);

  index = 0;
  size = 0;
  EXPECT_TRUE(pb.nextComponent(index, size));
  ASSERT_EQ(1, index);
  ASSERT_EQ(7, size);

  EXPECT_TRUE(pb.nextComponent(index, size));
  ASSERT_EQ(9, index);
  ASSERT_EQ(4, size);

  EXPECT_FALSE(pb.nextComponent(index, size));
  ASSERT_EQ(14, index);
  ASSERT_EQ(0, size);
}

TEST(PathTest, PrevComponent) {
  Path pa("a/test/path");
  Path pb("/another/path/");
  size_t index = pa.size();
  size_t size = 0;
  
  EXPECT_TRUE(pa.prevComponent(index, size));
  ASSERT_EQ(7, index);
  ASSERT_EQ(4, size);

  EXPECT_TRUE(pa.prevComponent(index, size));
  ASSERT_EQ(2, index);
  ASSERT_EQ(4, size);

  EXPECT_TRUE(pa.prevComponent(index, size));
  ASSERT_EQ(0, index);
  ASSERT_EQ(1, size);

  EXPECT_FALSE(pa.prevComponent(index, size));
  ASSERT_EQ(0, index);
  ASSERT_EQ(0, size);

  index = pb.size();
  size = 0;
  EXPECT_TRUE(pb.prevComponent(index, size));
  ASSERT_EQ(9, index);
  ASSERT_EQ(4, size);

  EXPECT_TRUE(pb.prevComponent(index, size));
  ASSERT_EQ(1, index);
  ASSERT_EQ(7, size);

  EXPECT_FALSE(pb.prevComponent(index, size));
  ASSERT_EQ(0, index);
  ASSERT_EQ(0, size);
}

TEST(PathTest, MakeRelative) {
  Path pa("a/test/path/thingy");
  Path pb("a/test");
  
  pa.makeRelative(pb);
  ASSERT_EQ("path/thingy", pa.str());
}

}}
