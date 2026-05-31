gcc -c hwrand.s -o hwrand.o -O3
gcc -c mt.c -o mt.o -O3
gcc -c xorshift.c -o xorshift.o -O3
ar rcs librandom.a hwrand.o xorshift.o mt.o
pause