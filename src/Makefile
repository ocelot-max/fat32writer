CC = gcc
RM = rm

CFLAGS = -I $(INCLUDE)

INCLUDE = ./include

OBJECTS = fs/devices.o fs/file.o test/test.o
EXEC = vdd

all: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS)
	$(MAKE) clean
.PHYON: clean
clean:
#	-$(RM) $(OBJECTS)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^