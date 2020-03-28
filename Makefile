.PHONY: main
main: $(OBJ)
	gcc adjarray.c -o main

.PHONY: clean
clean:
	rm *.o