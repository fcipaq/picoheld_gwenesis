/*
    created with FontEditor written by H. Reddmann
    HaReddmann at t-online dot de

    File Name           : f15x22.h
    Date                : 12.02.2023
    Font size in bytes  : 0x07DE, 2014
    Font width          : 15
    Font height         : 22
    Font first char     : 0x18
    Font last char      : 0x7E
    Font bits per pixel : 1
    Font is compressed  : true

    The font data are defined as

    struct _FONT_ {
     // common shared fields
       uint16_t   font_Size_in_Bytes_over_all_included_Size_it_self;
       uint8_t    font_Width_in_Pixel_for_fixed_drawing;
       uint8_t    font_Height_in_Pixel_for_all_Characters;
       uint8_t    font_Bits_per_Pixels;
                    // if MSB are set then font is a compressed font
       uint8_t    font_First_Char;
       uint8_t    font_Last_Char;
       uint8_t    font_Char_Widths[font_Last_Char - font_First_Char +1];
                    // for each character the separate width in pixels,
                    // characters < 128 have an implicit virtual right empty row
                    // characters with font_Char_Widths[] == 0 are undefined

     // if compressed font then additional fields
       uint8_t    font_Byte_Padding;
                    // each Char in the table are aligned in size to this value
       uint8_t    font_RLE_Table[3];
                    // Run Length Encoding Table for compression
       uint8_t    font_Char_Size_in_Bytes[font_Last_Char - font_First_Char +1];
                    // for each char the size in (bytes / font_Byte_Padding) are stored,
                    // this get us the table to seek to the right beginning of each char
                    // in the font_data[].

     // for compressed and uncompressed fonts
       uint8_t    font_data[];
                    // bit field of all characters
    }
*/

#include <inttypes.h>

#ifndef f15x22_H
#define f15x22_H

#define f15x22_WIDTH 15
#define f15x22_HEIGHT 22

