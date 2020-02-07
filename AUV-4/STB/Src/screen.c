#include "screen.h"


void writeCommand(uint8_t d){
 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
 	spiTxBuf[0] = 0x80;
 	spiTxBuf[1] = d;
 	HAL_SPI_Transmit(&hspi1,spiTxBuf,2,1);
 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
 }

 void writeData(uint8_t d){
 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
 	spiTxBuf[0] = 0x00;
 	spiTxBuf[1] = d;
 	HAL_SPI_Transmit(&hspi1,spiTxBuf,2,1);
 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
 }

 void writeReg(uint8_t reg, uint8_t val){
 	writeCommand(reg);
 	writeData(val);
 }


 uint8_t readData(void){
	 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
 	spiTxBuf[0]=0x40;
 	HAL_SPI_Transmit(&hspi1,spiTxBuf,1,1);
 	HAL_SPI_Receive(&hspi1,&spiRxBuf[0],1,1);
 	uint8_t x=spiRxBuf[0];
 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
 	return x;
 }
 uint8_t readReg(uint8_t reg){
 	writeCommand(reg);
 	return readData();
 }

 uint8_t readStatus(void){
	 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
 	spiTxBuf[0]=0xc0;
 	HAL_SPI_Transmit(&hspi1,spiTxBuf,1,1);
 	HAL_SPI_Receive(&hspi1,&spiRxBuf[0],1,1);
 	uint8_t x=spiRxBuf[0];
 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
 	return x;
 }


 void PLLint(void){
 	writeReg(0x88,0x0b);
 	HAL_Delay(1);
 	writeReg(0x89,0x02);
 	HAL_Delay(1);
 }

 void initialize(void){
	 PLLint();
     writeReg(0x10,0x0c|0x00);
	  /* Timing values */
	 uint8_t pixclk=0x80|0x01;
	 uint8_t hsync_start=32;
	 uint8_t hsync_pw=96;
	 uint8_t hsync_finetune=0;
	 uint8_t hsync_nondisp=26;
	 uint8_t vsync_pw=2;
	     uint16_t vsync_nondisp=32;
	     uint16_t vsync_start=23;
	    writeReg(0x04,0x81);
	    HAL_Delay(1);
	    //HORIZONTAL SETTINGS
	    writeReg(0x14,(800/8)-1);
	    writeReg(0x15,hsync_finetune+0x00);
	    writeReg(0x16,(hsync_nondisp - hsync_finetune - 2)/8);
	    writeReg(0x17, hsync_start/8 - 1);
	    writeReg(0x18, 0x00+ (hsync_pw/8 - 1));
	    //VERTICAL SETTINGS
	     writeReg(0x19, (uint16_t)(480 - 1) & 0xFF);
	     writeReg(0x1a, (uint16_t)(480 - 1) >> 8);
	     writeReg(0x1b, vsync_nondisp-1);
	     writeReg(0x1c, vsync_nondisp >> 8);
	     writeReg(0x1d, vsync_start-1);
	     writeReg(0x1e, vsync_start >> 8);
	     writeReg(0x1f, 0x00+ vsync_pw - 1);
	     //SET ACTIVE WINDOW X
	     writeReg(0x30, 0x00);
	     writeReg(0x31, 0x00);
	     writeReg(0x34, (uint16_t)(800 - 1) & 0xFF);
	     writeReg(0x35, (uint16_t)(800 - 1) >> 8);
	     //SET ACTIVE WINDOW Y
	     writeReg(0x32, 0x00);                                        // vertical start point
	     writeReg(0x33, 0x00);
	     writeReg(0x36, (uint16_t)(480 - 1) & 0xFF);           // horizontal end point
	     writeReg(0x37, (uint16_t)(480 - 1) >> 8);
	     //CLEAR ENTIRE WINDOW
	    writeReg(0x8E,0x80|0x00);
	    HAL_Delay(500);

 }

 void begin(void){
 	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);
 	HAL_Delay(1);
 	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
 	HAL_Delay(10);
 	initialize();
 }

 void displayOn(void){
 	writeReg(0x01,0x00|0x80);
 }

 void GPIOX(void){
 	writeReg(0xC7,1);
 }

 void PWM1Config(void){
 	writeReg(0x8A,0x80|(0x0A&0xF));
 }

 void PWM1out(void){
 	writeReg(0x8B,255);
 }

 uint8_t waitPoll(uint8_t reg,uint8_t waitflag){
 	while(1)
 	{
 		uint8_t temp = readReg(reg);
 		if(!(temp&waitflag))
 			return 1;
 	}
 	return 0;
 }

 void rectHelper(int16_t x,int16_t y, int16_t w, int16_t h, uint16_t color){
 	  writeCommand(0x91);
 	  writeData(x);
 	  writeCommand(0x92);
 	  writeData(x >> 8);

 	  /* Set Y */
 	  writeCommand(0x93);
 	  writeData(y);
 	  writeCommand(0x94);
 	  writeData(y >> 8);

 	  /* Set X1 */
 	  writeCommand(0x95);
 	  writeData(w);
 	  writeCommand(0x96);
 	  writeData((w) >> 8);

 	  /* Set Y1 */
 	  writeCommand(0x97);
 	  writeData(h);
 	  writeCommand(0x98);
 	  writeData((h) >> 8);

 	  /* Set Color */
 	  writeCommand(0x63);
 	  writeData((color & 0xf800) >> 11);
 	  writeCommand(0x64);
 	  writeData((color & 0x07e0) >> 5);
 	  writeCommand(0x65);
 	  writeData((color & 0x001f));

 	  /* Draw! */
 	  writeCommand(0x90);
 	  writeData(0xB0);

 	  /* Wait for the command to finish */
 	  waitPoll(0x90, 0x80);
 	  //to test
 	  HAL_Delay(10);
 }

 void fillScreen(uint16_t color)
 {
 	rectHelper(0,0,799,479,color);
 }

 void textMode(void){
 	writeCommand(0x40);
 	uint8_t temp = readData();
 	temp |= 0x80; // Set bit 7
 	writeData(temp);

 	/* Select the internal (ROM) font */
 	writeCommand(0x21);
 	temp = readData();
 	temp &= ~((1<<7) | (1<<5)); // Clear bits 7 and 5
 	writeData(temp);
 }

 void textSetCursor(uint16_t x, uint16_t y){
 	writeCommand(0x2A);
     writeData(x & 0xFF);
 	writeCommand(0x2B);
 	writeData(x >> 8);
 	writeCommand(0x2C);
 	writeData(y & 0xFF);
 	writeCommand(0x2D);
 	writeData(y >> 8);
 }

 void textTransparent(uint16_t foreColor)
 {
   /* Set Fore Color */
   writeCommand(0x63);
   writeData((foreColor & 0xf800) >> 11);
   writeCommand(0x64);
   writeData((foreColor & 0x07e0) >> 5);
   writeCommand(0x65);
   writeData((foreColor & 0x001f));

   /* Set transparency flag */
   writeCommand(0x22);
   uint8_t temp = readData();
   temp |= (1<<6); // Set bit 6
   writeData(temp);
 }

 uint8_t qstrlen(const char* var){
	  uint8_t i;
	  for (i = 0; var[i] != '\0'; i++) ;
      return i;
 }

 char *qstrcat(char *dest, const char *src)
 {
     size_t i,j;
     for (i = 0; dest[i] != '\0'; i++)
         ;
     for (j = 0; src[j] != '\0'; j++)
         dest[i+j] = src[j];
     dest[i+j] = '\0';
     return dest;
 }

 void textWrite(const char* buffer, uint16_t len)
 {
   if (len == 0) len = qstrlen(buffer);
   writeCommand(0x02);
   for (uint16_t i=0;i<len;i++)
   {
     writeData(buffer[i]);
     HAL_Delay(1);
   }
 }

 void textColor(uint16_t foreColor, uint16_t bgColor)
 {
   /* Set Fore Color */
   writeCommand(0x63);
   writeData((foreColor & 0xf800) >> 11);
   writeCommand(0x64);
   writeData((foreColor & 0x07e0) >> 5);
   writeCommand(0x65);
   writeData((foreColor & 0x001f));

   /* Set Background Color */
   writeCommand(0x60);
   writeData((bgColor & 0xf800) >> 11);
   writeCommand(0x61);
   writeData((bgColor & 0x07e0) >> 5);
   writeCommand(0x62);
   writeData((bgColor & 0x001f));

   /* Clear transparency flag */
   writeCommand(0x22);
   uint8_t temp = readData();
   temp &= ~(1<<6); // Clear bit 6
   writeData(temp);
 }
