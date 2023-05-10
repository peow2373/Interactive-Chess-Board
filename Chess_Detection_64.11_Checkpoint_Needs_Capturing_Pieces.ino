#include <Adafruit_NeoPixel.h>

//-        Black = pin50        -//     Row 8
//-          Pixels <--         -//
//- Yellow Board || Green Board -//                BOARD ORIENTATION
//- Red Board    || Blue Board  -//                --> PORTSIDE
//-        White = pin52        -//
//-          Pixels -->         -//     Row 1


// Create Neopixel objects
#define whiteStartPin    52  // Digital Output connected to the neopixels to the left of portside
#define blackStartPin    50  // Digital Output connected to the neopixels to the right of portside
#define pixelCount  512      // Number of neopixels

// Define White start and Black start pixels
Adafruit_NeoPixel pixels[2] = {Adafruit_NeoPixel(pixelCount, whiteStartPin, NEO_GRB + NEO_KHZ800),  Adafruit_NeoPixel(pixelCount, blackStartPin, NEO_GRB + NEO_KHZ800)};


// Multiplexer pins                                 BOARD ORDER--> Yellow, Green, Red, Blue
int SIG_pin[4] = {A0, A1, A2, A3};
int s0[4] = {35, 5, 38, 10};
int s1[4] = {33, 4, 40, 11};
int s2[4] = {31, 3, 42, 12};
int s3[4] = {29, 2, 44, 13};


const int rows = 8;
const int columns = 8;

// Array of Neopixel square states
int squareStates[rows][columns] = {
// Top Left--> A8
                  {-1, -2, -1, -2, -1, -2, -1, -2},                     // Row 8                      -2 = Empty BLACK square         (0, 0, 0)     
                  {-2, -1, -2, -1, -2, -1, -2, -1},                     // Row 7                      -1 = Empty WHITE square         (255, 255, 255)
                  {-1, -2, -1, -2, -1, -2, -1, -2},                     // Row 6                       0 = Piece on square            (0, 0, 0) or (255, 255, 255)
                  {-2, -1, -2, -1, -2, -1, -2, -1},                     // Row 5                       1 = Piece picked up            (0, 0, 255)
//                                                                                                     2 = Piece VALID movement       (0, 255, 0)
                  {-1, -2, -1, -2, -1, -2, -1, -2},                     // Row 4                       3 = Piece INVALID movement     (255, 0, 0)
                  {-2, -1, -2, -1, -2, -1, -2, -1},                     // Row 3                       4 = Indicated CHECK            (255, 255, 0)
                  {-1, -2, -1, -2, -1, -2, -1, -2},                     // Row 2                       5 = Indicated CHECKMATE        (255, 165, 0)
                  {-2, -1, -2, -1, -2, -1, -2, -1}                      // Row 1                       6 = Previous piece movement    (215, 190, 215)
//                                                 H1 <--Bottom Right
};


// Array of Hall Effect sensor positions
int sensorPositions[rows][columns] = {
// Top Left--> YELLOW                           GREEN <--Top Right
                  {12,  8,  4,  0,    12,  8,  4,  0},                     // Row 8   
                  {13,  9,  5,  1,    13,  9,  5,  1},                     // Row 7
                  {15, 11,  7,  3,    15, 11,  7,  3},                     // Row 6 
                  {14, 10,  6,  2,    14, 10,  6,  2},                     // Row 5

                  { 2,  6, 10, 14,     2,  6, 10, 14},                     // Row 4
                  { 3,  7, 11, 15,     3,  7, 11, 15},                     // Row 3 
                  { 1,  5,  9, 13,     1,  5,  9, 13},                     // Row 2 
                  { 0,  4,  8, 12,     0,  4,  8, 12}                      // Row 1
// Bottom Left--> RED                            BLUE <--Bottom Right 
};


