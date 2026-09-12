#include <cstdlib>
#include <string>
#include <iostream>
#include <time.h>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>
//#include <windows.h>
#include <unistd.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::tuple;
using std::vector;
using std::array;


bool pCsunk = false; //player carrier sunk
bool pDsunk = false; //player destroyer sunk
bool pPsunk = false; //player patrol boat sunk
bool pSsunk = false; //player submarine sunk
bool pBsunk = false; //player battleship sunk

bool cCsunk = false; //comp carrier sunk
bool cDsunk = false; //comp destroyer sunk
bool cPsunk = false; //comp patrol boat sunk
bool cSsunk = false; //comp submarine sunk
bool cBsunk = false; //comp battleship sunk

bool playerWon = false;
bool compWon = false;

bool devMode = false; //set to true to print comp's sonar and fleet for debugging purposes
bool ansi = false; // does the terminal support ANSI escape codes?

char playerSonar[10][10]; //represents the player's sonar
char playerBoard[10][10]; //represents the player's fleet
int playerMineCount = 1; // mines the player can use to hit multiple spaces at once (consumes a turn)

char compSonar[10][10]; //represents the computer's sonar
char compBoard[10][10]; //represents the computer's fleet
int compMineCount = 1; // mines the comp can use to hit multiple spaces at once (consumes a turn)

int compPrevRow = -1;
int compPrevCol = -1;

int oriPrevRow = -1;
int oriPrevCol = -1;

const char carrierChar = 'C';
const char battleshipChar = 'B';
const char destroyerChar = 'D';
const char submarineChar = 'S';
const char patrolChar = 'P';
const char mineChar = '@';
const char hitChar = 'X';
const char missChar = 'O';
const char hitMsgChar = 'x';
const char missMsgChar = 'o';

vector<tuple<int, int, char>> compLoggedCoordinates; //let the comp "remember" hit coordinates

/* Function declarations */

void printWelcome();
void askForDev();
int askForMines();
void initialize();
void printError(string);
void printAllBoards();
void printGame(char arr[10][10], bool isPlayerBoard);
void printComp(char arr[10][10], bool isCompBoard);
void printPlayerAndCompBoards(bool, char [10][10], char [10][10]);
bool occupiedSpace(int startRow, int startCol, int endRow, int endCol, bool isVertical, char arr[10][10]);
void setPlayerShips();
void setCompShips();
void setCarrier(char arr[10][10]);
void setBattleship(char arr[10][10]);
void setDestroyer(char arr[10][10]);
void setSubmarine(char arr[10][10]);
void setPatrol(char arr[10][10]);
void setMines(char arr[10][10], int);

bool supportsANSI();
string formatCharacterANSI(char, bool);

void prompt();
void playerFire(int row, int col);
void compFire(int row, int col);
void compSmartFire();
bool checkVessel(char arr[10][10], char code);
void checkAfterPlayer();
void checkAfterComp();
void pushCoordinates(int row, int col, char mark);
void printLoggedCoordinates();
void scrubLoggedCoordinates(char mark);
int pickAnIndex();
void assignNewCoordinates();

/* Functions defined below this line -------------------------------------- */

int main() {
  ansi = supportsANSI(); // Does the terminal support ANSI escape sequences?

  // Initialize and place ships
  int ref = 0;
  srand(time(0));
  initialize();
  printWelcome();
  askForDev();
  setPlayerShips();
  setCompShips();
  
  // Place the mines
  int mineCount = askForMines();
  cout << "Placing " << mineCount << " mines around player fleet..." << endl;
  sleep(1.5);
  setMines(playerBoard, mineCount);
  cout << "Placing " << mineCount << " mines around CPU fleet..." << endl;
  cout << endl;
  sleep(1.5);
  setMines(compBoard, mineCount);
  
  // TESTING: new board print function
  //printAllBoards();

  while (!playerWon && !compWon) {
    //printGame(playerSonar, false);
    //printGame(playerBoard, true);
    printAllBoards();
    
    //if (devMode) {
    //  cout << "devMode = true, printing comp boards" << endl;
    //  printComp(compSonar, false);
    //  printComp(compBoard, true);
    //} //if devMode is set to true
    
    prompt();
    checkAfterPlayer();
    if (cCsunk && cBsunk && cDsunk && cSsunk && cPsunk) {
      playerWon = true;
      break;
    } //if all comp ships sunk
    
    compSmartFire();
    checkAfterComp();
    if (pCsunk && pBsunk && pDsunk && pSsunk && pPsunk) {
      compWon = true;
      break;
    } //if all player ships sunk
  } //while gameplay loop
  
  if (playerWon) {
    cout << "All enemy vessels down! You win!" << endl;
    sleep(3);
  } //if player is victorious

  else {
    cout << "Your fleet is sunk! You lose!" << endl;
    sleep(3);
  } //else (comp is victorious)
  return 0;
} //main function

