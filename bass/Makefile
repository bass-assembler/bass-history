include nall/Makefile

application := bass
flags := -std=gnu++0x -I. -DBASS_BINARY -O3 -fomit-frame-pointer
link := -s

all: $(application).o
	$(cpp) -o $(application) $(application).o $(link)
	$(call delete,*.o)

$(application).o: $(application).cpp
	$(cpp) $(flags) -c -o $(application).o $(application).cpp

install:
	sudo cp $(application) /usr/local/bin/$(application)

clean:

sync:
	if [ -d ./nall ]; then rm -r ./nall; fi
	cp -r ../nall ./nall
	rm -r nall/test