// Array of Hall Effect sensor states
bool sensorStates[rows][columns] = {
// Top Left--> YELLOW                                                   GREEN <--Top Right
                  {false, false, false, false,    false, false, false, false},                     // Row 8   
                  {false, false, false, false,    false, false, false, false},                     // Row 7
                  {false, false, false, false,    false, false, false, false},                     // Row 6 
                  {false, false, false, false,    false, false, false, false},                     // Row 5

                  {false, false, false, false,    false, false, false, false},                     // Row 4
                  {false, false, false, false,    false, false, false, false},                     // Row 3 
                  {false, false, false, false,    false, false, false, false},                     // Row 2 
                  {false, false, false, false,    false, false, false, false}                      // Row 1
// Bottom Left--> RED                                                    BLUE <--Bottom Right   
};


// Array of Chess piece positions
char positions[rows][columns] = {
// Top Left--> A8
                  {'E','E','E','E','E','E','E','E'},                     // Row 8   
                  {'E','E','E','E','E','E','E','E'},                     // Row 7   
                  {'E','E','E','E','E','E','E','E'},                     // Row 6   
                  {'E','E','E','E','E','E','E','E'},                     // Row 5   
                  {'E','E','E','E','E','E','E','E'},                     // Row 4   
                  {'E','E','E','E','E','E','E','E'},                     // Row 3   
                  {'E','E','E','E','E','E','E','E'},                     // Row 2   
                  {'E','E','E','E','E','E','E','E'},                     // Row 1   
//                                                 H1 <--Bottom Right
};


// Variables to control neopixels
bool startUp = false;
int startDelay = 1000;
bool boardReset = true;
bool resetOnce = false;

// Keep track of a piece when it's picked up
int activeSquare[2] = {-1,-1};


void setup() {

  // Initialize magnet sensors and set their outputs to low to start
  for (int i = 0; i < 4; i++) {
    pinMode(s0[i], OUTPUT); 
    pinMode(s1[i], OUTPUT); 
    pinMode(s2[i], OUTPUT); 
    pinMode(s3[i], OUTPUT); 
  
    digitalWrite(s0[i], LOW);
    digitalWrite(s1[i], LOW);
    digitalWrite(s2[i], LOW);
    digitalWrite(s3[i], LOW);
  }

  // Initialize neopixel strips
  for (int j = 0; j < 2; j++) {
    pixels[j].begin(); // Initialize NeoPixel object
    pixels[j].show();  // Initialize all pixels to 'off'
    pixels[j].setBrightness(25);
  }

  Serial.begin(9600);
}


void loop() {

  // Neopixel wipe
  if (!startUp) {
    for(int i = 0; i < 16; i++) {
      for(int j = 0; j < 32; j++) {
        if(j % 2 == 0) {
          ALL_Pixels(j * 16 + i, 255, 255, 255);
        } else {
          ALL_Pixels((j + 1) * 16 - (i + 1), 255, 255, 255);
        }
      }
      for (int k = 0; k < 2; k++) pixels[k].show();
      delay(25);
    }
  
    // Wait a second, turn the Black squares pixels off, then wait another second
    for (int m = 8; m < 8; m++) {
      for (int n = 0; n < 8; n++) {
        if (standardSquare(m,n) == -2) lightSquare(m, n, 0, 0, 0);
      }
    }
    startUp = true;
    delay(startDelay);
  }

  // Determine the color of each square if a change in the board was detected
  for (int col = 0; col < 8; col++) {
    for (int row = 0; row < 8; row++) {
      switch(squareStates[row][col]) {

        // Empty BLACK square
        case -2:
          lightSquare(row, col, 0, 0, 0);
          break;

        // Empty WHITE square
        case -1:
          lightSquare(row, col, 255, 255, 255);
          break;

        // Piece detected on square
        case 0:
          if (standardSquare(row, col) == -2) lightSquare(row, col, 0, 0, 0);
          else if (standardSquare(row, col) == -1) lightSquare(row, col, 255, 255, 255);
          break;

        // Board reset or piece picked up and set down
        case 1:
          lightSquare(row, col, 0, 0, 255);
          break;

        // Valid piece movement option
        case 2:
          lightSquare(row, col, 0, 255, 0);
          break;

        // Invalid piece movement option
        case 3:
          lightSquare(row, col, 255, 0, 0);
          break;
                                                                                            // TO-DO: Add indicated check
       // Indicated CHECK
        case 4:
          lightSquare(row, col, 255, 255, 0);
          break;
                                                                                            // TO-DO: Add indicated checkmate
       // Indicated CHECKMATE
        case 5:
          lightSquare(row, col, 255, 165, 0);
          break;
      }
    }
  }

  // Iterate through the Hall Effect sensors for each square
  senseSquares();

  // Check if a piece has been picked up
  pieceMovement();

  Serial.println(activeSquare[0]);

  // Check if the board has been reset
  boardReset = true;
  for (int y = 0; y < 8; y++) {
    if (sensorStates[2][y] == true || sensorStates[3][y] == true || sensorStates[4][y] == true || sensorStates[5][y] == true) boardReset = false;
    else if (resetOnce && (positions[0][y] == 'E' || positions[1][y] == 'E' || positions[6][y] == 'E' || positions[7][y] == 'E')) boardReset = false;
  }
  if (boardReset) resetBoard();
}


