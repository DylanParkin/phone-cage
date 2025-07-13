#include <Keypad.h>
#include <TimeLib.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define N 0
#define A 1
#define B 2
#define C 3

void set_servo_angle(uint8_t angle);

const byte ROWS = 4, COLS = 4;

char keymap[ROWS][COLS] = {{'1', '2', '3', 'A'},
                           {'4', '5', '6', 'B'},
                           {'7', '8', '9', 'C'},
                           {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {0, 1, 2, 3};
byte colPins[COLS] = {4, 5, 6, 7};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int state;

void new_state(int);

int main() {
  Serial.begin(9600);

  setTime(12, 0, 0, 1, 1, 2025);  // Set time to 12:00:00 on Jan 1, 2025

  // Set PB1 (pin 9) as output
  DDRB |= (1 << DDB1);

  // Fast PWM, Mode 14: ICR1 is TOP
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);  // Prescaler = 8

  ICR1 = 39999;  // TOP for 20ms period (50 Hz)

  while (1) {
    char key_in = keypad.getKey();

    new_state(N);  // initial state is N

    switch (state) {
      case N:  // clock only mode
        if (key_in == *("A")) new_state(A);
        if (key_in == *("B")) new_state(B);
        if (key_in == *("C")) new_state(C);

        break;
      case A:  // Clock + alarm

        if (key_in == *("B")) new_state(B);
        break;

      case B:  // clock + lock
        if (key_in == *("C")) new_state(C);
        break;

      case C:  // clock + alarm + lock
        if (key_in == *("C")) new_state(C);
        break;
    }

    set_servo_angle(90);  // 1 ms
    Serial.print("OCR1A = ");
    Serial.println(OCR1A);

    return 0;
  }
}

void new_state(int ns) {
  // This function transitions to a new state, and reports that new state.
  state = ns;
  report_state();
}

void set_servo_angle(uint8_t angle) {
  OCR1A = 2000 + ((uint32_t)angle * 2000) / 180;
}