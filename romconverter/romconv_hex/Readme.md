# **HEADER FILE* ROM Converter for *Gwenesis* (a *Sega Genesis* emulator) for the *Pico Held*

## Summary

The endianess of the M68K CPU is different from the one of the RP2040. Swapping all bytes in a 16 bit word makes certain reads to the ROM more efficient thus gaining approx. 10% performance.
This emulator creates a header file which can the be included into the sketch. **This is mainly for debugging/development purposes.** (Since you cannot change the ROM once compiled.)

## Building

Enter the directory with the source code (main.c) and compile by issuing:

`make`

Note: Depending on the linux distribution used, you may need to install some compiler and build tools.

## Usage

`./romconv_bin [input file] [output file]`

## Example

Download the `DisableRegTestROM.bin` test ROM from [here](https://drive.google.com/uc?id=1W08doQKWZPEx7xJX4KlVgBsNMS--zzU3). The whole site can be found [here](http://techdocs.exodusemulator.com/Console/SegaMegaDrive/Software.html#test-roms) (but keep in mind that not all examples are working).

Then convert the test ROM:

`./romconv_bin DisableRegTestROM.bin DisableRegTestROM_be.bin`

output:

```
Byte swapper (swaps the bytes in a 16 bit word).
Swaps ROMS bytes for the *Pico Held* Sega Genesis emulator.

This tools creates a C HEADER file.

done.
```



