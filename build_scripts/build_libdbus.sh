make clean
echo "ac_cv_have_abstract_sockets=no" > config.hack
export LDFLAGS=-L${PREFIX}/lib
export CPPFLAGS=-I${PREFIX}/include
./configure --host=${HOST} --prefix=${PREFIX} --cache-file=config.hack --with-x=no --with-xml=expat 

