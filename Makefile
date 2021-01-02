compile=gcc
buildDir=bin
headersDir=headers

deps = headers/battleship.h headers/hashmap.h

obj = battleship.o hashmap.o

%.o: %.c
	$(compile) -c -o $@ $<

battleship: $(obj)
	$(compile) -o ${buildDir}/$@ $^ -I/$(headersDir)