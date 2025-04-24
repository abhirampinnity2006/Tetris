// define led pins

#define ROW_0 16
#define ROW_1 21
#define ROW_2 14
#define ROW_3 19
#define ROW_4 7
#define ROW_5 13
#define ROW_6 8
#define ROW_7 11
#define COL_0 20
#define COL_1 9
#define COL_2 10
#define COL_3 17
#define COL_4 12
#define COL_5 18
#define COL_6 22
#define COL_7 3
const int SEED_PIN = 26;

//var to track score and keep the line type that was previously cleared
int score = 0;
int last_lines_cleared = 0;

// define joystick pins

const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 28; // analog pin connected to X output
const int Y_pin = 27; // analog pin connected to Y output

// initialize variable for max number of columns

const int num_columns = 8;

// initialize variable for max number of rows

const int num_rows = 8; 

// initialize array for rows

const int row[8] = {16,21,14,19,7,13,8,11};

// initialize array for columns

const int col[8] = {20,9,10,17,12,18,22,3}; 

// initialize 2D matrix

int matrix[8][8]= {0};

//temp display buffer

int display_buffer[8][8] = {0};

/* 
note:
0 = led is off
1 = led is on but not locked
2 = led is on and locked
*/

// define current block

struct Block {

  int shape[4][2]; 
  int x; 
  int y; 

};

//function to turn on display

void refresh_display() {
  for (int frame = 0; frame < 8; frame++) { // repeat a few times to persist
    for (int r = 0; r < num_rows; r++) {
      for (int i = 0; i < num_rows; i++) {
        digitalWrite(row[i], LOW); // turn all rows off
      }

      for (int c = 0; c < num_columns; c++) {
        digitalWrite(col[c], display_buffer[r][c] ? LOW : HIGH); // active LOW
      }

      digitalWrite(row[r], HIGH); // turn on this row
      delayMicroseconds(500);     // shorter delay, helps persistence
    }
  }
}



Block current_block; 

// define each shape

const int shape_defenition[4][4][2] = {
  // 3-long line (horizontal)
  {{0,0}, {1,0}, {2,0}, {0,0}},  

  // dot
  {{0,0}, {0,0}, {0,0}, {0,0}},

  // square 
  {{0,0}, {0,1}, {1,0}, {1,1}},

  // L-shape (horizontal with hook down on right)
  {{0,0}, {1,0}, {2,0}, {2,1}},  
};

//Ensures that every game play will be different

unsigned long generate_random_seed() {
  unsigned long seed = 0;
  for (int i = 0; i < 32; i++) {
    seed <<= 1;
    seed |= analogRead(SEED_PIN) & 1; // A0 must be left floating (not connected to anything)
    delay(1);
  }
  return seed;
}

// Tracks the shape index from shape_defenition
int current_shape_index;  


void setup() {
  
  for (int i = 0; i < 8; i++) {

    // set each led to output 

    pinMode(col[i], OUTPUT); 
    pinMode(row[i], OUTPUT);

  }

  // set pin that takes button click from joystick to input

  pinMode(SW_pin, INPUT_PULLUP);

  //takes analog input from unused pin 26

  pinMode(SEED_PIN, INPUT);

  // seed random with analog pin

  randomSeed(generate_random_seed());

  // generate initial block

  generate_block();
  current_block.x = 3;
  current_block.y = 0;

  //UART
  Serial1.begin(9600);
}

// function to light up an led

void led(int c, int r) {

  // turn on desired row 

  digitalWrite(row[r], HIGH); 

  // turn off each column except desired column 

  for (int i = 0; i < 8; i++) {
    digitalWrite(col[i], HIGH);
    if (i == c) {
      digitalWrite(col[i], LOW);
    }
  }

  // give some time for led to turn on

  delay(1);  

  // turn off desired row

  digitalWrite(row[r], LOW); 
}

// function to generate a new block with a random shape

