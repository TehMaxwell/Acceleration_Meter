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
#define TOUCH_ENABLE_PIN 13   

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

//ACCELEROMETER

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
#define BUFFPIXEL 60  //The size of the buffer used to store BMP File data
#define BMP_HEIGHT 320   //BMP Image Height
#define BMP_WIDTH 240    //BMP Image Width

//GUI
//Menu Parameters
#define NUMERICAL_BUTTON_YMIN 0
#define NUMERICAL_BUTTON_YMAX 80
#define GRAPH_BUTTON_YMIN 81
#define GRAPH_BUTTON_YMAX 160
#define HIST_BUTTON_YMIN 161
#define HIST_BUTTON_YMAX 240
#define SET_BUTTON_YMIN 241
#define SET_BUTTON_YMAX 320
#define MENU_BUTTON_XMIN 0
#define MENU_BUTTON_XMAX 30

//Numerical Acceleration Screen Parameters
#define AXIAL_X_COORD_NAS 20
#define AXIAL_Y_COORD_NAS 79
#define LATERAL_X_COORD_NAS 20
#define LATERAL_Y_COORD_NAS 136
#define VERTICAL_X_COORD_NAS 20
#define VERTICAL_Y_COORD_NAS 193

//Graphical Acceleration Screen Parameters
#define GRAPH_BOX_XMIN 28
#define GRAPH_BOX_XMAX 313
#define GRAPH_BOX_YMIN 40
#define GRAPH_BOX_YMAX 232
#define GRAPH_NUM_POINTS 20
#define GRAPH_MAX_ACCEL 5
#define GRAPH_MIN_ACCEL -5

//History Data Screen Parameters
#define SESSION_AXIAL_XCOORD 105
#define SESSION_AXIAL_YCOORD 68
#define SESSION_LATERAL_XCOORD 105
#define SESSION_LATERAL_YCOORD 107
#define SESSION_VERTICAL_XCOORD 215
#define SESSION_VERTICAL_YCOORD 107
#define ALLTIME_AXIAL_XCOORD 105
#define ALLTIME_AXIAL_YCOORD 173
#define ALLTIME_LATERAL_XCOORD 105
#define ALLTIME_LATERAL_YCOORD 211
#define ALLTIME_VERTICAL_XCOORD 215
#define ALLTIME_VERTICAL_YCOORD 211

//Timing Parameters
#define DELAY_TIME 500  //The time between screen refreshes in milliseconds

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//VARIABLES
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//GUI
//GUI File Names
const char* const gui_file_names[] = {"NAS.bmp", "GAS.bmp", "HIST.bmp", "SET.bmp"};   //The names of the GUI BMP Files
const char boot_bmp_name[] = "BOOT.bmp";

//GUI Tracking Variables
//Screen Parameters
int screen_index = 0;
bool background_drawn = false;

//Timing Parameters
unsigned long start_time;
unsigned long current_time;

//TOUCHSCREEEN
//Touchscreen Tracking Variables
int x_coord = 0, y_coord = 0;

//BMP FILES
//BMP File Parameters
unsigned char bmp_data_offset = 0;  //The offset between the start of the BMP File and the RGB Data

//ACCELEROMETER
//Acceleration Values
float last_accel_vals[3] = {0.0, 0.0, 0.0}, accel_vals[3] = {0.0, 0.0, 0.0};
float graph_axial_accel_vals[GRAPH_NUM_POINTS], graph_lateral_accel_vals[GRAPH_NUM_POINTS], graph_vertical_accel_vals[GRAPH_NUM_POINTS];

//DATA
//Data File Names
const char data_file_name[] = "DT.txt";   //File containing all time maximum acceleration data

//Data Storage Variables
float all_time_max_accel[3] = {0.0, 0.0, 0.0}, last_all_time_max_accel[3] = {0.0, 0.0, 0.0};
float session_max_accel[3] = {0.0, 0.0, 0.0}, last_session_max_accel[3] = {0.0, 0.0, 0.0};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//OBJECTS
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//TFT LCD Object
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

//Touch Screen Object
TouchScreen touch = TouchScreen(XP, YP, XM, YM, TOUCH_RESISTANCE);
TSPoint touch_point;