// Detect pieces on each of the Hall Effect sensors
void senseSquares() {

  // Update the square state if it has changed
  for (int c = 0; c < 16; c++) {
    for (int index = 0; index < 4; index++) {

      // Read all 64 Hall Effect sensors
      int row = 0;
      int col = 0;
      if (index == 1) col = 4;
      else if (index == 2) {
        row = -7;
        col = -3;
      }
      else if (index == 3) {
        row = -7;
        col = -7;
      }
      
      switch(c) {
      // Row 1
        case 12:
          if (readMux(c, index) <= 1.00) changeState(abs(row), abs(col), true);
          else changeState(abs(row), abs(col), false);
          break;
        case 8:
          if (readMux(c, index) <= 1.00) changeState(abs(row), abs(col + 1), true);
          else changeState(abs(row), abs(col + 1), false);
          break;
        case 4:
          if (readMux(c, index) <= 1.00) changeState(abs(row), abs(col + 2), true);
          else changeState(abs(row), abs(col + 2), false);
          break;
        case 0:
          if (readMux(c, index) <= 1.00) changeState(abs(row), abs(col + 3), true);
          else changeState(abs(row), abs(col + 3), false);
          break;
  
      // Row 2
        case 13:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 1), abs(col), true);
          else changeState(abs(row + 1), abs(col), false);
          break;
        case 9:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 1), abs(col + 1), true);
          else changeState(abs(row + 1), abs(col + 1), false);
          break;
        case 5:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 1), abs(col + 2), true);
          else changeState(abs(row + 1), abs(col + 2), false);
          break;
        case 1:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 1), abs(col + 3), true);
          else changeState(abs(row + 1), abs(col + 3), false);
          break;
  
      // Row 3
        case 15:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 2), abs(col), true);
          else changeState(abs(row + 2), abs(col), false);
          break;
        case 11:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 2), abs(col + 1), true);
          else changeState(abs(row + 2), abs(col + 1), false);
          break;
        case 7:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 2), abs(col + 2), true);
          else changeState(abs(row + 2), abs(col + 2), false);
          break;
        case 3:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 2), abs(col + 3), true);
          else changeState(abs(row + 2), abs(col + 3), false);
          break;
  
      // Row 4
        case 14:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 3), abs(col), true);
          else changeState(abs(row + 3), abs(col), false);
          break;
        case 10:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 3), abs(col + 1), true);
          else changeState(abs(row + 3), abs(col + 1), false);
          break;
        case 6:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 3), abs(col + 2), true);
          else changeState(abs(row + 3), abs(col + 2), false);
          break;
        case 2:
          if (readMux(c, index) <= 1.00) changeState(abs(row + 3), abs(col + 3), true);
          else changeState(abs(row + 3), abs(col + 3), false);
          break;
      }
    }
  }
}

