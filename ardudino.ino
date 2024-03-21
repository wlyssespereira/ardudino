#include <Arduboy2.h>
#include "Menus.h"
#include "Creature.h"
#include "Foods.h"


Arduboy2 arduboy;

Menu currentMenu = DRINK;
SelectedSwitch selectedSwitch = MENU_TURN_ON;
HumorCreature humorCreature = NONE;
SelectedLunch selectedLunch = HAMBURGER;

// Array que relaciona o enum do menu com o sprite correspondente
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

const uint8_t spriteWidth = 8;   // Largura do sprite
const uint8_t spriteHeight = 8;  // Altura do sprite

// Calcula as coordenadas x e y para centralizar o sprite na tela
uint8_t spriteX = (arduboy.width() - spriteWidth) / 2;
uint8_t spriteY = (arduboy.height() - spriteHeight) / 2;

// Variáveis para a animação da criatura
uint8_t creatureCurrentFrame = 0;          // Índice do frame atual
const uint8_t creatureFrameDuration = 10;  // Duração de cada frame em frames de atualização
uint8_t currentFrame = 0;
unsigned long lastFrameTime = 0;
const unsigned long frameDuration = 1000;  // Um segundo
const unsigned long humorDuration = 10000;

unsigned long previousTime;

bool isStartScreen = true;
bool isControlsBlocked = false;
bool isMenuSelected = false;
bool isLightOn = true;
bool isDoingCaress = false;
bool isDrinkingWater = false;
bool isInFoodMenu = false;


void setup() {
  arduboy.begin();
  arduboy.setFrameRate(15);
  previousTime = millis(); // Inicializa o tempo anterior
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
    // Aguarda até que o botão B seja pressionado
    if (arduboy.pressed(A_BUTTON)) {
      isStartScreen = false;
    }
    return;
  }

  // Limpa a tela
  arduboy.clear();

  //Desenha os menus
  drawLateralMenus();

  // Calcula a posição central vertical entre os ícones do menu à esquerda
  uint8_t centerY = iconYOffset + (numIconsLeft * iconSpacing) / 2;

  // Calcula a largura e altura do retângulo central
  uint8_t rectWidth = (iconXRight - iconXLeft - 10) * 0.8;
  uint8_t rectHeight = (numIconsLeft * iconSpacing - 6) * 0.9;

  // Desenha a borda do retângulo central
  arduboy.drawRect(iconXLeft + 15, centerY - rectHeight / 2 + 3, rectWidth + 10, rectHeight, WHITE);

  // Desenha o nome do menu selecionado
  arduboy.setCursor(iconXLeft + 42, centerY - rectHeight / 2 - 4);
  arduboy.print(menuNames[currentMenu]);

  // Desenha um quadrado dentro do retângulo central
  if (isMenuSelected) {
    drawSelectedMenu(iconXLeft + 15, centerY - rectHeight / 2 + 3, rectWidth + 10, rectHeight);
    navigateLightSwitches();
    navigateSelectedLunch();
  }

  // Função de execução dos controles.
  executeControls();

  // Desenha a criatura no centro do retângulo central
  drawCreature(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Desenha os status da criatura
  drawHumorCreature(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Desenha uma acao com a criatura
  drawDoCaress(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);
  drawDrinkWater(rectX + (rectWidth - creatureSpriteWidth) / 2, rectY + (rectHeight - creatureSpriteHeight) / 2);

  // Atualiza a animação da criatura
  unsigned long currentTime = millis();
  if (arduboy.everyXFrames(creatureFrameDuration)) {
    if (currentTime - lastFrameTime >= frameDuration) {
      updateCreatureAnimation();
      currentFrame = (currentFrame + 1) % 3;
      lastFrameTime = currentTime;
    }
  }

  // Verifica se passaram-se 3 segundos desde a última mudança de estado e reseta o humor.
  resetHumor(currentTime);

  arduboy.display();
}


void executeControls() {
  if(isControlsBlocked) {
    return;
  }

  // Verifica se o botão esquerdo foi pressionado para navegar entre os menus da esquerda
  if (arduboy.justPressed(LEFT_BUTTON) && !isMenuSelected) {
    if (currentMenu >= (numMenus / 2)) {
      currentMenu = DRINK - 1;
    }
    currentMenu = static_cast<Menu>((currentMenu + 1) % (numMenus / 2));
    emitBeep();
  }

  // Verifica se o botão direito foi pressionado para navegar entre os menus da direita
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

  if (arduboy.justPressed(A_BUTTON)) {
    isMenuSelected = false;
  }
}


void resetHumor(long currentTime) {
  if (currentTime - previousTime >= humorDuration) {
    // Alterna o estado da variável
    humorCreature = NONE;
    // Atualiza o tempo anterior para o tempo atual
    previousTime = currentTime;
    // Carinho ja feito.
    isDoingCaress = false;
    // Bebeu agua
    isDrinkingWater = false;
    // Desbloqueia os controles.
    isControlsBlocked = false;
  }
}

void drawLateralMenus() {
  // Desenha os ícones do menu à esquerda
  drawMenuIcons(iconXLeft, numIconsLeft, 0);
  // Desenha os ícones do menu à direita
  drawMenuIcons(iconXRight, numIconsRight, numIconsLeft);
}

// Função para desenhar o menu selecionado
void drawSelectedMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  switch (currentMenu) {
    case DRINK:
      previousTime = millis();
      isDrinkingWater = true;
      isControlsBlocked = true;
      break;
    case FOOD:
      drawFoodMenu(rectX, rectY);
      break;
    case LIGHT:
      drawLightMenu(rectX, rectY, rectWidth, rectHeight);
      break;
    case CARESS:
      previousTime = millis();
      isDoingCaress = true;
      humorCreature = POSITIVE;
      isControlsBlocked = true;
      break;
    default:
      return;
  }
}

