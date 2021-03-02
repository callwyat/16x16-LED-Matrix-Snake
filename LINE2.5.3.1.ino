/*
   Kendall Callister
   2/11/2018
   Snake
   The goal of snake is to get the snake as long as possible without running off the edge of the map or colliding with yourself.
   The snake gets longer as the snake eats the cherries.
*/

//Libaries
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

#define ZigZag

// Defines the pin in which the data is sent to the matrix.
#define PIN 6

#define SCORE_BOARD_RESET_PIN 0

#define GOBBLE_CLOCK_PIN 1
#define GOBBLE_UPDOWN_SELECT_PIN 2

#define SCORE_COUNT_PIN 3
#define SCORE_100_PIN 4

#define COLOR_BACKGROUND 0

// Defines the Matrix.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                   NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

// Sets GREEN, RED, BLUE rotating colors for scrolling text.
const uint16_t colors[] = {
    matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)};

typedef enum
{
  PRE_GAME,
  START_GAME,
  RUNNING_GAME,
  END_GAME,
} GameState_e;

typedef enum
{
  EASY,
  HARD,
} Difficulty_e;

typedef enum
{
  NO_DIRECTION,
  UP,
  DOWN,
  LEFT,
  RIGHT,
} Direction_e;

typedef enum
{
  NO_COLLISION,
  BODY_COLLISION,
  CHERRY_COLLISION,
  GOBBLE_COLLISION,
} Collision_e;

typedef union
{
  struct
  {
    uint8_t Y : 4;
    uint8_t X : 4;
  };

  struct
  {
    uint8_t A : 8;
  };

} Location_t;

typedef struct
{
  uint8_t Length;
  Location_t Head;
} Snake_t;

// Static Variables
Snake_t Snake;
Location_t SnakeBody[256];

Location_t Gobble, Cherry;

GameState_e GameState = PRE_GAME;
Difficulty_e Difficulty = EASY;

int frames;

int i = 0;

unsigned char pointer = 4;
int x = matrix.width();
int pass = 0;
//Sets Snake Color
int sColor = matrix.Color(255, 102, 0);
//Sets Cherry Color
int cColor = matrix.Color(255, 0, 0);
//Sets the Eat Self Color for Hard Mode.
int gColor = matrix.Color(91, 215, 213);

int score = 0;
int times = 0;
int hold = 500;
int blueErase = 0;

Direction_e CurrentDirection = NO_DIRECTION;
Direction_e LastDirection = NO_DIRECTION;

boolean PRINT = true;

Direction_e GetDirection(void)
{

  if (!digitalRead(13))
  {
    return DOWN;
  }
  else if (!digitalRead(12))
  {
    return UP;
  }
  else if (!digitalRead(11))
  {
    return RIGHT;
  }
  else if (!digitalRead(10))
  {
    return LEFT;
  }
  else
  {
    return NO_DIRECTION;
  }
}

int GetPixel(Location_t location)
{

  uint8_t address = location.A;

#ifdef ZigZag
  // Correct for the ZigZag in the LED Array
  if (bitRead(address, 4))
  {
    address ^= 0x0F;
  }
#endif

  return matrix.getPixelColor(address);
}

int SetPixel(Location_t location, int color)
{
  matrix.drawPixel(location.X, location.Y, color);
}

inline boolean LocationsEqual(Location_t left, Location_t right)
{
  return left.A == right.A;
}

Collision_e GetCollision(Location_t location)
{

  if (LocationsEqual(location, Cherry))
  {
    return CHERRY_COLLISION;
  }
  else if (LocationsEqual(location, Gobble))
  {
    return GOBBLE_COLLISION;
  }
  else if (GetPixel(location) == COLOR_BACKGROUND)
  {
    return NO_COLLISION;
  }
  else
  {
    return BODY_COLLISION;
  }
}

Location_t GetRandomLocation(void)
{
  Location_t GetRandomLocation();
  result.X = random(16);
  result.Y = random(16);
  return result;
}