/*
  Determine whether the terminal supports ANSI escape codes for rendering
  color-formatted characters.
*/
bool supportsANSI() {
  #ifdef _WIN32
    // Windows check
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (!GetConsoleMode(hOut, &mode)) {
      return false; // Not a console
    }

    // Check if VTP is enabled
    return (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
  #else
    // Unix-like systems: check TERM or COLORTERM env variables
    const char* term = std::getenv("TERM");
    const char* colorterm = std::getenv("COLORTERM");
    return term || colorterm;
  #endif
}

/*
  This function is called once per execution and greets the player with
  explanatory messages meant to introduce the game.

  Also included in the output statements are color codes which will only render
  in unix-based environments.
*/

void printWelcome() {
  cout << "Welcome to Command-Line Battleship!" << endl;
  cout << "Play against an advanced AI for supremacy on the high seas." << endl;
  cout << "Ships are placed randomly across the board." << endl;
  cout << "Ship Codes: " << endl;
  cout << "==================" << endl;
  if (ansi) {
    cout << "\033[31mC = Carrier\033[0m" << endl;    
    cout << "\033[32mB = Battleship\033[0m" << endl;
    cout << "\033[34mD = Destroyer\033[0m" << endl;
    cout << "\033[33mS = Submarine\033[0m" << endl;
    cout << "\033[36mP = Patrol Boat\033[0m" << endl;
    cout << "\033[90m@ = Mine\033[0m\n";
  } else {
    cout << "C = Carrier" << endl;    
    cout << "B = Battleship" << endl;
    cout << "D = Destroyer" << endl;
    cout << "S = Submarine" << endl;
    cout << "P = Patrol Boat" << endl;
    cout << "@ = Mine" << endl;
  }
  cout << "==================" << endl;
} //printWelcome

/*
  This function prompts the player for whether or not they wish to enable
  DevMode. The typical case is that the user will play without DevMode, and so
  the devMode variable is set to false by default.
*/

void askForDev() {
  char response;
  cout << "DevMode allows you to see the computer's fleet and sonar." << endl;
  cout << "It is priarily used for debugging, but you can also use it to see how the ";
  cout << "computer side works." << endl;
  cout << "Would you like to enable DevMode? (y/n) ";
  cin >> response;

  if (response == 'Y' || response == 'y')
    devMode = true;
  cout << "Very well. Starting game." << endl;
  cout << endl;
  sleep(1.5);
} //askForDev

/*
  This function prompts the player for how many mines to lay on the board.
  Valid mine counts range from [0,5] and affect both the player's fleet and the CPU's fleet.
*/
int askForMines() {
  string response;
  int count;
  cout << "If hit, mines can blast multiple adjacent spaces." << endl;
  cout << "How many mines would you like to place? [MAX: 5]" << endl;
  cout << "Count: ";
  cin >> response;
  count = std::atoi(response.c_str()); // non-numeric interpreted as 0
  if (count > 5) {
    count = 5;
  } else if (count < 0) {
    count = 0;
  }

  return count;
}

/*
  This function initializes the player's sonar and fleet board
  as empty (represented as ~ spaces). It does the same for the 
  computer's sonar and fleet boards.
 */

void initialize() {

  for (int i = 0; i < 10; i++) {
    for (int ii = 0; ii < 10; ii++) {
      playerSonar[i][ii] = '~';
      playerBoard[i][ii] = '~';
      compSonar[i][ii] = '~';
      compBoard[i][ii] = '~';
    } //inner for-loop
  } //outter for-loop
} //initialize

/*
  Take a character from a space on the board, and format it to
  the appropriate ANSI escape sequence (if supported by the terminal).
  Return a string.
*/
string formatCharacterANSI(char c, bool isPlayer = false) {
  string formattedStr;
  switch(c) {
    //case carrierChar: formattedStr = "\033[31mC\033[0m"; break;
    case carrierChar: formattedStr = "\033[31m" + string{c} + "\033[0m"; break;
    case battleshipChar: formattedStr = "\033[32m" + string{c} + "\033[0m"; break;
    case destroyerChar: formattedStr = "\033[34m" + string{c} + "\033[0m"; break;
    case submarineChar: formattedStr = "\033[33m" + string{c} + "\033[0m"; break;
    case patrolChar: formattedStr = "\033[36m" + string{c} + "\033[0m"; break;
    case mineChar: formattedStr = "\033[90m" + string{c} + "\033[0m"; break;
    case hitChar: 
    case missChar: formattedStr = "\033[4;97m" + string{c} + "\033[0m"; break;
    case hitMsgChar:
      if (isPlayer) {
        formattedStr = "\033[31mHIT!\033[0m Enemy Sustained Damage"; 
      } else {
        formattedStr = "\033[31mHIT!\033[0m You've Sustained Damage";
      }
      break;
    case missMsgChar:
      if (isPlayer) {
        formattedStr = "\033[33mMISS!\033[0m Enemy Evaded Attack";
      } else {
        formattedStr = "\033[33mMISS!\033[0m You've Evaded Damage";
      }
      break;
    case '~': formattedStr = "~"; break;
  }
  return formattedStr;
}

/*
  This function prints a given error message with
  ANSI escape codes (if supported).
*/
void printError(string msg) {
  if (ansi) {
    cout << "\033[4;31mERROR: \033[0m" + msg << endl; 
  } else {
    cout << "ERROR: " << msg << endl;
  }
}

/*
  This function prints all of the game boards
  (player sonar/fleet, cpu sonar/fleet [with DevMode]).
*/
void printAllBoards() {
  if (devMode) {
    printPlayerAndCompBoards(true, playerSonar, compSonar); // sonar
    printPlayerAndCompBoards(false, playerBoard, compBoard); // fleet
  } else {
    printGame(playerSonar, false);
    printGame(playerBoard, true);
  }
}

/*
  Only called from DevMode: print player fleet/sonar AND cpu fleet/sonar.
*/
void printPlayerAndCompBoards(bool isSonar, char playerBoard[10][10], char compBoard[10][10]) {
  // GOAL: print player sonar and comp sonar side by side, with both fleets side by side below them
  int currentRow = 0;
  int currentCol = 0;
  int count = 0;
  bool blankSpace = false;

  isSonar ? cout << "Your Sonar:" : cout << "Your Fleet:";
  for (int s = 0; s < 87; s++) {
    if (s == 34) {
      isSonar ? cout << "Comp Sonar:" : cout << "Comp Fleet:";
    } else {
      cout << " ";
    }
  }
  cout << endl;

  // Print player sonar first, then comp sonar
  for (int h = 0; h < 87; h++) { // loop for printing col indices
    if (count % 4 == 0 && count != 0 && count != 11) { // 11th col is spacer between the boards
      blankSpace ? cout << " " : cout << currentCol;
      currentCol++;
    } else {
      cout << "-";
    } // if-else for printing next column index or -
    count++;
    blankSpace = (count >= 44 && count <= 47);
    if (count == 48) { // need to reset the col number when we switch over to the comp sonar
      currentCol = 0;
    }
  } // print the top row;
  cout << endl;

  for (int i = 0; i < 10; i++) { // loop for printing row indices
    cout << currentRow << " | ";
    for (int j = 0; j < 20; j++) { // inner for traversing cols
      if (j < 10) {
        ansi ? cout << formatCharacterANSI(playerBoard[i][j]) : cout << playerBoard[i][j];
      } else {
        if (j == 10) { // need spaces to visually separate the two boards
          cout << "    ";
        }
        ansi ? cout << formatCharacterANSI(compBoard[i][j-10]) : cout << compBoard[i][j-10];
      }
      cout << "   ";
    }
    currentRow++;
    cout << "\n" << endl;
  }
  for (int k = 0; k < 87; k++) {
    k == 44 ? cout << " " : cout << "-";
  }
  cout << endl;

  if (isSonar) {
    cout << string{hitChar} << " = HIT" << endl;
    cout << string{missChar} << " = MISS" << endl;
    cout << endl;
  }
}

/*
  This function prints the player's game boards to standard output.
  If the board in question is the player's fleet board, then the player's
  fleet is printed. Else, the player's sonar is printed.
*/

void printGame(char arr[10][10], bool isPlayerBoard) {
  int currentRow = 0;
  int currentCol = 0;
  int count = 0;
  if (!isPlayerBoard) {
    cout << "Your Sonar:" << endl;
  } else {
    cout << "Your Fleet:" << endl;
  } //if-else for screen message
  
  for (int h = 0; h < 43; h++) {
    if (count % 4 == 0 && count != 0) {
      cout << currentCol;
      currentCol++;
    } else {
      cout << "-";
    } //if-else for printing next column index or -
    count++;
  } //print the top row -
  cout << endl;

  for (int i = 0; i < 10; i++) {
    cout << currentRow << " | ";
    for (int j = 0; j < 10; j++) {
      //cout << arr[i][j] << "   ";
      ansi ? cout << formatCharacterANSI(arr[i][j]) : cout << arr[i][j];
      cout << "   ";
    } //inner for traversing columns
    currentRow++;
    cout << "\n" << endl;
  } //outter for traversing rows (block prints vertical separators)
  for (int k = 0; k < 43; k++) {
    cout << "-";
  } //print bottom row -
  cout << endl;

  if (!isPlayerBoard) {
    cout << string{hitChar} << " = HIT" << endl;
    cout << string{missChar} << " = MISS" << endl;
    cout << endl;
  } //if printing player's sonar
} //printGame function

/*
  This function prints the computer's game boards to standard output.
  If the board in question is the computer's fleet board, then the 
  computer's fleet is printed. Else, the computer's sonar is printed.
*/

void printComp(char arr[10][10], bool isCompBoard) {
  int currentRow = 0;
  int currentCol = 0;
  int count = 0;
  if (!isCompBoard) {
    cout << "Comp Sonar:" << endl;
  } else {
    cout << "Comp Fleet:" << endl;
  } //if-else for screen message

  for (int h = 0; h < 43; h++) {
    if (count % 4 == 0 && count != 0) {
      cout << currentCol;
      currentCol++;
    } else {
      cout << "-";
    } //if-else for printing next column index or -
    count++;
  } //for printing top row -
  cout << endl;

  for (int i = 0; i < 10; i++) {
    cout << currentRow << " | ";
    for (int j = 0; j < 10; j++) {
      ansi ? cout << formatCharacterANSI(arr[i][j]) << " | ": cout << arr[i][j] << " | ";
    } //inner for traversing columns
    currentRow++;
    cout << endl;
  } //outter for traversing rows (block prints vertical separators)
  for (int k = 0; k < 43; k++) {
    cout << "-";
  } //for printing bottom row of -
  cout << endl;

  if (!isCompBoard) {
    cout << string{hitChar} << " = HIT" << endl;
    cout << string{missChar} << " = MISS" << endl;
    cout << endl;
  } //if printing comp sonar
} //printComp

/*
  This function takes in parameters for starting coordinates, ending coordinates, and a boolean
  for vertical/horizontal orientation. It then checks to ensure the space on the board enclosed
  within those coordinates contains no pre-occupied spaces, returning true if the space is occupied
  and false otherwise.
*/

bool occupiedSpace(int startRow, int startCol, int endRow, int endCol, bool isVertical, char arr[10][10]) {
  if (isVertical) {
    for (int i = startRow; i <= endRow; i++) {
      if (arr[i][endCol] != '~') {
        return true;
      } //occupied space found
    }
  } else {
    for (int j = startCol; j <= endCol; j++) {
      if (arr[endRow][j] != '~') {
        return true;
      } //occupied space found
    }
  }
  return false;
} //occupiedSpace

/*
  This function simply calls the necessary functions to set the player's ships.
*/

void setPlayerShips() {
  setCarrier(playerBoard);
  setBattleship(playerBoard);
  setDestroyer(playerBoard);
  setSubmarine(playerBoard);
  setPatrol(playerBoard);
} //setPlayerShips

/*
  This function simple calls the necessary functions to set the computer's ships.
*/

void setCompShips() {
  setCarrier(compBoard);
  setBattleship(compBoard);
  setDestroyer(compBoard);
  setSubmarine(compBoard);
  setPatrol(compBoard);
} //setCompShips

/*
  This function sets the player's carrier gamepiece. It uses random integer generation to determine
  if the current placement is valid. Since the carrier is the first ship to be placed, this function does not
  incorporate any error-checking for overlapping another ship.
*/

void setCarrier(char arr[10][10]) {

  int direction = rand() % 2;
  bool isVertical = false;
  int endRow = 0;
  int endCol = 0;

  int startRow = rand() % 10;
  int startCol = rand() % 10;

  if (startRow >= 6 && startCol <= 5) { //case for horizontal placement due to startRow being too large
    isVertical = false;
  }

  else if (startRow >= 6 && startCol >= 6) { //case for horizontal placement due to startCol being too large
    isVertical = false;
    startCol = rand() % 6;
  }

  else if (startRow <= 5 && startCol >= 6) { //case for vertical placement due to startCol being too large
    isVertical = true;
  }

  else if (startRow <= 5 && startCol <= 5) { //optimal case where both startRow & startCol are in bounds
    isVertical = (direction % 2 == 0);
  }

  if (isVertical) {
    endRow = startRow + 4;
    endCol = startCol;

    for (int i = startRow; i <= endRow; i++) {
      arr[i][endCol] = carrierChar;
    }

  } else {
    endRow = startRow;
    endCol = startCol + 4;

    for (int i = startCol; i <= endCol; i++) {
      arr[endRow][i] = carrierChar;
    }
  }
} //setCarrier

/*
  This function sets the Battleship gamepiece. Since the Battleship is the second piece to be placed,
  this function incorporates error checking to ensure that the Battleship does not overlap the Carrier.
*/

void setBattleship(char arr[10][10]) {
  int direction = rand() % 2;
  bool isVertical = false;
  int endRow = 0;
  int endCol = 0;

  int startRow = rand() % 10;
  int startCol = rand() % 10;
  while (arr[startRow][startCol] != '~') {
    startRow = rand() % 10;
    if (arr[startRow][startCol] == '~') {
      break;
    }
    startCol = rand() % 10;
  }

  if (startRow >= 7 && startCol <= 6) { //case for horizontal placement due to startRow being too large
    isVertical = false;
  }

  else if (startRow >= 7 && startCol >= 7) { //case for horizontal placement due to startCol & startRow being too large
    isVertical = false;
    startCol = rand() % 7;
  }

  else if (startRow <= 6 && startCol >= 7) { //case for vertical placement due to startCol being too large
    isVertical = true;
  }

  else if (startRow <= 6 && startCol <= 6) { //optimal case where both startRow & startCol are in bounds
    isVertical = (direction % 2 == 0);
  }

  if (isVertical) {
    endRow = startRow + 3;
    endCol = startCol;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int i = startRow; i <= endRow; i++) {
        if (arr[i][endCol] == '~') {
	        arr[i][endCol] = battleshipChar;
	      }
      }
    } else {
      setBattleship(arr);
      return;
    } //restart the process via recursion to place the battleship

  } else {
    endRow = startRow;
    endCol = startCol + 3;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int i = startCol; i <= endCol; i++) {
	      if (arr[endRow][i] == '~') {
          arr[endRow][i] = battleshipChar;
	      }
      }
    } else {
      setBattleship(arr);
      return;
    } //restart the process via recursion to place the battleship
  } //if-else for vertical/horizontal
} //setBattleship

