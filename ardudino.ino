#include <Arduboy2.h>
#include <EEPROM.h>

#include "Menus.h"
#include "Creature.h"
#include "Foods.h"
#include "Play.h"
#include "PetCare.h"
#include "Evolution.h"

#ifdef ARDUDINO_SELF_TEST
void runSelfTests();
#endif

struct LegacySaveData {
  uint8_t magic;
  uint8_t version;

  uint8_t hunger;
  uint8_t thirst;
  uint8_t happiness;
  uint8_t education;

  uint8_t age;
  uint8_t weight;
  uint8_t temperature;

  uint8_t hour;
  uint8_t minute;

  uint8_t flags;     // bit0 = lightOn
  uint8_t checksum;  // XOR checksum of all bytes except checksum
};

struct SaveData {
  LegacySaveData base;
  PetCare care;
  uint8_t checksum;
};

PetCare petCare;

Arduboy2 arduboy;

Menu               currentMenu        = DRINK;
SelectedSwitch     selectedSwitch     = MENU_TURN_ON;
HumorCreature      humorCreature      = NONE;
SelectedLunch      selectedLunch      = HAMBURGER;
SelectedStatus     selectedStatus     = HAPPINESS;
SelectedPlay       selectedPlay       = ROCK;
SelectedPlay       selectedDinoPlay   = NONE_PLAY;

SelectedLunch eatingLunch             = HAMBURGER;

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
const unsigned long humorDuration   = 2000;

// Calculate the x and y coordinates to center the sprite on the screen
uint8_t spriteX                    = (arduboy.width()   - spriteWidth)  / 2;
uint8_t spriteY                    = (arduboy.height()  - spriteHeight) / 2;

uint8_t creatureCurrentFrame       = 0;            // Index of the current frame
uint8_t currentFrame               = 0;
unsigned long lastFrameTime        = 0;
unsigned long previousTime;

// Creature status
uint8_t happiness       = 2;  // Sprite
uint8_t education       = 2;  // Sprite
uint8_t hunger          = 2;  // Sprite
uint8_t thirst          = 2;  // Sprite
uint8_t weight          = 2;  // Text
uint8_t age             = 0;  // Text
uint8_t temperature     = 25; // Text

// -----------------------------
// PATCH 2: Metabolism / pacing
// -----------------------------

static const uint8_t kStatMin = 0;
static const uint8_t kStatMax = 4;

// Every N in-game minutes, apply "metabolism" once.
static const uint8_t kMetabolismIntervalMinutes = 120;

// Helpers (cheap + safe)
inline void decClamp(uint8_t &v) { if (v > kStatMin) v--; }
inline void incClamp(uint8_t &v) { if (v < kStatMax) v++; }

void clampCoreStats() {
  if (hunger > kStatMax) hunger = kStatMax;
  if (thirst > kStatMax) thirst = kStatMax;
  if (happiness > kStatMax) happiness = kStatMax;
  if (education > kStatMax) education = kStatMax;
}

// Rock / Paper / Scissors (Play) state.
static const uint8_t kPlayTotalRounds = 5;
bool playInProgress = false;
bool playFinished   = false;
uint8_t playRound   = 0;
uint8_t playerWins  = 0;
uint8_t dinoWins    = 0;
int8_t lastPlayResult = 0;

// Result screen timing for Play (RPS)
static const uint16_t kPlayResultScreenMs = 1500;
uint32_t playFinishedAtMs = 0;
bool playResultApplied = false;

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
uint8_t pendingButtonPresses = 0;
uint8_t lastRawButtons = 0;

// -----------------------------
// GAME CLOCK
// -----------------------------
uint8_t gameHour                    = 12;
uint8_t gameMinute                  = 0;
unsigned long lastMinuteTick        = 0;
#ifndef ARDUDINO_MINUTE_MS
#define ARDUDINO_MINUTE_MS 60000UL
#endif
const unsigned long minuteInterval = ARDUDINO_MINUTE_MS;
bool isSleeping                     = false;

// -----------------------------
// UX FEEDBACK (Toast + Shake + Beeps)
// -----------------------------

const __FlashStringHelper* toastMsg = nullptr;
uint8_t toastFramesLeft = 0;
uint8_t shakeFramesLeft = 0;

