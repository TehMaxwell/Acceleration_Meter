/*
 * This is the main software for the Acceleration Meter. This device is designed to measure the acceleration of an object using an accelerometer and gyroscope.
 * Acceleration results are outputed on a TFT LCD with fuller colour graphics, a touch screen user interface is provided to switch between screens.
 * Users can either view the acceleration results as individual numbers, or as a graph. Session results can be saved, and all time maximum results can also be viewed.
 * 
 * Originially Developed By: Sam Maxwell
 * Release Date: INSERT RELEASE DATE HERE
 * Version Number: INSERT INITIAL VERSION NUMBER HERE
 * For more Information on Hardware, Software and Design Files please visit the Github Page for this project located here: https://github.com/TehMaxwell/Acceleration_Meter
 */
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MODULES
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Elegoo_GFX.h>       //Core TFT Graphics Library
#include <Elegoo_TFTLCD.h>    //Hardware Specific Library
#include <TouchScreen.h>      //Touch Screen Hardware Specific Library
#include <SD.h>               //SD Card Library
#include <SPI.h>              //Serial Peripheral Interface (SPI) Library
#include <pin_magic.h>
#include <registers.h>
#include <helper_3dmath.h>    //Library for 3 Dimensional Mathematics Calculations
#include <MPU60X0.h>          //I2C Comms with Accelerometer Unit
#include <MPU60X0_6Axis_MotionApps20.h>

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//DEFINITIONS
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//TOUCH SCREEN
//Touch Screen Pins
#define YP A3
#define XM A2
#define YM 9
#define XP 8

//Touch Screen Parameters
#define TOUCH_RESISTANCE 300  //The Resistance of the Touch Screen in Ohms
#define TOUCH_MIN_X 120       //The Minimum X Value Outputted by the Touch Screen
#define TOUCH_MAX_X 900       //The Maximum X Value Outputted by the Touch Screen
#define TOUCH_MIN_Y 70        //The Minimum Y Value Outputted by the Touch Screen
#define TOUCH_MAX_Y 920       //The Maximum Y Value Outputted by the Touch Screen

//TFT LCD
//TFT LCD Pins
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//TFT LCD Parameters
#define TFT_IDENTIFIER 0x9341   //The Identifier for the TFT LCD Driver Chip

//SD CARD READER
//SD Card Pins
#define SD_CS 10  //The SD Card Reader Chip Select Pin

//COLOURS
//Colour Value Definitions
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0xB3B3B3

//BMP FILES
//BMP File Parameters
#define BUFFPIXEL 240  //The size of the buffer used to store BMP File data
#define BMP_HEIGHT 320   //BMP Image Height
#define BMP_WIDTH 240    //BMP Image Width

//GUI
//Numerical Acceleration Screen Parameters
#define AXIAL_X_COORD_NAS 20
#define AXIAL_Y_COORD_NAS 79
#define LATERAL_X_COORD_NAS 20
#define LATERAL_Y_COORD_NAS 136
#define VERTICAL_X_COORD_NAS 20
#define VERTICAL_Y_COORD_NAS 193

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//VARIABLES
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//GUI
//GUI File Names
char* gui_file_names[4] = {"NAS.bmp", "GAS.bmp", "HIST.bmp", "SET.bmp"};   //The names of the GUI BMP Files

//BMP FILES
//BMP File Parameters
unsigned char bmp_data_offset = 0;  //The offset between the start of the BMP File and the RGB Data

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//OBJECTS
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Touch Screen Object
TouchScreen touch = TouchScreen(XP, YP, XM, YM, TOUCH_RESISTANCE);

//TFT LCD Object
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//FUNCTIONS
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//SD CARD FUNCTIONS
//Read the next 16 Bit Word from the File and convert it to Little Endian
uint16_t read16(File f)
{
    uint16_t d;
    uint8_t b;
    b = f.read();
    d = f.read();
    d <<= 8;
    d |= b;
    return d;
}

//Read the next 32 Bit Word from the File and convert it to Little Endian
uint32_t read32(File f)
{
    uint32_t d;
    uint16_t b;
    b = read16(f);
    d = read16(f);
    d <<= 16;
    d |= b;
    return d;
}

//Get the RGB Data offset value from the BMP File
void get_bmp_rgb_data_offset(File bmpFile) 
{ 
    bmpFile.seek(10); //Seeking to the RGB Data offset value
    bmp_data_offset = read32(bmpFile);
}

//TFT LCD FUNCTIONS
//Write a BMP Image to the TFT LCD Display
void bmp_draw(File bmpFile){
    uint8_t bmp_buff[BUFFPIXEL * 3]; //Buffer to store RGB Data

    get_bmp_rgb_data_offset(bmpFile);
    bmpFile.seek(bmp_data_offset);  //Seeking to RGB Data Start in BMP File
    
    //For all columns of the Image
    for (int column = 0; column < BMP_HEIGHT; column++) {
        //For all Rows of the Image, in chunks the size of the RGB Buffer
        for(int rgb_data_chunk = 0; rgb_data_chunk < (BMP_WIDTH / BUFFPIXEL); rgb_data_chunk++) {
            int buff_index = 0;
            int offset_x = rgb_data_chunk * BUFFPIXEL;  //Current Data Chunk Offset along Screen Width in Pixels
            unsigned int color[BUFFPIXEL];
            
            bmpFile.read(bmp_buff, BUFFPIXEL * 3);
            
            for(int row = 0; row < BUFFPIXEL; row++) {
                color[row] = bmp_buff[buff_index + 2] >> 3;                      //Red Colour Data
                color[row] = color[row] << 6 | (bmp_buff[buff_index + 1] >> 2);    //Green Colour Data
                color[row] = color[row] << 5 | (bmp_buff[buff_index + 0] >> 3);    //Blue Colour Data
                tft.drawPixel(row + offset_x, column, color[row]);
                buff_index += 3;
            }
        }
    }
}

//Write some Text to the TFT LCD at the given Coordinates
void text_draw(char* text, int colour, int x, int y){
  tft.setRotation(135);   //Setting the screen rotation to landscape
  tft.setCursor(x, y);
  tft.setTextColor(colour);
  tft.setTextSize(2);
  tft.println(text);
  tft.setRotation(0);
}

//TEST FUNCTIONS
//Generate a random floating point number
float gen_rand_num(){
  return (random(-500, 500) / 100.0);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN SETUP
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup(){
  //Beginning Serial Communications for Debugging
  Serial.begin(9600);
  
  //TFT LCD Setup
  tft.reset();  //Resetting the LCD Display
  tft.begin(TFT_IDENTIFIER);  //Beginning Communications with the TFT LCD Driver Chip

  //SD Card Reader Setup
  //Initializing SD Card Communications
  if (!SD.begin(SD_CS)){
    //Informing User of SD Card Initialization Failure
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println("Failed to Initialize\nSD Card");
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN LOOP
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop(){
  
}

