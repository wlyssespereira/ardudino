#ifndef SELF_TEST_H
#define SELF_TEST_H

uint8_t testChecks = 0;
uint8_t testFailures = 0;

void check(bool result, const __FlashStringHelper *name) {
  ++testChecks;
  Serial.print(result ? F("PASS ") : F("FAIL "));
  Serial.println(name);
  if (!result) ++testFailures;
}

void runSelfTests() {
  Serial.begin(9600); delay(500);
  check(!isBedtime(9) && !isBedtime(20), F("awake 09-20"));
  check(isBedtime(21) && isBedtime(0) && isBedtime(8), F("sleep 21-09"));
  PetCare c;
  recordMeal(c, 0); recordMeal(c, 0); recordMeal(c, 1);
  evolvePet(c, 1); check(c.species == BABY, F("no early evolution"));
  evolvePet(c, 2); check(c.species == TREX, F("meat evolves trex"));
  recordMeal(c, 1); check(c.meals[1] == 1, F("diet locks after evolution"));
  c = PetCare(); recordMeal(c, 1); evolvePet(c, 2);
  check(c.species == BRONTO, F("plants evolve bronto"));
  c = PetCare(); recordMeal(c, 2); evolvePet(c, 2);
  check(c.species == TRICERA, F("pasta evolves tricera"));
  c = PetCare(); recordMeal(c, 0); recordMeal(c, 1); evolvePet(c, 2);
  check(c.species == TRICERA, F("tie mixed branch"));
  c = PetCare(); c.meals[0] = 255; recordMeal(c, 0);
  check(c.meals[0] == 128, F("diet overflow safe"));
  c = PetCare();
  assessCare(c, 0, 4, 25, false, true);
  check(!c.sick && c.neglect == 1, F("grace period"));
  assessCare(c, 4, 4, 25, false, true);
  check(c.neglect == 0, F("good care resets risk"));
  assessCare(c, 0, 4, 25, false, true); assessCare(c, 0, 4, 25, false, true);
  check(c.sick, F("neglect causes illness"));
  assessCare(c, 4, 4, 25, false, true);
  check(c.sick, F("illness needs medicine"));
  c = PetCare(); c.dirt = 3;
  assessCare(c, 4, 4, 25, false, true); assessCare(c, 4, 4, 25, false, true);
  check(c.sick, F("filth causes illness"));
  c = PetCare();
  assessCare(c, 4, 4, 31, false, true); assessCare(c, 4, 4, 31, false, true);
  check(c.sick, F("heat causes illness"));
  c = PetCare();
  assessCare(c, 4, 4, 17, false, true); assessCare(c, 4, 4, 17, false, true);
  check(c.sick, F("cold causes illness"));
  c = PetCare();
  assessCare(c, 4, 4, 25, true, true); assessCare(c, 4, 4, 25, true, true);
  check(c.sick, F("lights disturb sleep"));
  c = PetCare();
  check(careNeed(c, 4, 4, 4, 25, false, true) == 0, F("healthy no alert"));
  check(careNeed(c, 4, 1, 4, 25, false, true) == 3, F("water alert"));
  check(careNeed(c, 1, 4, 4, 25, false, true) == 4, F("food alert"));
  c.dirt = 2;
  check(careNeed(c, 4, 4, 4, 25, false, true) == 5, F("bath alert"));

  // Exercise real sketch actions and EEPROM, not a duplicate implementation.
  petCare = PetCare(); hunger = 0; weight = 2; eatingLunch = MEAT;
  applyFoodSelection();
  check(hunger == 2 && petCare.meals[0] == 1 && weight == 3, F("meal integration"));
  hunger = 4; applyFoodSelection();
  check(petCare.meals[0] == 1, F("refused meal not counted"));
  thirst = 0; isDrinkingWater = false; stateDirty = false; doDrinkingWater();
  check(thirst == 1 && stateDirty, F("water marks save dirty"));
  happiness = 1; isDoingCaress = false; doCaress();
  check(happiness == 2, F("caress increases happiness"));
  petCare = PetCare(); age = 0; hunger = 4; thirst = 4; happiness = 4;
  updateMetabolismTick();
  check(hunger == 3 && thirst == 3 && petCare.dirt == 1, F("baby metabolism hygiene"));
  hunger = 4; thirst = 4; isSleeping = true; isLightOn = true;
  updateSleepEffects();
  check(hunger == 4 && thirst == 4, F("sleep avoids rapid starvation"));

  arduboy.currentButtonState = B_BUTTON; arduboy.previousButtonState = 0;
  isShowering = false; petCare.dirt = 3; stateDirty = false;
  drawShowerMenu(20, 12, 88, 46);
  check(petCare.dirt == 0 && stateDirty && isShowering, F("bath removes dirt"));
  isSleeping = false; isLightOn = true; petCare.sick = true;
  isHealing = false; hunger = 0; thirst = 0;
  drawHealMenu(20, 12, 88, 46);
  check(!petCare.sick && hunger == 0 && thirst == 0, F("medicine cures only illness"));
  check(!canOpenMenu(HEAL), F("healthy refuses medicine"));
  temperature = 25; selectedClimate = CLIMATE_COOL; stateDirty = false;
  drawClimateMenu(20, 12, 88, 46);
  check(temperature == 24 && stateDirty, F("climate persists"));
  isSleeping = true;
  check(!canOpenMenu(FOOD) && canOpenMenu(LIGHT) && canOpenMenu(STATUS), F("sleep menu restrictions"));
  isSleeping = false; isLightOn = false;
  check(!canOpenMenu(PLAY) && canOpenMenu(LIGHT), F("dark menu restrictions"));
  isLightOn = true; happiness = 2; dinoWins = 2; playerWins = 1; playResultApplied = false;
  applyPlayOutcomeOnce();
  check(happiness == 2, F("two wins not enough"));
  dinoWins = 3; playerWins = 1; playResultApplied = false;
  applyPlayOutcomeOnce();
  check(happiness == 3, F("three dino wins reward"));
  applyPlayOutcomeOnce();
  check(happiness == 3, F("reward applied once"));
  arduboy.currentButtonState = 0; arduboy.previousButtonState = 0;

  // Three actual evolution paths must survive a complete save/load cycle.
  uint8_t systemBytes[EEPROM_STORAGE_SPACE_START];
  for (uint8_t i = 0; i < sizeof(systemBytes); ++i) systemBytes[i] = EEPROM.read(i);
  for (uint8_t species = TREX; species <= TRICERA; ++species) {
    petCare = PetCare(); petCare.species = species; petCare.dirt = 3;
    petCare.sick = true; petCare.metabolicMinutes = 119;
    age = 5; gameHour = 21; gameMinute = 10; temperature = 25; isLightOn = false;
    saveState(); petCare = PetCare(); age = 0;
    check(loadState() && petCare.species == species && petCare.sick &&
          petCare.dirt == 3 && petCare.metabolicMinutes == 119 && age == 5 &&
          isSleeping && !isLightOn, F("save roundtrip branch"));
  }
  bool systemUnchanged = true;
  for (uint8_t i = 0; i < sizeof(systemBytes); ++i)
    if (systemBytes[i] != EEPROM.read(i)) systemUnchanged = false;
  check(systemUnchanged, F("system EEPROM preserved"));
  EEPROM.update(kEepromAddr + sizeof(SaveData) - 1,
                EEPROM.read(kEepromAddr + sizeof(SaveData) - 1) ^ 0x01);
  check(!loadState(), F("corrupted save rejected"));

  // Legacy migration only touches a disposable emulator EEPROM.
  LegacySaveData legacy = {};
  legacy.magic = kSaveMagic; legacy.version = 1; legacy.hunger = 3;
  legacy.thirst = 2; legacy.happiness = 2; legacy.education = 1;
  legacy.age = 1; legacy.weight = 4; legacy.temperature = 25;
  legacy.hour = 22; legacy.minute = 15; legacy.flags = 0;
  legacy.checksum = checksumBytes((const uint8_t*)&legacy, sizeof(legacy) - 1);
  EEPROM.update(kEepromAddr, 0xFF);
  eepromWriteUpdate(0, (const uint8_t*)&legacy, sizeof(legacy));
  check(loadState() && age == 1 && hunger == 3 && isSleeping && stateDirty,
        F("legacy save migration"));
  for (uint8_t i = 0; i < sizeof(systemBytes); ++i) EEPROM.update(i, systemBytes[i]);

  gameHour = 20; gameMinute = 59; isSleeping = false;
  lastMinuteTick = millis() - minuteInterval; updateClock();
  check(gameHour == 21 && gameMinute == 0 && isSleeping, F("bedtime transition"));
  gameHour = 8; gameMinute = 59; isSleeping = true; isLightOn = false;
  lastMinuteTick = millis() - minuteInterval; updateClock();
  check(gameHour == 9 && !isSleeping && isLightOn, F("wake restores lights"));
  petCare = PetCare(); recordMeal(petCare, 1); age = 1;
  gameHour = 23; gameMinute = 59;
  lastMinuteTick = millis() - minuteInterval; updateClock();
  check(age == 2 && petCare.species == BRONTO, F("midnight evolution"));

  prepareDemoScenario(2);
  check(petCare.species == TREX && age == 5 && !petCare.sick, F("trex demo"));
  prepareDemoScenario(3); check(petCare.species == BRONTO, F("bronto demo"));
  prepareDemoScenario(4); check(petCare.species == TRICERA, F("tricera demo"));
  prepareDemoScenario(5);
  check(petCare.sick && petCare.dirt == 3 && thirst == 1, F("care demo"));
  prepareDemoScenario(6);
  check(isSleeping && isLightOn && gameHour == 21, F("bedtime demo"));
  prepareDemoScenario(1);
  check(petCare.species == BABY && age == 0 && !petCare.sick, F("new pet demo"));

  Serial.print(F("RESULT ")); Serial.print(testChecks);
  Serial.print(F(" checks, ")); Serial.print(testFailures); Serial.println(F(" failures"));
  Serial.flush();
  arduboy.clear(); arduboy.setCursor(0, 8);
  arduboy.print(testFailures ? F("FAIL") : F("PASS"));
  arduboy.setCursor(0, 24); arduboy.print(testChecks); arduboy.print(F(" checks"));
  arduboy.setCursor(0, 40); arduboy.print(testFailures); arduboy.print(F(" failures"));
  arduboy.display();
  for (;;) {} // Deterministic final screen for headless emulator validation.
}
#endif