/*
  This function sets the Destroyer gamepiece. Since the Destroyer is the third ship to be set, 
  this function incorporates error checking to ensure the Destroyer does not overlap
  the Carrier or the Battleship.
*/

void setDestroyer(char arr[10][10]) {

  int direction = rand() % 2;
  bool isVertical = false;
  int endRow = 0;
  int endCol = 0;

  int startRow = rand() % 10;
  int startCol = rand() % 10;
  while (arr[startRow][startCol] != '~') {
    startRow = rand() % 10;
    if (arr[startRow][startCol] == '~') {
      break;
    } //if starting space is empty
    startCol = rand() % 10;
  } //recalculate startRow and startCol until empty space found

  if (startRow >= 8 && startCol <= 7) {
    isVertical = false;
  } //case for horizontal placement due to startRow being too large

  else if (startRow >= 8 && startCol >= 8) {
    isVertical = false;
    startCol = rand() % 8;
  } //case for horizontal placement due to startCol & startRow being too large

  else if (startRow <= 7 && startCol >= 8) {
    isVertical = true;
  } //case for vertical placement due to startCol being too large

  else if (startRow <= 7 && startCol <= 7) {
    isVertical = (direction % 2 == 0);
  } //optimal case where both startRow & startCol are in bounds

  if (isVertical) {
    endRow = startRow + 2;
    endCol = startCol;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int i = startRow; i <= endRow; i++) {
	      arr[i][endCol] = destroyerChar;
      } //for-loop placing all destroyer spaces
    } else {
      setDestroyer(arr);
      return;
    } //restart process via recusion to place the destroyer
    
  } else {
    endRow = startRow;
    endCol = startCol + 2;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int j = startCol; j <= endCol; j++) {
	      arr[endRow][j] = destroyerChar;
      }
    } else {
      setDestroyer(arr);
      return;
    } //restart the process via recursion to place the destroyer
  } //if-else for vertical/horizontal placement
} //setDestroyer

