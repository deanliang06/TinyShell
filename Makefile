CC = x86_64-w64-mingw32-gcc
objects = window.o memory.o display.o handleText.o


terminal.exe: $(objects) 
	$(CC) -municode $^ -o $@ -lgdi32 -lcomctl32

window.o display.o memory.o : types.h
window.o : memory.h display.h display.o handleText.o handleText.h
display.o memory.o: memory.h 
display.o: display.h
