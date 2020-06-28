CC        = gcc 
CFLAGS    = -Wall -O -g
CXXFLAGS  = 
INCLUDE   = -I ./include
TARGET    = libhello.a
LIBPATH   = ./build/lib/
 
vpath %.h ./include

OBJS      = hello.o
SRCS      = ./src/hello.c

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $^

all:$(OBJS)
	ar rcs $(TARGET) $^
	mv $(TARGET) $(LIBPATH)

install:
	cp $(LIBPATH)/$(TARGET) $(DESTDIR)


clean:
	rm -f *.o
	rm -f $(LIBPATH)*
