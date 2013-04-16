#g++-4.7 -std=gnu++11 -Wzero-as-null-pointer-constant -g -o test test.cpp -I../.. -lX11 -pthread
g++-4.7 -std=gnu++11 -Wzero-as-null-pointer-constant -Wreturn-type -O3 -fomit-frame-pointer -s -o test test.cpp -I../.. -lX11 -pthread
