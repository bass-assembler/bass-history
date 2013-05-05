include ../nall/Makefile

application := bass
flags := $(flags) -O3 -fomit-frame-pointer -I.. -DBASS_BINARY
link := $(link) -s
objects := obj/$(application).o

all: $(objects)
	$(compiler) $(flags) -o $(application) $(objects) $(link)
	$(call delete,obj/*.o)

obj/$(application).o: $(application).cpp $(application).hpp $(call rwildcard,core/*) $(call rwildcard,arch/*)
	$(compiler) $(cppflags) $(flags) -o obj/$(application).o -c $(application).cpp

install:
	sudo cp $(application) /usr/local/bin/$(application)

clean:
	$(call delete,obj/*.o)
