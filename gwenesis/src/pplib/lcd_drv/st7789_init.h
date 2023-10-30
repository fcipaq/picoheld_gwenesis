void glcdSetAddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {

  uint8_t coordinates[8] = {
    (uint8_t) (x1 >> 8),
    (uint8_t) (x1 & 0xff),
    (uint8_t) (x2 >> 8),
    (uint8_t) (x2 & 0xff),
    (uint8_t) (y1 >> 8),
    (uint8_t) (y1 & 0xff),
    (uint8_t) (y2 >> 8),
    (uint8_t) (y2 & 0xff)
  };
  
  glcdSendCmdByte(ST7789_CASET); // column address set
  for (int i = 0; i < 4; i++)
    glcdSendDatByte(coordinates[i]);

    
  glcdSendCmdByte(ST7789_PASET);  // page address set
  for (int i = 0; i < 4; i++)
    glcdSendDatByte(coordinates[i + 4]);

  glcdSendCmdByte(ST7789_RAMWR); //memory write 

}

void lcdControllerInit() {
  set_rst(HIGH);
  delay(20);
  set_rst(LOW);
  delay(20);
  set_rst(HIGH);

  delay(ST7789_SLPOUT_DELAY);
  
    glcdSendCmdByte(ST7789_SLPOUT); // 2: Out of sleep mode, no args, w/delay

    delay(ST7789_SLPOUT_DELAY);

    glcdSendCmdByte(ST7789_COLMOD);
	glcdSendDatByte(0x55); // 3: Set color mode, 16-bit color
    glcdSendCmdByte(0x36);
	glcdSendDatByte(0x00);

    glcdSendCmdByte(0xb0);
    glcdSendDatByte(0x00);
	glcdSendDatByte(0xC0);

    glcdSendCmdByte(0xb2);
    glcdSendDatByte(0x0C);
	glcdSendDatByte(0x0C);
	glcdSendDatByte(0x00);
	glcdSendDatByte(0x33);
	glcdSendDatByte(0x33);

    glcdSendCmdByte(0xb7);
	glcdSendDatByte(0x35);
    glcdSendCmdByte(0xbb);
	glcdSendDatByte(0x19);
    glcdSendCmdByte(0xC0);
	glcdSendDatByte(0x2C);
    glcdSendCmdByte(0xC2);
	glcdSendDatByte(0x01);
    glcdSendCmdByte(0xC3);
	glcdSendDatByte(0x12);
    glcdSendCmdByte(0xC4);
	glcdSendDatByte(0x20);
    glcdSendCmdByte(0xC6);
	glcdSendDatByte(0x0F);

    glcdSendCmdByte(0xD0);
	glcdSendDatByte(0xA4);
	glcdSendDatByte(0xA1);

    glcdSendCmdByte(0xe0);
    glcdSendDatByte(0b11110000); // V63P3, V63P2, V63P1, V63P0,  V0P3,  V0P2,  V0P1,  V0P0
    glcdSendDatByte(0b00001001); //     0,     0,  V1P5,  V1P4,  V1P3,  V1P2,  V1P1,  V1P0
    glcdSendDatByte(0b00010011); //     0,     0,  V2P5,  V2P4,  V2P3,  V2P2,  V2P1,  V2P0
    glcdSendDatByte(0b00010010); //     0,     0,     0,  V4P4,  V4P3,  V4P2,  V4P1,  V4P0
    glcdSendDatByte(0b00010010); //     0,     0,     0,  V6P4,  V6P3,  V6P2,  V6P1,  V6P0
    glcdSendDatByte(0b00101011); //     0,     0,  J0P1,  J0P0, V13P3, V13P2, V13P1, V13P0
    glcdSendDatByte(0b00111100); //     0, V20P6, V20P5, V20P4, V20P3, V20P2, V20P1, V20P0
    glcdSendDatByte(0b01000100); //     0, V36P2, V36P1, V36P0,     0, V27P2, V27P1, V27P0
    glcdSendDatByte(0b01001011); //     0, V43P6, V43P5, V43P4, V43P3, V43P2, V43P1, V43P0
    glcdSendDatByte(0b00011011); //     0,     0,  J1P1,  J1P0, V50P3, V50P2, V50P1, V50P0
    glcdSendDatByte(0b00011000); //     0,     0,     0, V57P4, V57P3, V57P2, V57P1, V57P0
    glcdSendDatByte(0b00010111); //     0,     0,     0, V59P4, V59P3, V59P2, V59P1, V59P0
    glcdSendDatByte(0b00011101); //     0,     0, V61P5, V61P4, V61P3, V61P2, V61P1, V61P0
    glcdSendDatByte(0b00100001); //     0,     0, V62P5, V62P4, V62P3, V62P2, V62P1, V62P0

    glcdSendCmdByte(0xe1);
    glcdSendDatByte(0b11110000); // V63P3, V63P2, V63P1, V63P0,  V0P3,  V0P2,  V0P1,  V0P0
    glcdSendDatByte(0b00001001); //     0,     0,  V1P5,  V1P4,  V1P3,  V1P2,  V1P1,  V1P0
    glcdSendDatByte(0b00010011); //     0,     0,  V2P5,  V2P4,  V2P3,  V2P2,  V2P1,  V2P0
    glcdSendDatByte(0b00001100); //     0,     0,     0,  V4N4,  V4N3,  V4N2,  V4N1,  V4N0
    glcdSendDatByte(0b00001101); //     0,     0,     0,  V6N4,  V6N3,  V6N2,  V6N1,  V6N0
    glcdSendDatByte(0b00100111); //     0,     0,  J0N1,  J0N0, V13N3, V13N2, V13N1, V13N0
    glcdSendDatByte(0b00111011); //     0, V20N6, V20N5, V20N4, V20N3, V20N2, V20N1, V20N0
    glcdSendDatByte(0b01000100); //     0, V36N2, V36N1, V36N0,     0, V27N2, V27N1, V27N0
    glcdSendDatByte(0b01001101); //     0, V43N6, V43N5, V43N4, V43N3, V43N2, V43N1, V43N0
    glcdSendDatByte(0b00001011); //     0,     0,  J1N1,  J1N0, V50N3, V50N2, V50N1, V50N0
    glcdSendDatByte(0b00010111); //     0,     0,     0, V57N4, V57N3, V57N2, V57N1, V57N0
    glcdSendDatByte(0b00010111); //     0,     0,     0, V59N4, V59N3, V59N2, V59N1, V59N0
    glcdSendDatByte(0b00011101); //     0,     0, V61N5, V61N4, V61N3, V61N2, V61N1, V61N0
    glcdSendDatByte(0b00100001); //     0,     0, V62N5, V62N4, V62N3, V62N2, V62N1, V62N0
  
  
  glcdSendCmdByte(ST7789_INVON); // IPS display - invert colors
  
  glcdSendCmdByte(ST7789_MADCTL);    // Memory Access Control

  #if LCD_ROTATION==0
    glcdSendDatByte(ST7789_MADCTL_RGB); // Rotation 0 (portrait mode)
 	glcdSetAddr(0, 0, PHYS_SCREEN_WIDTH - 1, PHYS_SCREEN_HEIGHT - 1);
  #elif LCD_ROTATION==1
    glcdSendDatByte(ST7789_MADCTL_MV | ST7789_MADCTL_MY | ST7789_MADCTL_RGB); // Rotation 90 (landscape mode)
	glcdSetAddr(0, 0, PHYS_SCREEN_HEIGHT - 1, PHYS_SCREEN_WIDTH - 1);
  #elif LCD_ROTATION==2
    // TODO
	glcdSendDatByte(ST7789_MADCTL_MY | ST7789_MADCTL_RGB); // Rotation 180 (portrait mode)
 	glcdSetAddr(0, 0, PHYS_SCREEN_WIDTH - 1, PHYS_SCREEN_HEIGHT - 1);
  #elif LCD_ROTATION==3
    // TODO
    glcdSendDatByte(ST7789_MADCTL_MV | ST7789_MADCTL_RGB); // Rotation 270 (landscape mode)
	glcdSetAddr(0, 0, PHYS_SCREEN_HEIGHT - 1, PHYS_SCREEN_HEIGHT - 1);
  #endif
  
  // clear screen
  for (uint32_t i = 0; i < PHYS_SCREEN_HEIGHT * PHYS_SCREEN_HEIGHT * 2; i++)
    glcdSendDatByte(0x00);

  glcdSendCmdByte(ST7789_NORON); // 4: Normal display on, no args, w/delay
  glcdSendCmdByte(ST7789_DISPON);    //Display on
  glcdSendCmdByte(ST7789_RAMWR); //memory write 

  // WA for a software bug in the PIO engine
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x00);
       
}
