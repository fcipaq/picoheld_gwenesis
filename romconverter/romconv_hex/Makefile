default: romconv_hex

romconv_hex.o: main.c
	gcc -c main.c -o romconv_hex.o

romconv_hex: romconv_hex.o
	gcc romconv_hex.o -o romconv_hex

clean:
	-rm -f romconv_hex.o
	-rm -f romconv_hex
    
