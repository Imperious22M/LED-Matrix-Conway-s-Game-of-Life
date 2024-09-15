// This is a small program for simulating Conway's Game of Life
// on a HUB75 32x64 LED matrix on a Raspberry Pi Pico
// Requires the maxgerhardt Arduino core for this
// project to work in PlatformIO. 
// We will be using Adafruit's Protomatter library and 
// their GFX library for this project.
#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include "simulation.h"

// C definitions for the LED matrix and the simulation
#define matrix_chain_width 64 // total matrix chain width (width of the array)
#define bit_depth 4 // Number of bit depth of the color plane, higher = greater color fidelity
#define address_lines_num 4 // Number of address lines of the LED matrix
#define double_buffered true // Makes animation smother if true, at the cost of twice the RAM usage
// See Adafruits documentation for more details



// Arrays for the Raspberry Pi pinouts.
// These are in GP number format, which is different from
// the silkscreen numbers, please see a pinout diagram.
// Adafruit has wonderful documentation on the pin layout
// of the LCD matrix.
// See: https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/connecting-with-jumper-wires
// NOTE: If you are using the ribbon cable to connect, pins are mirrored relative
// to their order on the PCB in the Y axis. Please see Adafruit documentation
// for further details.
uint8_t rgbPins[]  = {0, 1, 2, 3, 4, 5}; //LED matrix: R1, G1, B1, R2, G2, B2
uint8_t addrPins[] = {6, 7, 8, 9}; // LED matrix: A,B,C,D
uint8_t clockPin   = 11; // LED matrix: CLK
uint8_t latchPin   = 12; // LED matrix: LAT
uint8_t oePin      = 13; // LED matrix: OE
uint8_t powerButton = 14; // GPIO of the power button
uint8_t modeButton = 15; // GPIO of the mode button
// GPIO 26 is unconnected and used as the seed for randomInit()

// Simulation variables
uint8_t rgbSimColor[] = {125,76,0}; // Custom color for the simulation
const uint32_t refreshRate = 1000; // refresh rate of the simulation in milliseconds
uint32_t oldTime = millis(); // timer used for refreshing the matrix
uint32_t cellsUpdated = 0; // Used as a reset measure if the pattern becomes "static"
const uint32_t updateThreshold = 35; // Sets the minimum amount of cells we need to update to keep the simulation going.

// For details on the constructor arguments please see:
// https://learn.adafruit.com/adafruit-matrixportal-m4/protomatter-arduino-library
Adafruit_Protomatter matrix(
  matrix_chain_width, bit_depth, 1, rgbPins, 
  address_lines_num, addrPins, clockPin, latchPin, 
  oePin, double_buffered);

// Initialize the simulation
ConwaysGame simulation(
  &matrix,
  matrix.color565(rgbSimColor[0],rgbSimColor[1],rgbSimColor[2]));

// Initial setup
void setup(void) {

  Serial.begin(9600);
  pinMode(26,INPUT); // Set unconnected GPIO26 (ADC0) as input for random seed if needed
  pinMode(powerButton,INPUT);
  pinMode(modeButton,INPUT);


  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status == PROTOMATTER_ERR_PINS) {
    while(true){
      Serial.println("RGB and clock pins are not on the same PORT!\n");
    }
  }else if(status != PROTOMATTER_OK){
    while(true){
      // If we get here, please see:
      // https://learn.adafruit.com/adafruit-matrixportal-m4/protomatter-arduino-library
      // for error state definitions and troubleshooting info
      Serial.println("Error initializing the matrix!");
    }
  }

  // Any function that has color must use matrix.color(uint8_t r,g,b) call to obtain a
  //16-bit integer with the desired color, which is passed as the color argument.
  matrix.setTextSize(1);
  matrix.println("Conway's \nGame \nof \nLife."); // Default text color is white

  // AFTER DRAWING, A show() CALL IS REQUIRED TO UPDATE THE MATRIX!
  matrix.show(); // Copy data to matrix buffers

  delay(5000);
  // Start the simulation by drawing the initial seed
  // and loading it into the screen.
  simulation.initSeed(true); // do  random pattern every startup
  simulation.drawCurGen();
  matrix.show();
  delay(2000);
}

// Run simulation forever
void loop(void) {
  // Run the simulation every 100 ms and show it in the matrix
  // This code can be vastly improved, but for now it serves its purpose.
  if(cellsUpdated<=updateThreshold){ // If there isn't enough activity in the simulation, make a new random pattern
    simulation.initSeed(true); // do  random pattern every startup
  }
  cellsUpdated = simulation.calcNextGen();
  simulation.drawCurGen();
  matrix.show();
  delay(100); // Ideally should be a non-blocking millis timer
}
