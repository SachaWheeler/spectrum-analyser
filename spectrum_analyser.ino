/*
 *  references:
 *
 *  spectrum shield
 *  https://learn.sparkfun.com/tutorials/spectrum-shield-hookup-guide
 *  https://www.szynalski.com/tone-generator/
 *
 *  16x32 led grid setup
 *  https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/new-wiring
 *  https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/connecting-with-jumper-wires
 *
 *  logic
 *  https://github.com/snskreationz/Real-Time-RGB-Matrix-Spectrum-Analyzer/blob/master/arduino_code_real_time_spectrum_analyzer.ino
 *  https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/test-example-code
 *
 */

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <SPI.h>

//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1

int freq_amp;
int Frequencies_Left[7];
int Frequencies_Right[7];
int i;

// matrix connections
#define CLK 8  // MUST be on PORTB!
#define LAT A3
#define OE  9
#define A   A0 // move this
#define B   A1 // move this
#define C   A2
// RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);


void setup() {
  Serial.begin(9600);
  // matrix.begin();  

  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);

  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);
}

/*************Pull frquencies from Spectrum Shield****************/
void ReadFrequencies() {
  //Read frequencies for each band
  for (freq_amp = 0; freq_amp < 7; freq_amp++)
  {
    Frequencies_Left[freq_amp] = analogRead(DC_One);
    Frequencies_Right[freq_amp] = analogRead(DC_Two);
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

void SerialOutput() {
  Serial.print("F1:   ");
  for ( i = 0; i < 7; i++)
  {
    Serial.print(Frequencies_Left[i]);
    Serial.print("  ");

  }
  Serial.println();
  Serial.print("F2:   ");
  for ( i = 0; i < 7; i++)
  {
    Serial.print(Frequencies_Right[i]);
    Serial.print("  ");

  }
  Serial.println();
}

void PlotFrequencies(){
    
}

void lightcolumns(int rownum, int amplitude)
{
  if (amplitude > 15) // <-O-> set the threshold for the band to turn red
  {
    for ( int y = 0; y < amplitude; y++) {
      matrix.drawPixel(rownum, y, matrix.Color333(7, 0, 0));
    }
    for (int y = amplitude; y < 16; y++)
    {
      matrix.drawPixel(rownum, y, matrix.Color333(0, 0, 0));
    }
  }

  else if (amplitude > 13) // <-O-> set the threshold for the band to turn yellow
  {
    for ( int y = 0; y < amplitude; y++) {
      matrix.drawPixel(rownum, y, matrix.Color333(4, 4, 0));
    }
    for (int y = amplitude; y < 16; y++)
    {
      matrix.drawPixel(rownum, y, matrix.Color333(0, 0, 0));
    }
  }

  else if (amplitude > 9) // <-O-> set the threshold for the band to turn green
  {
    for ( int y = 0; y < amplitude; y++) {
      matrix.drawPixel(rownum, y, matrix.Color333(0, 5, 0));
    }
    for (int y = amplitude; y < 16; y++)
    {
      matrix.drawPixel(rownum, y, matrix.Color333(0, 0, 0));
    }
  }

  else
  {
    for ( int y = 0; y < amplitude; y++) {
      matrix.drawPixel(rownum, y, matrix.Color333(0, 0, 7));
    }
    for (int y = amplitude; y < 16; y++)
    {
      matrix.drawPixel(rownum, y, matrix.Color333(0, 0, 0));
    }
  }
}


void loop() {

  ReadFrequencies();
  SerialOutput();
  PlotFrequencies();
  delay(1500);

}
