#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <util/delay.h>

int main() {
  DDRB |= (1 << DDB1);

  TCCR1A |= (1 << WGM11) | (1 << COM1A1);
  TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

  ICR1 = 39999;

  while (1) {
    set_servo_angle(0);
    _delay_ms(1000);

    set_servo_angle(90);
    _delay_ms(1000);

    set_servo_angle(180);
    _delay_ms(1000);
  }
}

void set_servo_angle(uint8_t angle) {
  OCR1A = 2000 + ((uint32_t)angle * 2000) / 180;
}