// Update the square if it detects a piece
void changeState(int x, int y, bool state) {
  if (state == false) {
    
    // Piece has been picked up
    if (positions[x][y] != 'E' && activeSquare[0] == -1 && activeSquare[1] == -1) {
      squareStates[x][y] = 1;                                                           
      
      if (resetOnce) {
        activeSquare[0] = x;
        activeSquare[1] = y;
        pieceOptions(positions[x][y], x, y); 
      }     
    }
    
    // Reset square
    else if (squareStates[x][y] != 2 && positions[x][y] == 'E' && squareStates[x][y] != 3) squareStates[x][y] = standardSquare(x,y);             
  } 
  
  // Indicate a piece is on a square
  else {
    if (squareStates[x][y] == 2 && positions[x][y] != 'E') squareStates[x][y] = 3;                              // TO-DO: Add square states here to be reset once used
    else if (squareStates[x][y] == 3) squareStates[x][y] = 3;
    else squareStates[x][y] = 0;                                              
  }
  
  sensorStates[x][y] = state;
}


// Determine if a piece has been picked up or moved
void pieceMovement() {
      
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {

      // Check if the piece movement option is invalid because it overlaps another piece                            TO-DO: Add White/Black piece identification
      if (squareStates[x][y] == 2 && positions[x][y] != 'E') squareStates[x][y] = 3;
      

      // Check if the lifted piece was set down in a different position                 // TO DO: FIX LATER (This code won't work when taking another piece)
      if (squareStates[x][y] == 0 && positions[x][y] == 'E') {
        positions[x][y] = positions[activeSquare[0]][activeSquare[1]];
        positions[activeSquare[0]][activeSquare[1]] = 'E';

        activeSquare[0] = -1;
        activeSquare[1] = -1;
      }
      
      // Check if a piece was replaced after being lifted up
      if (squareStates[activeSquare[0]][activeSquare[1]] == 0) {
        activeSquare[0] = -1;
        activeSquare[1] = -1;
      }
    }
  }

  // Reset the squares that show the piece movement options                                           // IMPORTANT: Update for every new square state
  if (activeSquare[0] == -1 && activeSquare[1] == -1) {
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        if (squareStates[row][col] == 1 || squareStates[row][col] == 2 || squareStates[row][col] == 3) squareStates[row][col] = standardSquare(row, col);
      }
    }
  }
}


