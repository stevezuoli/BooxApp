#include "image_items_manager.h"

#include <gtest/gtest.h>

#include "onyx/base/base.h"

namespace image {

class Item {
};

class ItemsManagerTest : public testing::Test {
  protected:
    virtual void SetUp() {
    }

    ItemsManager<QString, Item> items_manager_;
};

TEST_F(ItemsManagerTest, AddAndGet) {
    shared_ptr<Item> a(new Item());
    shared_ptr<Item> b(new Item());
    items_manager_.addImage("a", a);
    items_manager_.addImage("b", b);
    EXPECT_EQ(a, items_manager_.getImage("a"));
    EXPECT_EQ(b, items_manager_.getImage("b"));
    EXPECT_TRUE(NULL == items_manager_.getImage("c"));
}

TEST_F(ItemsManagerTest, Count) {
    EXPECT_EQ(0, items_manager_.itemCount());
    items_manager_.addImage("a", shared_ptr<Item>(new Item()));
    EXPECT_EQ(1, items_manager_.itemCount());
    items_manager_.addImage("b", shared_ptr<Item>(new Item()));
    EXPECT_EQ(2, items_manager_.itemCount());
    items_manager_.addImage("a", shared_ptr<Item>(new Item()));
    EXPECT_EQ(2, items_manager_.itemCount());
    EXPECT_TRUE(items_manager_.removeImage("a"));
    EXPECT_EQ(1, items_manager_.itemCount());
    EXPECT_FALSE(items_manager_.removeImage("a"));
    EXPECT_TRUE(items_manager_.removeImage("b"));
    EXPECT_EQ(0, items_manager_.itemCount());
}

}  // namespace image
