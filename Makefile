.PHONY: tina cgi-src install clean

all: tina cgi-src

tina:
	(cd src && make)
	
cgi-src:
	(cd cgi-src && make)

install:
	(cd src && make install)
	(cd conf && make install)

clean:
	(cd src && make clean)
	(cd cgi-src && make clean)
