/**
 * CONFIG.h -- Pin and gameplay configuration for Mega_Game_Boi.
 * The pin assignments below target the Arduino Mega 2560.
 */

// [U]p button pin
#define uButton   2

// [D]own button pin
#define dButton   3

// [L]eft button pin
#define lButton   4

// [R]ight button pin
#define rButton   5

// red LED pin
#define R_LED     7

// green LED pin
#define G_LED     6

// passive buzzer
#define BUZZER    A3

// [A] button
#define aButton   A1

// [B] button toggles the buzzer mute state
#define bButton   23

// Two-axis analog joystick module
#define JOYSTICK_X_PIN   A8
#define JOYSTICK_Y_PIN   A9
#define JOYSTICK_SW_PIN  22
#define JOYSTICK_CENTER  512
#define JOYSTICK_DEADZONE 180

// Four-digit TM1637 display module
#define DISPLAY_CLK_PIN  24
#define DISPLAY_DIO_PIN  25

// Ten-segment LED bar: one output and one series resistor per segment
#define LED_BAR_FIRST_PIN 30
#define LED_BAR_LENGTH    10
#define LED_BAR_RESISTOR_OHMS 470

// the 2d size of a single segment on the snake
#define snakeSize 10

// how often to update the game, for each time reading the input
#define INPUT_TO_UPDATE_RATIO   3

// game FPS
#define FPS                     30


// pin to read random values from. must be different than any other pins defined here
#define ANALOG_READ_PIN         A0
