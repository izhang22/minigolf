#include "color.h"
#include <stdlib.h>

const int COLOR_WIDTH = 11;
const float COLOR_PRECISION = 10.0F;

// Returns random rgb_color_t from 0 to 1 with 1 significant digit
rgb_color_t get_random_color() {
  return (rgb_color_t) {(float) (rand() % COLOR_WIDTH) / COLOR_PRECISION,
    (float) (rand() % COLOR_WIDTH) / COLOR_PRECISION,
    (float) (rand() % COLOR_WIDTH) / COLOR_PRECISION};
}