// -----------------------------
// PATCH 3: EEPROM persistence
// -----------------------------

static const uint8_t kSaveMagic   = 0xAD;
static const uint8_t kSaveVersion = 2;

static const uint16_t kEepromAddr = EEPROM_STORAGE_SPACE_START;

// Autosave throttle (avoid EEPROM wear)
static const uint32_t kAutosaveIntervalMs = 300000; // 5 minutes

bool stateDirty         = false;
uint32_t lastAutosaveMs = 0;

bool isEating           = false;
uint8_t eatStep         = 0;
uint32_t lastEatFrameMs = 0;

#ifdef ARDUDINO_DEMO
uint8_t demoScenario = 0;

void prepareDemoScenario(uint8_t scenario) {
  if (scenario == 0) return;
  petCare = PetCare();
  hunger = thirst = 4;
  happiness = 3; education = 2; weight = 2; age = 0; temperature = 25;
  gameHour = 12; gameMinute = 0; isLightOn = true; isSleeping = false;
  if (scenario >= 2 && scenario <= 4) {
    age = 5; weight = 12; petCare.species = scenario - 1;
  } else if (scenario == 5) {
    petCare.sick = true; petCare.dirt = 3;
    hunger = thirst = 1; temperature = 32;
  } else if (scenario == 6) {
    gameHour = 21; isSleeping = true;
  }
  markStateDirty();
}

void drawDemoScenario() {
  arduboy.setCursor(2, 55);
  arduboy.print(F("B: "));
  switch (demoScenario) {
    case 0: arduboy.print(F("Continue")); break;
    case 1: arduboy.print(F("New pet")); break;
    case 2: arduboy.print(F("T-Rex")); break;
    case 3: arduboy.print(F("Bronto")); break;
    case 4: arduboy.print(F("Tricera")); break;
    case 5: arduboy.print(F("Needs care")); break;
    case 6: arduboy.print(F("Bedtime")); break;
  }
}
#endif


uint8_t checksumBytes(const uint8_t* p, uint16_t size) {
  uint8_t x = 0;
  for (uint16_t i = 0; i < size; i++) {
    x ^= p[i];
  }
  return x;
}

uint8_t computeChecksum(const SaveData &d) {
  return checksumBytes((const uint8_t*)&d, sizeof(d) - 1);
}

void eepromWriteUpdate(uint16_t addr, const uint8_t* data, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    EEPROM.update(addr + i, data[i]); // update avoids writing same value
  }
}

bool loadState() {
  SaveData saved;
  EEPROM.get(kEepromAddr, saved);
  LegacySaveData d;
  PetCare loadedCare;
  if (saved.base.magic == kSaveMagic && saved.base.version == kSaveVersion &&
      computeChecksum(saved) == saved.checksum) {
    d = saved.base;
    loadedCare = saved.care;
    if (loadedCare.species > TRICERA || loadedCare.dirt > 3 ||
        loadedCare.neglect > 3 || loadedCare.metabolicMinutes >= 120 ||
        loadedCare.metabolicPhase > 1) return false;
  } else {
    if (saved.base.magic == kSaveMagic && saved.base.version == kSaveVersion)
      return false; // A damaged current save must not resurrect an old v1 pet.
    // Read v1 once from its historical address; never write system bytes.
    EEPROM.get(0, d);
    if (d.magic != kSaveMagic || d.version != 1 ||
        checksumBytes((const uint8_t*)&d, sizeof(d) - 1) != d.checksum)
      return false;
  }
  if (d.hour >= 24 || d.minute >= 60) return false;
  petCare = loadedCare;

  hunger      = d.hunger;
  thirst      = d.thirst;
  happiness   = d.happiness;
  education   = d.education;

  age         = d.age;
  weight      = d.weight;
  temperature = d.temperature;

  gameHour    = d.hour;
  gameMinute  = d.minute;

  isLightOn   = (d.flags & 0x01) != 0;

  // Safety clamps
  clampCoreStats();
  if (temperature < 10) temperature = 10;
  if (temperature > 40) temperature = 40;

  // Recompute sleep state based on schedule
  isSleeping = isBedtime(gameHour);

  // Avoid instant clock tick after load
  lastMinuteTick = millis();

  evolvePet(petCare, age);
  stateDirty = d.version == 1;
  return true;
}

