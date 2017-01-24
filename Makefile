bin:
	mkdir bin

entityx: bin entityx.cpp
	c++ -O3 -std=c++11 -Wall entityx.cpp -o bin/entityx $(shell pkg-config --libs entityx)

anax: bin anax.cpp
	c++ -O3 -std=c++11 -Wall anax.cpp -o bin/anax -lanax

kult: bin kult.cpp
	c++ -O3 -std=c++11 kult.cpp -o bin/kult

clean-entityx:
	rm bin/entityx

clean-anax:
	rm bin/anax

clean-kult:
	rm bin/kult

clean:
	rm -rf bin
