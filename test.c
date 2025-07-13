#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

void set_servo_angle(uint8_t angle);

int main() {
  Serial.begin(9600);

  // Set PB1 (pin 9) as output
  DDRB |= (1 << DDB1);

  // Fast PWM, Mode 14: ICR1 is TOP
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);  // Prescaler = 8

  ICR1 = 39999;  // TOP for 20ms period (50 Hz)

  while (1) {
    set_servo_angle(90);  // 1 ms
    Serial.print("OCR1A = ");
    Serial.println(OCR1A);
  }
}

void set_servo_angle(uint8_t angle) {
  OCR1A = 2000 + ((uint32_t)angle * 2000) / 180;
}
