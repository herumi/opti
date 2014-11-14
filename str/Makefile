
TARGET=mie_findstr_test

CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wwrite-strings -Wfloat-equal -Wpointer-arith
CFLAGS+=$(CFLAGS_WARN) -msse4 -O3 -DNDEBUG

all:$(TARGET)

.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)
.s.o:
	$(CC) -c $< -o $@

clean:
	$(RM) *.o $(TARGET)

mie_findstr_test: mie_findstr_test.o mie_findstr.o
	$(CC) -o $@ $(LDFLAGS) mie_findstr_test.o mie_findstr.o

mie_findstr.o: mie_findstr.s
mie_findstr_test.o: mie_findstr_test.c mie_findstr.h