static glcdFont_t font_15x22[] = {
    0x07, 0xDE, 0x0F, 0x16, 0x81, 0x18, 0x7E,
    0x09, 0x0B, 0x0B, 0x0A, 0x08, 0x08, 0x08, 0x09, 0x07, 0x02, 0x05, 0x0B, 0x08, 0x0E, 0x0A, 0x02, 
    0x05, 0x05, 0x06, 0x0A, 0x02, 0x05, 0x02, 0x05, 0x08, 0x05, 0x08, 0x08, 0x09, 0x08, 0x08, 0x08, 
    0x08, 0x08, 0x02, 0x02, 0x08, 0x08, 0x08, 0x08, 0x0E, 0x0B, 0x09, 0x0A, 0x0A, 0x09, 0x09, 0x0B, 
    0x0A, 0x02, 0x07, 0x0A, 0x08, 0x0D, 0x0A, 0x0B, 0x09, 0x0B, 0x0B, 0x09, 0x0A, 0x0A, 0x0B, 0x0D, 
    0x0B, 0x0C, 0x0A, 0x04, 0x05, 0x04, 0x07, 0x0A, 0x03, 0x08, 0x08, 0x07, 0x08, 0x08, 0x06, 0x08, 
    0x08, 0x02, 0x03, 0x07, 0x02, 0x0C, 0x08, 0x08, 0x08, 0x08, 0x05, 0x07, 0x05, 0x08, 0x09, 0x0D, 
    0x08, 0x09, 0x08, 0x06, 0x02, 0x06, 0x08, 
    0x01, 0x02, 0x03, 0x07, 
    0x1B, 0x1A, 0x1A, 0x15, 0x18, 0x12, 0x12, 0x14, 0x09, 0x06, 0x09, 0x1B, 0x17, 0x27, 0x18, 0x04, 
    0x0B, 0x0B, 0x0D, 0x13, 0x05, 0x0A, 0x05, 0x09, 0x11, 0x0A, 0x14, 0x14, 0x14, 0x14, 0x14, 0x12, 
    0x16, 0x14, 0x05, 0x05, 0x12, 0x12, 0x12, 0x11, 0x27, 0x18, 0x17, 0x16, 0x14, 0x19, 0x13, 0x1B, 
    0x12, 0x04, 0x0E, 0x15, 0x0F, 0x18, 0x14, 0x16, 0x14, 0x18, 0x1A, 0x1A, 0x12, 0x12, 0x15, 0x18, 
    0x18, 0x17, 0x19, 0x09, 0x0A, 0x09, 0x0E, 0x10, 0x05, 0x16, 0x11, 0x10, 0x12, 0x15, 0x0B, 0x12, 
    0x10, 0x06, 0x07, 0x10, 0x04, 0x18, 0x11, 0x11, 0x11, 0x11, 0x0B, 0x14, 0x0B, 0x11, 0x11, 0x18, 
    0x15, 0x13, 0x18, 0x0E, 0x04, 0x0E, 0x0F, 
    0x0B, 0xC1, 0x4D, 0x9F, 0x74, 0x73, 0x72, 0x08, 0xC2, 0x1C, 0x2C, 0x82, 0xF2, 0x2A, 0x10, 0x14, 
    0xBA, 0x0A, 0x04, 0x85, 0xAE, 0x02, 0x41, 0xA1, 0x43, 0x97, 0x2A, 0x5B, 0xBA, 0xC5, 0x9E, 0x72, 
    0x4A, 0xD5, 0x3B, 0x14, 0xB6, 0x3A, 0xA1, 0xEB, 0x04, 0x5B, 0x1D, 0xAA, 0x77, 0xA8, 0x74, 0x6E, 
    0x82, 0xBD, 0x49, 0x15, 0x00, 0xFB, 0xA4, 0x9B, 0x93, 0x67, 0xA5, 0xF0, 0x62, 0xA1, 0xE8, 0x84, 
    0xAE, 0x13, 0xBA, 0x4E, 0xA8, 0x50, 0x78, 0xB1, 0xD0, 0x78, 0x9E, 0x6E, 0x4E, 0xFA, 0x00, 0xBA, 
    0x3B, 0xFD, 0x4A, 0x27, 0x59, 0xE8, 0xD6, 0x4D, 0xEA, 0x26, 0x55, 0xE8, 0x56, 0xA1, 0x93, 0x3A, 
    0xFC, 0xC3, 0xDD, 0x01, 0x0B, 0x65, 0x2E, 0x4D, 0x63, 0x0F, 0x45, 0x53, 0x75, 0x42, 0x53, 0x75, 
    0x42, 0x53, 0x71, 0x29, 0x8A, 0xAA, 0x4B, 0x71, 0x73, 0xCA, 0x2D, 0x00, 0xCB, 0xA7, 0xE6, 0x5D, 
    0x0A, 0x2B, 0x9E, 0xD0, 0x75, 0x42, 0xD7, 0xA1, 0xB0, 0xE2, 0xA1, 0x79, 0xB7, 0x4B, 0xBB, 0x97, 
    0xE2, 0xE6, 0xD0, 0xC5, 0x9B, 0xD4, 0x4D, 0xEA, 0xD0, 0xA5, 0x4B, 0x71, 0x73, 0xC2, 0x4D, 0x01, 
    0xCA, 0xED, 0xE1, 0x9F, 0x37, 0xA5, 0x13, 0x97, 0xCA, 0xF2, 0xF0, 0x0A, 0x1D, 0x4C, 0x55, 0xDD, 
    0x62, 0x6F, 0x32, 0x15, 0xDB, 0xB6, 0x6D, 0xDB, 0xB6, 0x6D, 0xDB, 0xB6, 0x01, 0xBA, 0x99, 0xEE, 
    0x66, 0x2A, 0x00, 0x72, 0x37, 0xB9, 0xDB, 0x36, 0xC1, 0xDD, 0xE4, 0x6E, 0xAB, 0xBA, 0xA1, 0x2A, 
    0x3C, 0xA1, 0x38, 0xA7, 0x7B, 0x39, 0xD1, 0x09, 0x46, 0xD5, 0x0D, 0x55, 0xE1, 0x09, 0xC5, 0x39, 
    0xDD, 0xCB, 0x89, 0x4E, 0x30, 0xAA, 0x0E, 0x8A, 0xA9, 0x74, 0xB2, 0x05, 0x0F, 0x8E, 0x05, 0xAF, 
    0x42, 0x95, 0x4A, 0xBF, 0xA9, 0x54, 0xA9, 0xD0, 0xC1, 0xFA, 0x54, 0xC1, 0x95, 0x00, 0x8A, 0xB9, 
    0x0E, 0xB6, 0xE2, 0x55, 0xA8, 0x3C, 0x54, 0x68, 0xBC, 0x54, 0x28, 0x3C, 0xC1, 0x86, 0x27, 0x99, 
    0xF0, 0x26, 0x18, 0xE6, 0x24, 0xC3, 0x9E, 0x60, 0x54, 0xE8, 0xE2, 0x54, 0xE8, 0x60, 0x55, 0xE8, 
    0xAC, 0xD8, 0x43, 0x67, 0x2A, 0x53, 0xCC, 0x4D, 0xF6, 0xE4, 0x54, 0xDE, 0xA9, 0x74, 0x95, 0x95, 
    0xAE, 0x32, 0x85, 0xCE, 0x8E, 0xD1, 0xC5, 0x94, 0xBB, 0xA9, 0xF0, 0x36, 0x2A, 0x72, 0x37, 0xB9, 
    0x1B, 0xCB, 0xA7, 0xDC, 0x9E, 0x3B, 0x57, 0xF0, 0x24, 0x8B, 0x9B, 0x08, 0xE2, 0x26, 0xCA, 0x93, 
    0x2C, 0xB8, 0x73, 0x77, 0x7B, 0xCA, 0x25, 0x42, 0xB7, 0x11, 0xDD, 0xE2, 0x6E, 0x72, 0x37, 0x85, 
    0xE8, 0xA6, 0x6E, 0x01, 0x13, 0xBA, 0x49, 0xDD, 0xA4, 0x6E, 0x52, 0x37, 0x9D, 0xDB, 0xB9, 0xA9, 
    0x9B, 0xD4, 0x4D, 0xEA, 0x26, 0x75, 0x01, 0x5B, 0x1A, 0x6E, 0x32, 0x03, 0x13, 0xBA, 0x49, 0xDD, 
    0xA4, 0x6E, 0x52, 0x37, 0xA9, 0x0B, 0x5B, 0xBA, 0x49, 0x15, 0x00, 0x93, 0xDC, 0xED, 0x13, 0x3E, 
    0xE5, 0x1B, 0xBC, 0x05, 0xD2, 0x3B, 0xE1, 0xF6, 0xE0, 0xC5, 0xEB, 0x84, 0xAE, 0x13, 0x3A, 0x2F, 
    0x1E, 0x6E, 0x4F, 0x78, 0x07, 0xEB, 0xA6, 0x74, 0x53, 0xBA, 0x09, 0x3F, 0xF7, 0x53, 0x00, 0x4A, 
    0x97, 0x0E, 0x9E, 0x39, 0xAF, 0xE8, 0xBA, 0xA6, 0xAB, 0x54, 0x75, 0x96, 0x85, 0x0E, 0x57, 0xBA, 
    0x98, 0xAB, 0x00, 0x4A, 0xD7, 0xC9, 0xF3, 0xE0, 0xC5, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0x73, 
    0x4C, 0x79, 0x70, 0xF1, 0xC9, 0x72, 0x25, 0x13, 0xBC, 0xC9, 0xDD, 0x60, 0xA2, 0x93, 0x4C, 0x75, 
    0x82, 0xA9, 0x74, 0xC2, 0xCF, 0xFD, 0xDC, 0xD2, 0x4D, 0xEA, 0x00, 0x33, 0xD5, 0xE5, 0x94, 0x07, 
    0x13, 0x55, 0xBC, 0xAA, 0x52, 0x57, 0x55, 0xEA, 0x2A, 0x0B, 0x5E, 0x85, 0x73, 0x83, 0x3B, 0xD2, 
    0x3B, 0xE1, 0xF6, 0x52, 0x55, 0xF0, 0xAA, 0x4A, 0x5D, 0x55, 0xA9, 0x73, 0x2C, 0x78, 0x70, 0xCE, 
    0xA9, 0xB2, 0x25, 0xEA, 0x26, 0x75, 0x93, 0xBA, 0x98, 0xAB, 0x72, 0xAE, 0xB2, 0x27, 0x14, 0xE6, 
    0x06, 0x73, 0x13, 0xBA, 0x09, 0x8A, 0x19, 0x77, 0xBA, 0x3D, 0x38, 0xA6, 0xBC, 0x0A, 0x55, 0xBA, 
    0x0A, 0x55, 0x3A, 0xC7, 0x94, 0x87, 0xDB, 0x93, 0x19, 0x57, 0x02, 0x8A, 0xAD, 0x4E, 0x67, 0x3C, 
    0x58, 0x70, 0xBC, 0x4A, 0x55, 0x57, 0xA9, 0xEA, 0x2C, 0x54, 0x5D, 0x6E, 0x4F, 0x38, 0x07, 0xEB, 
    0x3A, 0xA1, 0xAB, 0x00, 0xEB, 0x1A, 0x0E, 0x9D, 0x19, 0x8B, 0x37, 0xC5, 0x9B, 0x50, 0x74, 0x53, 
    0xD1, 0x2D, 0x55, 0xB7, 0x2A, 0x9D, 0x54, 0xA5, 0x53, 0xBA, 0x4A, 0xAB, 0xBA, 0xA1, 0xEA, 0x86, 
    0xAA, 0x1B, 0xAA, 0x6E, 0xA8, 0xBA, 0xA1, 0xEA, 0x86, 0xAA, 0x1B, 0xAA, 0x0E, 0x4A, 0xD7, 0x29, 
    0x55, 0x3A, 0xA9, 0x4A, 0xB7, 0xAA, 0x5B, 0x8A, 0x6E, 0x2A, 0xBA, 0x09, 0xDE, 0x14, 0x4F, 0x4A, 
    0x37, 0xC5, 0x9B, 0xE0, 0x4D, 0xE9, 0x8C, 0xAE, 0x8A, 0x8B, 0xCE, 0xF2, 0xF6, 0x2D, 0xE6, 0x24, 
    0xCB, 0x39, 0xE9, 0xE6, 0x64, 0xC9, 0x1C, 0xBC, 0x78, 0x95, 0x29, 0x54, 0xAA, 0x1E, 0x2B, 0x55, 
    0x63, 0x55, 0xA9, 0x0A, 0x53, 0x55, 0xAA, 0xAB, 0x52, 0xE3, 0x34, 0x96, 0xAC, 0x38, 0x1D, 0x2C, 
    0xA7, 0xD3, 0xD3, 0x49, 0x16, 0x28, 0x00, 0x5B, 0xBA, 0xC5, 0x9E, 0x72, 0x4E, 0x78, 0x27, 0xD8, 
    0xEA, 0x84, 0xAE, 0x13, 0x6C, 0x75, 0xD2, 0xBB, 0xE5, 0xDC, 0x04, 0x7B, 0x93, 0x2A, 0x00, 0xFA, 
    0xB9, 0x9F, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0x73, 0x4C, 
    0x79, 0x70, 0xF1, 0xC9, 0x72, 0x25, 0xFB, 0xA4, 0x9B, 0x93, 0xE7, 0xC1, 0x8B, 0xD7, 0x09, 0x5D, 
    0x27, 0x74, 0x9D, 0x3C, 0x78, 0x5E, 0x4C, 0xC1, 0x9C, 0x52, 0xA1, 0x03, 0xFA, 0xB9, 0x9F, 0xEB, 
    0x84, 0xAE, 0x13, 0xBA, 0x4E, 0xE8, 0x3A, 0xA1, 0xF3, 0xE2, 0xC1, 0xF3, 0x74, 0x73, 0xD2, 0x07, 
    0xFA, 0xB9, 0x9F, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0xAB, 
    0x50, 0xA5, 0xAB, 0x50, 0xA5, 0xAB, 0x50, 0xA5, 0x02, 0xFA, 0xB9, 0x9F, 0xAB, 0xD0, 0xAD, 0x42, 
    0xB7, 0x0A, 0xDD, 0x2A, 0x74, 0xAB, 0xD0, 0xAD, 0x42, 0xB7, 0x6E, 0x02, 0xFB, 0xA4, 0x9B, 0x93, 
    0xE7, 0xA5, 0x4B, 0x87, 0x4E, 0xE8, 0x3A, 0xA1, 0xAB, 0x52, 0xA1, 0xAB, 0x52, 0xA1, 0x43, 0x85, 
    0xAA, 0x8B, 0x99, 0x4F, 0x68, 0x6C, 0x09, 0xFA, 0xB9, 0x9F, 0x93, 0xBA, 0x49, 0xDD, 0xA4, 0x6E, 
    0x52, 0x37, 0xA9, 0x9B, 0xD4, 0xED, 0xE7, 0x7E, 0x0A, 0xFA, 0xB9, 0x9F, 0x02, 0x93, 0xBC, 0x29, 
    0xE6, 0x26, 0x79, 0x93, 0xBA, 0x49, 0xDD, 0x3F, 0xDC, 0x96, 0x00, 0xFA, 0xB9, 0x9F, 0x5B, 0x37, 
    0xC1, 0x9B, 0xE0, 0x6E, 0x69, 0xCC, 0x49, 0x55, 0x3C, 0x79, 0xF0, 0xD0, 0xC9, 0xDB, 0x28, 0x00, 
    0xFA, 0xB9, 0x9F, 0x9B, 0xD4, 0x4D, 0xEA, 0x26, 0x75, 0x93, 0xBA, 0x49, 0xDD, 0xA4, 0x0A, 0xFA, 
    0xB9, 0x9F, 0xB3, 0x37, 0xE1, 0x9B, 0xF2, 0x4D, 0xB0, 0x37, 0xA9, 0x5B, 0xEC, 0x49, 0x9F, 0xF2, 
    0x29, 0xF6, 0x96, 0x9F, 0xFB, 0x29, 0x00, 0xFA, 0xB9, 0x9F, 0x83, 0xB9, 0x49, 0xE6, 0x26, 0x99, 
    0x9B, 0x62, 0x6E, 0x92, 0xB9, 0x49, 0xE6, 0xF0, 0x73, 0x3F, 0x05, 0xFB, 0xA4, 0x9B, 0x93, 0xE7, 
    0xC1, 0x8B, 0xD7, 0x09, 0x5D, 0x27, 0x74, 0x9D, 0xD0, 0x79, 0xF1, 0xE0, 0x79, 0xBA, 0x39, 0xE9, 
    0x03, 0xFA, 0xB9, 0x9F, 0xAB, 0xD2, 0x49, 0x55, 0x3A, 0xA9, 0x4A, 0x27, 0x55, 0xE9, 0x24, 0xCB, 
    0xDB, 0xB7, 0xB8, 0x53, 0x00, 0xFB, 0xA4, 0x9B, 0x93, 0xE7, 0xC1, 0x8B, 0xD7, 0x09, 0x5D, 0x27, 
    0x74, 0x5D, 0x88, 0xCE, 0x4B, 0x17, 0xCF, 0xD3, 0x8D, 0x4E, 0xB0, 0x85, 0x0A, 0xFA, 0xB9, 0x9F, 
    0xAB, 0xD0, 0xAD, 0x42, 0xB7, 0x0A, 0xDD, 0x2A, 0x78, 0x52, 0x05, 0xA2, 0x53, 0x9C, 0xC6, 0x13, 
    0x6C, 0x99, 0x93, 0x29, 0x79, 0x1B, 0x05, 0x8A, 0xA9, 0x74, 0xB2, 0x05, 0x0F, 0x4E, 0x15, 0xAF, 
    0x42, 0x95, 0xAE, 0x42, 0x95, 0xAE, 0x4A, 0x85, 0xCE, 0x82, 0xE3, 0xC1, 0x82, 0x3D, 0x55, 0x31, 
    0x25, 0xEA, 0x26, 0x75, 0x93, 0xBA, 0x49, 0xDD, 0xA4, 0x9F, 0xFB, 0xB9, 0x6E, 0x52, 0x37, 0xA9, 
    0x9B, 0xD4, 0x4D, 0xBA, 0x3B, 0xFD, 0x9B, 0xE4, 0x4D, 0xEA, 0x26, 0x75, 0x93, 0xBA, 0x49, 0xDD, 
    0x94, 0x0E, 0xFF, 0x70, 0x77, 0xEA, 0x26, 0xD9, 0x9B, 0xF0, 0x4D, 0xF9, 0x26, 0xD8, 0x9B, 0xD4, 
    0x2D, 0xF6, 0xA4, 0x4F, 0xF8, 0x24, 0x7B, 0x4B, 0x37, 0x01, 0x3A, 0xB7, 0xDB, 0x9B, 0xBE, 0x49, 
    0x9D, 0xEE, 0xEE, 0xEE, 0xD4, 0x4D, 0xBA, 0x3B, 0xE9, 0xEE, 0x26, 0x75, 0xC3, 0x77, 0x7B, 0x39, 
    0x27, 0x00, 0xE2, 0x24, 0xCE, 0x8B, 0x07, 0xCF, 0x93, 0x29, 0x73, 0xD2, 0x37, 0x79, 0xD3, 0x27, 
    0x99, 0x32, 0x27, 0xCF, 0x43, 0x27, 0x8F, 0x93, 0x28, 0x00, 0xE2, 0x36, 0x6F, 0x92, 0x37, 0xC9, 
    0xDC, 0x24, 0x73, 0x93, 0xDE, 0x49, 0xEF, 0x04, 0x73, 0x93, 0x37, 0x79, 0x13, 0xBC, 0x29, 0xDC, 
    0x04, 0xEA, 0xE4, 0x75, 0x31, 0xD7, 0x19, 0x5D, 0x25, 0xA7, 0xAB, 0x62, 0x75, 0x95, 0x95, 0xAE, 
    0xB1, 0xD4, 0x15, 0x1D, 0x3A, 0x73, 0xE9, 0x3C, 0x55, 0x00, 0xFA, 0x5D, 0xFD, 0xAE, 0xBA, 0xA1, 
    0xEA, 0x86, 0x02, 0xF2, 0xA6, 0x9C, 0x5B, 0x6E, 0x6E, 0x39, 0x37, 0xC5, 0x02, 0xEA, 0x86, 0xAA, 
    0x1B, 0xAA, 0xDF, 0xD5, 0xEF, 0x02, 0x4B, 0xB7, 0xD8, 0x9B, 0xBD, 0xA5, 0x9B, 0x64, 0x6F, 0x82, 
    0xBD, 0x49, 0x9D, 0x00, 0x9B, 0xC4, 0x6D, 0xDC, 0xC6, 0x6D, 0xDC, 0xC6, 0x6D, 0xDC, 0xC6, 0x6D, 
    0xDC, 0xC6, 0x6D, 0x04, 0xE2, 0xB6, 0x6E, 0xEB, 0x16, 0x0B, 0x65, 0x4E, 0x6A, 0xEC, 0x09, 0x4D, 
    0xD5, 0x09, 0x4D, 0xD5, 0x09, 0x4D, 0xC5, 0x29, 0x45, 0xD5, 0x29, 0x37, 0xA7, 0xDC, 0x02, 0xFA, 
    0xB9, 0x9F, 0x53, 0xAA, 0x74, 0x4A, 0xD7, 0x09, 0x5D, 0x27, 0x58, 0xF1, 0x94, 0x77, 0xB3, 0x07, 
    0xCB, 0xB7, 0x77, 0x8A, 0x15, 0x4F, 0xE8, 0x3A, 0xA1, 0xEB, 0x04, 0x0B, 0xDE, 0x28, 0x54, 0x02, 
    0x8B, 0xBD, 0xE1, 0x9D, 0x62, 0xC5, 0x13, 0xBA, 0x4E, 0xE8, 0x3A, 0xA5, 0x4A, 0x87, 0x9F, 0xFB, 
    0x29, 0x00, 0x93, 0xBD, 0xBD, 0x53, 0x8C, 0xC6, 0x13, 0x9A, 0xAA, 0x13, 0x9A, 0xAA, 0x13, 0x8C, 
    0xAA, 0x53, 0xDC, 0x74, 0x33, 0x43, 0x09, 0xEB, 0xA6, 0x7F, 0x3F, 0xD7, 0x74, 0x4B, 0xD3, 0x2D, 
    0xDD, 0x04, 0x93, 0xAD, 0x2E, 0x6F, 0x3C, 0x2B, 0x4E, 0xD7, 0x35, 0x5D, 0xD7, 0x74, 0xA8, 0xD2, 
    0x78, 0xFF, 0x70, 0x3B, 0xFA, 0xB9, 0x9F, 0x53, 0xBA, 0x29, 0xDD, 0xA4, 0x6E, 0x92, 0x37, 0xE5, 
    0xE6, 0x94, 0x5B, 0x00, 0x6A, 0x6E, 0xAE, 0xB9, 0x29, 0x00, 0x9B, 0x54, 0x35, 0x3F, 0xD5, 0xFC, 
    0x00, 0xFA, 0xB9, 0x9F, 0x1B, 0xBA, 0x09, 0xEE, 0x96, 0xC6, 0x9C, 0x54, 0xC5, 0x9C, 0xC0, 0xA1, 
    0x02, 0xFA, 0xB9, 0x9F, 0x02, 0xBB, 0x39, 0xE1, 0xE6, 0x94, 0x6E, 0x4A, 0x37, 0xA9, 0x9B, 0x74, 
    0x73, 0xD2, 0x3B, 0xA5, 0x9B, 0xD2, 0x4D, 0xEA, 0x26, 0xDD, 0x9C, 0x72, 0x0B, 0xBB, 0x39, 0xE1, 
    0xE6, 0x94, 0x6E, 0x4A, 0x37, 0xA9, 0x9B, 0xE4, 0x4D, 0xB9, 0x39, 0xE5, 0x16, 0x00, 0xCB, 0xB7, 
    0x77, 0x8A, 0x15, 0x4F, 0xE8, 0x3A, 0xA1, 0xEB, 0x04, 0x2B, 0x9E, 0xF2, 0x6E, 0x97, 0x00, 0xFB, 
    0xB9, 0x9F, 0x43, 0x95, 0x4E, 0xE9, 0x3A, 0xA1, 0xEB, 0x04, 0x2B, 0x9E, 0xF2, 0x6E, 0xF6, 0x00, 
    0x8B, 0xBD, 0xE1, 0x9D, 0x62, 0xC5, 0x13, 0xBA, 0x4E, 0xE8, 0x3A, 0xA5, 0x4A, 0xA7, 0xFC, 0xDC, 
    0x4F, 0xBB, 0x39, 0xE1, 0xE6, 0x14, 0x6E, 0x52, 0x37, 0xA9, 0x93, 0x00, 0x4B, 0x85, 0x4E, 0x71, 
    0xE5, 0x09, 0x4D, 0xD5, 0x09, 0x4D, 0xD5, 0x09, 0x4D, 0xD5, 0x29, 0x8D, 0xBB, 0x51, 0x96, 0x00, 
    0xEB, 0xA6, 0xDB, 0xC3, 0xCF, 0x09, 0x5D, 0x27, 0x74, 0x15, 0x00, 0xBB, 0xA7, 0xDC, 0xDC, 0x14, 
    0x6F, 0x52, 0x37, 0xA9, 0x9B, 0xD2, 0x29, 0x37, 0x27, 0xDC, 0x14, 0x00, 0xEB, 0x26, 0xD9, 0x9B, 
    0xF0, 0x4D, 0xB0, 0x37, 0xA9, 0x5B, 0xEC, 0x49, 0x9F, 0x64, 0x6F, 0xE9, 0x24, 0xF3, 0xA6, 0xBC, 
    0x5B, 0xCE, 0x4D, 0xEA, 0x76, 0x4E, 0x38, 0x37, 0x6F, 0xCA, 0xBB, 0xE5, 0xDC, 0xA4, 0x6E, 0xE7, 
    0x84, 0x77, 0x92, 0xA7, 0x00, 0xE3, 0xC2, 0x09, 0x56, 0x3C, 0xC5, 0x84, 0xB9, 0xC1, 0xDD, 0xE4, 
    0x6E, 0x30, 0x61, 0x4E, 0xB1, 0xE2, 0x09, 0x5C, 0x28, 0x00, 0xE3, 0x36, 0x73, 0xE9, 0x60, 0x4B, 
    0x9D, 0x70, 0x6F, 0xB0, 0x27, 0xD9, 0x93, 0xEC, 0x0D, 0xE6, 0x26, 0x70, 0x03, 0xAB, 0xE4, 0x09, 
    0x55, 0xCC, 0x09, 0x15, 0x8A, 0x4E, 0xA8, 0x9A, 0x4E, 0x68, 0xAA, 0x4E, 0x28, 0x2A, 0x74, 0x82, 
    0xA9, 0x74, 0x82, 0xA5, 0x0A, 0x13, 0xBA, 0x29, 0xE6, 0x94, 0xE3, 0x15, 0xAE, 0x57, 0xDD, 0x50, 
    0x75, 0x43, 0x01, 0xFA, 0x5D, 0xFD, 0x2E, 0xEA, 0x86, 0xAA, 0x1B, 0xAA, 0xEB, 0x15, 0x8E, 0x77, 
    0x82, 0xB9, 0x29, 0x5D, 0x00, 0x53, 0x37, 0xA5, 0x9B, 0xD4, 0x4D, 0xEA, 0xB6, 0x6E, 0x52, 0x37, 
    0xA9, 0x9B, 0xD2, 0x09, 0x00
};

#endif

