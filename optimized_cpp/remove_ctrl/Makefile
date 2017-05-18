SRC=main.cpp remove_ctrl.cpp

CFLAGS+=-Wall -Wextra -I../../../cybozulib/include -Ofast -march=native
TARGET=main

OBJ=main.o remove_ctrl.o
%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) ../../../mie_string/mie_string_x64.o

sample.txt:
	find /usr/src/linux-headers-`uname -r` | xargs cat > sample.txt

clean:
	rm -rf $(TARGET) $(OBJ) *.d

test: $(TARGET)
	./$(TARGET) sample.txt