/*
  This function sets the Submarine gamepiece. Since the Submarine is the fourth piece to be set,
  this function incorporates error checking to ensure that the Submarine does not overlap
  the Carrier, Battleship, or Destroyer.
*/

void setSubmarine(char arr[10][10]) {

  int direction = rand() % 2;
  bool isVertical = false;
  int endRow = 0;
  int endCol = 0;

  int startRow = rand() % 10;
  int startCol = rand() % 10;
  while (arr[startRow][startCol] != '~') {
    startRow = rand() % 10;
    if (arr[startRow][startCol] == '~') {
      break;
    } //if starting space is empty
    startCol = rand() % 10;
  } //recalculate startRow and startCol until empty space found

  if (startRow >=8 && startCol <= 7) {
    isVertical = false;
  } //case for horizontal placement with startRow too large

  else if (startRow >= 8 && startCol >= 8) {
    isVertical = false;
    startCol = rand() % 8;
  } //case for horizontal placement with startRow and startCol too large

  else if (startRow <= 7 && startCol >= 8) {
    isVertical = true;
  } //case for vertical placement with startCol too large

  else if (startRow <= 7 && startCol <= 7) {
    isVertical = (direction % 2 == 0);
  } //optimal case with startRow and startCol within bounds


  if (isVertical) {
    endRow = startRow + 2;
    endCol = startCol;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int i = startRow; i <= endRow; i++) {
	      arr[i][endCol] = submarineChar;
      } //for-loop placing all submarine pieces
    } else {
      setSubmarine(arr);
      return;
    } //restart the process via recursion to place the submarine
    
  } else {
    endRow = startRow;
    endCol = startCol + 2;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int j = startCol; j <= endCol; j++) {
	      arr[endRow][j] = submarineChar;
      } //for-loop placing all submarine pieces
    } else {
      setSubmarine(arr);
      return;
    } //restart process via recursion to place the submarine
  } //if-else for vertical/horizontal orientation
} //setSubmarine

