find . -name CMakeFiles | xargs rm -rf
find . -name '*~' | xargs rm -f
rm -f CMakeCache.txt
rm -rf bin