void saveState() {
  SaveData saved = {};
  LegacySaveData &d = saved.base;
  d.magic   = kSaveMagic;
  d.version = kSaveVersion;

  d.hunger    = hunger;
  d.thirst    = thirst;
  d.happiness = happiness;
  d.education = education;

  d.age         = age;
  d.weight      = weight;
  d.temperature = temperature;

  d.hour   = gameHour;
  d.minute = gameMinute;

  d.flags = 0;
  if (isLightOn) d.flags |= 0x01;

  saved.care = petCare;
  saved.checksum = computeChecksum(saved);

  eepromWriteUpdate(kEepromAddr, (const uint8_t*)&saved, sizeof(saved));

  stateDirty = false;
  lastAutosaveMs = millis();
}

void updateAutosave() {
  if (!stateDirty) return;

  uint32_t now = millis();
  if (now - lastAutosaveMs >= kAutosaveIntervalMs) {
    saveState();
  }
}

// Toast: show a short message for N frames (cheap UX feedback)
void showToast(const __FlashStringHelper* msg, uint8_t frames = 30) {
  toastMsg = msg;
  toastFramesLeft = frames;
}

void updateToast() {
  if (toastFramesLeft > 0) {
    toastFramesLeft--;
    if (toastFramesLeft == 0) {
      toastMsg = nullptr;
    }
  }
}

void drawToast() {
  if (toastMsg == nullptr || toastFramesLeft == 0) {
    return;
  }

  // Bottom-center, small box. Keep it simple & readable.
  const uint8_t w = 64;
  const uint8_t h = 10;
  const uint8_t x = (arduboy.width() - w) / 2;
  const uint8_t y = arduboy.height() - h - 1;

  // White box with black text
  arduboy.fillRect(x, y, w, h, WHITE);
  arduboy.drawRect(x, y, w, h, BLACK);

  arduboy.setTextColor(BLACK);
  arduboy.setCursor(x + 4, y + 2);
  arduboy.print(toastMsg);
  arduboy.setTextColor(WHITE);
}

// Shake: quick 1px wobble for the currently selected icon
void startIconShake(uint8_t frames = 10) {
  shakeFramesLeft = frames;
}

// Beeps: consistent patterns
void beepOnce(uint16_t freq, uint16_t durMs) {
  if (!arduboy.audio.enabled()) return;
  tone(5, freq, durMs);
  delay(10);
}

void beepNav() {
  beepOnce(1200, 30);
}

void beepOk() {
  beepOnce(1400, 25);
  beepOnce(1600, 25);
}

void beepError() {
  beepOnce(400, 35);
  beepOnce(350, 35);
}

bool isActionRunning() {
  return isControlsBlocked;
}

bool canOpenMenu(Menu m) {
  // While sleeping: only STATUS and LIGHT
  if (isSleeping && m != STATUS && m != LIGHT) {
    return false;
  }

  // Light OFF: only STATUS and LIGHT
  if (!isLightOn && m != STATUS && m != LIGHT) {
    return false;
  }

  // Heal: only when sick
  if (m == HEAL && !isCreatureSick()) {
    return false;
  }

  return true;
}

void drawSleepZ(int16_t x, int16_t y, uint16_t timeOffset) {
  uint8_t frame = ((millis() + timeOffset) / 450) % 2;
  int8_t floatY = ((millis() + timeOffset) / 700) % 2;

  int16_t yy = y - floatY;
  if (yy < 0) yy = 0;

  drawCustomBitmapCreature((uint8_t)x, (uint8_t)yy, sleepZSprites[frame], 8, 8);
}

void setup() {
#ifdef ARDUDINO_SELF_TEST
  // Headless Ardens has no physical button release phase for begin().
  arduboy.boot();
  arduboy.audio.begin();
#else
  arduboy.begin();
#endif
  arduboy.setFrameRate(15);
  arduboy.initRandomSeed();
  previousTime = millis();
  loadState();
#ifdef ARDUDINO_SELF_TEST
  runSelfTests();
#endif
}


