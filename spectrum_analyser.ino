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
int amp_max;

int RED_THRESHOLD = 12;
int YELLOW_THRESHOLD = 8;
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

int mode = 0;

// sensitivity
int sensitivity = 2; // this could come from a pot
const int MAX_SENSITIVITY = 12;
unsigned long previousMillis = 0;
const long interval = 1000;

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

  int average_total = 0;

  for (int i = 1; i < 15; i += 2) {
    int idx = (i - 1) / 2;
    lightcolumns(i, freq_left[idx]);
    lightcolumns(i + 1, freq_left[idx]);
    average_total += freq_left[idx];
    average_total += freq_left[idx];
  }

  // lightcolumns(15, 0);
  // lightcolumns(16, 0);

  for (int j = 17; j < 31; j += 2) {
    int idx = (j - 17) / 2;
    lightcolumns(j, freq_right[idx]);
    lightcolumns(j + 1, freq_right[idx]);
    average_total += freq_right[idx];
    average_total += freq_right[idx];

  }

  // lightcolumns(31, sensitivity - 1);
  for ( int y = 0; y < 16; y++) {
    // Serial.print(y);
    // Serial.print(" ");
    // Serial.println(sensitivity);
    if (y == sensitivity) matrix.drawPixel(31, y, matrix.Color333(7, 7, 7));
    else  matrix.drawPixel(31, y, matrix.Color333(0, 0, 0));
  }

  int average = int(average_total / 14);
  //Serial.println(average);
  if (average < 100)
    amp_max = 0;

  matrix.swapBuffers(false);
}

void lightcolumns(int row_num, int amp_1024)
{
  if (amp_1024 > 32) amp_1024 -= 32;
  else amp_1024 = 0;

  int amplitude = int((amp_1024 * sensitivity) / 63.5);

  if (amplitude > amp_max) amp_max = amplitude;
  if (amp_max > 15) amp_max = 15;

  for ( int y = 0; y < 16; y++) {
    if (amplitude >= y) {
      if (amplitude >= RED_THRESHOLD)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(7, 0, 0));
      else if (amplitude >= YELLOW_THRESHOLD)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(4, 4, 0));
      else if (amplitude >= GREEN_THRESHOLD)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 5, 0));
      else
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 7));
    } else {
      matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));
    }
  }
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Serial.print(sensitivity);
    // Serial.print(" ");
    // Serial.println(amp_max);

    if (amp_max < 10 and sensitivity <= MAX_SENSITIVITY) {
      sensitivity += 1;
      // Serial.println("increasing");
    } else if (amp_max >= 15 and sensitivity > 0) {
      sensitivity -= 1;
      // Serial.println("decreasing");
    }
  }

  ReadFrequencies();
  // SerialOutput();
  PlotFrequencies();

  delay(30);
}
