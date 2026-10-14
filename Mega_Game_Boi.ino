#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <TM1637Display.h>

#include "CONFIG.h"

const int cycleDelayAmount = static_cast<int>(FPS);

// Initialize the display
// Hardware I2C follows the selected board: on a Mega 2560, wire SDA to D20/SDA
// and SCL to D21/SCL. The Wire/U8g2 hardware-I2C constructors select these pins.
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // uncomment for high quality
//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // uncomment for less RAM usage
TM1637Display scoreDisplay(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);

byte snakeX[75], snakeY[75];
byte snakeLength;
byte foodX, foodY;
// 0: right, 1: down, 2: left, 3: up
byte direction = 0;
byte changeDirection = 0;

bool gameOver = false;
bool hasStarted = false;
bool soundMuted = false;


void setup() {
  /* uncomment the following line the first time you upload to reset EEPROM. Then, once uploaded, comment the line again and re-upload, to prevent the high score
  * being reset every setup()
   */
  //EEPROM.write(0, 0);  // Reset high score once
  randomSeed(analogRead(A0));
  u8g2.begin();
  scoreDisplay.setBrightness(0x0f);
  scoreDisplay.showNumberDec(0, false, 4, 0);

  pinMode(uButton, INPUT_PULLUP);
  pinMode(dButton, INPUT_PULLUP);
  pinMode(lButton, INPUT_PULLUP);
  pinMode(rButton, INPUT_PULLUP);
  pinMode(aButton, INPUT_PULLUP);
  pinMode(bButton, INPUT_PULLUP);
  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  for (byte i = 0; i < LED_BAR_LENGTH; i++) {
    pinMode(LED_BAR_FIRST_PIN + i, OUTPUT);
    digitalWrite(LED_BAR_FIRST_PIN + i, LOW);
  }

  digitalWrite(G_LED, HIGH);
  digitalWrite(R_LED, HIGH);
}

void loop() {
  static bool wasGameOver = false;
  static unsigned long int k = 0;
  processMuteButton();

  // ----------- START SCREEN ------------
  if (!hasStarted) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso16_tr);

    u8g2.drawStr(27,17,  "S");
    u8g2.drawStr(37,20,  "N");
    u8g2.drawStr(47,17,  "A");
    u8g2.drawStr(57,20,  "K");
    u8g2.drawStr(67,17,  "E");
    u8g2.drawStr(77,20, "!");
    u8g2.drawStr(84,20, "!");
    u8g2.drawStr(91,17, "!");
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(15, 50);
    u8g2.print(F("A / STICK TO START"));
    u8g2.sendBuffer();

    if (startButtonPressed()) {
      hasStarted = true;
      resetGame();
      delay(300); // debounce
    }
    return;
  }

  if (gameOver) {
    // Static local variables inside gameOver block
    static int buzzerTimer = 0;
    static bool highScoreChecked = false;
    static bool showHighScore = false;
    static byte highScore = 0;

    // Detect rising edge of gameOver state
    if (!wasGameOver) {
      // Just entered gameOver state, reset statics
      buzzerTimer = 0;
      highScoreChecked = false;
      showHighScore = false;
      highScore = 0;
    }

    if (!highScoreChecked) {
      byte storedScore = EEPROM.read(0);
      byte currentScore = snakeLength - 2;
      if (currentScore > storedScore) {
        EEPROM.write(0, currentScore);
        highScore = currentScore;
        showHighScore = true;
      } else {
        showHighScore = false;
      }
      highScoreChecked = true;
    }

    if (startButtonPressed()) {
      gameOver = false;
      resetGame();
      hasStarted = true;
      noTone(BUZZER);
      delay(300);
      // Also update wasGameOver so next frame it knows we left gameOver state
      wasGameOver = false;
      return; // immediately exit so loop continues fresh next time
    }

    if (buzzerTimer < 15) {  // about 1 second at 34ms delay + calculation time
      playGameTone(400);
      buzzerTimer++;
    } else {
      noTone(BUZZER);
    }

    digitalWrite(R_LED, LOW);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.setCursor(18, 20);
    u8g2.print(F("GAME OVER!"));

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(15, 40);
    u8g2.print(F("A / STICK TO RESTART"));

    if (showHighScore) {
      u8g2.setCursor(12, 60);
      u8g2.print(F("NEW HI SCORE: "));
      u8g2.print(highScore);
    }

    u8g2.sendBuffer();

    wasGameOver = true;  // Remember state for next frame
    return;
  } else {
    wasGameOver = false; // Update if we are not in game over state
  }
  

  // ----------- MAIN GAME LOOP ------------
  readJoystick();

  // Update the state INPUT_TO_UPDATE_RATIO more times than the output,
  // to allow the game to be more responsive, but not too fast.
  if (!(k % INPUT_TO_UPDATE_RATIO)) {
    digitalWrite(G_LED, HIGH);
    noTone(BUZZER);
    digitalWrite(R_LED, HIGH);

    // Update direction safely
    if (changeDirection == 0 && direction != 2) direction = 0;
    else if (changeDirection == 2 && direction != 0) direction = 2;
    else if (changeDirection == 3 && direction != 1) direction = 3;
    else if (changeDirection == 1 && direction != 3) direction = 1;

    moveSnake();
    checkCollision();
    draw();
  }

  // default FPS==30, cycleDelayAmount==33
  delay(cycleDelayAmount);
  k++;
}

