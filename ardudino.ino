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

// Climate selection inside the Climate menu.
enum SelectedClimate {
  CLIMATE_COOL,
  CLIMATE_WARM
};

SelectedClimate selectedClimate = CLIMATE_COOL;

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

// Rock / Paper / Scissors (Play) state.
static const uint8_t kPlayTotalRounds = 5;
bool playInProgress = false;
bool playFinished   = false;
uint8_t playRound   = 0;
uint8_t playerWins  = 0;
uint8_t dinoWins    = 0;
int8_t lastPlayResult = 0;

bool isStartScreen      = true;
bool isControlsBlocked  = false;
bool isBackDinoBlocked  = false; // Kept for backwards compatibility; no longer used by Play.
bool isMenuSelected     = false;
bool isLightOn          = true;
bool isDoingCaress      = false;
bool isDrinkingWater    = false;
bool isInFoodMenu       = false;

// One-shot action flags for menus that should not be repeated every frame.
bool isStudying         = false;
bool isShowering        = false;
bool isHealing          = false;

// -----------------------------
// GAME CLOCK
// -----------------------------
uint8_t gameHour                    = 8;
uint8_t gameMinute                  = 0;
unsigned long lastMinuteTick        = 0;
const unsigned long minuteInterval  = 1000;
bool isSleeping                     = false;

const uint8_t z5x5_0[] PROGMEM = {
  B11111000,
  B00010000,
  B00100000,
  B01000000,
  B11111000
};

const uint8_t z5x5_1[] PROGMEM = {
  B00000000,
  B11111000,
  B00100000,
  B01000000,
  B11111000
};

