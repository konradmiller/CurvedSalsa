LDFLAGS   += -lboost_program_options
CXXFLAGS  += -I. -Wall -W

OBJECTS = curvedsalsa.o curve25519-donna.o cubehash.o program_options.o random.o salsa.o

all: ${OBJECTS} main.cpp
	g++ ${CXXFLAGS} -o curvedsalsa main.cpp ${OBJECTS} ${LDFLAGS}

curvedsalsa.o: curvedsalsa.h curvedsalsa.cpp
	g++ ${CXXFLAGS} -c -o curvedsalsa.o curvedsalsa.cpp

curve25519-donna.o: curve25519-donna.c
	gcc ${CXXFLAGS} -c -o curve25519-donna.o curve25519-donna.c

cubehash.o: cubehash.h cubehash.cpp
	g++ ${CXXFLAGS} -c -o cubehash.o cubehash.cpp

program_options.o: program_options.h

random.o: random.h random.cpp
	g++ ${CXXFLAGS} -c -o random.o random.cpp

salsa.o: salsa.h salsa.cpp
	g++ ${CXXFLAGS} -c -o salsa.o salsa.cpp

clean:
	rm -f curvedsalsa ${OBJECTS}