/*
  This function sets the Patrol Boat gamepiece. Since the Patrol Boat is the final piece
  to be set, this function incorporates error checking to ensure that the Boat does not
  overlap the Carrier, Battleship, Destroyer, or Submarine.
*/

void setPatrol(char arr[10][10]) {
  int direction = rand() % 2;
  bool isVertical = false;
  int endRow = 0;
  int endCol = 0;

  int startRow = rand() % 10;
  int startCol = rand() % 10;
  while (arr[startRow][startCol] != '~') {
    startRow = rand() % 10;
    if (arr[startRow][startCol] == '~') {
      break;
    } //if starting space is empty
    startCol = rand() % 10;
  } //recalculate startRow and startCol until empty space found

  if (startRow >= 9 && startCol <= 8) {
    isVertical = false;
  } //case for horizontal placement with startRow too large

  else if (startRow >= 8 && startCol >= 8) {
    isVertical = false;
    startCol = rand() % 9;
  } //case for horizontal placement with startRow and startCol too large

  else if (startRow <= 8 && startCol >= 9) {
    isVertical = true;
  } //case for vertical placement with startCol too large

  else if (startRow <= 8 && startCol <= 8) {
    isVertical = (direction % 2 == 0);
  } //optimal case with startRow and startCol within bounds


  if (isVertical) {
    endRow = startRow + 1;
    endCol = startCol;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int i = startRow; i <= endRow; i++) {
	      arr[i][endCol] = patrolChar;
      } //for-loop placing all patrol pieces
    } else {
      setPatrol(arr);
      return;
    } //restart the process via recursion to place the patrol boat
    
  } else {
    endRow = startRow;
    endCol = startCol + 1;

    if (!occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr)) {
      for (int j = startCol; j <= endCol; j++) {
	      arr[endRow][j] = patrolChar;
      } //for-loop placing all patrol pieces
    } else {
      setPatrol(arr);
      return;
    } //restart process via recursion to place the patrol boat
  } //if-else for vertical/horizontal orientation
} //setPatrol

/*
  This function prompts the player to provide input for firing coordinates, asking for Row then Column.
  Should the player input non-numeric data for either, it will be interpreted as a 0. If the input is
  numeric but is out of range, the player will be repeatedly prompted until valid input is provided.
*/

void prompt() {
  bool rowGood = false;
  bool colGood = false;
  string rowStr;
  string colStr;
  int rowCoor;
  int colCoor;
  
  cout << endl;
  cout << "Enter your firing coordinates." << endl;
  cout << "NOTE: Non-numeric characters interpreted as 0" << endl;

  while (!rowGood) {
    cout << "Row: ";
    cin >> rowStr;
    //cin.clear();
    rowCoor = std::atoi(rowStr.c_str());
    if (rowCoor >= 0 && rowCoor <= 9) {
      rowGood = true;
    } else {
      printError("Row out of range [0,9]");
    } //if-else
  } //while validating row input

  while (!colGood) {
    cout << "Column: ";
    cin >> colStr;
    //cin.clear();
    colCoor = std::atoi(colStr.c_str());
    if (colCoor >= 0 && colCoor <= 9) {
      colGood = true;
    } else {
      printError("Column out of range [0,9]");
    } //of-else
  } //while validating col input
  playerFire(rowCoor, colCoor);
} //prompt

/*
  This function takes in the player's row and column firing coordinates and "fires"
  on the computer's fleet. The results of the firing are reported to the player's sonar,
  be it a hit or a miss.

  When detonating a mine, explosion covers coordinates as well as x+1, x-1, y+1, y-1, for 5 spaces total.
*/

