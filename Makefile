main: main.c implement.c
	clear
	gcc -o main -g -Wall -fno-diagnostics-show-caret main.c implement.c
	./main

debug: main
	gdb main

clean:
	rm -rf main
	
