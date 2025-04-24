#include <LiquidCrystal.h>  // Include the LCD library

// Define LCD pin connections (to Raspberry Pi Pico or Arduino)
const int d7 = 16;
const int d6 = 17;
const int d5 = 18;
const int d4 = 19;
const int e  = 20;
const int rs = 21;

// Initialize the LCD object with the defined pins
LiquidCrystal lcd(rs, e, d4, d5, d6, d7);

// Define UART RX pin (used to receive data from the main game Pico)
const int UART_RX = 1;

String lastScore = "0"; // Variable to store the last received score
bool gameOver = false;  // Flag to track whether the game is over

void setup() {
  Serial1.setRX(UART_RX); // Set UART RX pin to pin 1
  Serial1.begin(9600);    // Start serial communication at 9600 baud

  // Initialize the LCD with 16 columns and 2 rows
  lcd.begin(16, 2);

  // Initial message on the LCD
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("score...");
}

void loop() {
  // Check if data is available to read from UART
  if (Serial1.available()) {
    // Read the incoming string until newline
    String incoming = Serial1.readStringUntil('\n');
    incoming.trim();  // Remove any trailing newline or carriage return

    // Check if the message contains a score update
    if (incoming.startsWith("SCORE:")) {
      // Extract the score part from the string
      lastScore = incoming.substring(6);

      // Clear LCD and display the score
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Score: ");
      lcd.print(lastScore);

    } else if (incoming == "GAME OVER") {
      // Handle game over message
      gameOver = true;
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("GAME OVER");
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(lastScore);

    } else if (incoming == "PLAY AGAIN") {
      // Handle "play again" prompt
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Play Again?");
      lcd.setCursor(0, 1);
      lcd.print("Press Button");

      // Reset gameOver flag
      gameOver = false;
    }
  }

  // If game is over, show message for 5 seconds then reset display
  if (gameOver) {
    delay(5000);  // Wait 5 seconds
    gameOver = false;  // Clear the flag
    lcd.clear();       // Clear the screen
    lcd.print("Waiting for");
    lcd.setCursor(0, 1);
    lcd.print("score...");
  }
}
