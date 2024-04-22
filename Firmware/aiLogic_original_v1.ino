#include <avr/io.h>//for fast PWM
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define ENCODER_OPTIMIZE_INTERRUPTS //countermeasure of encoder noise
#include <Encoder.h>
//rotery encoder
#define ENCODER_COUNT_PER_CLICK 4
Encoder myEnc(4, 2);//use 4pin 2pin
int oldPosition  = -999;
int newPosition = -999;
// Define the display dimensions (128x64 for most common OLED displays)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Create an instance for the OLED display object
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define SSD1306_NO_SPLASH

const int LSB_threshold = 511;
const int A6_pin = A6;
const int A7_pin = A7;
const int A1_pin = A1; // New CV input on Analog Pin 1
const int D9_pin = 9;
const int D10_pin = 10;
const int D3_pin = 3;
const int D11_pin = 11;

int valueA6 = 0;
int outA6 = 0;
int valueA7 = 0;
int outA7 = 0;
int AND = 0;
int NAND = 0;
int OR = 0;
int NOR = 0;
int XOR = 0;
int XNOR = 0;
int EXOR = 0;
int NEXOR = 0;
int high = 0;
int low = 0;
const int LSB_resolution = 256;
int A7Value = 0;
int A6Value = 0;
int A1Value = 0; // New for CV input on Analog Pin 1
int A7LastValue = 0;
int A6LastValue = 0;
int A1LastValue = 0; //  CV input Analog Pin 1
int button = 7;
int mode = 0;
int subMode = 0; // Sub-mode for the "logMODE"
int previousButtonState = HIGH;
unsigned long debounceTime = 0;
const int debounceDelay = 150;
int cvValue = 0;

const char subModes[] = {'A', 'O', 'X', 'E'}; // First letter of the logic modes for sub-mode display

void setup() {
  Serial.begin(57600);
  // Fast PWM setting
  TCCR1B &= B11111000; // Hand coding
  TCCR1B |= B00000001; // Hand coding

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the display buffer and update the display
  display.clearDisplay();
  display.display();

  pinMode(button, INPUT_PULLUP);
  pinMode(A6_pin, INPUT);
  pinMode(A7_pin, INPUT);
  pinMode(D9_pin, OUTPUT);
  pinMode(D10_pin, OUTPUT);
  pinMode(D3_pin, OUTPUT);
  pinMode(D11_pin, OUTPUT);
  pinMode(4, INPUT_PULLUP); // Encoder A?
  pinMode(2, INPUT_PULLUP); // Encoder B?
  pinMode(A1, INPUT); // Analog Pin 1 as input for the control voltage
}

void loop() {
  // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);        // Text size 1, smaller text
  display.setTextColor(SSD1306_WHITE); // White text color

  // Print the mode on the OLED display
  display.setCursor(8, 10); // Set the cursor to the top-left corner

  int buttonState = digitalRead(button);
  if (buttonState == LOW && previousButtonState == HIGH) {
    if (millis() - debounceTime > debounceDelay) {
      mode = (mode + 1) % 2; // Two main modes: 0 - logMODE, 1 - CV MODE
      subMode = 0; // Reset sub-mode when changing the main mode
      debounceTime = millis();
    }
  }

  // If in the logMODE, use the encoder to switch sub-modes
  if (mode == 0) {
    newPosition = myEnc.read();
    subMode = (newPosition + 4) % 4; // Use encoder to rotate between sub-modes (A, O, X, E)
  }

  previousButtonState = buttonState;

  // Read the CV input on Analog Pin 1
  A1Value = analogRead(A1_pin);

  // If there's CV input, use the encoder as an attenuator for the CV input
  if (A1Value >= 100) {
    newPosition = myEnc.read();
    int attenuatedValue = map(newPosition, 0, 255, 0, A1Value);
    A1Value = attenuatedValue;
  }

  switch (mode) {
    case 0://logMODE
      // Sub-mode title
      display.println(F("logMDE"));

      // Display the sub-mode as a square with the first letter of the logic mode
      display.drawRect(50 + subMode * 18, 20, 16, 16, SSD1306_WHITE);
      display.setCursor(55 + subMode * 18, 22);
      display.print(subModes[subMode]);

      // Switch between sub-modes with a single button press
      if (buttonState == LOW && previousButtonState == HIGH) {
        subMode = (subMode + 1) % 4;
      }

      // Update the logic modes based on the current sub-mode
      switch (subMode) {
        case 0: // AND/NAND
          valueA6 = analogRead(A6_pin);
          outA6 = (valueA6 >= LSB_threshold) ? HIGH : LOW;
          valueA7 = analogRead(A7_pin);
          outA7 = (valueA7 >= LSB_threshold) ? HIGH : LOW;
          AND = outA6 & outA7;
          NAND = !(outA6 & outA7);

          digitalWrite(D9_pin, AND);
          digitalWrite(D10_pin, NAND);
          digitalWrite(D3_pin, outA6);
          digitalWrite(D11_pin, outA7);
          Serial.println("AND | NAND");
          break;

        case 1: // OR/XOR
          valueA6 = analogRead(A6_pin);
          outA6 = (valueA6 >= LSB_threshold) ? HIGH : LOW;
          valueA7 = analogRead(A7_pin);
          outA7 = (valueA7 >= LSB_threshold) ? HIGH : LOW;
          OR = outA6 | outA7;
          NOR = !(outA6 | outA7);
          digitalWrite(D9_pin, OR);
          digitalWrite(D10_pin, NOR);
          digitalWrite(D3_pin, outA6);
          digitalWrite(D11_pin, outA7);
          Serial.println("OR | XOR");
          break;

        case 2: // XOR/XNOR
          outA6 = (analogRead(A6_pin) >= LSB_threshold) ? HIGH : LOW;
          outA7 = (analogRead(A7_pin) >= LSB_threshold) ? HIGH : LOW;
          XOR = outA6 ^ outA7;
          XNOR = !(outA6 ^ outA7);
          digitalWrite(D9_pin, XOR);
          digitalWrite(D10_pin, XNOR);
          digitalWrite(D3_pin, outA6);
          digitalWrite(D11_pin, outA7);
          Serial.println("XOR | XNOR");
          break;

        case 3: // EXNOR/NEXOR
          outA6 = (analogRead(A6_pin) >= LSB_threshold) ? HIGH : LOW;
          outA7 = (analogRead(A7_pin) >= LSB_threshold) ? HIGH : LOW;
          EXOR = outA6 ^ !outA7;      // EXOR = A6 XOR (NOT A7)
          NEXOR = !(outA6 ^ !outA7);  // NEXOR = NOT (A6 XOR (NOT A7))
          digitalWrite(D9_pin, EXOR);
          digitalWrite(D10_pin, NEXOR);
          digitalWrite(D3_pin, outA6);
          digitalWrite(D11_pin, outA7);
          Serial.println("EXOR|NEXOR");
          break;
      }
      break;

    case 1: // CV MODE
      display.println(F("CV MODE"));

      // Display the CV input as a bar graph
      display.fillRect(8, 20, A1Value / 8, 6, SSD1306_WHITE); // Scale the bar graph to fit the OLED screen
      break;
  }

  // Update the display
  display.display();

  // Add other debug information as needed
  Serial.print("ValueA6: ");
  Serial.println(valueA6);
  Serial.print("ValueA7: ");
  Serial.println(valueA7);
  Serial.print("OutA6: ");
  Serial.println(outA6);
  Serial.print("OutA7: ");
  Serial.println(outA7);

  delay(50);
}
