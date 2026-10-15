CC = x86_64-w64-mingw32-gcc
objects = window.o memory.o display.o


terminal: $(objects) 
	$(CC) -municode $^ -o terminal -lgdi32

window.o display.o memory.o : types.h
window.o : memory.h display.h display.o
display.o memory.o: memory.h 
display.o: display.h
