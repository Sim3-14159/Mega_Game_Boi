/**
 * CONFIG.h -- Pin and gameplay configuration for Snake_Game.ino.
 * These digital and analog pin numbers are valid on the Arduino Mega 2560.
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

// the 2d size of a single segment on the snake
#define snakeSize 10

// how often to update the game, for each time reading the input
#define INPUT_TO_UPDATE_RATIO   3

// game FPS
#define FPS                     30


// pin to read random values from. must be different than any other pins defined here
#define ANALOG_READ_PIN         A0