void drawFoodMenu(uint8_t x, uint8_t y) {
  // Ajuste da posição para desenhar o sprite
  x += 30;
  y += 15;

  uint8_t creatPosArr[4][2];
  buildCreatPosArr(x, y, creatPosArr);

  for (uint8_t i = 0; i < 4; i++) {
    drawCustomBitmap(creatPosArr[i] , foodSprites[selectedLunch][i], 10, 10);
  }

}

// Função para desenhar os menus "Turn On" e "Turn Off" dentro do quadrado
void drawLightMenu(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  // Define as coordenadas dos menus de texto
  uint8_t menuX1 = rectX + 10;
  uint8_t menuX2 = rectX + 10;
  uint8_t menuY1 = rectY + rectHeight / 2 - 10;  // Ajusta a altura verticalmente para o primeiro menu
  uint8_t menuY2 = menuY1 + 20;                  // Define a posição vertical do segundo menu abaixo do primeiro

  // Desenha os menus de texto
  arduboy.setCursor(menuX1, menuY1);
  arduboy.print(F("Turn On"));
  if(selectedSwitch == MENU_TURN_ON) {
    arduboy.drawRect(menuX1 - 2, menuY1 - 2, arduboy.getCharacterWidth() * 7 + 10, 12 + 4, WHITE);  // Desenha a borda em volta do menu "Turn On"
  }

  arduboy.setCursor(menuX2, menuY2);
  arduboy.print(F("Turn Off"));
  if(selectedSwitch == MENU_TURN_OFF) { 
    arduboy.drawRect(menuX2 - 2, menuY2 - 2, arduboy.getCharacterWidth() * 8 + 12, 12 + 2, WHITE);  // Desenha a borda em volta do menu "Turn Off"
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

// Função para navegar entre os menus "Turn On" e "Turn Off"
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

// Função para navegar entre os lanches
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

// Função para desenhar os menus "Turn On" e "Turn Off" dentro do quadrado
void drawTextMenus(uint8_t rectX, uint8_t rectY, uint8_t rectWidth, uint8_t rectHeight) {
  // Define as coordenadas dos menus de texto
  uint8_t menuX1 = rectX + 10;
  uint8_t menuX2 = rectX + 10;
  uint8_t menuY1 = rectY + rectHeight / 2 - 10;  // Ajusta a altura verticalmente para o primeiro menu
  uint8_t menuY2 = menuY1 + 12;                  // Define a posição vertical do segundo menu abaixo do primeiro

  // Desenha os menus de texto
  arduboy.setCursor(menuX1, menuY1);
  arduboy.print(F("Turn On"));

  arduboy.setCursor(menuX2, menuY2);
  arduboy.print(F("Turn Off"));
}


// Função para desenhar a tela inicial.
void drawStartScreen(uint8_t x, uint8_t y) {

  uint8_t startScrSprSize = 4;

  // Ajuste da posição para desenhar o sprite
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

// Função para desenhar o bichinho
void drawCreature(uint8_t x, uint8_t y) {
  if (isMenuSelected) {
    return;
  }

  // Ajuste da posição para desenhar o sprite
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

  // Ajuste da posição para desenhar o sprite
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

  // Ajuste da posição para desenhar o sprite
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

  // Ajuste da posição para desenhar o sprite
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

// Função para atualizar a animação da criatura
void updateCreatureAnimation() {
  if (isMenuSelected) {
    return;
  }

  // Incrementa o contador do frame atual
  creatureCurrentFrame++;

  // Verifica se é hora de mudar para o próximo frame
  if (creatureCurrentFrame >= creatureNumFrames) {
    creatureCurrentFrame = 0;  // Reinicia a animação
  }
}

// Função para emitir um bip
void emitBeep() {
  // Liga o alto-falante por um curto período para produzir um bip
  tone(5, 1000, 50);  // Toca um som de 1000Hz por 50ms
  delay(10);          // Pequeno atraso para garantir que o bip termine completamente
}

// Função para desenhar os ícones do menu
void drawMenuIcons(uint8_t x, uint8_t numIcons, uint8_t startMenu) {
  for (uint8_t i = 0; i < numIcons; i++) {
    Menu menu = static_cast<Menu>(startMenu + i);

    // Desenha o contorno ao redor do ícone selecionado
    if (currentMenu == menu) {
      arduboy.drawRect(x, iconYOffset + i * iconSpacing - 2, iconWidth + 4, iconHeight + 4, WHITE);
    }

    // Desenha os ícones
    arduboy.setCursor(x + 4, iconYOffset + i * iconSpacing + 4);
    drawCustomBitmap(x, iconYOffset + i * iconSpacing, menuSprites[menu], iconWidth, iconHeight);
  }
}

void drawCustomBitmap(const uint8_t* posXaY, const uint8_t* sprite, uint8_t width, uint8_t height) {
  drawCustomBitmap(posXaY[0], posXaY[1], sprite, width, height);
}

// Função para desenhar um sprite na tela, permitindo ajustes nas colunas
void drawCustomBitmap(uint8_t x, uint8_t y, const uint8_t* sprite, uint8_t width, uint8_t height) {
  uint8_t color = WHITE;
  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      // Verificar se o bit correspondente está definido no sprite
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
