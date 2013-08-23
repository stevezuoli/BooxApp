// Author: Hong Jiang <hjiang@18scorpii.com>

#include <sstream>

#include "fix_source.h"

#include "onyx/base/base.h"
#include "testing/testing.h"

namespace {

using devtools::fix_source;
using std::istringstream;
using std::ostringstream;

TEST(RemoveTrailingSpaces)
{
    istringstream input("hello world   \n");
    ostringstream output;
    EXPECT_TRUE(fix_source(&input, &output));
    EXPECT_EQ("hello world\n", output.str());
}

TEST(NoTrailingSpaces)
{
    istringstream input("hello world\n");
    ostringstream output;
    EXPECT_TRUE(fix_source(&input, &output));
    EXPECT_EQ("hello world\n", output.str());
}

TEST(TrailingSpacesInMiddle)
{
    istringstream input("hello world  \nI'm a program \n");
    ostringstream output;
    EXPECT_TRUE(fix_source(&input, &output));
    EXPECT_EQ("hello world\nI'm a program\n", output.str());
}

TEST(TabConvertion)
{
    istringstream input("hello world  \t I'm a program \n");
    ostringstream output;
    EXPECT_TRUE(fix_source(&input, &output));
    EXPECT_EQ("hello world       I'm a program\n", output.str());
}

TEST(EolConvertion)
{
    istringstream input("hello world  \r\n I'm a program\r\n");
    ostringstream output;
    EXPECT_TRUE(fix_source(&input, &output));
    EXPECT_EQ("hello world\n I'm a program\n", output.str());
}


}
