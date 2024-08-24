all : test_cmod.exe

test_cmod.exe : cmod.c test.c
	gcc -Wall -g -o $@ $^

clean :
	rm -f test_cmod.exe
