#ifndef FOODS_H
#define FOODS_H

#include <Arduboy2.h>

enum SelectedLunch {
  HAMBURGER,
  MEAT,
  VEGGIES,
  FRUIT,
  PASTA,
  ICECREAM,
  NO_LUNCH
};

// https://xantorohara.github.io/led-matrix-editor/

const PROGMEM uint8_t foodSprites[6][4][10] = {
  // HAMBURGER
  { { 8, 8,
      0b00000000,
      0b00001111,
      0b00010000,
      0b00100000,
      0b01000000,
      0b01000000,
      0b01000000,
      0b11111111 },
    { 8, 8,
      0b00000000,
      0b11110000,
      0b00001000,
      0b00000100,
      0b00000010,
      0b00000010,
      0b00000010,
      0b11111111 },
    { 8, 8,
      0b01000000,
      0b11111111,
      0b01011101,
      0b01000000,
      0b01000000,
      0b00111111,
      0b00000000,
      0b00000000 },
    { 8, 8,
      0b00000010,
      0b11111111,
      0b10111010,
      0b00000010,
      0b00000010,
      0b11111100,
      0b00000000,
      0b00000000 } },
  // MEAT
  { { 8, 8,
      0b00000000,
      0b00000000,
      0b01111000,
      0b10000110,
      0b10000001,
      0b10000000,
      0b10000000,
      0b10000000 },
    { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b10000000,
      0b01000000,
      0b00110000 },
    { 8, 8,
      0b01100000,
      0b00111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8, 8,
      0b00011011,
      0b11110111,
      0b00011101,
      0b00000101,
      0b00000111,
      0b00000000,
      0b00000000,
      0b00000000 } },
  // VEGGIES
  { { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000001,
      0b00000010,
      0b00000100,
      0b00001000 },
    { 8, 8,
      0b01110000,
      0b01010000,
      0b10011100,
      0b10000100,
      0b11100100,
      0b00111000,
      0b00100000,
      0b01000000 },
    { 8, 8,
      0b00010000,
      0b00100001,
      0b00100010,
      0b00100100,
      0b00101000,
      0b00110000,
      0b00000000,
      0b00000000 },
    { 8, 8,
      0b10000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 } },
  // FRUIT
  { { 8, 8,
      0b00000011,
      0b00000100,
      0b00000100,
      0b00000010,
      0b00000111,
      0b00001000,
      0b00010000,
      0b00010000 },
    { 8, 8,
      0b10000000,
      0b10000000,
      0b10000000,
      0b10000000,
      0b11000000,
      0b00110000,
      0b00010000,
      0b00010000 },
    { 8, 8,
      0b00010000,
      0b00010000,
      0b00001000,
      0b00000100,
      0b00000011,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8, 8,
      0b00010000,
      0b00010000,
      0b00100000,
      0b01100000,
      0b11000000,
      0b00000000,
      0b00000000,
      0b00000000 } }
};


#endif
