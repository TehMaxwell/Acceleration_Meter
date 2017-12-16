/*
 * This is the main software for the Acceleration Meter. This device is designed to measure the acceleration of an object using an accelerometer and gyroscope.
 * Acceleration results are outputed on a TFT LCD with fuller colour graphics, a touch screen user interface is provided to switch between screens.
 * Users can either view the acceleration results as individual numbers, or as a graph. Session results can be saved, and all time maximum results can also be viewed.
 * 
 * Originially Developed By: Sam Maxwell
 * Release Date: INSERT RELEASE DATE HERE
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//VARIABLES
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN SETUP
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup(){
  //TFT LCD Setup
  tft.reset();  //Resetting the LCD Display
  tft.begin(TFT_IDENTIFIER);  //Beginning Communications with the TFT LCD Driver Chip
  tft.setRotation(135);

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