void playerFire(int row, int col) {

  cout << "Provided Coordinates: " << row << " " << col << endl;
  
  sleep(1.5); //timed delay
  
  if (playerSonar[row][col] != '~') {
    cout << "Provided coordinates already used. Try again." << endl;
    prompt();
    return;
  } //if user enters coordinates already used

  if (compBoard[row][col] != '~' && compBoard[row][col] != mineChar) { // if player scores a hit against the computer
    compBoard[row][col] = '!';
    playerSonar[row][col] = hitChar;
    if (ansi) {
      cout << formatCharacterANSI(hitMsgChar, true) << endl;
    } else {
      cout << "HIT! Enemy Sustained Damage" << endl;
    }
  } else if (compBoard[row][col] == mineChar) { // if player hits a mine in the CPU fleet
    array<tuple<int, int>, 5> blastCoordinates = {
      std::make_tuple(row, col),
      std::make_tuple(row + 1, col),
      std::make_tuple(row - 1, col),
      std::make_tuple(row, col + 1),
      std::make_tuple(row, col - 1)
    };
    cout << "You struck a mine!" << endl;
    sleep(1.5);
    for (int i = 0; i < 5; i++) {
      int new_col = std::get<1>(blastCoordinates[i]);
      int new_row = std::get<0>(blastCoordinates[i]);
  
      if (new_col >= 0 && new_col <= 9 && new_row >= 0 && new_row <= 9) {
        if (compBoard[new_row][new_col] != '~' && compBoard[new_row][new_col] != mineChar) {
          compBoard[new_row][new_col] = '!';
          playerSonar[new_row][new_col] = hitChar;
          if (ansi) {
            cout << formatCharacterANSI(hitMsgChar, true) << " at (" << new_row << ", " << new_col << ")" << endl;
          } else {
            cout << "HIT! Enemy Sustained Damage at (" << new_row << ", " << new_col << ")" << endl;
          }
        } else {
          playerSonar[new_row][new_col] = missChar;
        }
      }
    }
  }

  else {
    playerSonar[row][col] = missChar;
    if (ansi) {
      cout << formatCharacterANSI(missMsgChar, true) << endl;
    } else {
      cout << "MISS! Enemy Evaded Attack" << endl;
    }
  } //else (if user misses)

  sleep(2);

} //playerFire

/*
  Add a new tuple<int, int, char> to the comp's logged coordinates vector
  to maintain a memory of this hit.
*/
void pushCoordinates(int row, int col, char mark) {
  compLoggedCoordinates.push_back(std::make_tuple(row, col, mark));
}

/*
  Display the comp's logged coordinates in a graceful manner.
*/
void printLoggedCoordinates() {
  for (auto tuple : compLoggedCoordinates) {
    cout << "(" << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ", " << std::get<2>(tuple) << ") ";
  }
  cout << endl;
}

/*
  This function takes in firing coordinates and 'fires' on the player's fleet.
  It will output appropriate messages to the screen based on whether the computer
  scored a hit or a miss. It will add a '!' to the player's fleet in the event of a hit.
  Firing coordinates are determined by the compSmartFire function, which calls this function.
  
  This function also has a role to play in the AI's operation, since it will set
  the oriPrevRow and oriPrevCol variables if they are currently unset. It will also
  update compPrevRow and compPrevCol if the computer scores a hit on the player.

  This function also updates the computer's sonar with an X or O, which can be seen
  if devMode is set to true.
*/

void compFire(int row, int col) {
  cout << "Computer is firing" << endl;
  sleep(2);
  
  char target = playerBoard[row][col];
  if (target != '~' && target != '!' && target != mineChar) {
    pushCoordinates(row, col, target);
    //printLoggedCoordinates();
    playerBoard[row][col] = '!';
    compSonar[row][col] = hitChar;
    if (ansi) {
      cout << formatCharacterANSI(hitMsgChar) <<  " at (" << row << ", " << col << ")" << endl;
    } else {
      cout << "HIT! You've sustained damage at (" << row << ", " << col << ")" << endl;
    }
    if (oriPrevRow == -1 && oriPrevCol == -1) {
      oriPrevRow = row;
      oriPrevCol = col;
    } //if original coordinates not yet set
    
    compPrevRow = row;
    compPrevCol = col;
  } //if player takes a hit
  
  else {
    if (ansi) {
      cout << formatCharacterANSI(missMsgChar) << " at (" << row << ", " << col << ")" << endl;
    } else {
      cout << "MISS! You've Evaded Damage at (" << row << ", " << col << ")" << endl;
    }
    compSonar[row][col] = missChar;
  } //else (computer missed)
  
  sleep(2);
} //compFire

/*
  This function houses much of the AI capabilities in the game and allows
  the computer to make 'smarter' firing decisions. To summarize, if the computer
  has not yet scored any hits, then it will fire on random unused coordinates
  on each turn until a hit is made.

  It will then log the hit's coordinates as oriPrevRow, oriPrevCol, compPrevRow, and compPrevCol
  for the row and column coordinates. On its next turn, the computer will fire on an unused coordinate
  that is adjacent to compPrevRow, compPrevCol. Should this yield a hit, then compPrevRow and compPrevCol
  are updated to the coordinates of the hit.

  If there are no unused coordinates around the computer's most recent hit, then compPrevRow and compPrevCol
  are updated to the values of oriPrevRow and oriPrevCol so that the computer can reach the rest of the ship
  being attacked. The function actually calls the compFire function to carry out the firing and board-marking
  operations. This function contains exhaustive cases for different coordinate scenarios since a hit in certain
  regions of the board may necessitate a unique course of action.
*/