Location_t DrawRandomLocation(int color)
{
  Location_t result = GetRandomLocation();
  SetPixel(result, color);
  return result;
}

void ResetGame(void)
{

  // Reset the score
  digitalWrite(SCORE_BOARD_RESET_PIN, LOW);

  matrix.fillScreen(COLOR_BACKGROUND);
  matrix.show();

  tailLength = 4;
  blueErase = 0;

  score = 0;

  Snake.Head.X = 5;
  Snake.Head.Y = 5;

  hold = 100;

  Cherry = DrawRandomLocation(cColor);

  STARTED = false;
  OVER = false;
  hold = 500;

  if (Hard)
  {
    Gobble = DrawRandomLocation(gColor);
  }

  LastDirection = NO_DIRECTION;

  digitalWrite(SCORE_BOARD_RESET_PIN, HIGH);
}

void setup()
{
  // put your setup code here, to run once:
  //tells matrix to start, clear the screen, and set the Brightness of the martix.
  matrix.begin();
  matrix.fillScreen(0);
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);
  matrix.show();

  //PULLUPs the controller so that when direction made that arduino takes input.
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);

  //Setups the pins need to interface with external digital logic that will display information about the game.
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);

  // Randomizes where the cherry starts
  randomSeed(analogRead(0));
  // Allows for the arduino to talk to the computer.
  //Serial.begin(9600);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(1);

  //Changes the Direction of the snake.
  Direction_e activeDirection = GetDirection();

  if (activeDirection != NO_DIRECTION)
  {
    LastDirection = activeDirection;

    // Only allow changes from Vertical to Horizontal or vise versa
    if (CurrentDirection == UP || CurrentDirection == DOWN)
    {
      if (activeDirection == LEFT || activeDirection == RIGHT)
      {
        CurrentDirection = activeDirection;
      }
    }
    else
    {
      if (activeDirection == UP || activeDirection || DOWN)
      {
        CurrentDirection = activeDirection;
      }
    }
    if (CurrentDirection == LEFT || CurrentDirection)
      CurrentDirection = activeDirection;
  }

  //Allows controls to be input separately from movement in game.
  if (++frames >= hold)
  {
    frames = 0;

    switch (GameState)
    {
    case PRE_GAME:
      matrix.show();
      matrix.fillScreen(0);
      matrix.setCursor(x, 0);

      // Displays how to select the hard difficulty.
      matrix.setTextColor(matrix.Color(255, 93, 21));
      matrix.print("Hard");
      // Draw up arrow
      matrix.drawLine(x + 27, 0, x + 27, 6, matrix.Color(255, 0, 0));
      matrix.drawLine(x + 27, 0, x + 25, 2, matrix.Color(255, 0, 0));
      matrix.drawLine(x + 27, 0, x + 29, 2, matrix.Color(255, 0, 0));
      matrix.setCursor(x, 8);

      // Displays how to select the easy difficulty.
      matrix.print("Easy");
      matrix.drawLine(x + 27, 15, x + 27, 8, matrix.Color(255, 0, 0));
      matrix.drawLine(x + 27, 15, x + 25, 13, matrix.Color(255, 0, 0));
      matrix.drawLine(x + 27, 15, x + 29, 13, matrix.Color(255, 0, 0));

      if (--x < -29)
      {
        x = matrix.width();
        if (++pass >= 3)
          pass = 0;
      }

      // Grabs the player input if they select hard.
      /*
        * Hard Mode adds shedding to the game. If the Player hits the she--dded skin it will also end the game.
        * An new item is added to the game that allows the player to eat themselves and the shedded skin, but
        * eating the item will increase the speed of the snake without increasing the score.
        */

      switch (LastDirection)
      {
      case UP:
        Difficulty = HARD;
        GameState = START_GAME;
        break;

      case DOWN:
        Difficulty = EASY;
        GameState = START_GAME;
        break;
      }
      break;

    case START_GAME:
      ResetGame();
      GameState = RUNNING_GAME;
      break;

    case RUNNING_GAME:

      digitalWrite(GOBBLE_UPDOWN_SELECT_PIN, LOW);
      digitalWrite(GOBBLE_CLOCK_PIN, LOW);
      digitalWrite(SCORE_COUNT_PIN, LOW);
      delay(1);

      // Makes sure the snake is not outside of the boundaries.
      switch (CurrentDirection)
      {
      case UP:
        if (Snake.Head.Y == 0)
        {
          GameState = END_GAME;
        }
        break;
      case DOWN:
        if (Snake.Head.Y == 15)
        {
          GameState = END_GAME;
        }
        break;
      case LEFT:
        if (Snake.Head.X == 0)
        {
          GameState = END_GAME;
        }
        break;
      case RIGHT:
        if (Snake.Head.X == 15)
        {
          GameState = END_GAME;
        }
        break;
      }

      if (GameState == END_GAME)
      {
        return;
      }

      // Adds the color of the snake to the board and detects collision with the snake.

      Location_t collisionLocation = Snake.Head;

      switch (CurrentDirection)
      {
      case UP:
        --collisionLocation.Y;
        break;
      case DOWN:
        ++collisionLocation.Y;
        break;
      case LEFT:
        --collisionLocation.X;
        break;
      case RIGHT:
        ++collisionLocation.X;
        break;
      }

      switch (GetCollision(collisionLocation))
      {
      case BODY_COLLISION:
        if (blueErase == 0)
        {
          OVER = true;
        }
        else
        {
          digitalWrite(GOBBLE_CLOCK_PIN, HIGH);
          --blueErase;
          ++Snake.Length;
        }
        break;

      case CHERRY_COLLISION:

        Cherry = DrawRandomLocation(cColor);

        ++Snake.Length;
        ++score;

        digitalWrite(SCORE_COUNT_PIN, HIGH);
        digitalWrite(SCORE_100_PIN, score >= 100);

        if (hold > 100)
        {
          hold -= 10;
        }
        break;

      case GOBBLE_COLLISION:

        Gobble = DrawRandomLocation(gColor);

        ++blueErase;

        digitalWrite(GOBBLE_UPDOWN_SELECT_PIN, HIGH);
        delay(1);
        digitalWrite(GOBBLE_CLOCK_PIN, HIGH);

        break;

      case NO_COLLISION:
        // Nothing to do sir
        break;
      }

      SetPixel(collisionLocation, sColor);

      // Erases the snakes tail.
      if (++pointer == 256)
      {
        pointer = 0;
      }

      SnakeBody[pointer] = Snake.Head = collisionLocation;

      uint8_t tailPointer = pointer - tailLength;

      if (tailPointer < 0 && Difficulty != HARD)
      {
        tailPointer += 256;
      }

      if (tailPointer > 0)
      {
        Location_t clearLocation = SnakeBody[tailPointer];

        int color = COLOR_BACKGROUND;

        if (LocationsEqual(clearLocation, Cherry))
        {
          color = cColor;
        }
        else if (LocationsEqual(clearLocation, Gobble))
        {
          color = gColor;
        }

        SetPixel(clearLocation, color);

        matrix.show();
        break;

      case END_GAME:

        static loopTimes = 0;

        // delay the matrix then repeat.
        if ((loopTimes >= 1 && LastDirection == RIGHT || LastDirection == LEFT) || loopTimes >= 3)
        {
          loopTimes = 0;
          GameState = PRE_GAME;
        }
        else
        {

          hold = 100;
          matrix.fillScreen(0);
          matrix.setCursor(x, 0);
          matrix.print(F("Gameover"));

          if (--x < -46)
          {
            x = matrix.width();
            if (++pass >= 3)
              pass = 0;
            matrix.setTextColor(colors[pass]);
            times++;
          }
          matrix.setCursor(0, 8);
          matrix.print(score);
          matrix.show();
        }

        break;
      }
    }
  }