void generate_block() {

  // generate a random number between 1 to 4 
  // Note: add random seed

  int random_shape = random(0, 4); 

  //Track the shape index
  current_shape_index = random_shape;


  for (int i = 0; i < 4; i++) {

    // assign x coordinate to current block

    current_block.shape[i][0] = shape_defenition[random_shape][i][0]; 

    // assign y coordinate to current block

    current_block.shape[i][1] = shape_defenition[random_shape][i][1];

  }
}

// function to check if a block can move in a certain direction

bool can_block_move (int change_in_x, int change_in_y) {

  for (int i = 0; i < 4; i++) {

    // find the current x position 

    int current_x = current_block.x + current_block.shape[i][0] + change_in_x; 

    // find the current y position 

    int current_y = current_block.y + current_block.shape[i][1] + change_in_y;

    // if block cannot move, return false 

    if (current_x < 0 || current_x >= num_columns || current_y >= num_rows || matrix[current_y][current_x] == 2) {
      return false; 
    }
  }
  // if block can move return true

  return true; 
}

// function to lock the current block in place

void lock_block () {
  for (int i = 0; i < 4; i++) {

    // find the current x position 

    int current_x = current_block.x + current_block.shape[i][0];

    // find the current y position 

    int current_y = current_block.y + current_block.shape[i][1];

    // lock block 

    matrix[current_y][current_x] = 2; 

  }
}

// function to clear row

int erase_row() {
  int lines_cleared = 0;

  for (int row = 0; row < num_rows; row++) {
    bool full_row = true;

    for (int col = 0; col < num_columns; col++) {
      if (matrix[row][col] != 2) {
        full_row = false;
        break;
      }
    }

    if (full_row) {
      for (int r = row; r > 0; r--) {
        for (int col = 0; col < num_columns; col++) {
          matrix[r][col] = matrix[r - 1][col];
        }
      }

      for (int col = 0; col < num_columns; col++) {
        matrix[0][col] = 0;
      }

      row--;
      lines_cleared++;
    }
  }

  return lines_cleared;
}

//function to track score
void update_score(int lines_cleared) {
  int points = 0;

  if (lines_cleared == 1) points = 100;
  else if (lines_cleared == 2) points = 250;
  else if (lines_cleared == 3) points = 1000;
  else if (lines_cleared >= 4) points = 4000;

  // Apply combo bonus
  if (lines_cleared == last_lines_cleared && lines_cleared > 0) {
    points *= 2;
  }

  score += points;
  last_lines_cleared = lines_cleared;

  // NEW: Send score to LCD
  Serial1.print("SCORE:");
  Serial1.println(score); // includes the newline!
}




//animation for the game ending
// function to animate game over screen
void game_over_animation() {
  // Show the final board for 1 second
  unsigned long start_time = millis();
  while (millis() - start_time < 1000) {
    refresh_display();
  }

  // Turn off all LEDs
  for (int y = 0; y < num_rows; y++) {
    for (int x = 0; x < num_columns; x++) {
      display_buffer[y][x] = 0;
    }
  }

  Serial1.println("GAME OVER");

  // Keep screen off for 5 seconds
  start_time = millis();
  while (millis() - start_time < 5000) {
    refresh_display();
  }

  // Prompt LCD to say "Play Again?"
  Serial1.println("PLAY AGAIN");

  // Wait for joystick button press to restart
  while (digitalRead(SW_pin) == HIGH) {
    refresh_display(); // Keep display off
  }

  // Reset game state
  score = 0;
  last_lines_cleared = 0;

  // Clear matrix
  for (int y = 0; y < num_rows; y++) {
    for (int x = 0; x < num_columns; x++) {
      matrix[y][x] = 0;
    }
  }

  // Generate new block and reset position
  generate_block();
  current_block.x = 3;
  current_block.y = 0;

  // Let player start again
}





// function to move the block

