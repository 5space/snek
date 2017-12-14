/*
  General notes:
 
  -if you are only using one max7219, then use the function maxSingle to control
  the little guy ---maxSingle(register (1-8), collum (0-255))
 
  -if you are using more than one max7219, and they all should work the same,
  then use the function maxAll ---maxAll(register (1-8), collum (0-255))
 
  -if you are using more than one max7219 and just want to change something
  at one little guy, then use the function maxOne
  ---maxOne(Max you want to control (1== the first one), register (1-8),
  column (0-255))
 
  During initiation, be sure to send every part to every max7219 and then
  upload it.
  For example, if you have five max7219's, you have to send the scanLimit 5 times
  before you load it-- otherwise not every max7219 will get the data. the
  function maxInUse keeps track of this, just tell it how many max7219 you are
  using.
*/
 
const int dataIn = 4;
const int load = 3;
const int clk = 2;
 
const int maxInUse = 1;    //change this variable to set how many MAX7219's you'll use

int snakeLength;
int snakeDir;
int inputDir;
int head[2];

float snakeTick = 0;
const float snakeTickMax = 150;
const float snakeTickMult = 0.9;

float displayTick = 0;
const float displayTickMax = 50;

const int controlPinY = A0;
const int controlPinX = A1;
const int controlButton = 12;

const int inputThreshold = 64;

bool showFood;
bool showSnake;
const bool doesLoop = true;
bool paused = false;

const int x = 8;
const int y = 8;
int board[x][y];

void initialize() {
  head[0] = 0;
  head[1] = 3;

  for (int i = 0; i < x; i++)
    for (int j = 0; j < y; j++)
      board[i][j] = 0;

  board[head[0]][head[1]] = 1;
  snakeLength = 3;
  snakeDir = 0;
  inputDir = 0;

  bool showFood = true;
  bool showSnake = true;

  spawnFood();
}

void handleInput() {

  int inputX = map(analogRead(controlPinX), 0, 1023, -512, 511);
  int inputY = map(analogRead(controlPinY), 0, 1023, -512, 511);

  if (abs(inputX) > inputThreshold || abs(inputY) > inputThreshold) {
    if (abs(inputX) > abs(inputY)) {

      if (inputX > 0) {
        inputDir = 0;
      } else {
        inputDir = 2;
      }
      
    } else {
      
      if (inputY > 0) {
        inputDir = 3;
      } else {
        inputDir = 1;
      }
    }
  }

  //inputDir += 3;  //For rotation (change 3 to any int)
  //inputDir %= 4;
}

bool refresh() {
  // TODO: Please help
  // Increment snake values
  
  for (int m = 0; m < x; m++) {
    for (int n = 0; n < y; n++) {
      if (board[m][n] > 0) {
        board[m][n]++;
      }
    }
  }

  // Update head position

  if (/*inputDir > -1 && */(inputDir - snakeDir) % 2 != 0) {
    snakeDir = inputDir;
  }

  switch (snakeDir) { //Update head position
    case 0: //X+
      head[0] += 1;
      head[0] %= x;
      break;
    case 1: //Y-
      head[1] += 7;
      head[1] %= y;
      break;
    case 2: //X-
      head[0] += 7;
      head[0] %= x;
      break;
    case 3: //Y+
      head[1] += 1;
      head[1] %= y;
      break;
  }

  

  for (int m = 0; m < x; m++) {
    for (int n = 0; n < y; n++) {
      if (board[m][n] > snakeLength) {
        //Setting end of snake to zero
        board[m][n] = 0;
      }
    }
  }
  
  if (board[head[0]][head[1]] > 0) {
    return true; // TODO: AM I DOING THIS RIGHT
  } else if (board[head[0]][head[1]] == -1) {
    snakeLength++;
    snakeTick *= snakeTickMult;
    spawnFood();
  }
  
  board[head[0]][head[1]] = 1;
  
  return false;
}

void displayBoard() {
  
  int data;

  for (int m = 0; m < x; m++) {
    
    data = 0;

    for (int n = 0; n < y; n++) {
      if ((board[m][n] > 0 || (board[m][n] < 0 && showFood)) && showSnake) {
        data += 1 << n;
      }
    }

    maxSingle(m + 1, data);
  }
}

void spawnFood() {

  int foodRandom = (int) random(0, x*y - snakeLength);
  int count = 0;

  for (int m = 0; m < x; m++) {
    for (int n = 0; n < y; n++) {

      if (board[m][n] <= 0) {
        count++;
      }

      if (count > foodRandom) {
        board[m][n] = -1;
        return;
      }
    }
  }
}

void endGame() {
  //showFood = False;
  for (int i = 0; i < 4; i++) {
    displayBoard();
    delay(displayTick);
    showSnake = !showSnake;
  }
  initialize();
}

// define max7219 registers
byte reg_decodeMode  = 0x09;
byte reg_intensity   = 0x0a;
byte reg_scanLimit   = 0x0b;
byte reg_shutdown    = 0x0c;
byte reg_displayTest = 0x0f;
 
