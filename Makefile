include ${HOME}/Makefile

CXXFLAGS:=${CXXFLAGS} -pthread -g

all: apiano
	./$<

clean:
	${RM} *.o
.PHONY: all clean

apiano: main.o player.o
	${CXX} ${CXXFLAGS} $^ -o $@
