# Ardudino

<img src="/assets/banner.png" width="2600" height="200" />

Ardudino is a simple **virtual pet game for Arduboy**, inspired by classic handheld pets like Dinkie Dino and early Tamagotchi-style games.

You take care of a small creature by managing its basic needs, interacting with it, and keeping it healthy and happy over time.

The game is designed to be **lightweight, easy to understand, and fully compatible with Arduboy hardware limitations**.

---

## Controls

- **LEFT / RIGHT**: Navigate side menus
- **UP / DOWN**: Change options inside menus
- **B**: Confirm / Select
- **A**: Back / Cancel

---

## Start Screen

<img src="/assets/start_screen1.png" width="300" height="200" />

Press **A** to start the game.

---

## Gameplay Overview

The main screen shows your creature inside a central viewport, with side menus used to interact with it.

Each action affects the creature’s internal state, such as hunger, thirst, happiness, temperature, and health.

---

## Menus

### Drink

<img src="/assets/menu_drink_water.png" width="300" height="200" />

Gives water to your creature.

- Increases **thirst**
- Has no effect if the creature is already hydrated

---

### Food

<img src="/assets/menu_food.png" width="300" height="200" />

Feed your creature with different types of food.

Available options:

<img src="/assets/menu_food_options.png" width="150" height="350" />

Each food affects **hunger** and **happiness** in different ways.

---

### Light

<img src="/assets/menu_light.png" width="300" height="200" />

Turns the light **ON** or **OFF**.

- When the light is OFF:
  - The creature goes to sleep
  - Most menus are locked
  - Only **Status** and **Light** menus remain accessible

---

### Caress

<img src="/assets/menu_caress.png" width="300" height="200" />

Interact with the creature to show affection.

- Increases **happiness**
- Triggers a short animation

---

### Status

The status menu is divided into multiple screens:

<img src="/assets/menu_status_1.png" width="300" height="200" />
<img src="/assets/menu_status_2.png" width="300" height="200" />
<img src="/assets/menu_status_3.png" width="300" height="200" />
<img src="/assets/menu_status_4.png" width="300" height="200" />
<img src="/assets/menu_status_5.png" width="300" height="200" />
<img src="/assets/menu_status_6.png" width="300" height="200" />
<img src="/assets/menu_status_7.png" width="300" height="200" />

They show information such as:

- Happiness
- Education
- Hunger
- Thirst
- Weight
- Age
- Temperature

---

### Play

<img src="/assets/menu_play.png" width="300" height="200" />

Play a **Rock / Paper / Scissors** minigame against the creature.

- Always played in **5 rounds**
- Cannot be exited before finishing
- The result affects the creature’s happiness

---

### Study

<img src="/assets/menu_study.png" width="300" height="200" />

Teach your creature.

- Increases **education**
- May slightly affect happiness
- Includes a short study animation

---

### Shower

<img src="/assets/menu_shower.png" width="300" height="200" />

Give the creature a shower.

- Improves **happiness**
- Displays a shower animation

---

### Climate

<img src="/assets/menu_climate.png" width="300" height="200" />

Adjust the environment temperature.

- Temperature affects health
- Extreme values can make the creature sick

---

### Heal

<img src="/assets/menu_heal.png" width="300" height="200" />

Use medicine to heal the creature.

- Only works when the creature is sick
- Restores happiness and stabilizes temperature

---

## Health and Sleep

- The creature can become **sick** due to:
  - Extreme temperature
  - Hunger or thirst being too low
- Medicine only works when the creature is sick
- When the light is OFF, the creature sleeps and recovers passively

---

## Goal

There is no strict win condition.

The goal is to **keep your creature alive, healthy, and happy for as long as possible**.

---

## Technical Notes

- Built using **Arduboy2**
- Designed for **ATmega32U4**
- Uses fixed-size bitmap matrices for sprites
- Optimized for low memory usage

---

## License

This project is open source and intended for educational and hobby purposes.