void putByte(byte data) {
  byte i = 8;
  byte mask;
  while (i > 0) {
    mask = 0x01 << (i - 1);      // get bitmask
    digitalWrite(clk, LOW);   // tick
    if (data & mask) {            // choose bit
      digitalWrite(dataIn, HIGH);// send 1
    } else {
      digitalWrite(dataIn, LOW); // send 0
    }
    digitalWrite(clk, HIGH);   // tock
    --i;                         // move to lesser bit
  }
}
 
void maxSingle(byte reg, byte col) {    
//maxSingle is the "easy"  function to use for a single max7219
 
  digitalWrite(load, LOW);       // begin    
  putByte(reg);                  // specify register
  putByte(col);                  //((data & 0x01) * 256) + data >> 1); // put data  
  digitalWrite(load, LOW);       // and load da stuff
  digitalWrite(load, HIGH);
}
 
void maxAll(byte reg, byte col) {    // initialize  all  MAX7219's in the system
  
  int c = 0;
  digitalWrite(load, LOW);  // begin    
  for (c = 1; c <= maxInUse; c++) {
    putByte(reg);  // specify register
    putByte(col);//((data & 0x01) * 256) + data >> 1); // put data
  }
  
  digitalWrite(load, LOW);
  digitalWrite(load, HIGH);
}
 
void maxOne(byte maxNr, byte reg, byte col) {
//maxOne is for addressing different MAX7219's,
//while having a couple of them cascaded
 
  int c = 0;
  digitalWrite(load, LOW);  // begin    
 
  for (c = maxInUse; c > maxNr; c--) {
    putByte(0);    // means no operation
    putByte(0);    // means no operation
  }
 
  putByte(reg);  // specify register
  putByte(col);//((data & 0x01) * 256) + data >> 1); // put data
 
  for (c = maxNr-1; c >= 1; c--) {
    putByte(0);    // means no operation
    putByte(0);    // means no operation
  }
 
  digitalWrite(load, LOW); // Loading the data
  digitalWrite(load, HIGH);
}
 
 
void setup() {

  //NEVER TOUCH THIS THIS IS SACRED IDK HOW IT WORKS
 
  pinMode(dataIn, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(load, OUTPUT);

  pinMode(controlPinX, INPUT);
  pinMode(controlPinY, INPUT);
  pinMode(controlButton, INPUT);
 
  digitalWrite(13, HIGH);

  Serial.begin(9600);
 
  //initiation of the max 7219
  maxAll(reg_scanLimit, 0x07);      
  maxAll(reg_decodeMode, 0x00);  // using an led matrix (not digits)
  maxAll(reg_shutdown, 0x01);    // not in shutdown mode
  maxAll(reg_displayTest, 0x00); // no display test
  
  for (int e = 1; e <= 8; e++) {    // empty registers, turn all LEDs off
    maxAll(e, 0);
  }
  
  maxAll(reg_intensity, 0x0f & 0x0f);    // the first 0x0f is the value you can set
                                                  // range: 0x00 to 0x0f
  
  initialize();
}
 
void loop() {
  
  delay(1);
  
  snakeTick += 1;
  displayTick += 1;

  if (snakeTick >= snakeTickMax * pow(snakeTickMult, snakeLength)) {

    snakeTick = 0;

    if (refresh())
      endGame();
  }

  if (displayTick >= displayTickMax) {

    displayTick = 0;
    showFood = !showFood;
  }

  handleInput();
  displayBoard();

//Tutorial
  //if you use just one MAX7219 it should look like this
  /*
  maxSingle(1, 1);                       //  + - - - - - - -
  maxSingle(2, 2);                       //  - + - - - - - -
  maxSingle(3, 4);                       //  - - + - - - - -
  maxSingle(4, 8);                       //  - - - + - - - -
  maxSingle(5, 16);                      //  - - - - + - - -
  maxSingle(6, 32);                      //  - - - - - + - -
  maxSingle(7, 64);                      //  - - - - - - + -
  maxSingle(8, 128);                     //  - - - - - - - +
  */
 
 
  //if you use more than one MAX7219, it should look like this
  /*
  maxAll(1,1);                       //  + - - - - - - -
  maxAll(2,3);                       //  + + - - - - - -
  maxAll(3,7);                       //  + + + - - - - -
  maxAll(4,15);                      //  + + + + - - - -
  maxAll(5,31);                      //  + + + + + - - -
  maxAll(6,63);                      //  + + + + + + - -
  maxAll(7,127);                     //  + + + + + + + -
  maxAll(8,255);                     //  + + + + + + + +
  */
 
 
  //if you use more than one max7219 the second one should look like this
  /*
  maxOne(2,1,1);                       //  + - - - - - - -
  maxOne(2,2,2);                       //  - + - - - - - -
  maxOne(2,3,4);                       //  - - + - - - - -
  maxOne(2,4,8);                       //  - - - + - - - -
  maxOne(2,5,16);                      //  - - - - + - - -
  maxOne(2,6,32);                      //  - - - - - + - -
  maxOne(2,7,64);                      //  - - - - - - + -
  maxOne(2,8,128);                     //  - - - - - - - +
  */
}