//SD Object
File file;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//FUNCTIONS
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//SD CARD FUNCTIONS
//Setup the SD Card Reader
void setup_SD(void){
  //Initializing SD Card Communications
  if (!SD.begin(SD_CS)){
    //Informing User of SD Card Initialization Failure
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println("Failed to Initialize\nSD Card");
    while(1);
  }
}

//Read the next 16 Bit Word from the File and convert it to Little Endian
uint16_t read16(void){
    uint16_t d;
    uint8_t b;
    b = file.read();
    d = file.read();
    d <<= 8;
    d |= b;
    return d;
}

//Read the next 32 Bit Word from the File and convert it to Little Endian
uint32_t read32(void){
    uint32_t d;
    uint16_t b;
    b = read16();
    d = read16();
    d <<= 16;
    d |= b;
    return d;
}

//Get the next line from a text file on the SD Card
String read_text_line(void){
  String text = "";
  char current_byte = 0;
  while(current_byte != '\r'){
    current_byte = file.read();
    if(current_byte != '\r'){
      text += current_byte;
    }
  }
  return(text);
}

//Get the all time maximum accelerations from the Data File
void get_all_time_max_accel(void){
  String accel_val_string = "";
  file = SD.open(data_file_name);   //Opening the All Time Maximum Acceleration Data File
  for(int index = 0; index < 3; index++){
    accel_val_string = read_text_line();
    all_time_max_accel[index] = accel_val_string.toFloat();
  }
  file.close();
}

//Set the all time maximum accelerations in the Data File
void set_all_time_max_accel(void){
  char accel_val_string[] = "";
  String file_output = "";
  SD.remove(data_file_name);
  file = SD.open(data_file_name, FILE_WRITE);   //Opening the All Time Maximum Acceleration Data File
  for(int index = 0; index < 3; index++){
    dtostrf(all_time_max_accel[index], 5, 3, accel_val_string);
    file_output = file_output + String(accel_val_string) + "\r\n";
  }
  file.println(file_output);   //Writing the new all time maximum acceleration values into the Data File
  file.close();
}

//Get the RGB Data offset value from the BMP File
void get_bmp_rgb_data_offset(void){ 
    file.seek(10); //Seeking to the RGB Data offset value
    bmp_data_offset = read32();
}

//TFT LCD FUNCTIONS
//Setup the TFT LCD
void setup_tft(void){
  tft.reset();  //Resetting the LCD Display
  tft.begin(TFT_IDENTIFIER);  //Beginning Communications with the TFT LCD Driver Chip
}

