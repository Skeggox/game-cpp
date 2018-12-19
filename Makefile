CXX=g++
CXXFLAGS=-Wall -Wextra -Werror -pedantic
CXXFLAGS+=-std=c++17
CXXFLAGS+=-MMD

ifeq ($(BUILD), RELEASE)
	CXXFLAGS+=-03
	CXXFLAGS+=-march=native
else
	CPPFLAGS+=-g -O0
	CPPFLAGS+=-DDEBUG -D_DEBUG
endif

LDFLAGS=-lcppunit -Wl,-Bdynamic

all: build

build: q1/main.exe q2/main.exe

q1/main.exe: q1/main.o q1/card.o
	$(CXX) -o $@ $^

q2/main.exe: q2/main.o q2/memoryallocator.o q2/tests/memoryallocator.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f q1/*.o q1/*.exe q1/*.d
	rm -f q2/*.o q2/*.exe q2/*.d
	rm -f q2/tests/*.o q2/tests/*.d

-include q1/*.d
-include q2/*.d
-include q2/tests/*.d
