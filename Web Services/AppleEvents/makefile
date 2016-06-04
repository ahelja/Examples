# Its a makefile!

LIBS = -framework ApplicationServices -lstdc++

all: xmlrpc soap

xmlrpc: xmlrpc.cp
	cc -g -o xmlrpc xmlrpc.cp $(LIBS)

soap: soap.cp
	cc -g -o soap soap.cp $(LIBS)

clean:
	rm -f soap xmlrpc
