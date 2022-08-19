# Project Info
This repository contains a battleship player, implemented according to information-theoretic principles. With each guess, the computer tries to maximally decrease the entropy of the potential locations of the battleships.

Note: this program utilizes a random number generator (mt.c) called the Mersenne Twister, implemented by Makoto Matsumoto and Takuji Nishimura (http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html).

# Running battleship

Using make:
```
$ make
$ ./bin/battleship.exe
```

Using gcc:
```
$ gcc -c -o battleship.o battleship.c
$ gcc -c -o hashmap.o hashmap.c
$ gcc -c -o mt.o mt.c
$ gcc -o bin/battleship battleship.o hashmap.o mt.o -I/headers
$ ./bin/battleship.exe
```