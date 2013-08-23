// Author: Hong Jiang <hjiang@dev-gems.com>

/// This file is used to isolate QString-related functionality. See
/// the comments in string_cast.h for details.

#ifndef UTIL_QSTRING_SUPPORT_H__
#define UTIL_QSTRING_SUPPORT_H__

#include <QString>
#include "onyx/base/base.h"
#include "util/string_cast.h"


namespace util {

template <>
inline QString string_cast<QString>(const std::string& other) {
    return QString::fromUtf8(other.c_str());
}


template <typename T>
inline T string_cast(const QString& other);


template <>
inline std::string string_cast<std::string>(const QString& other) {
    return other.toUtf8().data();
}


template <>
inline QString string_cast<QString>(const QString& other) {
    return other;
}

}  // namespace util

#endif  // UTIL_QSTRING_SUPPORT_H__
