#include <Arduboy2.h>
#include "Menus.h"
#include "Creature.h"
#include "Foods.h"
#include "Play.h"

Arduboy2 arduboy;

Menu               currentMenu        = DRINK;
SelectedSwitch     selectedSwitch     = MENU_TURN_ON;
HumorCreature      humorCreature      = NONE;
SelectedLunch      selectedLunch      = HAMBURGER;
SelectedStatus     selectedStatus     = HAPPINESS;
SelectedPlay       selectedPlay       = ROCK;
SelectedPlay       selectedDinoPlay   = NONE_PLAY;

// Array associating the menu enum with the corresponding sprite
const uint8_t* menuSprites[] = {
  juiceSprite,           // DRINK
  hamburgerSprite,       // FOOD
  lightbulbSprite,       // LIGHT
  heartSprite,           // CARESS
  clipboardSprite,       // STATUS
  baseballBatSprite,     // PLAY
  bookSprite,            // STUDY
  showerSprite,          // SHOWER
  airConditionerSprite,  // CLIMATE
  firstAidKitSprite,     // HEAL
};

const uint8_t spriteWidth           = 8;            // Width of the sprite
const uint8_t spriteHeight          = 8;            // Height of the sprite
const uint8_t creatureFrameDuration = 10;           // Duration of each frame in update frames
const unsigned long frameDuration   = 1000;         // One second
const unsigned long humorDuration   = 10000;

// Calculate the x and y coordinates to center the sprite on the screen
uint8_t spriteX                    = (arduboy.width() - spriteWidth) / 2;
uint8_t spriteY                    = (arduboy.height() - spriteHeight) / 2;

uint8_t creatureCurrentFrame       = 0;            // Index of the current frame
uint8_t currentFrame               = 0;
unsigned long lastFrameTime        = 0;
unsigned long previousTime;

// Creature status
uint8_t happiness       = 1;  // Sprite
uint8_t education       = 1;  // Sprite
uint8_t hunger          = 1;  // Sprite
uint8_t thirst          = 1;  // Sprite
uint8_t weight          = 1;  // Text
uint8_t age             = 0;  // Text
uint8_t temperature     = 25; // Text

uint16_t playMenuResult = 0;
bool hasPlayResult      = false;
int8_t lastPlayResult   = 0;

bool isStartScreen      = true;
bool isControlsBlocked  = false;
bool isBackDinoBlocked  = false;
bool isMenuSelected     = false;
bool isLightOn          = true;
bool isDoingCaress      = false;
bool isDrinkingWater    = false;
bool isInFoodMenu       = false;


void setup() {
  arduboy.begin();
  arduboy.setFrameRate(15);
  randomSeed(millis());
  previousTime = millis();
}


