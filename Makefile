TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) --new-file -f test.db 
	./$(TARGET) -f test.db -a "Bob,123 Main ST,40"
	./$(TARGET) -f test.db -a "Joe,400 Big Lane,32"
	./$(TARGET) -f test.db -a "Steven,323 Town Circle,24"

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -g -c $< -o $@ -Iinclude