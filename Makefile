CC=gcc
#CFLAGS=-c -Wall 
CFLAGS=-c -Wall -O3
LDFLAGS=-lrt
SOURCES=linuxcpu.c ../common/util.c comm.c cpumon.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test

all: clear $(SOURCES) $(EXECUTABLE)

clear:
	clear
	echo ####################################################################################################
clean:
	rm -rf *o $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

