IDIR = ../include
CXX = g++
CXXFLAGS = -I$(IDIR) -std=c++17 -pedantic -pedantic-errors

ODIR = obj
LDIR = ../lib

_DEPS = autocomplete.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = startup.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cc $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

autocomplete.out: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
