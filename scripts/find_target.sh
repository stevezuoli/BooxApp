# Run this script with one argument with find where a target is defined.

find . -name CMakeLists.txt | xargs grep $1 
