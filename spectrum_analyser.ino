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
#define STROBE 12 // 4 on the shield, moved on the arduino due to library conflict with RGBmatrixPanel
#define RESET 13 // 5 on the shield, moved on the arduino
#define CHANNEL_ONE  A4 // A0 moved
#define CHANNEL_TWO A5 // A1 moved

int freq_amp;
int freq_left[7];
int freq_right[7];
int i;

int RED_THRESHOLD = 13;
int YELLOW_THRESHOLD = 9;
int GREEN_THRESHOLD = 6;
int row_max[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// matrix connections
#define CLK 8  // MUST be on PORTB!
#define LAT 10 // LAT == STROBE
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

void setup() {
  Serial.begin(9600);
  matrix.begin();

  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(CHANNEL_ONE, INPUT);
  pinMode(CHANNEL_TWO, INPUT);
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
    freq_left[freq_amp] = analogRead(CHANNEL_ONE);
    freq_right[freq_amp] = analogRead(CHANNEL_TWO);
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

void SerialOutput() {
  Serial.print("Left:   ");
  for ( i = 0; i < 7; i++)
  {
    Serial.print(freq_left[i]);
    Serial.print("  ");
  }
  Serial.println();
  Serial.print("Right:   ");
  for ( i = 0; i < 7; i++)
  {
    Serial.print(freq_right[i]);
    Serial.print("  ");
  }
  Serial.println();
}

void PlotFrequencies() {
  // lightcolumns(0, 0);
  for (int i = 1; i < 15; i += 2) {
    int idx = (i - 1) / 2;
    lightcolumns(i, freq_left[idx]);
    lightcolumns(i + 1, freq_left[idx]);
  }

  // lightcolumns(15, 0);
  // lightcolumns(16, 0);

  for (int j = 17; j < 31; j += 2) {
    int idx = (j - 17) / 2;
    lightcolumns(j, freq_right[idx]);
    lightcolumns(j + 1, freq_right[idx]);
  }
  // lightcolumns(31, 0);

  matrix.swapBuffers(false);
}

void lightcolumns(int row_num, int amp_1024)
{
  int amplitude = amp_1024 / 16;
  // Serial.print("amplitude:");
  // Serial.println(amplitude);

  if (amplitude > RED_THRESHOLD) // <-O-> set the threshold for the band to turn red
  {
    for ( int y = 0; y < amplitude; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(7, 0, 0));
    for (int y = amplitude; y < 16; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));

  } else if (amplitude > YELLOW_THRESHOLD) { // <-O-> set the threshold for the band to turn yellow
    for ( int y = 0; y < amplitude; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(4, 4, 0));
    for (int y = amplitude; y < 16; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));

  } else if (amplitude > GREEN_THRESHOLD) { // <-O-> set the threshold for the band to turn green
    for ( int y = 0; y < amplitude; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 5, 0));
    for (int y = amplitude; y < 16; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));

  } else {
    for ( int y = 0; y < amplitude; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 7));
    for (int y = amplitude; y < 16; y++) matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));
  }
}

void snowfall(int row_num, int amp_1024)
{
  int amplitude = amp_1024 / 16;

  // Serial.println(amplitude);

  if (amplitude > row_max[row_num]) {
    row_max[row_num] = amplitude;
  } else {
    row_max[row_num] -= 1;
    amplitude = row_max[row_num];
  }

  for ( int y = 0; y < 16; y++) {
    if (y == amplitude) {
      if (amplitude > RED_THRESHOLD)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(7, 0, 0));
      else if (amplitude > YELLOW_THRESHOLD)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(4, 4, 0));
      else if (amplitude > GREEN_THRESHOLD)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 5, 0));
      else
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 7));
    } else {
      matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));
    }
  }

}


void loop() {

  ReadFrequencies();
  // SerialOutput();
  PlotFrequencies();

  delay(50);
}
