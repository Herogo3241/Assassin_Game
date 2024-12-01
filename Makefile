build: 
	gcc main.c -lncurses -lm -o main
run: build
	./main

clean:
	rm -rf main