CC = x86_64-w64-mingw32-gcc
all: cat.exe ls.exe mkdir.exe touch.exe

cat.exe : cat.c
	$(CC) $< -o $@

ls.exe : ls.c
	$(CC) $< -o $@

mkdir.exe : mkdir.c
	$(CC) $< -o $@

touch.exe : touch.c
	$(CC) $< -o $@

.PHONY: all