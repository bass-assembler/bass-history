include ../nall/Makefile

application := bass
flags := $(flags) -O3 -fomit-frame-pointer -I..
link := $(link) -s
objects := obj/bass.o

all: $(objects)
	$(compiler) -o $(application) $(objects) $(link)

obj/bass.o: bass.cpp $(call rwildcard,core/*) $(call rwildcard,arch/*)
	$(compiler) $(cppflags) $(flags) -o obj/bass.o -c bass.cpp

install:
	sudo cp $(application) /usr/local/bin/$(application)

uninstall:
	sudo rm /usr/local/bin/$(application)

clean:
	-@$(call delete,obj/*.o)