void drawSleepZ(uint8_t x, uint8_t y) {
  uint8_t frame = (millis() / 350) % 2;
  uint8_t floatY = (millis() / 500) % 2; // sobe/desce 1px

  const uint8_t* zSpr = frame ? z5x5_1 : z5x5_0;
  drawCustomBitmapCreature(x, y - floatY, zSpr, 5, 5);
}

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

  updateClock();
  updateSleepEffects();

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
  arduboy.setCursor(iconXLeft + 40, centerY - rectHeight / 2 - 4);
  arduboy.print(menuNames[currentMenu]);

  // Fill the creature viewport when the light is OFF.
  // This affects only the central rectangle interior (not the side menus).
  if (!isMenuSelected && !isLightOn) {
    uint8_t viewportX = iconXLeft + 16;
    uint8_t viewportY = centerY - rectHeight / 2 + 4;
    uint8_t viewportW = rectWidth + 8;
    uint8_t viewportH = rectHeight - 2;
    arduboy.fillRect(viewportX, viewportY, viewportW, viewportH, WHITE);
  }

  // Draw a square inside the central rectangle
  if (isMenuSelected) {
    drawSelectedMenu(iconXLeft + 15, centerY - rectHeight / 2 + 3, rectWidth + 10, rectHeight);
    navigateLightSwitches();
    navigateSelectedLunch();
    navigateSelectedStatus();
    navigateSelectedPlay();
    navigateSelectedClimate();
  }

  // Execute the controls function
  executeControls();

  // Draw the creature in the center of the central rectangle
  drawCreature(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Draw the creature's status
  drawHumorCreature(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Render the clock
  renderClock();

  // Draw an action with the creature
  drawDoCaress(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDrinkWater(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDoStudy(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDoShower(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

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

void renderClock() {
  char buffer[6];
  sprintf(buffer, "%02d:%02d", gameHour, gameMinute);

  arduboy.setCursor(80, 0);
  arduboy.print(buffer);
}

void updateClock() {
  if (millis() - lastMinuteTick >= minuteInterval) {
    lastMinuteTick = millis();

    gameMinute++;

    if (gameMinute >= 60) {
      gameMinute = 0;
      gameHour++;

      if (gameHour >= 24) {
        gameHour = 0;
      }
    }

    // Sleep schedule
    if (gameHour == 0 && gameMinute == 0) {
      isSleeping = true;
    }

    if (gameHour == 9 && gameMinute == 0) {
      isSleeping = false;
    }
  }
}

// Function to execute the controls.
void executeControls() {
  if(isControlsBlocked) {
    return;
  }

  // Navigate menus when not inside a menu:
  // UP/DOWN = move inside the current side
  // LEFT/RIGHT = switch side (keep same index)
  if (!isMenuSelected) {

    const uint8_t half = numMenus / 2;

    // Determine current side and index inside the side
    bool onLeftSide = ((uint8_t)currentMenu < half);
    uint8_t idx = onLeftSide ? (uint8_t)currentMenu : ((uint8_t)currentMenu - half);

    // UP = previous item in the same side
    if (arduboy.justPressed(UP_BUTTON)) {
      if (idx == 0) idx = half - 1;
      else idx--;

      currentMenu = onLeftSide ? (Menu)idx : (Menu)(half + idx);
      emitBeep();
    }

    // DOWN = next item in the same side
    if (arduboy.justPressed(DOWN_BUTTON)) {
      idx = (idx + 1) % half;

      currentMenu = onLeftSide ? (Menu)idx : (Menu)(half + idx);
      emitBeep();
    }

    // RIGHT = go to right side (keep idx)
    if (arduboy.justPressed(RIGHT_BUTTON) && onLeftSide) {
      currentMenu = (Menu)(half + idx);
      emitBeep();
    }

    // LEFT = go to left side (keep idx)
    if (arduboy.justPressed(LEFT_BUTTON) && !onLeftSide) {
      currentMenu = (Menu)idx;
      emitBeep();
    }
  }

  // B selects the current menu.
  // When the light is OFF, only Status and Light menus can be opened.
  if (arduboy.justPressed(B_BUTTON)) {
    if (!isLightOn && currentMenu != STATUS && currentMenu != LIGHT) {
      emitBeep();
      return;
    }
    isMenuSelected = true;
  }

  // A is the global back/cancel.
  // Exception: Play (RPS) must be finished once started.
  if (arduboy.justPressed(A_BUTTON) && !(currentMenu == PLAY && (playInProgress || playFinished))) {
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
    // Study already done.
    isStudying = false;
    // Shower already done.
    isShowering = false;
    // Heal already done.
    isHealing = false;
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
    case STUDY:   drawStudyMenu(rectX, rectY, rectWidth, rectHeight); break;
    case SHOWER:  drawShowerMenu(rectX, rectY, rectWidth, rectHeight);break;
    case CLIMATE: drawClimateMenu(rectX, rectY, rectWidth, rectHeight);break;
    case HEAL:    drawHealMenu(rectX, rectY, rectWidth, rectHeight);  break;
    default: return;
  }
}

void doCaress() {
  // Caress should be applied once per selection, not every frame.
  if (isDoingCaress) {
    return;
  }

  previousTime = millis();
  isControlsBlocked = true;

  isDoingCaress = true;
  humorCreature = POSITIVE;
}

void doDrinkingWater() {
  // Drinking should be applied once per selection, not every frame.
  if (isDrinkingWater) {
    return;
  }

  previousTime = millis();
  isControlsBlocked = true;

  if(thirst < 4) {
    isDrinkingWater = true;
    thirst++;
    humorCreature = POSITIVE;
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

// Function to draw the play menu (Rock / Paper / Scissors)
// Rules:
//   - Always 5 rounds
//   - Once started, the player must finish (A is blocked)
//   - B starts / advances rounds
//   - After 5 rounds, B exits
void drawPlayMenu(uint8_t x, uint8_t y) {
  // Adjust the position to draw the sprite
  x += 10;
  y += 6;

  // Header / round counter
  arduboy.setCursor(x, y);
  if (!playInProgress && !playFinished) {
    arduboy.print(F("B Start"));
  } else {
    arduboy.print(F("Round "));
    arduboy.print(playRound);
    arduboy.print(F("/"));
    arduboy.print(kPlayTotalRounds);
  }

  // Labels
  arduboy.setCursor(x, y + 10);
  arduboy.print(F("You"));
  arduboy.setCursor(x + 44, y + 10);
  arduboy.print(F("Dino"));

  // Draw player move
  uint8_t creatPosArr[4][2];
  uint8_t px = x;
  uint8_t py = y + 20;
  buildCreatPosArr(px, py, creatPosArr);
  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i], playSprites[selectedPlay][i], 10, 10);
  }

  // Draw dino move when available
  if (selectedDinoPlay != NONE_PLAY) {
    px = x + 44;
    buildCreatPosArr(px, py, creatPosArr);
    for (uint8_t i = 0; i < 4; i++) {
      drawCustomBitmap(creatPosArr[i], playSprites[selectedDinoPlay][i], 10, 10);
    }
  }

  // Scoreboard
  arduboy.setCursor(x, y + 52);
  arduboy.print(F("W "));
  arduboy.print(playerWins);
  arduboy.print(F("  L "));
  arduboy.print(dinoWins);

  // Input
  if (arduboy.justPressed(B_BUTTON)) {
    if (!playInProgress && !playFinished) {
      startPlayIfNeeded();
      advancePlayRound();
    } else if (playInProgress) {
      advancePlayRound();
    } else if (playFinished) {
      // Apply mood/result once when leaving.
      if (dinoWins > playerWins) {
        if (happiness < 4) happiness++;
        humorCreature = POSITIVE;
      } else if (playerWins > dinoWins) {
        if (happiness > 0) happiness--;
        humorCreature = NEGATIVE;
      } else {
        humorCreature = NONE;
      }

      // Reset play state and exit.
      playInProgress = false;
      playFinished = false;
      playRound = 0;
      playerWins = 0;
      dinoWins = 0;
      selectedDinoPlay = NONE_PLAY;
      isMenuSelected = false;
    }
  }
}

void startPlayIfNeeded() {
  playInProgress = true;
  playFinished = false;
  playRound = 0;
  playerWins = 0;
  dinoWins = 0;
  selectedDinoPlay = NONE_PLAY;
}

void advancePlayRound() {
  if (!playInProgress) {
    return;
  }

  // Generate moves
  selectedDinoPlay = getDinoMove();
  lastPlayResult = determineWinner(selectedPlay, selectedDinoPlay);

  // Update score
  if (lastPlayResult == 1) {
    playerWins++;
  } else if (lastPlayResult == -1) {
    dinoWins++;
  }

  // Advance round counter
  if (playRound < kPlayTotalRounds) {
    playRound++;
  }

  // Finish
  if (playRound >= kPlayTotalRounds) {
    playInProgress = false;
    playFinished = true;
  }
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
  x += 10;
  y += 6;

  // Title / hint
  arduboy.setCursor(x, y);
  arduboy.print(F("B Eat"));

  // Food name
  arduboy.setCursor(x, y + 10);
  switch (selectedLunch) {
    case HAMBURGER: arduboy.print(F("Hamburger")); break;
    case MEAT:      arduboy.print(F("Meat")); break;
    case VEGGIES:   arduboy.print(F("Veggies")); break;
    case FRUIT:     arduboy.print(F("Fruit")); break;
    case PASTA:     arduboy.print(F("Pasta")); break;
    case ICECREAM:  arduboy.print(F("Ice Cream")); break;
    default:        arduboy.print(F("Food")); break;
  }

  // Sprite
  uint8_t sx = x + 20;
  uint8_t sy = y + 22;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(sx, sy, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , foodSprites[selectedLunch][i], 10, 10);
  }

  // Confirm
  if (arduboy.justPressed(B_BUTTON)) {
    applyFoodSelection();
    isMenuSelected = false;
  }
}

void applyFoodSelection() {
  previousTime = millis();
  isControlsBlocked = true;

  // If not hungry, reject food.
  if (hunger >= 4) {
    humorCreature = NEGATIVE;
    return;
  }

  // Default reaction.
  humorCreature = POSITIVE;

  switch (selectedLunch) {
    case HAMBURGER:
      if (hunger < 4) hunger++;
      if (hunger < 4) hunger++;
      if (happiness < 4) happiness++;
      break;
    case MEAT:
      if (hunger < 4) hunger++;
      if (hunger < 4) hunger++;
      break;
    case VEGGIES:
      if (hunger < 4) hunger++;
      if (happiness > 0) happiness--;
      break;
    case FRUIT:
      if (hunger < 4) hunger++;
      if (happiness < 4) happiness++;
      break;
    case PASTA:
      if (hunger < 4) hunger++;
      if (hunger < 4) hunger++;
      break;
    case ICECREAM:
      if (hunger < 4) hunger++;
      if (happiness < 4) happiness++;
      if (happiness < 4) happiness++;
      break;
    default:
      // Unknown food; do nothing.
      humorCreature = NONE;
      break;
  }
}

// Study menu: increases education, may reduce happiness a bit.
void drawStudyMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  (void)rectWidth;
  (void)rectHeight;

  uint8_t x = rectX + 8;
  uint8_t y = rectY + 8;

  arduboy.setCursor(x, y);
  arduboy.print(F("B Study"));
  arduboy.setCursor(x, y + 12);
  arduboy.print(F("Edu +1"));

  if (arduboy.justPressed(B_BUTTON)) {
    if (isStudying) {
      return;
    }
    isStudying = true;
    previousTime = millis();
    isControlsBlocked = true;

    if (education < 4) {
      education++;
      // Studying is good, but can be boring.
      if (happiness > 0 && random(100) < 40) {
        happiness--;
      }
      humorCreature = POSITIVE;
    } else {
      humorCreature = NEGATIVE;
    }

    isMenuSelected = false;
  }
}

// Shower menu: small happiness boost.
void drawShowerMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  (void)rectWidth;
  (void)rectHeight;

  uint8_t x = rectX + 8;
  uint8_t y = rectY + 8;

  arduboy.setCursor(x, y);
  arduboy.print(F("B Shower"));
  arduboy.setCursor(x, y + 12);
  arduboy.print(F("Happy +1"));

  if (arduboy.justPressed(B_BUTTON)) {
    if (isShowering) {
      return;
    }
    isShowering = true;
    previousTime = millis();
    isControlsBlocked = true;

    if (happiness < 4) {
      happiness++;
      humorCreature = POSITIVE;
    } else {
      humorCreature = NONE;
    }

    isMenuSelected = false;
  }
}

// Climate menu: adjust temperature up/down.
void drawClimateMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  (void)rectWidth;
  (void)rectHeight;

  uint8_t x = rectX + 10;
  uint8_t y = rectY + 8;

  arduboy.setCursor(x, y);
  arduboy.print(F("Temp: "));
  arduboy.print(temperature);
  arduboy.print(F("C"));

  uint8_t optY1 = y + 14;
  uint8_t optY2 = y + 28;

  arduboy.setCursor(x, optY1);
  arduboy.print(F("Cool"));
  if (selectedClimate == CLIMATE_COOL) {
    arduboy.drawRect(x - 2, optY1 - 2, 28, 12, WHITE);
  }

  arduboy.setCursor(x, optY2);
  arduboy.print(F("Warm"));
  if (selectedClimate == CLIMATE_WARM) {
    arduboy.drawRect(x - 2, optY2 - 2, 28, 12, WHITE);
  }

  if (arduboy.justPressed(B_BUTTON)) {
    previousTime = millis();
    isControlsBlocked = true;

    if (selectedClimate == CLIMATE_COOL) {
      if (temperature > 10) temperature--;
    } else {
      if (temperature < 40) temperature++;
    }

    humorCreature = POSITIVE;
    isMenuSelected = false;
  }
}

// Heal menu: converts happiness loss from low stats into a small recovery.
// Returns true if the creature is currently sick.
// This is a lightweight rule-based check that relies on existing stats (no new persistent state).
bool isCreatureSick() {
  // Temperature out of the comfort range makes the creature sick.
  if (temperature <= 14 || temperature >= 32) {
    return true;
  }

  // Being very hungry or very thirsty also makes the creature sick.
  if (hunger <= 1 || thirst <= 1) {
    return true;
  }

  return false;
}

void drawHealMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  (void)rectWidth;
  (void)rectHeight;

  uint8_t x = rectX + 8;
  uint8_t y = rectY + 8;

  const bool sick = isCreatureSick();

  arduboy.setCursor(x, y);
  arduboy.print(F("B Heal"));

  arduboy.setCursor(x, y + 12);
  if (sick) {
    arduboy.print(F("Happy +1"));
  } else {
    arduboy.print(F("Not sick"));
  }

  if (arduboy.justPressed(B_BUTTON)) {
    if (isHealing) {
      return;
    }

    isHealing = true;
    previousTime = millis();
    isControlsBlocked = true;

    if (!sick) {
      // Healing has no effect when the creature is not sick.
      humorCreature = NEGATIVE;
      emitBeep();
      return;
    }

    // Apply heal effects only when sick.
    if (happiness < 4) {
      happiness++;
    }

    // Push temperature back toward a safe default.
    const uint8_t targetTemp = 22;
    if (temperature < targetTemp) {
      temperature = (uint8_t)min((int)temperature + 2, (int)targetTemp);
    } else if (temperature > targetTemp) {
      temperature = (uint8_t)max((int)temperature - 2, (int)targetTemp);
    }

    humorCreature = POSITIVE;

    markStateDirty();
    isMenuSelected = false;
  }
}