void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }
  
  arduboy.pollButtons();

  if(isStartScreen) {
    arduboy.clear();
    drawStartScreen(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
    arduboy.setCursor(60, 40);
    arduboy.print(F("Press A..."));
    arduboy.display();
    // Wait until the B button is pressed
    if (arduboy.pressed(A_BUTTON)) {
      isStartScreen = false;
    }
    return;
  }

  // Clear the screen
  arduboy.clear();

  //Draw the menus
  drawLateralMenus();

  // Calculate the vertical center position between the left menu icons
  uint8_t centerY = iconYOffset + (numIconsLeft * iconSpacing) / 2;

  // Calculate the width and height of the central rectangle
  uint8_t rectWidth = (iconXRight - iconXLeft - 10) * 0.8;
  uint8_t rectHeight = (numIconsLeft * iconSpacing - 6) * 0.9;

  // Draw the border of the central rectangle
  arduboy.drawRect(iconXLeft + 15, centerY - rectHeight / 2 + 3, rectWidth + 10, rectHeight, WHITE);

  // Draw the name of the selected menu
  arduboy.setCursor(iconXLeft + 42, centerY - rectHeight / 2 - 4);
  arduboy.print(menuNames[currentMenu]);

  // Draw a square inside the central rectangle
  if (isMenuSelected) {
    drawSelectedMenu(iconXLeft + 15, centerY - rectHeight / 2 + 3, rectWidth + 10, rectHeight);
    navigateLightSwitches();
    navigateSelectedLunch();
    navigateSelectedStatus();
    navigateSelectedPlay();
  }

  // Execute the controls function
  executeControls();

  // Draw the creature in the center of the central rectangle
  drawCreature(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Draw the creature's status
  drawHumorCreature(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Draw an action with the creature
  drawDoCaress(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDrinkWater(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Update the creature's animation
  unsigned long currentTime = millis();
  if (arduboy.everyXFrames(creatureFrameDuration)) {
    if (currentTime - lastFrameTime >= frameDuration) {
      updateCreatureAnimation();
      currentFrame = (currentFrame + 1) % 3;
      lastFrameTime = currentTime;
    }
  }

  // Check if 3 seconds have passed since the last state change and reset the mood.
  resetHumor(currentTime);

  arduboy.display();
}

// Function to execute the controls.
void executeControls() {
  if(isControlsBlocked) {
    return;
  }

  // Check if the left button was pressed to navigate between the left menus
  if (arduboy.justPressed(LEFT_BUTTON) && !isMenuSelected) {
    if (currentMenu >= (numMenus / 2)) {
      currentMenu = DRINK - 1;
    }
    currentMenu = static_cast<Menu>((currentMenu + 1) % (numMenus / 2));
    emitBeep();
  }

  // Check if the right button was pressed to navigate between the right menus
  if (arduboy.justPressed(RIGHT_BUTTON) && !isMenuSelected) {
    if (currentMenu < (numMenus / 2)) {
      currentMenu = PLAY - 1;
    }
    currentMenu = static_cast<Menu>((currentMenu + 1) % (numMenus / 2) + (numMenus / 2));
    emitBeep();
  }

  if (arduboy.justPressed(B_BUTTON)) {
    isMenuSelected = true;
  }

  if (arduboy.justPressed(A_BUTTON) && (isBackDinoBlocked == false)) {
    isMenuSelected = false;
  }
}

// Function to reset the mood after a certain duration
void resetHumor(long currentTime) {
  if (currentTime - previousTime >= humorDuration) {
    // Toggle the variable state
    humorCreature = NONE;
    // Update the previous time to the current time
    previousTime = currentTime;
    // Caress already done.
    isDoingCaress = false;
    // Drank water
    isDrinkingWater = false;
    // Unblock controls.
    isControlsBlocked = false;
  }
}

// Function to draw the lateral menus
void drawLateralMenus() {
  // Draw the left menu icons
  drawMenuIcons(iconXLeft, numIconsLeft, 0);
  // Draw the right menu icons
  drawMenuIcons(iconXRight, numIconsRight, numIconsLeft);
}

// Function to draw the selected menu
void drawSelectedMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  switch (currentMenu) {
    case DRINK:   doDrinkingWater();                                  break;
    case FOOD:    drawFoodMenu(rectX, rectY);                         break;
    case LIGHT:   drawLightMenu(rectX, rectY, rectWidth, rectHeight); break;
    case CARESS:  doCaress();                                         break;
    case STATUS:  drawStatusMenu(rectX, rectY);                       break;
    case PLAY:    drawPlayMenu(rectX, rectY);                         break;
    default: return;
  }
}

void doCaress() {
  previousTime = millis();
  isControlsBlocked = true;

  isDoingCaress = true;
  humorCreature = POSITIVE;
}

void doDrinkingWater() {
  previousTime = millis();
  isControlsBlocked = true;

  if(thirst < 4) {
    isDrinkingWater = true;
    thirst++;
  } else {
    humorCreature = NEGATIVE;
    isMenuSelected = false;
  }
}

// Function to draw the status menu
void drawStatusMenu(uint8_t x, uint8_t y) {
  // Adjust the position to draw the sprite
  uint8_t creatPosArr[4][2];
  
  // Draw the name of the selected status
  arduboy.setCursor(x + 5, y + 2);
  arduboy.print(statusNames[selectedStatus]);

  if(selectedStatus < WEIGHT) {
    drawStatusByIcons(x, y, creatPosArr);
  } else {
    drawStatusByText(x, y);
  }
}

void drawStatusByText(uint8_t x, uint8_t y) {
  char text[10];
  switch (selectedStatus) {
      case WEIGHT:
        sprintf(text, "%d Kg", weight);
        break;
      case AGE:
        sprintf(text, "%d Days", age);
        break;
      case TEMPERATURE:
        sprintf(text, "%d %cC", temperature, (char)167);
        break;
      default:
        return;
  }
  arduboy.setCursor(x + 30, y + 20);
  arduboy.print(text);
}

void drawStatusByIcons(uint8_t x, uint8_t y, uint8_t creatPosArr[][2]) {
  uint8_t counter; 
  switch (selectedStatus) {
    case HAPPINESS:
      counter = happiness;
      break;
    case EDUCATION:
      counter = education;
      break;
    case HUNGER:
      counter = hunger;
      break;
    case THIRST:
      counter = thirst;
      break;
    default:
      return;
  }

  x += 20; y += 10;
  drawStatusIconByCount(x, y, creatPosArr, counter, 0);
  x += 30; y += 0;
  drawStatusIconByCount(x, y, creatPosArr, counter, 1);
  x -= 30; y += 15;
  drawStatusIconByCount(x, y, creatPosArr, counter, 2);
  x += 30; y += 0;
  drawStatusIconByCount(x, y, creatPosArr, counter, 3);
}

void drawStatusIconByCount(uint8_t x, uint8_t y, uint8_t creatPosArr[][2], uint8_t counter, uint8_t value) {
  if(counter > value) {
    buildCreatPosArr(x, y, creatPosArr);
    drawStatusSprites(creatPosArr);
  }
}

// Function to draw creature sprites
void drawStatusSprites(uint8_t creatPosArr[][2]) {
  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i], statusSprites[selectedStatus][i], 10, 10);
  }
}

