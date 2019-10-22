OBJECTS := main.o player.o
CXXFLAGS += -std=c++11

all: apiano

clean:
	$(RM) *.o
distclean: clean
	$(RM) apiano
.PHONY: all clean

apiano: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) -o $@
