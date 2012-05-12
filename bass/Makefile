application := bass
flags := -std=gnu++0x -I. -DBASS_BINARY -O3 -fomit-frame-pointer
link := -s

all: $(application).o
	g++-4.7 -o $(application) $(application).o $(link)
	rm *.o

$(application).o: $(application).cpp
	g++-4.7 -c -o $(application).o $(application).cpp $(flags)

install:
	sudo cp $(application) /usr/local/bin/$(application)

clean:
	rm *.o

sync:
	if [ -d ./nall ]; then rm -r ./nall; fi
	cp -r ../nall ./nall
	rm -r nall/test
