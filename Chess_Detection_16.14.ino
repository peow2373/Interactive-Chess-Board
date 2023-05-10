#include <Adafruit_NeoPixel.h>

// Create Neopixel object
#define pixelPin    7      // Digital Output connected to the neopixels
#define pixelCount  256     // Number of neopixels
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(pixelCount, pixelPin, NEO_GRB + NEO_KHZ800);

// Variables to control neopixels
bool startUp = false;
int startDelay = 1000;
int resetIndicated = false;

// Neopixel 4x4 pixel positions

//      Breadboard       //
//- 256 240 .... 17 16  -//
//- 255 239 .... 18 15  -//
//-         ....        -//
//- 242 226 .... 31  2  -//
//- 241 225 .... 32  1  -//
//        Front          //

// Neopixel 4x4 square positions

//      Breadboard       //
//-  A4 - B4 - C4 - D4  -//
//-  A3 - B3 - C3 - D3  -//
//-  A2 - B2 - C2 - D2  -//
//-  A1 - B1 - C1 - D1  -//
//        Front          //


// Hall Effect sensor 4x4 grid positions

//      Breadboard       //
//- c13 - c9 - c7  - c3 -//
//- c15 - c11 - c5 - c1 -//       TO DO: FIX LATER - (right 2 columns need to be re-oriented)
//- c12 - c8 - c6  - c2 -//
//- c14 - c10 - c4 - c0 -//
//        Front          //

//- c11 - c15 - c12  - c8 -//
//- c9 - c13 - c14 - c10 -//       TO DO: FIX LATER - (recalibrate sensor location)
//- c1 - c5 - c6  - c2 -//
//- c3 - c7 - c4 - c0 -//

//- c11 - c9 - c7  - c5 -//
//- c15 - c13 - c3 - c1 -//       TO DO: FIXED TEMPORARILY - (corrected sensor location)
//- c10 - c8 - c6  - c4 -//
//- c14 - c12 - c2 - c0 -//


// Starting position for pieces
//- . . . . -//                     //- 0,0  0,1  0,2  0,3 -//        Notation: row, col
//- . . . . -//                     //- 1,0  1,1  1,2  1,3 -//
//- P P P P -//                     //- 2,0  2,1  2,2  2,3 -//
//- R N B K -//                     //- 3,0  3,1  3,2  3,3 -//      

const int rows = 4;
const int columns = 4;

char positions[rows][columns] = {
  {'E', 'E', 'E', 'E'},             // Row 4
  {'E', 'E', 'E', 'E'},             // Row 3
  {'E', 'E', 'E', 'E'},             // Row 2
  {'E', 'E', 'E', 'E'}              // Row 1
};

int squareStates[rows][columns] = {
  {0, 0, 0, 0},                     // Row 4                    // -1 = No piece picked up
  {0, 0, 0, 0},                     // Row 3                    // 0 = No piece on the square
  {0, 0, 0, 0},                     // Row 2                    // 1 = Piece on the square
  {0, 0, 0, 0}                      // Row 1                    // 2 = Piece movement option
};

// Keep track of a piece when it's lifted
int activeSquare[2] = {-1,-1};


// Multiplexer pins
int s0 = 2;
int s1 = 3;
int s2 = 4;
int s3 = 5;

// Multiplexer "SIGNAL" pin
int SIG_pin = A0;

void setup() {
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  // Initialize neopixel strip
  pixels.begin(); // Initialize NeoPixel object
  pixels.show();  // Initialize all pixels to 'off'
  pixels.setBrightness(50);

  Serial.begin(9600);
}

void loop() {

// Neopixel wipe
  if (!startUp) {
    for(int i = 0; i < 16; i++) {
      for(int j = 0; j < 16; j++) {
        if(j % 2 == 0) {
          pixels.setPixelColor(j * 16 + i, pixels.Color(255, 255, 255));
        } else {
          pixels.setPixelColor((j + 1) * 16 - (i + 1), pixels.Color(255, 255, 255));
        }
      }
      pixels.show();
      delay(50);
    }
  
    // Wait a second, turn all pixels off, then wait another second
    delay(startDelay);
    for (int k = 0; k < pixelCount; k++) pixels.setPixelColor(k, pixels.Color(0, 0, 0));
    pixels.show();
    startUp = true;
    delay(startDelay);
  }

// CRITICAL FUNCTION
  chessLogic();



// Turn the white squares on
//  lightSquare(0, 1, 255, 255, 255);
//  lightSquare(0, 3, 255, 255, 255);
//  lightSquare(1, 0, 255, 255, 255);
//  lightSquare(1, 2, 255, 255, 255);
//  lightSquare(2, 1, 255, 255, 255);
//  lightSquare(2, 3, 255, 255, 255);
//  lightSquare(3, 0, 255, 255, 255);
//  lightSquare(3, 2, 255, 255, 255);
}

