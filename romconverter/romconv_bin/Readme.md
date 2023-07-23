# **BINARY** ROM Converter for *Gwenesis* (a *Sega Genesis* emulator) for the *Pico Held*

## Summary

The endianess of the M68K CPU is different from the one of the RP2040. Swapping all bytes in a 16 bit word makes certain reads to the ROM more efficient thus gaining approx. 10% performance.
This creates a binary file which can then be uploaded to the *Pico Held* without the need to reupload the emulator sketch.

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
Swaps ROM bytes for the Pico Held Sega Genesis emulator.

This tools creates a BINARY file.

done.
```

uploading to the *Pico Held*:

Put the *Pico Held* into *bootloader mode* (press all three buttons while turning on the device). Then use `picotool` to upload the image:

`sudo ./picotool load DisableRegTestROM_be.bin -t bin -o 0x100c3500`

Note: The ROM address `0x100c3500` can be set in the emulator sketch. The variable is called `ROM_DATA`.


