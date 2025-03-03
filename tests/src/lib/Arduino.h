#ifndef Arduino_h
#define Arduino_h

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Print.h"

extern "C" {
typedef uint8_t byte;
typedef uint8_t boolean;

/* sketch */
extern void setup(void);
extern void loop(void);
unsigned long millis(void);
}

#define PROGMEM
#define strnlen_P strnlen
#define pgm_read_byte_near(x) *(x)

#define yield(x) {}

#endif  // Arduino_h