// Update the square if it detects a piece
void changeState(int x, int y) {
  if (positions[x][y] == 'E' && squareStates[x][y] == 2) squareStates[x][y] = 1;
  else if (squareStates[x][y] != 2) squareStates[x][y] = 1;
}


// Main logic for determining piece moves
void chessLogic() {
  
  // Detect if a square has a piece on it
  bool topRowsEmpty = true;
  bool botRowsFull = true;

  // Reset square states
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      // Make sure they aren't indicating a piece's movement options
      if (squareStates[x][y] != 2) squareStates[x][y] = 0;
    }
  }

  // Update the square state if it has changed
  for(int c = 0; c < 16; c ++){

    switch(c) {
    // Row 1
      case 11:
        if (readMux(c) <= 1.00) changeState(0,0);
        break;
      case 9:
        if (readMux(c) <= 1.00) changeState(0,1);
        break;
      case 7:
        if (readMux(c) <= 1.00) changeState(0,2);
        break;
      case 5:
        if (readMux(c) <= 1.00) changeState(0,3);
        break;

    // Row 2
      case 15:
        if (readMux(c) <= 1.00) changeState(1,0);
        break;
      case 13:
        if (readMux(c) <= 1.00) changeState(1,1);
        break;
      case 3:
        if (readMux(c) <= 1.00) changeState(1,2);
        break;
      case 1:
        if (readMux(c) <= 1.00) changeState(1,3);
        break;

    // Row 3
      case 10:
        if (readMux(c) <= 1.00) changeState(2,0);
        break;
      case 8:
        if (readMux(c) <= 1.00) changeState(2,1);
        break;
      case 6:
        if (readMux(c) <= 1.00) changeState(2,2);
        break;
      case 4:
        if (readMux(c) <= 1.00) changeState(2,3);
        break;

    // Row 4
      case 14:
        if (readMux(c) <= 1.00) changeState(3,0);
        break;
      case 12:
        if (readMux(c) <= 1.00) changeState(3,1);
        break;
      case 2:
        if (readMux(c) <= 1.00) changeState(3,2);
        break;
      case 0:
        if (readMux(c) <= 1.00) changeState(3,3);
        break;
    }

    // Conditions for board to be "reset"
    if (c % 2 != 0 && readMux(c) <= 1.00) topRowsEmpty = false;
    else if (c % 2 == 0 && readMux(c) > 1.00) botRowsFull = false;
  }

  // Assign piece positions when the board has been "reset"
  if (topRowsEmpty && botRowsFull) {

    // Indicate the board has been reset
    if (!resetIndicated) {
      for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
          lightSquare(x, y, 0, 255, 0);
          positions[x][y] = 'E';
        }
      }
      
      activeSquare[0] = -1;
      activeSquare[1] = -1;
 
      delay(startDelay);

      // Change the board back to normal
      for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
          standardSquare(x, y);
        }
      }

      for (int i = 0; i < 4; i++) positions[2][i] = 'P';
      positions[3][0] = 'R';
      positions[3][1] = 'N';
      positions[3][2] = 'B';
      positions[3][3] = 'K';
      
      resetIndicated = true;
    }
  } 

  // Determine if a piece has been lifted up
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {

      // Highlight the proper squares when a piece is lifted
      if (positions[x][y] != 'E') {
        if (squareStates[x][y] == 0 && activeSquare[0] == -1 && activeSquare[1] == -1) {
          pieceOptions(positions[x][y], x, y); 
          activeSquare[0] = x;
          activeSquare[1] = y;
        }
        else if (squareStates[x][y] == 1) standardSquare(x, y);
      } else if (squareStates[x][y] != 2) standardSquare(x, y);


      // Check if a piece was replaced after being lifted up
      if (squareStates[activeSquare[0]][activeSquare[1]] == 1) {
        activeSquare[0] = -1;
        activeSquare[1] = -1;
      }

      // Check if the lifted piece was set down in a different position                 // TO DO: FIX LATER (This code won't work when taking another piece)
      if (squareStates[x][y] == 1 && positions[x][y] == 'E') {
        positions[x][y] = positions[activeSquare[0]][activeSquare[1]];
        positions[activeSquare[0]][activeSquare[1]] = 'E';
        activeSquare[0] = -1;
        activeSquare[1] = -1;
      }

      // Reset the squares that show the piece movement options
      if (activeSquare[0] == -1 && activeSquare[1] == -1) {
        if (squareStates[x][y] == 2) {
          squareStates[x][y] = 0;
          standardSquare(x, y);
        }
      }
    }
  }
}


