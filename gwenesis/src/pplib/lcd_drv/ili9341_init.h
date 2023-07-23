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
  
  glcdSendCmdByte(TFT_CASET); // column address set
  for (int i = 0; i < 4; i++)
    glcdSendDatByte(coordinates[i]);

    
  glcdSendCmdByte(TFT_PASET);  // page address set
  for (int i = 0; i < 4; i++)
    glcdSendDatByte(coordinates[i + 4]);

  glcdSendCmdByte(TFT_RAMWR); //memory write 
  
  // WA framebuffer starts one pixel too late (Why?)
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x00);
}

void lcdControllerInit() {
  set_rst(HIGH);
  delay(20);
  set_rst(LOW);
  delay(20);
  set_rst(HIGH);

  delay(50);

  glcdSendCmdByte(0xEF);
  glcdSendDatByte(0x03);
  glcdSendDatByte(0x80);
  glcdSendDatByte(0x02);

  glcdSendCmdByte(0xCF);
  glcdSendDatByte(0x00);
  glcdSendDatByte(0XC1);
  glcdSendDatByte(0X30);

  glcdSendCmdByte(0xED);
  glcdSendDatByte(0x64);
  glcdSendDatByte(0x03);
  glcdSendDatByte(0X12);
  glcdSendDatByte(0X81);

  glcdSendCmdByte(0xE8);
  glcdSendDatByte(0x85);
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x78);

  glcdSendCmdByte(0xCB);
  glcdSendDatByte(0x39);
  glcdSendDatByte(0x2C);
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x34);
  glcdSendDatByte(0x02);

  glcdSendCmdByte(0xF7);
  glcdSendDatByte(0x20);

  glcdSendCmdByte(0xEA);
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x00);

  glcdSendCmdByte(ILI9341_PWCTR1);    //Power control
  glcdSendDatByte(0x23);   //VRH[5:0]

  glcdSendCmdByte(ILI9341_PWCTR2);    //Power control
  glcdSendDatByte(0x10);   //SAP[2:0];BT[3:0]

  glcdSendCmdByte(ILI9341_VMCTR1);    //VCM control
  glcdSendDatByte(0x3e);
  glcdSendDatByte(0x28);

  glcdSendCmdByte(ILI9341_VMCTR2);    //VCM control2
  glcdSendDatByte(0x86);  //--

  glcdSendCmdByte(ILI9341_PIXFMT);
  glcdSendDatByte(0x55);

  glcdSendCmdByte(ILI9341_FRMCTR1);
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x13); // 0x18 79Hz, 0x1B default 70Hz, 0x13 100Hz

  glcdSendCmdByte(ILI9341_DFUNCTR);    // Display Function Control
  glcdSendDatByte(0x08);
  glcdSendDatByte(0x82);
  glcdSendDatByte(0x27);

  glcdSendCmdByte(0xF2);    // 3Gamma Function Disable
  glcdSendDatByte(0x00);

  glcdSendCmdByte(ILI9341_GAMMASET);    //Gamma curve selected
  glcdSendDatByte(0x01);

  glcdSendCmdByte(ILI9341_GMCTRP1);    //Set Gamma
  glcdSendDatByte(0x0F);
  glcdSendDatByte(0x31);
  glcdSendDatByte(0x2B);
  glcdSendDatByte(0x0C);
  glcdSendDatByte(0x0E);
  glcdSendDatByte(0x08);
  glcdSendDatByte(0x4E);
  glcdSendDatByte(0xF1);
  glcdSendDatByte(0x37);
  glcdSendDatByte(0x07);
  glcdSendDatByte(0x10);
  glcdSendDatByte(0x03);
  glcdSendDatByte(0x0E);
  glcdSendDatByte(0x09);
  glcdSendDatByte(0x00);

  glcdSendCmdByte(ILI9341_GMCTRN1);    //Set Gamma
  glcdSendDatByte(0x00);
  glcdSendDatByte(0x0E);
  glcdSendDatByte(0x14);
  glcdSendDatByte(0x03);
  glcdSendDatByte(0x11);
  glcdSendDatByte(0x07);
  glcdSendDatByte(0x31);
  glcdSendDatByte(0xC1);
  glcdSendDatByte(0x48);
  glcdSendDatByte(0x08);
  glcdSendDatByte(0x0F);
  glcdSendDatByte(0x0C);
  glcdSendDatByte(0x31);
  glcdSendDatByte(0x36);
  glcdSendDatByte(0x0F);

  glcdSendCmdByte(ILI9341_SLPOUT);    //Exit Sleep
 
  set_rs(LOW);
  delay(120);
  set_rs(HIGH);
  
  glcdSendCmdByte(ILI9341_DISPON);    //Display on

  glcdSendCmdByte(ILI9341_MADCTL);    // Memory Access Control

  #if LCD_ROTATION==0
    glcdSendDatByte(TFT_MAD_MX | TFT_MAD_COLOR_ORDER); // Rotation 0 (portrait mode)
 	glcdSetAddr(0, 0, PHYS_SCREEN_WIDTH, PHYS_SCREEN_HEIGHT);
 #elif LCD_ROTATION==1
    glcdSendDatByte(TFT_MAD_MV | TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER); // Rotation 90 (landscape mode)
	glcdSetAddr(0, 0, PHYS_SCREEN_HEIGHT, PHYS_SCREEN_WIDTH);
  #elif LCD_ROTATION==2
    glcdSendDatByte(TFT_MAD_MY | TFT_MAD_COLOR_ORDER); // Rotation 180 (portrait mode)
 	glcdSetAddr(0, 0, PHYS_SCREEN_WIDTH, PHYS_SCREEN_HEIGHT);
  #elif LCD_ROTATION==3
    glcdSendDatByte(TFT_MAD_MV | TFT_MAD_COLOR_ORDER); // Rotation 270 (landscape mode)
	glcdSetAddr(0, 0, PHYS_SCREEN_HEIGHT, PHYS_SCREEN_WIDTH);
  #endif
}