// Function to draw the play menu
void drawPlayMenu(uint8_t x, uint8_t y) {
  // Adjust the position to draw the sprite
  x += 20;
  y += 15;

  arduboy.setCursor(x, y - 10);
  arduboy.print(F("You"));

  arduboy.setCursor(x + 30, y - 10);
  arduboy.print(F("Dino"));

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , playSprites[selectedPlay][i], 10, 10);
  }

  if(selectedDinoPlay != NONE_PLAY) {
    x += 30;
    buildCreatPosArr(x, y, creatPosArr);
    for (uint8_t i = 0; i < 4; i++) {
      drawCustomBitmap(creatPosArr[i] , playSprites[selectedDinoPlay][i], 10, 10);
    }
  }

  int16_t result;
  if (arduboy.justPressed(B_BUTTON)) {
    initPlayGame();
    selectedDinoPlay = getDinoMove();
    lastPlayResult = determineWinner(selectedPlay, selectedDinoPlay);
    hasPlayResult = true;

    // regra: dino feliz quando vence
    if (lastPlayResult == -1 && happiness < 4) happiness++;
    if (lastPlayResult ==  1 && happiness > 0) happiness--; // opcional: perder te deixa triste
  }

  if (hasPlayResult) {
    arduboy.setCursor(x, y);
    arduboy.print(lastPlayResult);
    // ... seu switch que imprime O/X/-
  }

}

void initPlayGame() {
  if(isBackDinoBlocked == true) {
    return;
  }
  playMenuResult = 0;
  isBackDinoBlocked = true;
}

// Get a random move for dino play.
SelectedPlay getDinoMove() {
  return static_cast<SelectedPlay>(random(3));
}

// Function to determine the winner of the Rock, Paper, Scissors game
// Parameters:
//   - selectedPlay: Move of the first player (of type Game)
//   - selectedDinoPlay: Move of the second player (of type Game)
// Return:
//   - Returns 1 if the first player wins
//   - Returns -1 if the second player wins
//   - Returns 0 if there is a tie
//   - Returns -2 in case of an unexpected error (value out of expected range)
int8_t determineWinner(SelectedPlay selectedPlay, SelectedPlay selectedDinoPlay) {
  if (selectedPlay == selectedDinoPlay) return 0;

  switch (selectedPlay) {
    case ROCK:     return (selectedDinoPlay == SCISSORS) ?  1 : -1;
    case PAPER:    return (selectedDinoPlay == ROCK)    ?  1 : -1;
    case SCISSORS: return (selectedDinoPlay == PAPER)   ?  1 : -1;
    default:       return -2;
  }
}

