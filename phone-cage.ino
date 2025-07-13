#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <util/delay.h>

int main() {
  DDRB |= (1 << DDB4);

  while (1) {
    PORTB |= (1 << PB4);
    _delay_ms(1000);
    PORTB &= ~(1 << PB4);
    _delay_ms(1000);
  }
}