CURDIR=$(pwd); cmake ../ && make -j8 && cd ./bin && ./unit-tests; cd $CURDIR
