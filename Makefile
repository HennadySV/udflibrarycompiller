# ---------------------------------------------------------------------
# Project Variables
# ---------------------------------------------------------------------
FB_DIR=/opt/firebird
FB_INC=$(FB_DIR)/include
FB_LIB=$(FB_DIR)/lib/libib_util.so
UDF_SRC=../SRC/FsFBUDF.cpp
UDF_OBJ=FsFBUDF.o
UDF_OBJ_3=FsFBUDF_3.o

# ---------------------------------------------------------------------
# Compiler Defines
# ---------------------------------------------------------------------

CXX=g++
CXXFLAGS=-fPIC -O -c -w -I$(FB_INC)
LIB_LINK=ld
LIB_LINK_FLAGS=-G -Bsymbolic -lgds -lm -lc

# ---------------------------------------------------------------------
# The UDF objects Dialect1
# ---------------------------------------------------------------------

all: ib_Fansy.so ib_Fansy_3.so


ib_Fansy.so:$(UDF_OBJ)
	$(LIB_LINK) $(UDF_OBJ) $(FB_LIB) -o $@ $(LIB_LINK_FLAGS)

ib_Fansy_3.so:$(UDF_OBJ_3)
	$(LIB_LINK) $(UDF_OBJ_3) $(FB_LIB) -o $@ $(LIB_LINK_FLAGS)

$(UDF_OBJ):$(UDF_SRC)
	$(CXX) $< $(CXXFLAGS) -o $@

$(UDF_OBJ_3):$(UDF_SRC)
	$(CXX) $< $(CXXFLAGS) -DFB_DIALECT3 -o $@
