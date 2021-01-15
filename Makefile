compile=gcc
buildDir=bin
headersDir=headers

# deps = headers/battleship.h headers/hashmap.h

Bobj = battleship.o hashmap.o mt.o
Hobj = hangman.o

%.o: %.c
	$(compile) -c -o $@ $<

battleship: $(Bobj)
	$(compile) -o ${buildDir}/$@ $^ -I/$(headersDir)

hangman: $(Hobj)
	$(compile) -o ${buildDir}/$@ $^ -I/$(headersDir)