//Write a BMP Image to the TFT LCD Display
void bmp_draw(void){
    uint8_t bmp_buff[BUFFPIXEL * 3]; //Buffer to store RGB Data

    get_bmp_rgb_data_offset();
    file.seek(bmp_data_offset);  //Seeking to RGB Data Start in BMP File
    
    //For all columns of the Image
    for (int column = 0; column < BMP_HEIGHT; column++) {
        //For all Rows of the Image, in chunks the size of the RGB Buffer
        for(int rgb_data_chunk = 0; rgb_data_chunk < (BMP_WIDTH / BUFFPIXEL); rgb_data_chunk++) {
            int buff_index = 0;
            int offset_x = rgb_data_chunk * BUFFPIXEL;  //Current Data Chunk Offset along Screen Width in Pixels
            unsigned int color[BUFFPIXEL];
            
            file.read(bmp_buff, BUFFPIXEL * 3);
            
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
  tft.print(text);
  tft.setRotation(0);
}

//Add lines to the Graphical Acceleration Screen Graph
void draw_graph_lines(float* accel_vals, int colour){
  int x1_coord, x2_coord, y1_coord, y2_coord;

  tft.setRotation(135);   //Setting screen coordinates to landscape
  
  //For all acceleration points
  for(int index = 1; index < GRAPH_NUM_POINTS; index++){
    x1_coord = ((GRAPH_BOX_XMAX - GRAPH_BOX_XMIN) / GRAPH_NUM_POINTS) * (index - 1) + GRAPH_BOX_XMIN;
    y1_coord = map(accel_vals[index - 1], GRAPH_MIN_ACCEL, GRAPH_MAX_ACCEL, GRAPH_BOX_YMIN, GRAPH_BOX_YMAX);
    x2_coord = ((GRAPH_BOX_XMAX - GRAPH_BOX_XMIN) / GRAPH_NUM_POINTS) * index + GRAPH_BOX_XMIN;
    y2_coord = map(accel_vals[index], GRAPH_MIN_ACCEL, GRAPH_MAX_ACCEL, GRAPH_BOX_YMIN, GRAPH_BOX_YMAX);
    tft.drawLine(x1_coord, y1_coord, x2_coord, y2_coord, colour);
  }

  tft.setRotation(0);   //Setting screen coordinates to portrait
}

//TOUCHSCREEN FUNCTIONS
//Setup Touch Screen
void setup_touch(void){
  pinMode(TOUCH_ENABLE_PIN, OUTPUT);
}

//Get the latest coordinates from the Touch Screen
void get_touch_point(void){
  digitalWrite(TOUCH_ENABLE_PIN, HIGH);
  touch_point = touch.getPoint();
  digitalWrite(TOUCH_ENABLE_PIN, LOW);
  touch_point.x = map(touch_point.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, tft.width());   //Mapping the X Value to Pixel Based Coordinates
  touch_point.y = map(touch_point.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, tft.height());  //Mapping the Y Value to Pixel Based Coordinates
  pinMode(XM, OUTPUT);  //Setting Pins as Output to ensure the LCD can function correctly
  pinMode(YP, OUTPUT);
}

//ACCELEROMETER FUNCTIONS
//Get the latest acceleration values - IMPLEMENT WITH ACTUAL CODE WHEN ACCELEROMETER READY
void get_accel_vals(void){
  for(int index = 0; index < 3; index++){
    last_accel_vals[index] = accel_vals[index];
    accel_vals[index] = get_rand_float();
  }
}

//Shift stored acceleration values along in an array and add the latest values to the end of the array
void shift_accel_vals(float *accel_vals, float latest_accel_val){
  for(int index = 0; index < GRAPH_NUM_POINTS - 1; index++){
    accel_vals[index] = accel_vals[index + 1];
  }
  accel_vals[GRAPH_NUM_POINTS - 1] = latest_accel_val;
}

//Update the session maximum acceleration values
void update_session_maximum(void){
  for(int index = 0; index < 3; index++){
    if(abs(accel_vals[index]) > abs(session_max_accel[index])){
      last_session_max_accel[index] = session_max_accel[index];
      session_max_accel[index] = accel_vals[index];   //Updating RAM stored value of the Session Maximum Acceleration Value
    }
  }
}

//Update the all time maximum acceleration values
void update_all_time_maximum(void){
  for(int index = 0; index < 3; index++){
    if(abs(accel_vals[index]) > abs(all_time_max_accel[index])){
      last_all_time_max_accel[index] = all_time_max_accel[index];
      all_time_max_accel[index] = accel_vals[index];  //Updating RAM stored value of the All Time Maximum Acceleration Value
    }
  }
  set_all_time_max_accel();
}

//GUI FUNCTIONS
//Decode a Touch Screen Coordinate Input into the corresponding button press
void get_menu_button_press(void){
  if(touch_point.x > 0){   //Ensuring that the touch pressure is great enough to avoid registering noise as a press
    if(touch_point.x > MENU_BUTTON_XMIN && touch_point.x < MENU_BUTTON_XMAX){   //Ensuring that a button on the menu has been pressed
      if(touch_point.y > NUMERICAL_BUTTON_YMIN && touch_point.y < NUMERICAL_BUTTON_YMAX){
        screen_index = 0;
      }
      else if(touch_point.y > GRAPH_BUTTON_YMIN && touch_point.y < GRAPH_BUTTON_YMAX){
        screen_index = 1;
      }
      else if(touch_point.y > HIST_BUTTON_YMIN && touch_point.y < HIST_BUTTON_YMAX){
        screen_index = 2;
      }
      else if(touch_point.y > SET_BUTTON_YMIN && touch_point.y < SET_BUTTON_YMAX){
        screen_index = 3;
      }
      background_drawn = false;
    }
  }
}

//Numerical Acceleration Screen Update Function
void update_NAS_screen(void){
  char accel_str[5];

  //Clearing the currently displayed acceleration values
  dtostrf(last_accel_vals[0], 5, 3, accel_str);
  text_draw(accel_str, WHITE, AXIAL_X_COORD_NAS, AXIAL_Y_COORD_NAS);
  dtostrf(last_accel_vals[1], 5, 3, accel_str);
  text_draw(accel_str, WHITE, LATERAL_X_COORD_NAS, LATERAL_Y_COORD_NAS);
  dtostrf(last_accel_vals[2], 5, 3, accel_str);
  text_draw(accel_str, WHITE, VERTICAL_X_COORD_NAS, VERTICAL_Y_COORD_NAS);

  //Displaying the new acceleration values
  dtostrf(accel_vals[0], 5, 3, accel_str);
  text_draw(accel_str, BLACK, AXIAL_X_COORD_NAS, AXIAL_Y_COORD_NAS);
  dtostrf(accel_vals[1], 5, 3, accel_str);
  text_draw(accel_str, BLACK, LATERAL_X_COORD_NAS, LATERAL_Y_COORD_NAS);
  dtostrf(accel_vals[2], 5, 3, accel_str);
  text_draw(accel_str, BLACK, VERTICAL_X_COORD_NAS, VERTICAL_Y_COORD_NAS);
}

//Graphical Acceleration Screen Update Function
void update_GAS_screen(void){
  draw_graph_lines(graph_axial_accel_vals, WHITE);  //Clearing the Axial Acceleration Lines
  draw_graph_lines(graph_lateral_accel_vals, WHITE);  //Clearing the Lateral Acceleration Lines
  draw_graph_lines(graph_vertical_accel_vals, WHITE); //Clearing the Vertical Acceleration Lines

  //Shifting acceleration data along array to add new acceleration point
  shift_accel_vals(graph_axial_accel_vals, accel_vals[0]);
  shift_accel_vals(graph_lateral_accel_vals, accel_vals[1]);
  shift_accel_vals(graph_vertical_accel_vals, accel_vals[2]);

  draw_graph_lines(graph_axial_accel_vals, RED);  //Drawing new Axial Acceleration Lines
  draw_graph_lines(graph_lateral_accel_vals, GREEN);  //Drawing new Lateral Acceleration Lines
  draw_graph_lines(graph_vertical_accel_vals, BLUE);  //Drawing new Vertical Acceleration Lines
}

//History Screen Update Function
void update_HIST_screen(void){
  bool update_screen = false;

  //Checking if any of the maximum acceleration values have changed
  for(int index = 0; index < 3; index++){
    Serial.print("Current Session Accel ");
    Serial.print(index);
    Serial.print(" ");
    Serial.println(session_max_accel[index]);
    Serial.print("Last Session Accel ");
    Serial.print(index);
    Serial.print(" ");
    Serial.println(last_session_max_accel[index]);
    if(all_time_max_accel[index] > last_all_time_max_accel[index] or session_max_accel[index] > last_session_max_accel[index]){
      update_screen = true;
    }
  }
  if(update_screen == true){
    update_HIST_screen_vals();
  }
}

//Function to update the History Screen Values
void update_HIST_screen_vals(void){
  char accel_str[5];
  
  //Clearing the currently displayed acceleration values - SESSION MAXIMUM
  dtostrf(last_session_max_accel[0], 5, 3, accel_str);
  text_draw(accel_str, WHITE, SESSION_AXIAL_XCOORD, SESSION_AXIAL_YCOORD);
  dtostrf(last_session_max_accel[1], 5, 3, accel_str);
  text_draw(accel_str, WHITE, SESSION_LATERAL_XCOORD, SESSION_LATERAL_YCOORD);
  dtostrf(last_session_max_accel[2], 5, 3, accel_str);
  text_draw(accel_str, WHITE, SESSION_VERTICAL_XCOORD, SESSION_VERTICAL_YCOORD);

  //Clearing the currently displayed acceleration values - ALL TIME MAXIMUM
  dtostrf(last_all_time_max_accel[0], 5, 3, accel_str);
  text_draw(accel_str, WHITE, ALLTIME_AXIAL_XCOORD, ALLTIME_AXIAL_YCOORD);
  dtostrf(last_all_time_max_accel[1], 5, 3, accel_str);
  text_draw(accel_str, WHITE, ALLTIME_LATERAL_XCOORD, ALLTIME_LATERAL_YCOORD);
  dtostrf(last_all_time_max_accel[2], 5, 3, accel_str);
  text_draw(accel_str, WHITE, ALLTIME_VERTICAL_XCOORD, ALLTIME_VERTICAL_YCOORD);

  //Displaying New Acceleration Values - SESSION MAXIMUM
  dtostrf(session_max_accel[0], 5, 3, accel_str);
  text_draw(accel_str, BLACK, SESSION_AXIAL_XCOORD, SESSION_AXIAL_YCOORD);
  dtostrf(session_max_accel[1], 5, 3, accel_str);
  text_draw(accel_str, BLACK, SESSION_LATERAL_XCOORD, SESSION_LATERAL_YCOORD);
  dtostrf(session_max_accel[2], 5, 3, accel_str);
  text_draw(accel_str, BLACK, SESSION_VERTICAL_XCOORD, SESSION_VERTICAL_YCOORD);

  //Displaying New Acceleration Values - ALL TIME MAXIMUM
  dtostrf(all_time_max_accel[0], 5, 3, accel_str);
  text_draw(accel_str, BLACK, ALLTIME_AXIAL_XCOORD, ALLTIME_AXIAL_YCOORD);
  dtostrf(all_time_max_accel[1], 5, 3, accel_str);
  text_draw(accel_str, BLACK, ALLTIME_LATERAL_XCOORD, ALLTIME_LATERAL_YCOORD);
  dtostrf(all_time_max_accel[2], 5, 3, accel_str);
  text_draw(accel_str, BLACK, ALLTIME_VERTICAL_XCOORD, ALLTIME_VERTICAL_YCOORD);
}

  
//TEST FUNCTIONS - COMMENT THIS SECTION OUT WHEN NOT IN USE
float get_rand_float(void){
  float random_float = random(-500, 500) / 100.0;
  return random_float;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN SETUP
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup(){
  //Beginning Serial Communications for Debugging
  Serial.begin(9600);

  //TFT LCD Setup
  setup_tft();

  //Filling Screen with Black to remove flicker
  tft.fillScreen(BLACK);

  //Touch Screen Setup
  setup_touch();

  //SD Card Reader Setup
  setup_SD();

  //Drawing the Boot Screen
  file = SD.open(boot_bmp_name);
  bmp_draw();
  file.close();

  //Getting All Time Maximum Data
  get_all_time_max_accel();

  //Pause for Boot Effect
  delay(2000);

  //Getting the Main Loop Start Time
  start_time = millis();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN LOOP
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop(){
  //Getting the Current Time
  current_time = millis();
  
  //Ensuring that the Timing Variable has not overflowed
  if(current_time < start_time){
    start_time = millis();
    current_time = millis();
  }
  
  //Getting the Current Touchscreen Values
  get_touch_point();
  
  //Registering a Menu Button Press
  get_menu_button_press();
  
  //Ensuring the Screen Background is Drawn
  if(background_drawn == false){
    file = SD.open(gui_file_names[screen_index]);
    bmp_draw();
    file.close();
    background_drawn = true;
  }
  
  //If the delay time has passed
  if(current_time - start_time > DELAY_TIME){
    //Getting the Current Acceleration Values
    get_accel_vals();

    //Updating the Session Maximum Values
    update_session_maximum();

    //Updating the All Time Maximum Values
    update_all_time_maximum();

    //Refreshing the Current Screen
    switch(screen_index){
      case 0:   //The index for the Numerical Acceleration Screen
        update_NAS_screen();
        break;

      case 1:   //The index for the Graphical Acceleration Screen
        update_GAS_screen();
        break;

      case 2:   //The index for the History Screen
        update_HIST_screen();
        break;
      
      default:
        break;
    }

    //Updating the timing loop
    start_time = millis();
  }
}