// Function to draw the food menu
void drawFoodMenu(uint8_t x, uint8_t y) {
  // Adjust the position to draw the sprite
  x += 30;
  y += 15;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , foodSprites[selectedLunch][i], 10, 10);
  }
}

// Function to draw the light menu
void drawLightMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  // Define the coordinates of the text menus
  uint8_t menuX1 = rectX + 10;
  uint8_t menuX2 = rectX + 10;
  uint8_t menuY1 = rectY + rectHeight / 2 - 10;  // Adjust the vertical height for the first menu
  uint8_t menuY2 = menuY1 + 20;                  // Set the vertical position of the second menu below the first

  // Draw the text menus
  arduboy.setCursor(menuX1, menuY1);
  arduboy.print(F("Turn On"));
  if(selectedSwitch == MENU_TURN_ON) {
    arduboy.drawRect(menuX1 - 2, menuY1 - 2, arduboy.getCharacterWidth() * 7 + 10, 12 + 4, WHITE);  // Draw the border around the "Turn On" menu
  }

  arduboy.setCursor(menuX2, menuY2);
  arduboy.print(F("Turn Off"));
  if(selectedSwitch == MENU_TURN_OFF) { 
    arduboy.drawRect(menuX2 - 2, menuY2 - 2, arduboy.getCharacterWidth() * 8 + 12, 12 + 2, WHITE);  // Draw the border around the "Turn Off" menu
  }

  if(arduboy.justPressed(B_BUTTON)) {
    if(selectedSwitch == MENU_TURN_ON) {
      isLightOn = true;
    } else {
      isLightOn = false;
    }
    arduboy.pollButtons();
    isMenuSelected = false;
  }
  if(arduboy.justPressed(A_BUTTON)) {
    selectedSwitch = isLightOn ? MENU_TURN_ON : MENU_TURN_OFF;
  }
}

// Function to navigate between the selected lunch options
void navigateSelectedLunch() {
  if(currentMenu != FOOD) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
      selectedLunch = (selectedLunch == NO_LUNCH - 1) ? HAMBURGER : selectedLunch + 1;
      emitBeep();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedLunch = (selectedLunch == HAMBURGER) ? NO_LUNCH - 1 : selectedLunch - 1;
      emitBeep();
  }
}

void navigateSelectedPlay() {
  if(currentMenu != PLAY) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
      selectedPlay = (selectedPlay == SCISSORS) ? ROCK : selectedPlay + 1;
      emitBeep();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedPlay = (selectedPlay == ROCK) ? SCISSORS : selectedPlay - 1;
      emitBeep();
  }
}

// Function to navigate between the selected status options
void navigateSelectedStatus() {
  if(currentMenu != STATUS) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
      selectedStatus = (selectedStatus == NO_STATUS - 1) ? HAPPINESS : selectedStatus + 1;
      emitBeep();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedStatus = (selectedStatus == HAPPINESS) ? NO_STATUS - 1 : selectedStatus - 1;
      emitBeep();
  }
}

// Function to navigate between the light menus
void navigateLightSwitches() {
    if(currentMenu != LIGHT) {
      return;
    }
    if (arduboy.justPressed(DOWN_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        emitBeep();
    } else if (arduboy.justPressed(UP_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        emitBeep();
    }
}

// Function to navigate between the lunch options
void navigateLunches() {
    if(currentMenu != LIGHT) {
      return;
    }
    if (arduboy.justPressed(DOWN_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        emitBeep();
    } else if (arduboy.justPressed(UP_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        emitBeep();
    }
}

// Function to draw the text menus
void drawTextMenus(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  // Define the coordinates of the text menus
  uint8_t menuX1 = rectX + 10;
  uint8_t menuX2 = rectX + 10;
  uint8_t menuY1 = rectY + rectHeight / 2 - 10;  // Adjust the vertical height for the first menu
  uint8_t menuY2 = menuY1 + 12;                  // Set the vertical position of the second menu below the first

  // Draw the text menus
  arduboy.setCursor(menuX1, menuY1);
  arduboy.print(F("Turn On"));

  arduboy.setCursor(menuX2, menuY2);
  arduboy.print(F("Turn Off"));
}


// Function to draw the start screen.
void drawStartScreen(uint8_t x, uint8_t y) {
  uint8_t startScrSprSize = 4;

  // Adjust the position to draw the sprite
  x -= 30;
  y -= 10;

  for (uint8_t spr = 0; spr < startScrSprSize; spr++) { 
    uint8_t creatPosArr[4][2];
    buildCreatPosArr(x, y, creatPosArr);

    for (uint8_t i = 0; i < 4; i++) {
      drawCustomBitmap(creatPosArr[i] , startScreenSprites[spr][i], 10, 10);
    }
    x += 16;
  }
}

// Function to draw the creature
void drawCreature(uint8_t x, uint8_t y) {
  if (isMenuSelected) {
    return;
  }

  // Adjust the position to draw the sprite
  x -= 10;
  y -= 10;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , creatureMatSprites[creatureCurrentFrame][i], 10, 10);
  }
}

void drawHumorCreature(uint8_t x, uint8_t y) {
  if (isMenuSelected || humorCreature == NONE) {
    return;
  }

  // Adjust the position to draw the sprite
  x += 15;
  y -= 10;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , humorIcons[humorCreature][i], 10, 10);
  }
}

