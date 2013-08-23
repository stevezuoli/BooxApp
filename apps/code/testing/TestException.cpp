#include "TestException.h"

namespace testing {

TestException::TestException( const char* file_, int line_, const char* message_ )
    : file( file_ )
    , line( line_ )
    , message( message_ )
{
}

}  // namespace testing
