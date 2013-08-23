#include "image_item.h"

#include <gtest/gtest.h>

#include "onyx/base/base.h"

namespace image {
namespace {

TEST(ImageItemTest, Reload ) {
    ImageItem image("testdata/nonexist.png");
    // TODO(hjiang): Reload() currently always return true. It should
    // return false when loading nonexistent files. But this need some
    // further refactoring.
    EXPECT_TRUE(image.reload());
}

}  // namespace
}  // namespace image