// Determine the movement options for the chosen piece
void pieceOptions(char piece, int row, int col) {

  switch(piece) {
  // Black Pawn                                                                                   // TO-DO: Code En-Passant
    case 'p':
      // Two space movement
      if (row + 1 < 8) squareStates[row + 1][col] = 2;
      if (row == 1) {
        squareStates[row + 2][col] = 2;
        if (positions[row + 1][col] != 'E') squareStates[row + 2][col] = 3;
      }

      // Diagonal capture
      if (row + 1 < 8 && col + 1 < 8 && squareStates[row + 1][col + 1] == 0 && isUpperCase(positions[row + 1][col + 1])) squareStates[row + 1][col + 1] = 3; 
      if (row + 1 < 8 && col - 1 >= 0 && squareStates[row + 1][col - 1] == 0 && isUpperCase(positions[row + 1][col - 1])) squareStates[row + 1][col - 1] = 3; 
      break;

  // White Pawn
    case 'P':
      // Two space movement
      if (row - 1 >= 0) squareStates[row - 1][col] = 2;
      if (row == 6) {
        squareStates[row - 2][col] = 2;
        if (positions[row - 1][col] != 'E') squareStates[row - 2][col] = 3;
      }

      // Diagonal capture
      if (row - 1 >= 0 && col + 1 < 8 && squareStates[row - 1][col + 1] == 0 && isLowerCase(positions[row - 1][col + 1])) squareStates[row - 1][col + 1] = 3; 
      if (row - 1 >= 0 && col - 1 >= 0 && squareStates[row - 1][col - 1] == 0 && isLowerCase(positions[row - 1][col - 1])) squareStates[row - 1][col - 1] = 3; 
      break;

  // Rook
    case 'R':
    case 'r':    
      for (int i = 0; i < 8; i++) {
        // Vertical movement
        if (row - i >= 0) squareStates[row - i][col] = 2;
        if (row + i < 8) squareStates[row + i][col] = 2;

        // Horizontal movement
        if (col - i >= 0) squareStates[row][col - i] = 2;
        if (col + i < 8) squareStates[row][col + i] = 2;
      }
      break;

  // Knight
    case 'N':
    case 'n':
      // North movement
      if (row - 2 >= 0) {
        if (col - 1 >= 0) squareStates[row - 2][col - 1] = 2;
        if (col + 1 < 8) squareStates[row - 2][col + 1] = 2;
      }

      // South movement  
      if (row + 2 < 8) {
        if (col - 1 >= 0) squareStates[row + 2][col - 1] = 2;
        if (col + 1 < 8) squareStates[row + 2][col + 1] = 2;
      }

      // East movement
      if (col - 2 >= 0) {
        if (row - 1 >= 0) squareStates[row - 1][col - 2] = 2;
        if (row + 1 < 8) squareStates[row + 1][col - 2] = 2;
      }

      // West movement  
      if (col + 2 < 8) {
        if (row - 1 >= 0) squareStates[row - 1][col + 2] = 2;
        if (row + 1 < 8) squareStates[row + 1][col + 2] = 2;
      }
      break;

  // Bishop
    case 'B':
    case 'b':
      for (int i = 0; i < 8; i++) {
        // Upwards diagonal movement
        if (row - i >= 0) {
          if (col - i >= 0) squareStates[row - i][col - i] = 2;
          if (col + i < 8) squareStates[row - i][col + i] = 2;
        }

        // Downwards diagonal movement
        if (row + i < 8) {
          if (col - i >= 0) squareStates[row + i][col - i] = 2;
          if (col + i < 8) squareStates[row + i][col + i] = 2;
        }
      }
      break;

  // Queen
    case 'Q':
    case 'q':
      for (int i = 0; i < 8; i++) {
        
        // Vertical movement
        if (row - i >= 0) {
          // Downwards diagonal movement
          if (col - i >= 0) squareStates[row - i][col - i] = 2;
          if (col + i < 8) squareStates[row - i][col + i] = 2;
          squareStates[row - i][col] = 2;
        }
        if (row + i < 8) {
          // Upwards diagonal movement
          if (col - i >= 0) squareStates[row + i][col - i] = 2;
          if (col + i < 8) squareStates[row + i][col + i] = 2;
          squareStates[row + i][col] = 2;
        }

        // Horizontal movement
        if (col - i >= 0) squareStates[row][col - i] = 2;
        if (col + i < 8) squareStates[row][col + i] = 2;
      }
      break;

  // King                                                                                   // TO-DO: Code Castling
    case 'K':
    case 'k':
      // Downwards movement
      if (row - 1 >= 0) {
        if (col - 1 >= 0) squareStates[row - 1][col - 1] = 2;
        if (col + 1 < 8) squareStates[row - 1][col + 1] = 2;
        squareStates[row - 1][col] = 2;
      }

      // Upwards movement
      if (row + 1 < 8) {
        if (col - 1 >= 0) squareStates[row + 1][col - 1] = 2;
        if (col + 1 < 8) squareStates[row + 1][col + 1] = 2;
        squareStates[row + 1][col] = 2;
      }

      // Horizontal movement
      if (col - 1 >= 0) squareStates[row][col - 1] = 2;
      if (col + 1 < 8) squareStates[row][col + 1] = 2;
      break;
  }

  // Indicate the square the piece was chosen from
  squareStates[row][col] = 1;
}



