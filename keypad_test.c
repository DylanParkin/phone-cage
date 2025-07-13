#include <Keypad.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

const byte ROWS = 4, COLS = 4;

char keymap[ROWS][COLS] = {{'1', '2', '3', 'A'},
                           {'4', '5', '6', 'B'},
                           {'7', '8', '9', 'C'},
                           {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {0, 1, 2, 3};
byte colPins[COLS] = {4, 5, 6, 7};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int main() {
  Serial.begin(9600);

  while (1) {
    char key = keypad.getKey();
    if (key) Serial.println(key);
  }
}
