# Project Variables
FB_DIR=/opt/firebird
FB_INC=$(FB_DIR)/include
FB_LIB=$(FB_DIR)/lib/libfbclient.so
UDF_SRC=FsFBUDF.cpp
UDF_OBJ=FsFBUDF.o
UDF_OBJ_3=FsFBUDF_3.o

# Compiler Defines
CXX=g++
CXXFLAGS=-fPIC -O -c -w -I$(FB_INC)
CXXFLAGS_3=
LIB_LINK_FLAGS=-shared -lfbclient -lm -lc

# The UDF objects Dialect!
all: ib_fansy.so ib_fansy_3.so

ib_fansy.so: $(UDF_OBJ)
	$(CXX) $(LIB_LINK_FLAGS) $(UDF_OBJ) -o $@

ib_fansy_3.so: $(UDF_OBJ_3)
	$(CXX) $(LIB_LINK_FLAGS) $(UDF_OBJ_3) -o $@

$(UDF_OBJ): $(UDF_SRC)
	$(CXX) $(CXXFLAGS) $(UDF_SRC) -o $@

$(UDF_OBJ_3): $(UDF_SRC)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_3) $(UDF_SRC) -o $@

clean:
	rm -f *.o *.so
