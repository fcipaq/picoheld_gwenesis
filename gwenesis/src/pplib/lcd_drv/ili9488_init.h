void glcdSetAddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
 
  uint8_t coordinates[8] = {
    x1 >> 8,
    x1 & 0xff,
    x2 >> 8,
    x2 & 0xff,
    y1 >> 8,
    y1 & 0xff,
    y2 >> 8,
    y2 & 0xff
  };
  
  glcdSendCmdByte(TFT_CASET); // column address set
  for (int i = 0; i < 4; i++)
    glcdSendDatByte(coordinates[i]);

    
  glcdSendCmdByte(TFT_PASET);  // page address set
  for (int i = 0; i < 4; i++)
    glcdSendDatByte(coordinates[i + 4]);

  glcdSendCmdByte(TFT_RAMWR); //memory write 
}

void lcdControllerInit() {
  set_rst(HIGH);
  delay(20);
  set_rst(LOW);
  delay(20);
  set_rst(HIGH);

  delay(50);

// taken from TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI/
// This is the command sequence that initialises the ILI9488 driver


// Configure ILI9488 display

    glcdSendCmdByte(0xE0); // Positive Gamma Control
    glcdSendDatByte(0x00);
    glcdSendDatByte(0x03);
    glcdSendDatByte(0x09);
    glcdSendDatByte(0x08);
    glcdSendDatByte(0x16);
    glcdSendDatByte(0x0A);
    glcdSendDatByte(0x3F);
    glcdSendDatByte(0x78);
    glcdSendDatByte(0x4C);
    glcdSendDatByte(0x09);
    glcdSendDatByte(0x0A);
    glcdSendDatByte(0x08);
    glcdSendDatByte(0x16);
    glcdSendDatByte(0x1A);
    glcdSendDatByte(0x0F);

    glcdSendCmdByte(0XE1); // Negative Gamma Control
    glcdSendDatByte(0x00);
    glcdSendDatByte(0x16);
    glcdSendDatByte(0x19);
    glcdSendDatByte(0x03);
    glcdSendDatByte(0x0F);
    glcdSendDatByte(0x05);
    glcdSendDatByte(0x32);
    glcdSendDatByte(0x45);
    glcdSendDatByte(0x46);
    glcdSendDatByte(0x04);
    glcdSendDatByte(0x0E);
    glcdSendDatByte(0x0D);
    glcdSendDatByte(0x35);
    glcdSendDatByte(0x37);
    glcdSendDatByte(0x0F);

    glcdSendCmdByte(0XC0); // Power Control 1
    glcdSendDatByte(0x17);
    glcdSendDatByte(0x15);

    glcdSendCmdByte(0xC1); // Power Control 2
    glcdSendDatByte(0x41);

    glcdSendCmdByte(0xC5); // VCOM Control
    glcdSendDatByte(0x00);
    glcdSendDatByte(0x12);
    glcdSendDatByte(0x80);

    glcdSendCmdByte(TFT_MADCTL); // Memory Access Control
    glcdSendDatByte(0x48);          // MX, BGR

    glcdSendCmdByte(0x3A); // Pixel Interface Format
    glcdSendDatByte(0x55);  // 16 bit colour for parallel

    glcdSendCmdByte(0xB0); // Interface Mode Control
    glcdSendDatByte(0x00);

    glcdSendCmdByte(0xB1); // Frame Rate Control
    glcdSendDatByte(0xA0);

    glcdSendCmdByte(0xB4); // Display Inversion Control
    glcdSendDatByte(0x02);

    glcdSendCmdByte(0xB6); // Display Function Control
    glcdSendDatByte(0x02);
    glcdSendDatByte(0x02);
    glcdSendDatByte(0x3B);

    glcdSendCmdByte(0xB7); // Entry Mode Set
    glcdSendDatByte(0xC6);

    glcdSendCmdByte(0xF7); // Adjust Control 3
    glcdSendDatByte(0xA9);
    glcdSendDatByte(0x51);
    glcdSendDatByte(0x2C);
    glcdSendDatByte(0x82);

    glcdSendCmdByte(TFT_SLPOUT);  //Exit Sleep
    delay(120);

    glcdSendCmdByte(TFT_DISPON);  //Display on
    delay(25);

// End of ILI9488 display configuration
 
  glcdSendCmdByte(TFT_MADCTL);    // Memory Access Control

  #if LCD_ROTATION==0
    glcdSendDatByte(TFT_MAD_MX | TFT_MAD_BGR); // Rotation 0 (portrait mode)
 	glcdSetAddr(0, 0, PHYS_SCREEN_WIDTH, PHYS_SCREEN_HEIGHT);
  #elif LCD_ROTATION==1
    glcdSendDatByte(TFT_MAD_MV | TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_BGR); // Rotation 90 (landscape mode)
	glcdSetAddr(0, 0, PHYS_SCREEN_HEIGHT, PHYS_SCREEN_WIDTH);
  #elif LCD_ROTATION==2
    glcdSendDatByte(TFT_MAD_MY | TFT_MAD_BGR); // Rotation 180 (portrait mode)
 	glcdSetAddr(0, 0, PHYS_SCREEN_WIDTH, PHYS_SCREEN_HEIGHT);
  #elif LCD_ROTATION==3
    glcdSendDatByte(TFT_MAD_MV | TFT_MAD_BGR); // Rotation 270 (landscape mode)
	glcdSetAddr(0, 0, PHYS_SCREEN_HEIGHT, PHYS_SCREEN_WIDTH);
  #endif
 
   // write bytes to the framebuffer, so it starts off by one
   // This is a WA for the fact that the LCD controller is assuming the (D0 is LSB) D0 -> D7 is G3,4,5, then R0,1,2,3,4
   // and in the next byte B0,1,2,3,4 and then G0,1,2.
   // Maybe the PIO can be reprogrammed to switch the bit order (I guess not, because I suggest a register 
   // is written at once)
   // This also makes BGR effectively RGB
   // Cleanest approach would probably be to alter the LCD controller's LUT
   #ifdef WA_FB_OFF_BY_ONE   
   for (uint32_t jj = 0; jj < PHYS_SCREEN_WIDTH * PHYS_SCREEN_HEIGHT * 2 - 1; jj++)
     glcdSendDatByte(0x00);
   #endif
}