void loop() {
  // Capture short presses even between the 15 FPS drawing frames.
  uint8_t rawButtons = arduboy.buttonsState();
  pendingButtonPresses |= rawButtons & ~lastRawButtons;
  lastRawButtons = rawButtons;
  if (!arduboy.nextFrame()) {
    return;
  }
  arduboy.currentButtonState = rawButtons | pendingButtonPresses;
  arduboy.previousButtonState = rawButtons & ~pendingButtonPresses;
  pendingButtonPresses = 0;

  if(isStartScreen) {
    arduboy.clear();
    drawStartScreen(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
    arduboy.setCursor(60, 40);
    arduboy.print(F("Press A..."));
#ifdef ARDUDINO_DEMO
    if (arduboy.justPressed(B_BUTTON)) demoScenario = (demoScenario + 1) % 7;
    drawDemoScenario();
#endif
    arduboy.display();
    // Start the game without counting time spent on the title screen.
    if (arduboy.justPressed(A_BUTTON)) {
#ifdef ARDUDINO_DEMO
      prepareDemoScenario(demoScenario);
#endif
      isStartScreen = false;
      lastMinuteTick = millis();
    }
    return;
  }

  updateClock();

  updateToast();
  if (shakeFramesLeft > 0) shakeFramesLeft--;

  updateAutosave();

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

  bool menuWasSelected = isMenuSelected;
  // Freeze submenu confirmations and navigation during action animations.
  if (isMenuSelected && !isActionRunning()) {
    drawSelectedMenu(iconXLeft + 15, centerY - rectHeight / 2 + 3, rectWidth + 10, rectHeight);
    navigateLightSwitches();
    navigateSelectedLunch();
    navigateSelectedStatus();
    navigateSelectedPlay();
    navigateSelectedClimate();
  }

  // A confirmation can close a submenu: do not process that B edge twice.
  if (!menuWasSelected) executeControls();
  else if (arduboy.justPressed(A_BUTTON) && !isActionRunning() &&
           !(currentMenu == PLAY && (playInProgress || playFinished)))
    isMenuSelected = false;

  // Draw the creature in the center of the central rectangle
  drawCreature      (rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Draw the creature's status
  drawHumorCreature (rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Render the clock
  renderClock();

  // Draw an action with the creature
  drawDoCaress  (rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDrinkWater(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDoStudy   (rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDoShower  (rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDoEat     (rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

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

  drawToast();
  drawCareAlert();

  arduboy.display();
}

void renderClock() {
  char buffer[6];
  sprintf(buffer, "%02d:%02d", gameHour, gameMinute);

  arduboy.setCursor(80, 0);
  arduboy.print(buffer);
}

void updateClock() {
  uint32_t now = millis();
  while ((uint32_t)(now - lastMinuteTick) >= minuteInterval) {
    lastMinuteTick += minuteInterval;
    bool wasSleeping = isSleeping;
    if (++gameMinute >= 60) {
      gameMinute = 0;
      if (++gameHour >= 24) {
        gameHour = 0;
        if (age < 255) ++age;
        uint8_t previousSpecies = petCare.species;
        evolvePet(petCare, age);
        if (previousSpecies != petCare.species) showToast(F("Evolved!"), 45);
      }
      if (random(100) < 35) {
        int16_t t = temperature + (random(2) ? 1 : -1);
        temperature = constrain(t, 10, 40);
      }
    }
    isSleeping = isBedtime(gameHour);
    if (wasSleeping && !isSleeping) isLightOn = true;
    // Leave an active menu/minigame cleanly when bedtime arrives.
    if (!wasSleeping && isSleeping) {
      resetPlayStateAndExitMenu();
      showToast(F("Bedtime"), 40);
    }
    if (++petCare.metabolicMinutes >= kMetabolismIntervalMinutes) {
      petCare.metabolicMinutes = 0;
      if (!isSleeping) updateMetabolismTick();
      updateSleepEffects();
      assessCare(petCare, hunger, thirst, temperature, isSleeping, isLightOn);
    }
    markStateDirty();
  }
}

// Function to execute the controls.
void executeControls() {
  // While an action animation is running, block input but show feedback
  if (isActionRunning()) {
    if (arduboy.justPressed(UP_BUTTON) ||
        arduboy.justPressed(DOWN_BUTTON) ||
        arduboy.justPressed(LEFT_BUTTON) ||
        arduboy.justPressed(RIGHT_BUTTON) ||
        arduboy.justPressed(A_BUTTON) ||
        arduboy.justPressed(B_BUTTON)) {

      showToast(F("Busy"), 20);
      beepError();
    }
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
      beepNav();
    }

    // DOWN = next item in the same side
    if (arduboy.justPressed(DOWN_BUTTON)) {
      idx = (idx + 1) % half;

      currentMenu = onLeftSide ? (Menu)idx : (Menu)(half + idx);
      beepNav();
    }

    // RIGHT = go to right side (keep idx)
    if (arduboy.justPressed(RIGHT_BUTTON) && onLeftSide) {
      currentMenu = (Menu)(half + idx);
      beepNav();
    }

    // LEFT = go to left side (keep idx)
    if (arduboy.justPressed(LEFT_BUTTON) && !onLeftSide) {
      currentMenu = (Menu)idx;
      beepNav();
    }
  }

  // B selects the current menu.
  // When the light is OFF, only Status and Light menus can be opened.
  if (!isMenuSelected && arduboy.justPressed(B_BUTTON)) {
    if (!canOpenMenu(currentMenu)) {
      // Feedback: message + error beep + icon shake
      if (isSleeping) {
        showToast(F("Sleeping"), 35);
      } else if (!isLightOn) {
        showToast(F("Too dark"), 35);
      } else if (currentMenu == HEAL && !isCreatureSick()) {
        showToast(F("Not sick"), 35);
      } else {
        showToast(F("Blocked"), 35);
      }

      beepError();
      startIconShake(12);
      return;
    }
    isMenuSelected = true;
    beepOk();
  }

  // A is the global back/cancel.
  // Exception: Play (RPS) must be finished once started.
  if (arduboy.justPressed(A_BUTTON) && !(currentMenu == PLAY && (playInProgress || playFinished))) {
    isMenuSelected = false;
    saveState();
    showToast(F("Saved"), 20);
  }
}

// Function to reset the mood after a certain duration
void resetHumor(unsigned long currentTime) {
  if ((uint32_t)(currentTime - previousTime) >= humorDuration && !isEating) {
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
    // Eat already done.
    isEating = false;
    eatStep = 0;
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

  // While sleeping, only allow Status and Light
  if (isSleeping && currentMenu != STATUS && currentMenu != LIGHT) {
    isMenuSelected = false;
    return;
  }

  switch (currentMenu) {
    case DRINK:   doDrinkingWater();                                    break;
    case FOOD:    drawFoodMenu(rectX, rectY);                           break;
    case LIGHT:   drawLightMenu(rectX, rectY, rectWidth, rectHeight);   break;
    case CARESS:  doCaress();                                           break;
    case STATUS:  drawStatusMenu(rectX, rectY);                         break;
    case PLAY:    drawPlayMenu(rectX, rectY);                           break;
    case STUDY:   drawStudyMenu(rectX, rectY, rectWidth, rectHeight);   break;
    case SHOWER:  drawShowerMenu(rectX, rectY, rectWidth, rectHeight);  break;
    case CLIMATE: drawClimateMenu(rectX, rectY, rectWidth, rectHeight); break;
    case HEAL:    drawHealMenu(rectX, rectY, rectWidth, rectHeight);    break;
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

  incClamp(happiness);
  markStateDirty();
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
    markStateDirty();
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
  if (selectedStatus == HYGIENE || selectedStatus == GROWTH || selectedStatus == HEALTH) {
    arduboy.setCursor(x + 6, y + 16);
    if (selectedStatus == HYGIENE) {
      arduboy.print(petCare.dirt >= 2 ? F("Needs bath") : F("Clean"));
      arduboy.setCursor(x + 6, y + 28);
      arduboy.print(F("Dirt ")); arduboy.print(petCare.dirt); arduboy.print(F("/3"));
    } else if (selectedStatus == HEALTH) {
      arduboy.print(petCare.sick ? F("Sick") : F("Healthy"));
      arduboy.setCursor(x + 6, y + 28);
      arduboy.print(petCare.sick ? F("Use Heal") : F("Keep care"));
    } else {
      arduboy.print(age < 2 ? F("Baby") : (age < 5 ? F("Young") : F("Adult")));
      arduboy.setCursor(x + 6, y + 28);
      switch (petCare.species) {
        case TREX: arduboy.print(F("T-Rex")); break;
        case BRONTO: arduboy.print(F("Bronto")); break;
        case TRICERA: arduboy.print(F("Tricera")); break;
        default: arduboy.print(F("Diet->Dino")); break;
      }
    }
    return;
  }
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
    arduboy.print(F("Let me win"));
  } else if (playFinished) {
    if (playerWins > dinoWins) {
      arduboy.print(F("Try again"));
    } else if (dinoWins > playerWins) {
      arduboy.print(dinoWins >= 3 ? F("Dino happy") : F("Try again"));
    } else {
      arduboy.print(F("Draw"));
    }
  } else {
    arduboy.print(F("Round "));
    arduboy.print(playRound);
    arduboy.print(F("/"));
    arduboy.print(kPlayTotalRounds);
  }

  // Labels
  arduboy.setCursor(x, y + 10);
  arduboy.print(F("Y"));
  arduboy.print(playerWins);
  arduboy.setCursor(x + 44, y + 10);
  arduboy.print(F("D"));
  arduboy.print(dinoWins);

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

  // Auto-exit after showing result
  if (playFinished) {
    if ((uint32_t)(millis() - playFinishedAtMs) >= kPlayResultScreenMs) {
      resetPlayStateAndExitMenu();
    }
    return;
  }

  // Input
  if (arduboy.justPressed(B_BUTTON)) {
    if (!playInProgress && !playFinished) {
      startPlayIfNeeded();
      advancePlayRound();
    } else if (playInProgress) {
      advancePlayRound();
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


void applyPlayOutcomeOnce() {
  if (playResultApplied) return;
  playResultApplied = true;

  // Apply mood/result once when the game finishes.
  if (dinoWins >= 3) {
    if (happiness < 4) happiness++;
    humorCreature = POSITIVE;
  } else if (playerWins > dinoWins) {
    if (happiness > 0) happiness--;
    humorCreature = NEGATIVE;
  } else {
    humorCreature = NONE;
  }

  previousTime = millis();
  isControlsBlocked = true;

  markStateDirty();
}

void resetPlayStateAndExitMenu() {
  playInProgress = false;
  playFinished = false;
  playRound = 0;
  playerWins = 0;
  dinoWins = 0;
  lastPlayResult = 0;
  selectedDinoPlay = NONE_PLAY;

  playFinishedAtMs = 0;
  playResultApplied = false;

  // Back to icon navigation
  isMenuSelected = false;
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

    playFinishedAtMs = millis();
    playResultApplied = false;
    applyPlayOutcomeOnce();
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
    startEating();
  }

}

void startEating() {
  // If not hungry, reject food immediately.
  if (hunger >= 4) {
    humorCreature = NEGATIVE;
    isMenuSelected = false;
    return;
  }

  // Snapshot which food we are eating (so menu navigation won't affect it)
  eatingLunch = selectedLunch;

  // Start action (same pattern as drink/caress/study/shower)
  previousTime = millis();
  isControlsBlocked = true;

  isEating = true;
  eatStep = 0;
  lastEatFrameMs = millis();

  humorCreature = POSITIVE;
  isMenuSelected = false;
}

void applyFoodSelection() {
  previousTime = millis();
  isControlsBlocked = true;
  if (hunger >= 4) {
    humorCreature = NEGATIVE;
    return;
  }
  humorCreature = POSITIVE;
  incClamp(hunger);
  if (eatingLunch == MEAT || eatingLunch == VEGGIES) incClamp(hunger);
  if (eatingLunch == HAMBURGER || eatingLunch == MEAT) recordMeal(petCare, 0);
  else if (eatingLunch == VEGGIES || eatingLunch == FRUIT) recordMeal(petCare, 1);
  else if (eatingLunch == PASTA) recordMeal(petCare, 2);
  if (eatingLunch == ICECREAM || eatingLunch == FRUIT) incClamp(happiness);
  if (weight < 250) ++weight;
  markStateDirty();
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
      markStateDirty();
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

// Shower removes dirt; affection is a separate action.
void drawShowerMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  (void)rectWidth;
  (void)rectHeight;

  uint8_t x = rectX + 8;
  uint8_t y = rectY + 8;

  arduboy.setCursor(x, y);
  arduboy.print(F("B Shower"));
  arduboy.setCursor(x, y + 12);
  arduboy.print(F("Wash dirt"));

  if (arduboy.justPressed(B_BUTTON)) {
    if (isShowering) {
      return;
    }
    isShowering = true;
    previousTime = millis();
    isControlsBlocked = true;

    petCare.dirt = 0;
    markStateDirty();

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

    markStateDirty();
    humorCreature = POSITIVE;
    isMenuSelected = false;
  }
}

// Medicine cures illness; its environmental causes still require care.
bool isCreatureSick() { return petCare.sick; }

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
    arduboy.print(F("Medicine"));
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
      beepNav();
      return;
    }

    petCare.sick = false;
    petCare.neglect = 0;
    showToast(F("Healed"), 35);

    humorCreature = POSITIVE;

    markStateDirty();
    isMenuSelected = false;
  }
}

void updateMetabolismTick() {
  decClamp(thirst);
  petCare.metabolicPhase ^= 1;
  if (petCare.metabolicPhase || age < 2) decClamp(hunger);
  if (petCare.dirt < 3) ++petCare.dirt;
  if (hunger == 0 || thirst == 0 || petCare.dirt == 3) decClamp(happiness);
  if (hunger >= 3 && weight < 250) ++weight;
  else if (hunger == 0 && weight > 1) --weight;
  clampCoreStats();
}

void updateSleepEffects() {
  if (!isSleeping) return;
  // Same two-hour cadence as care: never drain needs every few seconds.
  if (isLightOn) decClamp(happiness);
  else incClamp(happiness);
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
    markStateDirty();
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
      beepNav();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedLunch = (selectedLunch == HAMBURGER) ? NO_LUNCH - 1 : selectedLunch - 1;
      beepNav();
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
      beepNav();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedPlay = (selectedPlay == ROCK) ? SCISSORS : (SelectedPlay)(selectedPlay - 1);
      beepNav();
  }
}

// Function to navigate between the selected status options
void navigateSelectedStatus() {
  if(currentMenu != STATUS) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
      selectedStatus = (selectedStatus == NO_STATUS - 1) ? HAPPINESS : selectedStatus + 1;
      beepNav();
  } else if (arduboy.justPressed(UP_BUTTON)) {
      selectedStatus = (selectedStatus == HAPPINESS) ? NO_STATUS - 1 : selectedStatus - 1;
      beepNav();
  }
}

void navigateSelectedClimate() {
  if (currentMenu != CLIMATE) {
    return;
  }
  if (arduboy.justPressed(DOWN_BUTTON) || arduboy.justPressed(UP_BUTTON)) {
    selectedClimate = (selectedClimate == CLIMATE_COOL) ? CLIMATE_WARM : CLIMATE_COOL;
    beepNav();
  }
}

// Function to navigate between the light menus
void navigateLightSwitches() {
    if(currentMenu != LIGHT) {
      return;
    }
    if (arduboy.justPressed(DOWN_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        beepNav();
    } else if (arduboy.justPressed(UP_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        beepNav();
    }
}

// Function to navigate between the lunch options
void navigateLunches() {
    if(currentMenu != LIGHT) {
      return;
    }
    if (arduboy.justPressed(DOWN_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        beepNav();
    } else if (arduboy.justPressed(UP_BUTTON)) {
        selectedSwitch = (selectedSwitch == MENU_TURN_ON) ? MENU_TURN_OFF : MENU_TURN_ON;
        beepNav();
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

    if (petCare.species != BABY) drawEvolvedCreature(x, y);
    else for (uint8_t i = 0; i < 4; i++)
      drawCustomBitmapCreature(creatPosArr[i], sleepMatSprites[sleepFrame][i], 10, 10);

    // optional: draw a small "Z" floating (see below)
    drawSleepZ(x + 18, y - 4, 0);
    drawSleepZ(x + 22, y - 8, 300);

    return;
  }

  for (uint8_t i = 0; i < 4; i++) {
    if (petCare.species == BABY)
      drawCustomBitmapCreature(creatPosArr[i], creatureMatSprites[creatureCurrentFrame][i], 10, 10);
  }
  if (petCare.species != BABY) drawEvolvedCreature(x, y);
  if (petCare.dirt >= 2) {
    const uint8_t ink = isLightOn ? WHITE : BLACK;
    arduboy.drawPixel(x - 3, y + 5, ink);
    arduboy.drawPixel(x + 20, y + 10, ink);
    arduboy.fillRect(x + 19, y + 19, 5, 2, ink);
    arduboy.fillRect(x + 20, y + 17, 3, 2, ink);
  }
  if (petCare.sick) {
    const uint8_t ink = isLightOn ? WHITE : BLACK;
    arduboy.drawFastHLine(x + 24, y + 2, 7, ink);
    arduboy.drawFastVLine(x + 27, y - 1, 7, ink);
  }
}

void drawEvolvedCreature(uint8_t x, uint8_t y) {
  const uint8_t ink = isLightOn ? WHITE : BLACK;
  uint8_t bob = !isSleeping && creatureCurrentFrame == 1 ? 1 : 0;
  for (uint8_t row = 0; row < 16; ++row) {
    uint16_t bits = pgm_read_word(&evolvedSprites[petCare.species - 1][row]);
    for (uint8_t col = 0; col < 16; ++col) {
      if (bits & (0x8000U >> col)) arduboy.drawPixel(x + col, y + row - bob, ink);
    }
  }
  // Adults gain a tail extension; juveniles keep the compact silhouette.
  if (age >= 5) {
    arduboy.drawLine(x, y + 11 - bob, x - 5, y + 7 - bob, ink);
    arduboy.drawLine(x, y + 12 - bob, x - 5, y + 7 - bob, ink);
  }
}

void drawCareAlert() {
  static uint8_t previousNeed = 0;
  static uint32_t lastAlertMs = 0;
  uint8_t need = careNeed(petCare, hunger, thirst, happiness, temperature, isSleeping, isLightOn);
  if (need && need != previousNeed && (uint32_t)(millis() - lastAlertMs) >= 30000UL) {
    beepOnce(900, 80);
    lastAlertMs = millis();
  }
  previousNeed = need;
  if (isMenuSelected || toastFramesLeft || !need) return;
  arduboy.setCursor(27, 54);
  switch (need) {
    case 1: arduboy.print(F("! Lights off")); break;
    case 2: arduboy.print(F("! Medicine")); break;
    case 3: arduboy.print(F("! Water")); break;
    case 4: arduboy.print(F("! Food")); break;
    case 5: arduboy.print(F("! Bath")); break;
    case 6: arduboy.print(F("! Climate")); break;
    case 7: arduboy.print(F("! Play")); break;
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

void drawDoEat(uint8_t x, uint8_t y) {
  if (!isEating) {
    return;
  }

  // Position overlay near creature (similar to drink)
  x -= 30;
  y -= 10 + creatureCurrentFrame;

  // We already have a 2x2 tile layout helper
  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  // Draw the selected food (snapshot) as 2x2 tiles
  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i], foodSprites[eatingLunch][i], 10, 10);
  }

  // "Bite" animation: erase vertical slices from right to left
  // 4 steps -> 0, 4, 8, 12, 16 pixels erased (looks good and cheap)
  uint8_t erased = eatStep * 4; 
  if (erased > 16) erased = 16;

  // Combined food block is 16x16 starting at (x,y)
  if (erased > 0) {
    arduboy.fillRect(x + (16 - erased), y, erased, 16, BLACK);
  }

  // Advance animation timing
  uint32_t now = millis();
  if (now - lastEatFrameMs >= 180) {
    lastEatFrameMs = now;

    if (eatStep < 4) {
      eatStep++;
    } else {
      // Finish: apply food effects once
      applyFoodSelection();
      markStateDirty();

      // End action
      isEating = false;
    }
  }

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
    int8_t shakeX = 0;
    if (currentMenu == menu && shakeFramesLeft > 0) {
      shakeX = (shakeFramesLeft & 1) ? 1 : -1;
    }

    if (currentMenu == menu) {
      arduboy.drawRect(x + shakeX, iconYOffset + i * iconSpacing - 2, iconWidth + 4, iconHeight + 4, WHITE);
    }

    // Draw the icons
    arduboy.setCursor(x + 4, iconYOffset + i * iconSpacing + 4);
    drawCustomBitmap(x + shakeX, iconYOffset + i * iconSpacing, menuSprites[menu], iconWidth, iconHeight);

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
  stateDirty = true;
}

#ifdef ARDUDINO_SELF_TEST
#include "tests/SelfTest.h"
#endif
