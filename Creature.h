#ifndef CREATURE_H
#define CREATURE_H

// Constants for creature animation
const uint8_t creatureSpriteWidth = 16;   // Largura do sprite
const uint8_t creatureSpriteHeight = 16;  // Altura do sprite
const uint8_t creatureNumFrames = 3;      // Número de frames na animação

enum HumorCreature {
  POSITIVE,
  NEGATIVE,
  NONE
};

enum SelectedStatus {
  HAPPINESS,
  EDUCATION,
  HUNGER,
  THIRST,
  WEIGHT,
  AGE,
  TEMPERATURE,
  NO_STATUS
};

const char* statusNames[] = {
  "Happiness",
  "Education",
  "Hunger",
  "Thirst",
  "Weight",
  "Age",
  "Temperature"
};

// https://xantorohara.github.io/led-matrix-editor/

const PROGMEM uint8_t creatureMatSprites[][4][10] = {
  {
    { 8,
      8,
      0b00111111,
      0b00111111,
      0b11110000,
      0b11110000,
      0b11000000,
      0b11000110,
      0b11000110,
      0b11000110 },
    { 8,
      8,
      0b11111100,
      0b11111100,
      0b00001111,
      0b00001111,
      0b00000011,
      0b01100011,
      0b01100011,
      0b01100011 },
    { 8,
      8,
      0b11000110,
      0b11000110,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11111111,
      0b11111111 },
    { 8,
      8,
      0b01100011,
      0b01100011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b11111111,
      0b11111111 },
  },
  // 2
  {
    { 8,
      8,
      0b00111111,
      0b00111111,
      0b11110000,
      0b11110000,
      0b11000000,
      0b11000000,
      0b11011110,
      0b11011110 },
    { 8,
      8,
      0b11111100,
      0b11111100,
      0b00001111,
      0b00001111,
      0b00000011,
      0b00000011,
      0b01111011,
      0b01111011 },
    { 8,
      8,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11111111,
      0b11111111 },
    { 8,
      8,
      0b00000011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b11111111,
      0b11111111 },
  },
  // 3
  {
    { 8,
      8,
      0b00000000,
      0b00111111,
      0b00111111,
      0b11110000,
      0b11110000,
      0b11000000,
      0b11000110,
      0b11000110 },
    { 8,
      8,
      0b00000000,
      0b11111100,
      0b11111100,
      0b00001111,
      0b00001111,
      0b00000011,
      0b01100011,
      0b01100011 },
    { 8,
      8,
      0b11000110,
      0b11000110,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11000000,
      0b11111111,
      0b11111111 },
    { 8,
      8,
      0b01100011,
      0b01100011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b00000011,
      0b11111111,
      0b11111111 },
  }
};

const PROGMEM uint8_t humorIcons[][4][10] = {
  // Positive
  { { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000010,
      0b00000001,
      0b00000000 },
    { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b10100000,
      0b10100100,
      0b00001000,
      0b01100000,
      0b10010000 },
    { 8, 8,
      0b00111000,
      0b00111111,
      0b00111000,
      0b00111000,
      0b00111000,
      0b00111111,
      0b00111000,
      0b00000000 },
    { 8, 8,
      0b10010000,
      0b10001100,
      0b00000100,
      0b00000100,
      0b00000100,
      0b11000100,
      0b00111000,
      0b00000000 } },
  // Negative
  { { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000010,
      0b00000001,
      0b00000000 },
    { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b10100000,
      0b10100100,
      0b00001000,
      0b01100000,
      0b10010000 },
    { 8, 8,
      0b00111000,
      0b00111111,
      0b00111000,
      0b00111000,
      0b00111000,
      0b00111111,
      0b00111000,
      0b00000000 },
    { 8, 8,
      0b10010000,
      0b10001100,
      0b00000100,
      0b00000100,
      0b00000100,
      0b11000100,
      0b00111000,
      0b00000000 } }
};

