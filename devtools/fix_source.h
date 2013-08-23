// Author: Hong Jiang <hjiang@18scorpii.com>

/// This is a tool to clean up source files:
///
///  * Replace tabs with spaces.
///
///  * Convert DOS-style EOL to UNIX-style EOL.
///
///  * Remove trailing spaces.
///
/// Usage:
///
/// Pass the name of the source file as arguments. If no argument is
/// present, it reads from standard input.

#include <iosfwd>

namespace devtools {

/// Read from input and write the cleaned up (as defined at the
/// beginning of this file) stream to output. Returns true if no error
/// occured.
bool fix_source(std::istream* input, std::ostream* output);

}