void compSmartFire() {
  if (compPrevRow == -1 && compPrevCol == -1) {
    int fireRow = rand() % 10;
    int fireCol = rand() % 10;
    
    while (compSonar[fireRow][fireCol] != '~') {
      fireRow = rand() % 10;
      if (compSonar[fireRow][fireCol] == '~')
	break;
      else fireCol = rand() % 10;
    } //while validating the coordinates as unused
    
    compFire(fireRow, fireCol);
    return;
  } //if no previous hit coordinates set (start of game or after sinking a ship)
  
  if (compPrevRow == 0 && compPrevCol == 0) {
    //cout << "case 1" << endl;
    if (compSonar[compPrevRow][compPrevCol + 1] == '~') {
      compFire(compPrevRow, compPrevCol + 1);
      return;
    } //right
    else if (compSonar[compPrevRow + 1][compPrevCol] == '~') {
      compFire(compPrevRow + 1, compPrevCol);
      return;
    } //down
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev coordinates and try again
  } //check right, down for empty space

  if (compPrevRow == 0 && 0 < compPrevCol && compPrevCol <= 8) {
    //cout << "case 2" << endl;
    if (compSonar[compPrevRow][compPrevCol + 1] == '~') {
      compFire(compPrevRow, compPrevCol + 1);
      return;
    } //right
    else if (compSonar[compPrevRow + 1][compPrevCol] == '~') {
      compFire(compPrevRow + 1, compPrevCol);
      return;
    } //down
    else if (compSonar[compPrevRow][compPrevCol - 1] == '~') {
      compFire(compPrevRow, compPrevCol - 1);
      return;
    } //left
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev cooridnates and try again
  } //check right, down, left for empty space

  if (compPrevRow == 0 && compPrevCol == 9) {
    //cout << "case 3" << endl;
    if (compSonar[compPrevRow + 1][compPrevCol] == '~') {
      compFire(compPrevRow + 1, compPrevCol);
      return;
    } //down
    else if (compSonar[compPrevRow][compPrevCol - 1] == '~') {
      compFire(compPrevRow, compPrevCol - 1);
      return;
    } //left
  } //check down, left for empty space

  if (0 < compPrevRow && compPrevRow <= 8 && compPrevCol == 0) {
    //cout << "case 4" << endl;
    if (compSonar[compPrevRow + 1][compPrevCol] == '~') {
      compFire(compPrevRow + 1, compPrevCol);
      return;
    } //down
    else if (compSonar[compPrevRow - 1][compPrevCol] == '~') {
      compFire(compPrevRow - 1, compPrevCol);
      return;
    } //up
    else if (compSonar[compPrevRow][compPrevCol + 1] == '~') {
      compFire(compPrevRow, compPrevCol + 1);
      return;
    } //right
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev coordinates and try again
  } //check down, up, right for empty space

  if (0 < compPrevRow && compPrevRow <= 8 && compPrevCol == 9) {
    //cout << "case 5" << endl;
    if (compSonar[compPrevRow + 1][compPrevCol] == '~') {
      compFire(compPrevRow + 1, compPrevCol);
      return;
    } //down
    else if (compSonar[compPrevRow - 1][compPrevCol] == '~') {
      compFire(compPrevRow - 1, compPrevCol);
      return;
    } //up
    else if (compSonar[compPrevRow][compPrevCol - 1] == '~') {
      compFire(compPrevRow, compPrevCol - 1);
      return;
    } //left
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev coordinates and try again
  } //check down, up, left for empty space

  if (compPrevRow == 9 && compPrevCol == 0) {
    //cout << "case 6" << endl;
    if (compSonar[compPrevRow - 1][compPrevCol] == '~') {
      compFire(compPrevRow - 1, compPrevCol);
      return;
    } //up
    else if (compSonar[compPrevRow][compPrevCol + 1] == '~') {
      compFire(compPrevRow, compPrevCol + 1);
      return;
    } //right
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev coordintes and try again
  } //check up, right for empty space

  if (compPrevRow == 9 && 0 < compPrevCol && compPrevCol <= 8) {
    //cout << "case 7" << endl;
    if (compSonar[compPrevRow][compPrevCol - 1] == '~') {
      compFire(compPrevRow, compPrevCol - 1);
      return;
    } //left
    else if (compSonar[compPrevRow - 1][compPrevCol] == '~') {
      compFire(compPrevRow - 1, compPrevCol);
      return;
    } //up
    else if (compSonar[compPrevRow][compPrevCol + 1] == '~') {
      compFire(compPrevRow, compPrevCol + 1);
    } //right
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev coordinates and try again
  } //check left, up, right for empty space

  if (0 < compPrevRow & compPrevRow <= 8 && 0 < compPrevCol && compPrevCol <= 8) {
    //cout << "general case" << endl;
    if (compSonar[compPrevRow - 1][compPrevCol] == '~') {
      compFire(compPrevRow - 1, compPrevCol);
      return;
    } //up
    else if (compSonar[compPrevRow + 1][compPrevCol] == '~') {
      compFire(compPrevRow + 1, compPrevCol);
      return;
    } //down
    else if (compSonar[compPrevRow][compPrevCol - 1] == '~') {
      compFire(compPrevRow, compPrevCol - 1);
      return;
    } //left
    else if (compSonar[compPrevRow][compPrevCol + 1] == '~') {
      compFire(compPrevRow, compPrevCol + 1);
      return;
    } //right
    else {
      compPrevRow = oriPrevRow;
      compPrevCol = oriPrevCol;
      compSmartFire();
      return;
    } //else reset prev coordinates and try again
  } //general case (check all directions for empty space)
} //compSmartFire

/*
  When the computer sinks a player's ship, we want to remove the sunken ship's coordinates
  from the comp's logged coordinates vector. This ensures that any coordinates left over in the vector
  can quickly be used after sinking the first ship, making the ai "smarter" by remembering it has hit(s)
  on other ship(s).
*/
void scrubLoggedCoordinates(char mark) {
  for (auto it = compLoggedCoordinates.begin(); it != compLoggedCoordinates.end();) {
    if (std::get<2>(*it) == mark) {
      it = compLoggedCoordinates.erase(it);
    } else {
      it++;
    }
  }
  if (devMode) {
  cout << "scrubbed coordinates: " << endl;
  printLoggedCoordinates();
  }
}

/*
  This function checks the parameter array for a certain char (code).
  In terms of gameplay, it checks the parameter game board for a certain ship.
  It will return true if said ship was not found (i.e. is sunk) and false if
  it was found.
 */

