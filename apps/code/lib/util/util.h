#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include <sstream>
#include <string>

#include "onyx/base/base.h"

namespace util {

//template<typename T>
//string to_string(const T& i);
template<typename T>
base::string to_string(const T& i) {
  std::stringstream s;
  s << i;
  return s.str();
}

template<>
base::string to_string(const base::string& str);

// Case-insensitive string comparison.
int nocase_cmp(const base::string & s1, const base::string& s2);

base::string filename_suffix(const base::string& name);

// Remove extra "/"s in \name.
void remove_extra_slashes(base::string *name);

// Release each pointer in a container, and clear the container
// itself. The pointer to the container is NOT deleted.
template<typename Container>
void DeletePtrContainer(Container* container) {
    DCHECK(NULL != container);
    for(typename Container::iterator i = container->begin();
        i != container->end();
        ++i) {
        delete *i;
    }
    container->clear();
}

} // namespace util

#endif // UTIL_UTIL_H