void textenlarge(uint8_t scale)
{
	if(scale>3)scale=3;
	writeCommand(0x22);
	uint8_t temp=readData();
	temp&=~(0xF);
	temp|=scale<<2;
	temp|=scale;
	writeData(temp);
}
uint32_t _x;
uint32_t _y;

void set_cursor(uint32_t x,uint32_t y){
	_x=x;
	_y=y;
	textSetCursor(x,y);
}

void write_string(const char* var){
	textWrite(var,0);
	increment_row();
}
void increment_row(void){
	_y+=35;
	textSetCursor(_x,_y);
}

void screen_default(void){
	  set_cursor(0, 0);
	  textTransparent(WHITE);
	  write_string("Ext press:");
	  write_string("Int press:");
	  write_string("PMB1 press:");
	  write_string("PMB2 press:");
	  write_string("PMB1 temp:");
	  write_string("PMB2 temp:");
	  write_string("CPU temp:");
	  write_string("Humidity:");
	  write_string("ST temp:");
	  write_string("DNA press: ");
	  write_string("XAVIER OK:");
	  write_string("PCB OK");
	  write_string("SBC-CAN OK:");

	  set_cursor(400,0);
	  write_string("Batt1 Capacity:");
	  write_string("Batt2 Capacity:");
	  write_string("Batt1 current:");
	  write_string("Batt2 current:");
	  write_string("Batt1 Voltage");
	  write_string("Batt2 Voltage:");
	  write_string("Thruster OK:");
	  write_string("Manipulator OK:");
	  write_string("PMB1 OK:");
	  write_string("PMB2 OK:");
}

