#
# Copyright (c) 2014 DCOP. All rights reserved.
#
# Makefile
#


TARGET 	    := dcop_main
TEST        := test_main
OBJ_ROOT    := objs

CC          := gcc
CXX         := g++
CFLAGS      := -Wall -g
CXXFLAGS    := -Wall -g
LINKFLAGS   := -rdynamic
LD          := g++ #gcc -lstdc++

INC_ROOT    := ../../include
SRC_ROOT    := .

INC_DIRS    := $(shell find $(INC_ROOT) -maxdepth 3 -type d)
SRC_DIRS    := $(SRC_ROOT)

INCLUDES    := $(addprefix -I, $(INC_DIRS)) \
               -I./ \
               -I$(SRC_ROOT) \
               -I../../test

OBJ_DIRS    := $(subst $(SRC_ROOT), $(OBJ_ROOT), $(SRC_DIRS))

LIBS        := -lrt -lpthread -ldl \
               -L./components -lcomponents \
               -L./frameworks -lframeworks \
               -L./wrappers -lwrappers \
               -L./avl -lavl \
               -L./md5 -lmd5 \
               -L./xml -lxml \
	       -L../../extlib/json/jsoncpp-svn-release-0.5.0/build/linux -ljson \
	       -L../../extlib/mysql/mysql-connector-c-6.1.5-linux-glibc2.5-i686/lib -lmysqlclient \
	       -L../../extlib/zlib/zlib-1.2.8 -lz

C_SOURCES   := $(foreach d, $(SRC_DIRS), $(wildcard $(d)/*.c))
C_OBJS      := $(patsubst $(SRC_ROOT)/%.c, $(OBJ_ROOT)/%.o, $(C_SOURCES))

CPP_SOURCES := $(foreach d, $(SRC_DIRS), $(wildcard $(d)/*.cpp) )
CPP_OBJS    := $(patsubst $(SRC_ROOT)/%.cpp, $(OBJ_ROOT)/%.o, $(CPP_SOURCES) )


default: init compile test

$(C_OBJS): $(OBJ_ROOT)/%.o:$(SRC_ROOT)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(CPP_OBJS): $(OBJ_ROOT)/%.o:$(SRC_ROOT)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

init:
	$(foreach d, $(OBJ_DIRS), mkdir -p $(d);)


compile: $(C_OBJS) $(CPP_OBJS)
	$(LD) $^ -o $(TARGET) $(LINKFLAGS) $(LIBS)

test: $(C_OBJS) $(CPP_OBJS)
	$(LD) -o $(TEST) $(LINKFLAGS) -L./test -ltest $(LIBS) ../../extlib/gtest/gtest-1.7.0/make/gtest_main.a

clean:
	-rm -rf $(OBJ_ROOT)
	-rm -f $(TARGET)
	-rm -f $(TEST)

install: $(TARGET)
	-cp $(TARGET) $(PREFIX_BIN)

uninstall:
	-rm -f $(PREFIX_BIN)/$(TARGET)

rebuild: clean init compile test