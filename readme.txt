On my machine:
mingw32-make battleship
./bin/battleship.exe

If unable to use Makefile, run the following commands:
gcc -c -o battleship.o battleship.c
gcc -c -o hashmap.o hashmap.c
gcc -o bin/battleship battleship.o hashmap.o -I/headers
./bin/battleship.exe