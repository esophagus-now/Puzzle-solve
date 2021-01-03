main: main.c
	clear
	gcc -o main -g -Wall -fno-diagnostics-show-caret main.c
	./main

debug:
	gcc -g -o main main.c
	gdb main

clean:
	rm -rf main
	
