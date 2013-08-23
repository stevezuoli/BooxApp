#include "image_model.h"

#include <gtest/gtest.h>

#include "onyx/base/base.h"

namespace image {
namespace {

TEST(ImageModelTest, OpenAndClose) {
    ImageModel model;
    ASSERT_TRUE(model.open("testdata/a.png"));
    EXPECT_TRUE(model.close());
}

TEST(ImageModelTest, IsSameFile) {
    ImageModel model;
    ASSERT_TRUE(model.open("testdata/a.png"));
    EXPECT_TRUE(model.isTheDocument("testdata/a.png"));
    EXPECT_FALSE(model.isTheDocument("testdata/b.png"));
    EXPECT_TRUE(model.close());
}

}  // namespace
}  // namespace image
