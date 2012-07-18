g++ -fPIC -c custom_intercept.c
g++ -fPIC -c helper.cpp
ld custom_intercept.o helper.o -shared -o intercept.so -ldl -lrt
export LD_PRELOAD="`pwd`/intercept.so /usr/lib64/libstdc++.so.6"