// Indicate the board has been reset and reassign piece positions
void resetBoard() {
  char whitePieces[8] = {'R','N','B','K','Q','B','N','R'};
  char blackPieces[8] = {'r','n','b','k','q','b','n','r'};

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      // Change the color of the squares
      if (x == 0 || x == 1 || x == 6 || x == 7) {
        if (activeSquare[0] == -1 && activeSquare[1] == -1) squareStates[x][y] = 1;
      }
  
      // Reassign the piece positions
      if (x == 0) positions[x][y] = blackPieces[y];
      else if (x == 1) positions[x][y] = 'p';
      else if (x == 6) positions[x][y] = 'P';
      else if (x == 7) positions[x][y] = whitePieces[y];
      else positions[x][y] = 'E';
    }
  }

  // Reset piece position markers
  activeSquare[0] = -1;
  activeSquare[1] = -1;
  resetOnce = true;
}



// Turn on both the White and Black side neopixels
void ALL_Pixels(int index, int r, int g, int b) {
  for (int i = 0; i < 2; i++) {
    pixels[i].setPixelColor(index, pixels[i].Color(r, g, b));
  }
}

// Determine whether a square should be colored White or Black
int standardSquare(int row, int col) {
  if (row % 2 != 0 && col % 2 != 0) return -1;
  else if (row % 2 == 0 && col % 2 == 0) return -1;
  else return -2;
}


// Determine which Neopixels to activate depending on the chess square
void lightSquare(int row, int col, int R, int G, int B) {

  // Determine the corresponding row and column in numerical form
  col = 7 - col;
  row = 7 - row;

  // WHITE SIDE
  if (row < 4) {
    // First Neopixel grid
    if (col < 4) {
      for (int i = 0; i < 4; i++) {
        pixels[0].setPixelColor((row*4) + i + (col*64), pixels[0].Color(R, G, B));
        pixels[0].setPixelColor(31 - i + (col*64) - (row*4), pixels[0].Color(R, G, B));
        pixels[0].setPixelColor((row*4) + 32 + i + (col*64), pixels[0].Color(R, G, B));
        pixels[0].setPixelColor(63 - i + (col*64) - (row*4), pixels[0].Color(R, G, B));
      }
    }

    // Second Neopixel grid
    else {
      for (int i = 0; i < 4; i++) {
        pixels[0].setPixelColor((row*4) + i + (col*64), pixels[0].Color(R, G, B));
        pixels[0].setPixelColor(31 - i + (col*64) - (row*4), pixels[0].Color(R, G, B));
        pixels[0].setPixelColor((row*4) + 32 + i + (col*64), pixels[0].Color(R, G, B));
        pixels[0].setPixelColor(63 - i + (col*64) - (row*4), pixels[0].Color(R, G, B));
      }
    }
    pixels[0].show();
  } 

  // BLACK SIDE
  else {
    col = 7 - col;
    if (col < 4) {
      for (int i = 0; i < 4; i++) {
        pixels[1].setPixelColor((row*4) + i + (col*64), pixels[1].Color(R, G, B));
        pixels[1].setPixelColor(31 - i + (col*64) - (row*4), pixels[1].Color(R, G, B));
        pixels[1].setPixelColor((row*4) + 32 + i + (col*64), pixels[1].Color(R, G, B));
        pixels[1].setPixelColor(63 - i + (col*64) - (row*4), pixels[1].Color(R, G, B));
      }
    }

    // Second Neopixel grid
    else {
      for (int i = 0; i < 4; i++) {
        pixels[1].setPixelColor((row*4) + i + (col*64), pixels[1].Color(R, G, B));
        pixels[1].setPixelColor(31 - i + (col*64) - (row*4), pixels[1].Color(R, G, B));
        pixels[1].setPixelColor((row*4) + 32 + i + (col*64), pixels[1].Color(R, G, B));
        pixels[1].setPixelColor(63 - i + (col*64) - (row*4), pixels[1].Color(R, G, B));
      }
    }
    pixels[1].show();
  }
}


// Read the inputs through the Multiplexer
float readMux(int channel, int index){
  int controlPin[] = {s0[index], s1[index], s2[index], s3[index]};

  int muxChannel[16][4] = {
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  // Loop through the 4 signals
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  // Read the value at the "SIGNAL" pin
  int val = analogRead(SIG_pin[index]);

  // Return the value
  float voltage = (val * 5.0) / 1024.0;
  return voltage;
}
