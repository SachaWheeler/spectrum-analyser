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
#define CHANNEL_ONE  A5 // A1 moved
#define CHANNEL_TWO A4 // A0 moved

unsigned int freq_left[7];
unsigned int freq_right[7];
unsigned int amp_max;

#define RED_THRESHOLD 13
#define YELLOW_THRESHOLD 9
#define GREEN_THRESHOLD 5

// matrix connections
#define A   A0
#define B   A1
#define C   A2
#define CLK 8  // MUST be on PORTB!
#define LAT 10 // LAT == STROBE
#define OE  9
const bool double_buffering = false;
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, double_buffering); // last argument is for double-buffering

// sensitivity
unsigned int sensitivity = 2; // this could come from a pot
const unsigned int MAX_SENSITIVITY = 12;
unsigned long previousMillis = 0;
const unsigned int interval = 1400;

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
  for (unsigned int freq_amp = 0; freq_amp < 7; freq_amp++)
  {
    digitalWrite(STROBE, LOW);
    delayMicroseconds(50); // gather some data
    freq_left[freq_amp] = analogRead(CHANNEL_ONE);
    freq_right[freq_amp] = analogRead(CHANNEL_TWO);
    digitalWrite(STROBE, HIGH);

  }
}

void SerialOutput() {
  Serial.print("Left:   ");
  for (int i = 0; i < 7; i++)
  {
    Serial.print(freq_left[i]);
    Serial.print("  ");
  }
  Serial.println();
  Serial.print("Right:   ");
  for (int i = 0; i < 7; i++)
  {
    Serial.print(freq_right[i]);
    Serial.print("  ");
  }
  Serial.println();
}

void PlotFrequencies() {

  // lightcolumns(0, 0);
  unsigned int left_total = 0;
  unsigned int right_total = 0;

  for (unsigned int i = 1; i < 15; i += 2) {
    unsigned int idx = (i - 1) / 2;
    lightcolumns(i, freq_left[idx]);
    lightcolumns(i + 1, freq_left[idx]);
    left_total += freq_left[idx];
  }

  for (unsigned int j = 17; j < 31; j += 2) {
    unsigned int idx = (j - 17) / 2;
    lightcolumns(j, freq_right[idx]);
    lightcolumns(j + 1, freq_right[idx]);
    right_total += freq_right[idx];
  }

  lightcolumns(15, int(left_total / 7 ));
  lightcolumns(16, int(right_total / 7 ));

  // lightcolumns(31, sensitivity - 1);
  for (unsigned int k = 0; k < 16; k++) {
    if (k == sensitivity)
      matrix.drawPixel(31, k, matrix.Color333(7, 7, 7));
    else
      matrix.drawPixel(31, k, matrix.Color333(0, 0, 0));
  }

  if (double_buffering)
    matrix.swapBuffers(false); // we're only able to use single-buffering due to SRAM limitations
}

void lightcolumns(unsigned int row_num, unsigned int amp_1024)
{
  if (amp_1024 > 32)
    amp_1024 -= 32;  // trim some low end noise
  else
    amp_1024 = 0;
  unsigned int amplitude = int((amp_1024 * sensitivity) / 63.5);

  if (amplitude > amp_max && amplitude < 16)
    amp_max = amplitude;

  for (unsigned int y = 0; y < 16; y++) {

    if (row_num == 15 || row_num == 16) {
      if (y == amplitude)
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(7, 7, 7));
      else
        matrix.drawPixel(row_num, 15 - y, matrix.Color333(0, 0, 0));

    } else if (amplitude >= y) {

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

    if (amp_max < 10 and sensitivity <= MAX_SENSITIVITY)
      sensitivity += 1;
    else if (amp_max >= 15 and sensitivity > 1)
      sensitivity -= 1;

    amp_max = 0;
  }

  ReadFrequencies();
  // SerialOutput();
  PlotFrequencies();

  // delay(20);
}