void updateSleepEffects() {

  if (!isSleeping) return;

  static unsigned long lastSleepTick = 0;

  if (millis() - lastSleepTick >= 2000) { // each 2 seconds
    lastSleepTick = millis();

    if (!isLightOn) {
      // Sleeping in dark = better
      if (happiness < 4) happiness++;
    } else {
      // Sleeping in clear = worst
      if (hunger > 0) hunger--;
      if (thirst > 0) thirst--;
      if (happiness > 0) happiness--;
    }
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
  // Allow changing selection during the 5-round game.
  // Only block navigation on the final result screen.
  if (playFinished) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
      selectedPlay = (selectedPlay == SCISSORS) ? ROCK : (SelectedPlay)(selectedPlay + 1);
      emitBeep();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedPlay = (selectedPlay == ROCK) ? SCISSORS : (SelectedPlay)(selectedPlay - 1);
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

void navigateSelectedClimate() {
  if (currentMenu != CLIMATE) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON) || arduboy.justPressed(UP_BUTTON)) {
    selectedClimate = (selectedClimate == CLIMATE_COOL) ? CLIMATE_WARM : CLIMATE_COOL;
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

  // Sleeping animation depends ONLY on isSleeping (not on light)
  if (isSleeping) {
    uint8_t sleepFrame = (millis() / 500) % 2;

    for (uint8_t i = 0; i < 4; i++) {
      drawCustomBitmapCreature(creatPosArr[i], sleepMatSprites[sleepFrame][i], 10, 10);
    }

    // optional: draw a small "Z" floating (see below)
    drawSleepZ(x + 22, y - 6);

    return;
  }

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmapCreature(creatPosArr[i] , creatureMatSprites[creatureCurrentFrame][i], 10, 10);
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
    drawCustomBitmapCreature(creatPosArr[i] , humorIcons[humorCreature][i], 10, 10);
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
    drawCustomBitmapCreature(creatPosArr[i] , caressSprite[i], 10, 10);
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

void drawDoStudy(uint8_t x, uint8_t y) {
  if (!isStudying) {
    return;
  }

  // Position the overlay near the creature.
  x -= 22;

  // Small bob animation (1px up/down) - cheap and looks alive
  uint8_t bob = (millis() / 220) % 2;
  y -= (10 + creatureCurrentFrame + bob);

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  // Reuse the Education status book sprite for the study animation
  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmapCreature(creatPosArr[i], statusSprites[EDUCATION][i], 10, 10);
  }

  isMenuSelected = false;
}

void drawDoShower(uint8_t x, uint8_t y) {
  if(isShowering == false) {
    return;
  }

  // Position the overlay above the creature.
  x -= 10;
  y -= 22 + creatureCurrentFrame;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  uint8_t frame = (millis() / 180) % 2;

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmapCreature(creatPosArr[i], showerMatSprites[frame][i], 10, 10);
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

// Light-aware bitmap drawing for the creature viewport.
// When the light is OFF, the viewport background becomes WHITE and sprites are drawn inverted (BLACK on WHITE).
void drawCustomBitmapCreature(uint8_t x, uint8_t y, const uint8_t* sprite, uint8_t width, uint8_t height) {
  uint8_t color = WHITE;

  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      if (pgm_read_byte_near(sprite + row) & (1 << (width - col - 1))) {
        if (row < 2) {
          color = BLACK;
        }

        if (!isLightOn) {
          // Invert only inside the creature viewport.
          color = (color == WHITE) ? BLACK : WHITE;
        }

        arduboy.drawPixel(x + col, y + row, color);
        color = WHITE;
      }
    }
  }
}

void drawCustomBitmapCreature(const uint8_t* posXaY, const uint8_t* sprite, uint8_t width, uint8_t height) {
  drawCustomBitmapCreature(posXaY[0], posXaY[1], sprite, width, height);
}

void markStateDirty() {
  // Stub: EEPROM persistence is not enabled in this branch yet.
  // This function exists to keep patches compatible and will be replaced
  // by a real "dirty flag" + autosave logic when persistence is implemented.
}
