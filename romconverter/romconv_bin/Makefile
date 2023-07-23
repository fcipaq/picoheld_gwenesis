default: romconv_bin

romconv_bin.o: main.c
	gcc -c main.c -o romconv_bin.o

romconv_bin: romconv_bin.o
	gcc romconv_bin.o -o romconv_bin

clean:
	-rm -f romconv_bin.o
	-rm -f romconv_bin
    