void write_value_dp(uint32_t var,uint32_t dp){
	drawRect(_x, _y, 130, 30, BLACK);
	if(var==0xFFFF){
		textColor(WHITE,RED);
		write_string("N/A");
	}
	else{
		textTransparent(YELLOW);
		if(dp>0){
			char buf[20]={};
			char buf2[20]={};
			uint32_t dec = 0, whole =0, index = 1;
			whole = (uint32_t)(var / index);		// whole number
			dec = (uint32_t)(var % index);	// decimals
		    sprintf(buf, "%lu", whole);
			sprintf(buf2, "%lu", dec);
			qstrcat(buf, ".");
			qstrcat(buf, buf2);
			textWrite(buf,0);
			increment_row();
		}
		else{
			write_value_int(var);
		}
	}
}

void write_state(uint32_t var){
	if(var==0xFFFF){
		textColor(WHITE,RED);
		write_string("N/A");
	}
	else if(var==0x1111){
		textColor(YELLOW,GREEN);
		write_string("O N");
	}
}

void write_value_int(uint32_t var){
	drawRect(_x, _y, 130, 30, BLACK);
	textTransparent(YELLOW);
	if(var == 0xFFFF){
		write_string("N/A");
	}
	else{
		char buf[20]={};
		sprintf(buf,"%lu",var);
		textWrite(buf,0);
	}
	increment_row();
}