void readJoystick() {
  // The four tactile direction buttons take priority over the analog stick.
  if (!digitalRead(rButton)) {
    changeDirection = 0;
    return;
  } else if (!digitalRead(lButton)) {
    changeDirection = 2;
    return;
  } else if (!digitalRead(uButton)) {
    changeDirection = 3;
    return;
  } else if (!digitalRead(dButton)) {
    changeDirection = 1;
    return;
  }

  int xOffset = analogRead(JOYSTICK_X_PIN) - JOYSTICK_CENTER;
  int yOffset = analogRead(JOYSTICK_Y_PIN) - JOYSTICK_CENTER;
  int absX = abs(xOffset);
  int absY = abs(yOffset);

  if (absX < JOYSTICK_DEADZONE && absY < JOYSTICK_DEADZONE) {
    return; // Ignore the joystick's small center-position noise.
  }

  // Pick the stronger axis so a diagonal stick movement selects one direction.
  if (absX > absY) {
    changeDirection = xOffset < 0 ? 2 : 0;
  } else {
    // On common joystick modules, pushing up lowers the Y reading.
    changeDirection = yOffset < 0 ? 3 : 1;
  }
}

bool startButtonPressed() {
  return !digitalRead(aButton) || !digitalRead(JOYSTICK_SW_PIN);
}

void processMuteButton() {
  static int lastReading = HIGH;
  static int stableState = HIGH;
  static unsigned long lastDebounceTime = 0;

  int reading = digitalRead(bButton);
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 35 && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {
      soundMuted = !soundMuted;
      if (soundMuted) {
        noTone(BUZZER);
      }
    }
  }

  lastReading = reading;
}

void playGameTone(unsigned int frequency) {
  if (!soundMuted) {
    tone(BUZZER, frequency);
  }
}

void updateScoreOutputs() {
  byte score = snakeLength >= 2 ? snakeLength - 2 : 0;
  scoreDisplay.showNumberDec(score, false, 4, 0);

  // The bar shows progress through the current ten-food interval.
  byte litSegments = score == 0 ? 0 : ((score - 1) % LED_BAR_LENGTH) + 1;
  for (byte i = 0; i < LED_BAR_LENGTH; i++) {
    digitalWrite(LED_BAR_FIRST_PIN + i, i < litSegments ? HIGH : LOW);
  }
}

void moveSnake() {
  byte newX = snakeX[0];
  byte newY = snakeY[0];

  switch (direction) {
    case 0: newX += snakeSize; break;
    case 1: newY += snakeSize; break;
    case 2: newX -= snakeSize; break;
    case 3: newY -= snakeSize; break;
  }

  for (byte i = snakeLength; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  snakeX[0] = newX;
  snakeY[0] = newY;
}

void checkCollision() {
  if (((snakeX[0] <= foodX + 5) && (snakeX[0] >= foodX - 5)) &&
      ((snakeY[0] >= foodY - 5) && (snakeY[0] <= foodY + 5))) {
    snakeLength++;
    spawnFood();
    digitalWrite(G_LED, LOW);
    playGameTone(600);
  }

  if (snakeX[0] < 0 || snakeX[0] >= 128 || snakeY[0] < 0 || snakeY[0] >= 64) {
    gameOver = true;
  }

  for (byte i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
    }
  }
}

void draw() {
  u8g2.firstPage();
  do {
    u8g2.setDrawColor(1);
    u8g2.drawFrame(0, 0, 128, 64);

    // Draw the snake
    for (byte i = 0; i < snakeLength; i++) {
      u8g2.drawBox(snakeX[i], snakeY[i], snakeSize, snakeSize);
    }

    // Draw round food
    u8g2.drawDisc(2 + foodX + (snakeSize / 2), foodY + snakeSize / 2, snakeSize / 2);

    // Display the score
    u8g2.setFont(u8g2_font_5x8_tr); // Small readable font
    u8g2.setCursor(3, 8);
    u8g2.print(F("Score: "));
    u8g2.print(snakeLength - 2); // Score starts at 0

    if (soundMuted) {
      u8g2.setCursor(116, 8);
      u8g2.print(F("M"));
    }
  } while (u8g2.nextPage());
  updateScoreOutputs();
}

void spawnFood() {
  foodX = random(0, 128 / snakeSize) * snakeSize;
  foodY = random(0, 64 / snakeSize) * snakeSize;
}

void resetGame() {
  randomSeed(analogRead(ANALOG_READ_PIN));
  snakeLength = 2;
  snakeX[0] = 64;
  snakeY[0] = 32;
  snakeX[1] = snakeX[0] - snakeSize;
  snakeY[1] = snakeY[0];
  direction = 0;
  changeDirection = 0;
  gameOver = false;
  spawnFood();
  updateScoreOutputs();
}