// Determine the movement options for the chosen piece
void pieceOptions(char piece, int row, int col) {

  // Indicate the square the piece was chosen from
  lightSquare(row, col, 0, 0, 255);

  switch(piece) {
  // Pawn
    case 'P':
      // Two space movement
      if (row - 1 >= 0) squareStates[row - 1][col] = 2;
      if (row - 2 >= 0) squareStates[row - 2][col] = 2;
      break;

  // Rook
    case 'R':
      for (int i = 1; i < 4; i++) {
        // Vertical movement
        if (row - i >= 0) squareStates[row - i][col] = 2;
        else if (row + i < 4) squareStates[row + i][col] = 2;

        // Horizontal movement
        if (col - i >= 0) squareStates[row][col - i] = 2;
        else if (col + i < 4) squareStates[row][col + i] = 2;
      }
      break;

  // Knight
    case 'N':
      // North movement
      if (row - 2 >= 0) {
        if (col - 1 >= 0) squareStates[row - 2][col - 1] = 2;
        if (col + 1 < 4) squareStates[row - 2][col + 1] = 2;
      }

      // South movement  
      if (row + 2 < 4) {
        if (col - 1 >= 0) squareStates[row + 2][col - 1] = 2;
        if (col + 1 < 4) squareStates[row + 2][col + 1] = 2;
      }

      // East movement
      if (col - 2 >= 0) {
        if (row - 1 >= 0) squareStates[row - 1][col - 2] = 2;
        if (row + 1 < 4) squareStates[row + 1][col - 2] = 2;
      }

      // West movement  
      if (col + 2 < 4) {
        if (row - 1 >= 0) squareStates[row - 1][col + 2] = 2;
        if (row + 1 < 4) squareStates[row + 1][col + 2] = 2;
      }
      break;

  // Bishop
    case 'B':
      for (int i = 1; i < 4; i++) {
        // Upwards diagonal movement
        if (row - i >= 0) {
          if (col - i >= 0) squareStates[row - i][col - i] = 2;
          if (col + i < 4) squareStates[row - i][col + i] = 2;
        }

        // Downwards diagonal movement
        if (row + i < 4) {
          if (col - i >= 0) squareStates[row + i][col - i] = 2;
          if (col + i < 4) squareStates[row + i][col + i] = 2;
        }
      }
      break;

  // King
    case 'K':
      // Upwards movement
      if (row - 1 >= 0) {
        if (col - 1 >= 0) squareStates[row - 1][col - 1] = 2;
        if (col + 1 < 4) squareStates[row - 1][col + 1] = 2;
        squareStates[row - 1][col] = 2;
      }

      // Downwards movement
      if (row + 1 < 4) {
        if (col - 1 >= 0) squareStates[row + 1][col - 1] = 2;
        if (col + 1 < 4) squareStates[row + 1][col + 1] = 2;
        squareStates[row + 1][col] = 2;
      }

      // Sideways movement
      if (col - 1 >= 0) squareStates[row][col - 1] = 2;
      if (col + 1 < 4) squareStates[row][col + 1] = 2;
      break;
  }

  // Indicate the squares the piece can move to
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      if (squareStates[x][y] == 2) lightSquare(x, y, 0, 255, 0);
    }
  }
}


void standardSquare(int row, int col) {
  if (row % 2 != 0 && col % 2 != 0) lightSquare(row, col, 255, 255, 255);
  else if (row % 2 == 0 && col % 2 == 0) lightSquare(row, col, 255, 255, 255);
  else lightSquare(row, col, 0, 0, 0);
}


// Determine which Neopixels to activate depending on the chess square
void lightSquare(int row, int col, int R, int G, int B) {

  // Determine the corresponding column in numerical form
  col = 3 - col;
  row = 3 - row;

  // Initialize pixels in the square to their proper color
  for (int i = 0; i < 4; i++) {
    pixels.setPixelColor((row*4) + i + (col*64), pixels.Color(R, G, B));
    pixels.setPixelColor(31 - i + (col*64) - (row*4), pixels.Color(R, G, B));
    pixels.setPixelColor((row*4) + 32 + i + (col*64), pixels.Color(R, G, B));
    pixels.setPixelColor(63 - i + (col*64) - (row*4), pixels.Color(R, G, B));
  }
  pixels.show();
}

// Read the inputs through the Multiplexer
float readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

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
  int val = analogRead(SIG_pin);

  // Return the value
  float voltage = (val * 5.0) / 1024.0;
  return voltage;
}
