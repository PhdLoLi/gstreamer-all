# Set up basic variables:
CC = gcc
CFLAGS =  -Wall
LDFLAGS =
 
# List of sources:
SOURCES = helloworld.c
 
# Name of executable target:
EXECUTABLE = helloworld
 
# MRPT specific flags:
# Here we invoke "pkg-config" passing it as argument the list of the
# MRPT libraries needed by our program (see available libs
# with "pkg-config --list-all | grep mrpt").
#
CFLAGS += `pkg-config --cflags --libs gstreamer-1.0`
#LDFLAGS += `pkg-config --libs gstreamer-1.0`
 
 
$(EXECUTABLE):$(SOURCES)
	$(CC) $(SOURCES) -o $(EXECUTABLE) $(CFLAGS)   

lowerlevel:lowerlevel.c
	$(CC) init.c -o init $(CFLAGS)   

lowerlevel:lowerlevel.c
	$(CC) lowerlevel.c -o lowerlevel $(CFLAGS)   

playback_1.0:playback_1.0.c
	$(CC) playback_1.0 -o playback_1.0 $(CFLAGS)   

clean:
	rm  $(EXECUTABLE) lowerlevel playback_1.0
