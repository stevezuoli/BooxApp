#include "image_application.h"
#include "gtest/gtest.h"

namespace image{
namespace {

TEST(ImageApplicationTest, Initialization) {
    // Just make sure the application can be initialized. This is
    // identical to main() without entering the event loop.
    ImageApplication app(0, NULL);
    ImageApplicationAdaptor adaptor(&app);
}

}  // namespace
}  // namespace image