void drawDoCaress(uint8_t x, uint8_t y) {
  if(isDoingCaress == false) {
    return;
  }

  // Adjust the position to draw the sprite
  x -= 25;
  y -= 10 + creatureCurrentFrame;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , caressSprite[i], 10, 10);
  }

  humorCreature = POSITIVE;
  isMenuSelected = false;
}

void drawDrinkWater(uint8_t x, uint8_t y) {
  if(isDrinkingWater == false) {
    return;
  }

  // Adjust the position to draw the sprite
  x -= 30;
  y -= 10 + creatureCurrentFrame;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , glassOfWater[i], 10, 10);
  }

  isMenuSelected = false;
}

void buildCreatPosArr(uint8_t x, uint8_t y, uint8_t creatPosArr[][2]) {
    creatPosArr[0][0] = x;
    creatPosArr[0][1] = y;
    creatPosArr[1][0] = x + 8;
    creatPosArr[1][1] = y;
    creatPosArr[2][0] = x;
    creatPosArr[2][1] = y + 8;
    creatPosArr[3][0] = x + 8;
    creatPosArr[3][1] = y + 8;
}

// Function to update the creature's animation
void updateCreatureAnimation() {
  if (isMenuSelected) {
    return;
  }

  // Increment the current frame counter
  creatureCurrentFrame++;

  // Check if it's time to switch to the next frame
  if (creatureCurrentFrame >= creatureNumFrames) {
    creatureCurrentFrame = 0;  // Restart the animation
  }
}

// Function to emit a beep
void emitBeep() {
  // Turn on the speaker for a short period to produce a beep
  tone(5, 1000, 50);  // Play a 1000Hz sound for 50ms
  delay(10);          // Small delay to ensure the beep completes fully
}

// Function to draw the menu icons
void drawMenuIcons(uint8_t x, uint8_t numIcons, uint8_t startMenu) {
  for (uint8_t i = 0; i < numIcons; i++) {
    Menu menu = static_cast<Menu>(startMenu + i);

    // Draw the outline around the selected icon
    if (currentMenu == menu) {
      arduboy.drawRect(x, iconYOffset + i * iconSpacing - 2, iconWidth + 4, iconHeight + 4, WHITE);
    }

    // Draw the icons
    arduboy.setCursor(x + 4, iconYOffset + i * iconSpacing + 4);
    drawCustomBitmap(x, iconYOffset + i * iconSpacing, menuSprites[menu], iconWidth, iconHeight);
  }
}

void drawCustomBitmap(const uint8_t* posXaY, const uint8_t* sprite, uint8_t width, uint8_t height) {
  drawCustomBitmap(posXaY[0], posXaY[1], sprite, width, height);
}

// Function to draw a sprite on the screen, allowing adjustments in the columns
void drawCustomBitmap(uint8_t x, uint8_t y, const uint8_t* sprite, uint8_t width, uint8_t height) {
  uint8_t color = WHITE;
  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      // Check if the corresponding bit is set in the sprite
      if (pgm_read_byte_near(sprite + row) & (1 << (width - col - 1))) {
        if (row < 2) {
          color = BLACK;
        }
        arduboy.drawPixel(x + col, y + row, color);
        color = WHITE;
      }
    }
  }
}
