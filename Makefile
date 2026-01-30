.PHONY: all

all: systemq systemqinit sysqlib.so
	@echo

systemq: ./src/systemq/main.cpp
	g++ ./src/systemq/main.cpp -O2 -static-libgcc -static-libstdc++ -Wall -o systemq

systemqinit: ./src/systemqinit/main.cpp
	g++ ./src/systemqinit/main.cpp -O0 -static -Wall -Wextra -o systemqinit

sysqlib.so: ./src/sysq-lib/lib.cpp
	g++ ./src/sysq-lib/lib.cpp -fPIC -O0 -shared -o sysqlib.so

.PHONY: clean, HelloWorld_service

clean:
	rm -f ./systemq ./systemqinit ./sysqlib.so

HelloWorld_service:
	cd ./services/HelloWorld &&./build.sh ../../HelloWorld ../../HelloWorld_service.so && cd ../..