const PROGMEM uint8_t caressSprite[][10] = {
  { 8, 8,
    0b00000000,
    0b00000000,
    0b01110000,
    0b01110000,
    0b01111111,
    0b01111111,
    0b01110000,
    0b01110000 },
  { 8, 8,
    0b00111000,
    0b01100100,
    0b11000100,
    0b10011000,
    0b00011000,
    0b00000100,
    0b00000100,
    0b00011000 },
  { 8, 8,
    0b01110000,
    0b01110000,
    0b01110000,
    0b01111111,
    0b01111111,
    0b01110000,
    0b00000000,
    0b00000000 },
  { 8, 8,
    0b00011000,
    0b00000100,
    0b00000100,
    0b00011000,
    0b10011000,
    0b11000100,
    0b01100100,
    0b00111000 }
};

const PROGMEM uint8_t glassOfWater[][10] = {
  { 8, 8,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000001,
    0b00000011,
    0b00000110 },
  { 8, 8,
    0b00000000,
    0b00110000,
    0b01100111,
    0b11001000,
    0b11010010,
    0b00100110,
    0b01011100,
    0b10011000 },
  { 8, 8,
    0b00011001,
    0b00001100,
    0b00000110,
    0b00000011,
    0b00000001,
    0b00000000,
    0b00000000,
    0b00000000 },
  { 8, 8,
    0b01100000,
    0b11000000,
    0b10000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000 }
};

const PROGMEM uint8_t statusSprites[][4][10] = {
  // Happiness
  { { 8, 8,
      0b00000000,
      0b00000000,
      0b00000111,
      0b00001000,
      0b00010110,
      0b00100110,
      0b00100000,
      0b00100100 },
    { 8, 8,
      0b00000000,
      0b00000000,
      0b11100000,
      0b00010000,
      0b01101000,
      0b01100100,
      0b00000100,
      0b00100100 },
    { 8, 8,
      0b00100100,
      0b00100100,
      0b00100011,
      0b00010000,
      0b00001111,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8, 8,
      0b00100100,
      0b00100100,
      0b11000100,
      0b00001000,
      0b11110000,
      0b00000000,
      0b00000000,
      0b00000000 } },
  // Education
  {
    { 8, 8,
      0b00000000,
      0b00111110,
      0b01000001,
      0b01000000,
      0b01011110,
      0b01000000,
      0b01011110,
      0b01000000 },
    { 8, 8,
      0b00000000,
      0b01111100,
      0b10000010,
      0b00000010,
      0b01111010,
      0b00000010,
      0b01111010,
      0b00000010 },
    { 8, 8,
      0b01011110,
      0b01000000,
      0b01011110,
      0b01000000,
      0b00111110,
      0b00000001,
      0b00000000,
      0b00000000 },
    { 8, 8,
      0b01111010,
      0b00000010,
      0b01111010,
      0b00000010,
      0b01111100,
      0b10000000,
      0b00000000,
      0b00000000 },
  },
  //Hunger
  {
    { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000011,
      0b00000100,
      0b00001000 },
    { 8, 8,
      0b00000000,
      0b10100000,
      0b01010000,
      0b10100000,
      0b00000000,
      0b11000000,
      0b00100000,
      0b00010000 },
    { 8, 8,
      0b00010000,
      0b00110000,
      0b00101111,
      0b00100000,
      0b00100000,
      0b00010000,
      0b00001111,
      0b00000000 },
    { 8, 8,
      0b00001000,
      0b00001100,
      0b11110100,
      0b00000100,
      0b00000100,
      0b00001000,
      0b11110000,
      0b00000000 },
  },
  //Thirst
  {
    { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00001000,
      0b00001000,
      0b00001010,
      0b00001101,
      0b00001000 },
    { 8, 8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00010000,
      0b00010000,
      0b01010000,
      0b10110000,
      0b00010000 },
    { 8, 8,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00000111,
      0b00000000 },
    { 8, 8,
      0b00010000,
      0b01010000,
      0b01010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11100000,
      0b00000000 },
  },
};


#endif
