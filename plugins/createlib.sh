gcc -g -O -c testiplugin.c
ld -Bshareable -o libtesti.so.0.0 testiplugin.o
