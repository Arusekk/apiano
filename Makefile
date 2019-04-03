
CXXFLAGS:=${CXXFLAGS} -pthread -g

OBJECTS:=main.o player.o

all: apiano
	./$<

clean:
	${RM} *.o
.PHONY: all clean

apiano: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o $@