void move_block(int change_in_x, int change_in_y) {

  if (can_block_move(change_in_x, change_in_y)) {

    // if block can move, update position

    current_block.x += change_in_x;
    current_block.y += change_in_y;

  } else if (change_in_y == 1) {
    
    // if block can't move, lock block

    lock_block();

    // if row is full, clear row and shift each block down one

    int lines_cleared = erase_row();
    update_score(lines_cleared);

    // create a new block

    generate_block();

    // check if new block overlaps with top row 
    // note: add a clear row function

    if (matrix[0][current_block.x] == 2 || matrix[0][current_block.x + 1] == 2 || matrix[0][current_block.x + 2] == 2 || matrix[0][current_block.x + 3] == 2) {

      // if new block reaches top, stop the game
      // use infinite loop

     // Display game over animation
    game_over_animation();
    }

    // set initial position 

    current_block.x = 3;
    current_block.y = 0;
  }
}

// function to rotate the block

void rotate_block() {
  if (current_shape_index == 1 || current_shape_index == 2) return;

  int temp_shape[4][2];
  for (int i = 0; i < 4; i++) {
    temp_shape[i][0] = current_block.shape[i][0];
    temp_shape[i][1] = current_block.shape[i][1];
  }

  int pivot_x = temp_shape[0][0];
  int pivot_y = temp_shape[0][1];

  for (int i = 0; i < 4; i++) {
    int x = temp_shape[i][0] - pivot_x;
    int y = temp_shape[i][1] - pivot_y;

    current_block.shape[i][0] = pivot_x - y;
    current_block.shape[i][1] = pivot_y + x;
  }

  // Try basic wall kick offsets if initial position is invalid
  if (!can_block_move(0, 0)) {
    bool rotated = false;
    for (int dx = -1; dx <= 1; dx++) {
      current_block.x += dx;
      if (can_block_move(0, 0)) {
        rotated = true;
        break;
      }
      current_block.x -= dx; // reset if invalid
    }

    if (!rotated) {
      // fully revert if no offset worked
      for (int i = 0; i < 4; i++) {
        current_block.shape[i][0] = temp_shape[i][0];
        current_block.shape[i][1] = temp_shape[i][1];
      }
    }
  }
}


// time it took for the most recent block to move down 

unsigned long previous_move_time = 0;

// time it takes for block to move down 

const int move_interval = 500;

// time it took for joystick to last move

unsigned long previous_joystick_time = 0;

// time interval for joystick response

const int joystick_interval = 200;

void loop() {

  // current time for block to move down 

  unsigned long current_time = millis();

  // check if enough time has passed for block to move down

  if (current_time - previous_move_time >= move_interval) {

    // move block down one

    move_block(0, 1);

    // change current time to previous time 

    previous_move_time = current_time; 

  }

  // -----------------------------
  // handle joystick movement
  // -----------------------------

  if (current_time - previous_joystick_time >= joystick_interval) {

    int x_val = analogRead(X_pin);
    int y_val = analogRead(Y_pin);

    // Don't do anything if joystick is in the 700-800 range
    if (x_val < 20) {
      move_block(-1, 0); // move right
      previous_joystick_time = current_time;
    } else if (y_val == 1023) {
      move_block(1, 0); // move left
      previous_joystick_time = current_time;
    } else if (y_val < 20) {
      move_block(0, 1); // move down faster
      previous_joystick_time = current_time;
    }
  }

  // -----------------------------
  // handle joystick button press
  // -----------------------------

  if (digitalRead(SW_pin) == LOW) {
 rotate_block();
// keep refreshing during debounce delay
unsigned long start_time = millis();
while (millis() - start_time < 300) {
  refresh_display();
}

  }

 // Clear buffer
for (int y = 0; y < num_rows; y++) {
  for (int x = 0; x < num_columns; x++) {
    display_buffer[y][x] = (matrix[y][x] == 2) ? 1 : 0;
  }
}

// Add current moving block to display buffer
for (int i = 0; i < 4; i++) {
  int block_x = current_block.x + current_block.shape[i][0];
  int block_y = current_block.y + current_block.shape[i][1];
  if (block_y >= 0 && block_y < num_rows && block_x >= 0 && block_x < num_columns) {
    display_buffer[block_y][block_x] = 1;
  }
}

refresh_display();
}