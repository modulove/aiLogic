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
int high = 0;
int low = 0;
int D10 = 10;
int D9 = 9;
int D11 = 11;
int D3 = 3;
const int A6Pin = 6;
const int A7Pin = 7;
const int D3Pin = 3;
const int D9Pin = 9;
const int D10Pin = 10;
const int D11Pin = 11;
const int pinD9 = 9;
const int pinD10 = 10;
const int pinD3 = 3;
const int pinD11 = 11;
const int D9pin = 9;
const int D10pin = 10;
const int D3pin = 3;
const int D11pin = 11;
const int LSB_resolution = 256;
int A7Value = 0;
int A6Value = 0;
int A7LastValue = 0;
int A6LastValue = 0;
int button = 7;
int mode = 0;
int previousButtonState = HIGH;
unsigned long debounceTime = 0;
const int debounceDelay = 150;

void setup() {
    Serial.begin(57600);
 //fast pwm setting
 TCCR1B &= B11111000;
 TCCR1B |= B00000001;

/*
 // Initialize the OLED display with the I2C address (you may need to adjust this address based on your OLED module)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // If the OLED is not detected, print an error message and stop the program
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  */
  
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
  pinMode(4, INPUT_PULLUP); //ENCODER A?
  pinMode(2, INPUT_PULLUP); //ENCODER B?

}

void loop() {

    // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(2);      // Text size 1, smaller text
  display.setTextColor(SSD1306_WHITE); // White text color

  // Print the mode on the OLED display
  display.setCursor(8, 10); // Set the cursor to the top-left corner
  
  int buttonState = digitalRead(button);
  if (buttonState == LOW && previousButtonState == HIGH) {
    if (millis() - debounceTime > debounceDelay) {
      mode = (mode + 1) % 6;
      debounceTime = millis();

    }
  }
  previousButtonState = buttonState;

  switch (mode) {
    case 0://AND
      digitalWrite(14, HIGH);//hand coding
      digitalWrite(19, LOW);//hand coding
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
      display.println(F("AND | NAND"));
      Serial.println("AND | NAND");
      break;

    case 1://OR
      digitalWrite(15, HIGH);//hand coding
      digitalWrite(14, LOW);//hand coding
      valueA6 = analogRead(A6);
      outA6 = (valueA6 >= LSB_threshold) ? HIGH : LOW;

      valueA7 = analogRead(A7);
      outA7 = (valueA7 >= LSB_threshold) ? HIGH : LOW;

      OR = outA6 | outA7;
      digitalWrite(D9Pin, OR);

      NOR = !(outA6 | outA7);
      digitalWrite(D10Pin, NOR);

      digitalWrite(D3Pin, outA6);
      digitalWrite(D11Pin, outA7);
      display.println(F("OR | XOR"));
      break;

    case 2://XOR
      digitalWrite(16, HIGH);//hand coding
      digitalWrite(15, LOW);//hand coding

      outA6 = (analogRead(6) >= LSB_threshold) ? HIGH : LOW;
      outA7 = (analogRead(7) >= LSB_threshold) ? HIGH : LOW;

      XOR = outA6 ^ outA7;
      XNOR = !(outA6 ^ outA7);

      digitalWrite(9, XOR);
      digitalWrite(10, XNOR);

      digitalWrite(3, outA6);
      digitalWrite(11, outA7);
      display.println(F("XOR | XNOR"));
      break;

    case 3://compare
      digitalWrite(17, HIGH);//hand coding
      digitalWrite(16, LOW);//hand coding

      outA6 = analogRead(6);
      outA7 = analogRead(7);

      if (outA6 > outA7) {
        digitalWrite(pinD9, HIGH);
        digitalWrite(pinD10, LOW);
      } else if (outA6 < outA7) {
        digitalWrite(pinD9, LOW);
        digitalWrite(pinD10, HIGH);
      }

      outA6 = map(outA6, 0, 1023, 0, 256);
      outA7 = map(outA7, 0, 1023, 0, 256);

      analogWrite(pinD3, outA6);
      analogWrite(pinD11, outA7);
      display.println(F("COMPARE"));
      break;

    case 4://min max
      digitalWrite(18, HIGH);//hand coding
      digitalWrite(17, LOW);//hand coding

      outA6 = analogRead(6);
      outA7 = analogRead(7);

      high = max(outA6, outA7);
      low = min(outA6, outA7);

      analogWrite(D9pin, map(high, 0, 1023, 0, LSB_resolution - 1));
      analogWrite(D10pin, map(low, 0, 1023, 0, LSB_resolution - 1));
      analogWrite(D3pin, map(outA6, 0, 1023, 0, LSB_resolution - 1));
      analogWrite(D11pin, map(outA7, 0, 1023, 0, LSB_resolution - 1));
      display.println(F("MIN - MAX"));
      break;

    case 5:
      digitalWrite(19, HIGH);//hand coding
      digitalWrite(18, LOW);//hand coding

      A7Value = analogRead(7);
      A6Value = analogRead(6);

      if (A7Value >= 511) {
        A7Value = HIGH;
      } else {
        A7Value = LOW;
      }

      if (A6Value >= 511) {
        A6Value = HIGH;
      } else {
        A6Value = LOW;
      }

      if (A7Value != A7LastValue && A7Value == HIGH) {
        digitalWrite(D10, !digitalRead(D10));
      }
      if (A6Value != A6LastValue && A6Value == HIGH) {
        digitalWrite(D9, !digitalRead(D9));
      }

      digitalWrite(D11, A7Value);
      digitalWrite(D3, A6Value);

      A7LastValue = A7Value;
      A6LastValue = A6Value;
      display.println(F("TOGGLE"));
  }
  // Update the display
  display.display();


/*
   // Add other debug information as needed
  Serial.print("ValueA6: ");
  Serial.println(valueA6);
  Serial.print("ValueA7: ");
  Serial.println(valueA7);
  Serial.print("OutA6: ");
  Serial.println(outA6);
  Serial.print("OutA7: ");
  Serial.println(outA7);

  */
  
}