bool checkVessel(char arr[10][10], char code) {
  bool vesselGone = true;
  for (int i = 0; i <= 9; i++) {
    for (int j = 0; j <= 9; j++) {
      if (arr[i][j] == code)
	vesselGone = false;
    } //inner for
  } //outer for
  return vesselGone;
} //checkVessel

/*
  This function is called in main after the player has completed a turn and
  checks to see if said turn resulted in the sinking of any of the computer's ships.
  It does this by calling the checkVessel function with appropriate parameters.
  Boolean values returned by said function calls are used in main to determine if
  the player has won.
 */

void checkAfterPlayer() {
  if (!cCsunk) {
    cCsunk = checkVessel(compBoard, carrierChar);
    if (cCsunk)
      cout << "You've sunk the enemy \033[4;31mCarrier!\033[0m" << endl;
  } //if comp carrier not yet reported as sunk

  if (!cDsunk) {
    cDsunk = checkVessel(compBoard, destroyerChar);
    if (cDsunk)
      cout << "You've sunk the enemy \033[4;34mDestroyer!\033[0m" << endl;
  } //if comp destroyer not yet reported as sunk

  if (!cBsunk) {
    cBsunk = checkVessel(compBoard, battleshipChar);
    if (cBsunk)
      cout << "You've sunk the enemy \033[4;32mBattleship!\033[0m" << endl;
  } //if comp battleship not yet reported as sunk

  if (!cSsunk) {
    cSsunk = checkVessel(compBoard, submarineChar);
    if (cSsunk)
      cout << "You've sunk the enemy \033[4;33mSubmarine!\033[0m" << endl;
  } //if comp submarine not yet reported as sunk

  if (!cPsunk) {
    cPsunk = checkVessel(compBoard, patrolChar);
    if (cPsunk)
      cout << "You've sunk the enemy \033[4;36mPatrol Boat!\033[0m" << endl;
  } //if comp patrol boat not yet reported as sunk

  if (cCsunk && cDsunk && cBsunk && cSsunk && cPsunk)
    playerWon = true;
  sleep(1.5);
} //checkAfterPlayer

/*
  To enhance the AI, when it falls back on its vector of logged coordinates, we want to 
  allow it to select its new coordinates from a random tuple in the vector, rather than always
  falling back on a set index.
*/
int pickAnIndex() {
  int length = compLoggedCoordinates.size();
  if (length == 0) {
    return -1;
  }
  int newIndex = rand() % length;
  return newIndex;
}

/*
  This function will pick a set of coordinates from the comp's logged coordinates vector and then
  assign those new coordinates to the comp's prev row and prev col so the comp can resume play
  from a ship that it has already hit, but has not yet sunk.
*/
void assignNewCoordinates() {
  int newIndex = pickAnIndex();
  if (newIndex != -1) {
    compPrevRow = std::get<0>(compLoggedCoordinates[newIndex]);
    compPrevCol = std::get<1>(compLoggedCoordinates[newIndex]);
  } else {
    compPrevRow = -1;
    compPrevCol = -1;
  }
}

/*
  This function is called in main after the computer has finished
  a turn. It checks to see if the player's ships are sunk yet by calling
  the checkVessel function with appropriate parameters. If a player's ship
  was sunk on the previous turn, then the computer's prev coordinates and 
  original prev coordinates are reset to -1 to facilitate a new iteration of
  the smartFire operation. Boolean values returned by the checkVessel function
  are used in main to determine if the computer has just won.
 */

void checkAfterComp() {

  if (!pCsunk) {
    pCsunk = checkVessel(playerBoard, carrierChar);
    if (pCsunk) {
      cout << "Your " << formatCharacterANSI(carrierChar) << " has been sunk!" << endl;
      scrubLoggedCoordinates(carrierChar);
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player carrier now sunk
  } //if player carrier not yet reported as sunk

  if (!pDsunk) {
    pDsunk = checkVessel(playerBoard, destroyerChar);
    if (pDsunk) {
      cout << "Your " << formatCharacterANSI(destroyerChar) << " has been sunk!" << endl;
      scrubLoggedCoordinates(destroyerChar);
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player destroyer now sunk
  } //if player destroyer not yet reported as sunk

  if (!pBsunk) {
    pBsunk = checkVessel(playerBoard, battleshipChar);
    if (pBsunk) {
      cout << "Your " << formatCharacterANSI(battleshipChar) << " has been sunk!" << endl;
      scrubLoggedCoordinates(battleshipChar);
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player battleship now sunk
  } //if player battleship not yet reported as sunk

  if (!pSsunk) {
    pSsunk = checkVessel(playerBoard, submarineChar);
    if (pSsunk) {
      cout << "Your " << formatCharacterANSI(submarineChar) << " has been sunk!" << endl;
      scrubLoggedCoordinates(submarineChar);
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player sub now sunk
  } //if player sub not yet reported as sunk

  if (!pPsunk) {
    pPsunk = checkVessel(playerBoard, patrolChar);
    if (pPsunk) {
      cout << "Your " << formatCharacterANSI(patrolChar) << " has been sunk!" << endl;
      scrubLoggedCoordinates(patrolChar);
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player patrol now sunk
  } //if player patrol boat not yet reported as sunk
  sleep(1.5);  
} //checkAfterComp

// setMines() sets the mines on the game board according to the max number of mines allotted
void setMines(char arr[10][10], int mineCount) {
  int minesSet = 0;
  while (minesSet < mineCount) {
    int row = rand() % 10;
    int col = rand() % 10;

    if (arr[row][col] == '~') { // only set mines on empty spaces
      arr[row][col] = mineChar;
      minesSet++;
    }
  }
}