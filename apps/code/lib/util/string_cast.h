// Author: Hong Jiang <hjiang@dev-gems.com>

/// This file defines the template function string_cast which is used
/// to convert between different string types. Components should
/// always use base::string in their public interface, but some
/// components may need implementation specificy functionalities (such
/// as QString). string_cast is defined so that the user can convert
/// any string type to the desired string type. For example, the user
/// can use:
///
/// QString qstr = string_cast<QString>(str);  // str is base::string
///
/// without worrying about whether base::string is defined as
/// std::string, QString, or even C string.
///
/// Components that use QString should #include
/// "util/qstring_support.h" instead and link with QtCore.

#ifndef UTIL_STRING_CAST_H__
#define UTIL_STRING_CAST_H__


#include "onyx/base/base.h"

namespace util {

// Many string implementations have constructors that takes a C string.
template <typename T>
inline T string_cast(const char* other) {
    return T(other);
}

template <typename T>
inline T string_cast(const std::string& other);

template <>
inline const char* string_cast<const char*>(const std::string& other) {
    return other.c_str();
}


template <>
inline std::string string_cast<std::string>(const std::string& other) {
    return other;
}

}  // namespace util

#endif  // UTIL_STRING_CAST_H__
