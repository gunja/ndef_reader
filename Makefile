BIN=ndef_reader

OBJS= main.o pn532.o

all: $(BIN)

ndef_reader:    $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(BIN)