void update_screen(void){
	set_cursor(200,0);
	//INT_STAT_COUNT-9 because the last 9 data bytes are IMU readings and not displayed
	for (uint8_t i=0;i<(INT_STAT_COUNT-9);i++){
		write_value_dp(internalStats[i],0);
	}
	set_cursor(645,0);
	for (uint8_t i=0;i<POWER_STAT_COUNT;i++){
		write_value_dp(powerStats[i],1);
	}
}

void default_values(void){
	internalStats[EXT_PRESS]=0xFFFF;
	internalStats[INT_PRESS]=0xFFFF;
    internalStats[PMB1_PRESS]=0xFFFF;
    internalStats[PMB2_PRESS]=0xFFFF;
    internalStats[PMB1_TEMP]=0xFFFF;
    internalStats[PMB2_TEMP]=0xFFFF;
    internalStats[CPU_TEMP]=0xFFFF;
    internalStats[HUMIDITY]=0xFFFF;
    internalStats[ST_TEMP]=0xFFFF;
    internalStats[DNA_PRESS]=0xFFFF;

    powerStats[BATT1_CAPACITY]=0xFFFF;
    powerStats[BATT2_CAPACITY]=0xFFFF;
    powerStats[BATT1_CURRENT]=0xFFFF;
    powerStats[BATT2_CURRENT]=0xFFFF;
    powerStats[BATT1_VOLTAGE]=0xFFFF;
    powerStats[BATT2_VOLTAGE]=0xFFFF;

    boardHB[XAVIER]=0xFFFF;
    boardHB[SBC_CAN]=0xFFFF;
    boardHB[PCB]=0xFFFF;
    boardHB[Thruster]=0xFFFF;
    boardHB[Manipulator]=0xFFFF;
    boardHB[PMB1]=0xFFFF;
    boardHB[PMB2]=0xFFFF;
}

void update_internalstats(void){
	internalStats[EXT_PRESS]=0xFFFF;
	internalStats[INT_PRESS]=0xFFFF;
    internalStats[PMB1_PRESS]=0xFFFF;
    internalStats[PMB2_PRESS]=0xFFFF;
    internalStats[PMB1_TEMP]=0xFFFF;
    internalStats[PMB2_TEMP]=0xFFFF;
    internalStats[CPU_TEMP]=0xFFFF;
    internalStats[HUMIDITY]=0xFFFF;
    internalStats[ST_TEMP]=0xFFFF;
    internalStats[DNA_PRESS]=0xFFFF;
}
//update made for testing the values only
//to see if the main.c can change the array variable
void update(void){
	internalStats[EXT_PRESS]=0x1100;
	internalStats[INT_PRESS]=0xF000;
    internalStats[PMB1_PRESS]=0x000F;
    internalStats[PMB2_PRESS]=0xAABC;
    internalStats[PMB1_TEMP]=0xACCC;
    internalStats[PMB2_TEMP]=0x00A0;
    internalStats[CPU_TEMP]=0xFFFF;
    internalStats[HUMIDITY]=0xFFFF;
    internalStats[ST_TEMP]=0xFFFF;
    internalStats[DNA_PRESS]=0xFFFF;
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(x, y, x+w, y+h, color);
}

void check_hb(void){
	set_cursor(200,350);
	for (uint8_t i=0;i<3;i++){
		write_state(boardHB[i]);
	}
	set_cursor(645,210);
	for (uint8_t i=3;i<7;i++){
		write_state(boardHB[i]);
	}
}

void update_screen_can_test(uint8_t i){
	if(i%2==0){
		for(uint8_t i=0;i<7;i++){
			boardHB[i]=0xFFFF;
		}
	}
	else if(i%2==1){
		for(uint8_t i=0;i<7;i++){
			boardHB[i]=0x1111;
		}
	}
}
