BIN=ndef_reader

OBJS= main.o pn532.o

all: $(BIN)

ndef_reader: generated.h   $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(BIN)

generated.h:    Makefile main.cpp pn532.h pn532.cpp
	echo "#define BRANCH  \"$(shell git rev-parse --abbrev-ref HEAD)\"" > generated.h
	echo "#define REVISION \"$(shell git rev-parse  HEAD)\"" >> generated.h
	echo "#define IS_DIRTY \"$(shell (git diff --quiet && echo 'clean') || echo 'dirty')\"" >> generated.h
