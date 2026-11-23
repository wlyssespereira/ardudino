#ifndef PET_CARE_H
#define PET_CARE_H

#include <stdint.h>

// Persistent, allocation-free rules shared by the game and regression tests.
enum DinoSpecies : uint8_t { BABY, TREX, BRONTO, TRICERA };
struct PetCare {
  uint8_t meals[3] = {0, 0, 0}; // meat, plants, pasta
  uint8_t species = BABY;
  uint8_t dirt = 0;             // 0 clean, 1 dusty, 2 dirty, 3 filthy
  uint8_t neglect = 0;          // consecutive unhealthy two-hour ticks
  uint8_t metabolicMinutes = 0;
  uint8_t metabolicPhase = 0;
  bool sick = false;
};

inline bool isBedtime(uint8_t hour) { return hour >= 21 || hour < 9; }

inline void recordMeal(PetCare &care, uint8_t group) {
  if (care.species != BABY || group > 2) return;
  // Preserve proportions without overflow during very long baby stages.
  if (care.meals[group] == 255) {
    for (uint8_t i = 0; i < 3; ++i) care.meals[i] /= 2;
  }
  ++care.meals[group];
}

inline void evolvePet(PetCare &care, uint8_t age) {
  if (age < 2 || care.species != BABY) return;
  // A tied or unrecorded diet produces the mixed/pasta branch.
  if (care.meals[0] > care.meals[1] && care.meals[0] > care.meals[2])
    care.species = TREX;
  else if (care.meals[1] > care.meals[0] && care.meals[1] > care.meals[2])
    care.species = BRONTO;
  else
    care.species = TRICERA;
}

inline void assessCare(PetCare &care, uint8_t hunger, uint8_t thirst,
                       uint8_t temperature, bool sleeping, bool lightOn) {
  const bool unhealthy = hunger == 0 || thirst == 0 || care.dirt == 3 ||
    temperature < 18 || temperature > 30 || (sleeping && lightOn);
  if (unhealthy) {
    if (care.neglect < 3) ++care.neglect;
    if (care.neglect >= 2) care.sick = true;
  } else {
    care.neglect = 0;
  }
  // Illness remains until medicine is given, even after fixing its cause.
}

inline uint8_t careNeed(const PetCare &care, uint8_t hunger, uint8_t thirst,
                        uint8_t happiness, uint8_t temperature,
                        bool sleeping, bool lightOn) {
  if (sleeping && lightOn) return 1;
  if (care.sick) return 2;
  if (thirst <= 1) return 3;
  if (hunger <= 1) return 4;
  if (care.dirt >= 2) return 5;
  if (temperature < 18 || temperature > 30) return 6;
  if (happiness <= 1 && !sleeping) return 7;
  return 0;
}

#endif
