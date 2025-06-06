all: libreria src 

libreria:
	cd src.b && make clean todo
src:
	cd src.p && make clean todo

clean:
	cd src.b && make clean
	cd src.p && make clean
	cd install && rm *
