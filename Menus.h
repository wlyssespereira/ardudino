// Menus.h

#ifndef MENUS_H
#define MENUS_H

#include <Arduboy2.h>

// Constantes relacionadas aos ícones do menu
const uint8_t numIconsLeft = 5;
const uint8_t numIconsRight = 5;
const uint8_t iconWidth = 8;
const uint8_t iconHeight = 8;
const uint8_t iconSpacing = 12;
const uint8_t iconXLeft = 5;
const uint8_t iconXRight = 113;
const uint8_t iconYOffset = 2;
const uint8_t numMenus = 10;

// Variáveis para o retângulo central
const uint8_t rectX = 30;
const uint8_t rectY = 20;
const uint8_t rectWidth = 68;
const uint8_t rectHeight = 28;

// Enumeração dos menus
enum Menu {
  DRINK,
  FOOD,
  LIGHT,
  CARESS,
  STATUS,
  // Half
  PLAY,
  STUDY,
  SHOWER,
  CLIMATE,
  HEAL
};

// Enumeração para os menus selecionáveis
enum SelectedSwitch {
  MENU_TURN_ON,
  MENU_TURN_OFF
};

// Declarando um array de strings para os nomes dos menus
const char* menuNames[] = {
  "Drink",    // Drink
  "Food",     // FOOD
  "Light",    // LIGHT
  "Caress",   // CARESS
  "Status",   // STATUS
  "Play",     // PLAY
  "Study",    // STUDY
  "Shower",   // SHOWER
  "Climate",  // CLIMATE
  "Heal"      // HEAL
};


// https://xantorohara.github.io/led-matrix-editor/


const PROGMEM uint8_t juiceSprite[] = {
  8,
  8,
  0b00000010,
  0b00100011,
  0b00100101,
  0b00101001,
  0b00100001,
  0b00011110,
  0b00000000,
  0b00000000,
};

const PROGMEM uint8_t hamburgerSprite[] = {
  8,
  8,
  0b0000011110,
  0b0000100001,
  0b0000010010,
  0b0000100001,
  0b0000011110,
  0b0000000000,
};

const PROGMEM uint8_t lightbulbSprite[] = {
  8,
  8,
  0b0000000100,
  0b0000000100,
  0b0000000100,
  0b0000001010,
  0b0000010101,
  0b0000000000,
};

const PROGMEM uint8_t heartSprite[] = {
  8,
  8,
  0b0000110011,
  0b0000101101,
  0b0000100001,
  0b0000010010,
  0b0000001100,
  0b0000000000,
};

const PROGMEM uint8_t clipboardSprite[] = {
  8,
  8,
  0b0000111111,
  0b0000100001,
  0b0000100001,
  0b0000111111,
  0b0000100001,
  0b0000111111,
};

const PROGMEM uint8_t baseballBatSprite[] = {
  8,
  8,
  0b0000110011,
  0b0000110111,
  0b0000000110,
  0b0000001000,
  0b0000010000,
  0b0000110000,
};

const PROGMEM uint8_t bookSprite[] = {
  8,
  8,
  0b0000110011,
  0b0000101101,
  0b0000101001,
  0b0000101001,
  0b0000111111,
  0b0000000000,
};

const PROGMEM uint8_t showerSprite[] = {
  8,
  8,
  0b0000000100,
  0b0000001110,
  0b0000010101,
  0b0000000000,
  0b0000010101,
  0b0000010101,
};

const PROGMEM uint8_t airConditionerSprite[] = {
  8,
  8,
  0b0000111111,
  0b0000101101,
  0b0000100001,
  0b0000111111,
  0b0000010101,
  0b0000101010,
};

const PROGMEM uint8_t firstAidKitSprite[] = {
  8,
  8,
  0b0000001110,
  0b0000011111,
  0b0000010101,
  0b0000011111,
  0b0000010101,
  0b0000011111,
};


const PROGMEM uint8_t startScreenSprites[4][4][10] = {
  {
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000111,
      0b00001000,
      0b00010000,
      0b00100000,
      0b00100100 },
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11000001,
      0b00100001,
      0b00010001,
      0b00001001,
      0b01001001 },
    { 8,
      8,
      0b00100100,
      0b00100000,
      0b00100000,
      0b00111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8,
      8,
      0b01001001,
      0b00001001,
      0b00001001,
      0b11111001,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
  },
  // 2
  {
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11111111,
      0b10000000,
      0b00110011,
      0b01001010,
      0b01111011 },
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11111111,
      0b00000000,
      0b10011100,
      0b01010010,
      0b10010010 },
    { 8,
      8,
      0b01001010,
      0b01001010,
      0b10000000,
      0b11111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8,
      8,
      0b10010010,
      0b01011100,
      0b00000000,
      0b11111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
  },
  // 3
  {
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11111111,
      0b00000000,
      0b10010111,
      0b10010100,
      0b10010100 },
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11111111,
      0b00000000,
      0b00111010,
      0b10010011,
      0b10010010 },
    { 8,
      8,
      0b10010100,
      0b01100111,
      0b00000000,
      0b11111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8,
      8,
      0b10010010,
      0b00111010,
      0b00000000,
      0b11111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
  },
  // 4
  {
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11111111,
      0b00000000,
      0b01001111,
      0b01010000,
      0b11010101 },
    { 8,
      8,
      0b00000000,
      0b00000000,
      0b00000000,
      0b11110000,
      0b00110000,
      0b10010000,
      0b01010000,
      0b01010000 },
    { 8,
      8,
      0b01010000,
      0b01001111,
      0b00000000,
      0b11111111,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
    { 8,
      8,
      0b01010000,
      0b10010000,
      0b00110000,
      0b11110000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000 },
  }
};

#endif  // MENUS_H
