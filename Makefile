
SOURCES := $(wildcard src/*.cpp) $(wildcard src/linux/*.cpp)
OBJECTS := $(patsubst %.cpp, %.o, $(SOURCES) )
HEADERS := $(wildcard include/*.h) $(wildcard include/dojo/*.h) $(wildcard include/dojo/linux/*.h)

PROJDIR := $(CURDIR)

CFLAGS := -Winvalid-pch -I $(PROJDIR) -I $(PROJDIR)/include/dojo/ -I $(PROJDIR)/include/dojo/linux -I /usr/include/freetype2 -std=c++11 -D__STDC_LIMIT_MACROS

all: dojo

lib:
	@mkdir lib

stdafx.h.gch: stdafx.h $(HEADERS)
	g++ $(CFLAGS) -c stdafx.h -o $@

dojo: CFLAGS += -O3
dojo: lib stdafx.h.gch $(OBJECTS)
	ar rcs lib/lib$@.a $(OBJECTS)

dojo_d: CFLAGS += -g -D_DEBUG
dojo_d: lib stdafx.h.gch $(OBJECTS)
	ar rcs lib/lib$@.a $(OBJECTS)
debug: dojo_d

%.o: %.cpp
	g++ $(CFLAGS) -c -o $@ $^

.PHONY: clean debug pre

clean:
	rm -rf src/*.o dojo
	rm -rf stdafx.h.gch
