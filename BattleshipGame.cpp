#include <cstdlib>
#include <string>
#include <iostream>
#include <time.h>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>
#include <windows.h>
#include <unistd.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::tuple;
using std::vector;


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

char playerSonar[10][10]; //represents the player's sonar
char playerBoard[10][10]; //represents the player's fleet

char compSonar[10][10]; //represents the computer's sonar
char compBoard[10][10]; //represents the computer's fleet

int compPrevRow = -1;
int compPrevCol = -1;

int oriPrevRow = -1;
int oriPrevCol = -1;

vector<tuple<int, int, char>> compLoggedCoordinates; //let the comp "remember" hit coordinates

/* Function declarations */

void printWelcome();
void askForDev();
void printBoard();
void initialize();
void printGame(char arr[10][10], bool isPlayerBoard);
void printComp(char arr[10][10], bool isCompBoard);
bool occupiedSpace(int startRow, int startCol, int endRow, int endCol, bool isVertical, char arr[10][10]);
void setPlayerShips();
void setCompShips();
void setCarrier(char arr[10][10]);
void setBattleship(char arr[10][10]);
void setDestroyer(char arr[10][10]);
void setSubmarine(char arr[10][10]);
void setPatrol(char arr[10][10]);

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
  int ref = 0;
  srand(time(0));
  initialize();
  printWelcome();
  askForDev();
  setPlayerShips();
  setCompShips();
  
  while (!playerWon && !compWon) {
    printGame(playerSonar, false);
    printGame(playerBoard, true);
    
    if (devMode) {
      cout << "devMode = true, printing comp boards" << endl;
      printComp(compSonar, false);
      printComp(compBoard, true);
    } //if devMode is set to true
    
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
  cout << "\033[4;31mC = Carrier\033[0m" << endl;    
  cout << "\033[4;32mB = Battleship\033[0m" << endl;
  cout << "\033[4;34mD = Destroyer\033[0m" << endl;
  cout << "\033[4;33mS = Submarine\033[0m" << endl;
  cout << "\033[4;36mP = Patrol Boat\033[0m\n";
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
      cout << arr[i][j] << " | ";
    } //inner for traversing columns
    currentRow++;
    cout << endl;
  } //outter for traversing rows (block prints vertical separators)
  for (int k = 0; k < 43; k++) {
    cout << "-";
  } //print bottom row -
  cout << endl;

  if (!isPlayerBoard) {
    cout << "X = HIT" << endl;
    cout << "O = MISS" << endl;
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
      cout << arr[i][j] << " | ";
    } //inner for traversing columns
    currentRow++;
    cout << endl;
  } //outter for traversing rows (block prints vertical separators)
  for (int k = 0; k < 43; k++) {
    cout << "-";
  } //for printing bottom row of -
  cout << endl;

  if (!isCompBoard) {
    cout << "X = HIT" << endl;
    cout << "O = MISS" << endl;
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
  } //check for available vertical placement

  if (!isVertical) {
    for (int j = startCol; j <= endCol; j++) {
      if (arr[endRow][j] != '~') {
        return true;
      } //occupied space found
    }
  } //check for available horizontal placement
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
      arr[i][endCol] = 'C';
    }

  } else {
    endRow = startRow;
    endCol = startCol + 4;

    for (int i = startCol; i <= endCol; i++) {
      arr[endRow][i] = 'C';
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

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {

      for (int i = startRow; i <= endRow; i++) {
        if (arr[i][endCol] == '~') {
	  arr[i][endCol] = 'B';
	}
      }
    } //if space is available for the battleship

    else {
      setBattleship(arr);
      return;
    } //restart the process via recursion to place the battleship

  } else {
    endRow = startRow;
    endCol = startCol + 3;

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {

      for (int i = startCol; i <= endCol; i++) {
	if (arr[endRow][i] == '~') {
          arr[endRow][i] = 'B';
	}
      }
    } //if space is available for the battleship

    else {
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

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {
      for (int i = startRow; i <= endRow; i++) {
	arr[i][endCol] = 'D';
      } //for-loop placing all destroyer spaces
    } //if space is available for vertical placement

    else {
      setDestroyer(arr);
      return;
    } //restart process via recusion to place the destroyer
    
  } else {
    endRow = startRow;
    endCol = startCol + 2;

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {
      for (int j = startCol; j <= endCol; j++) {
	arr[endRow][j] = 'D';
      }
    } //if space is available for horizontal placement

    else {
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

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {
      for (int i = startRow; i <= endRow; i++) {
	arr[i][endCol] = 'S';
      } //for-loop placing all submarine pieces
    } //if space is available for vertical placement

    else {
      setSubmarine(arr);
      return;
    } //restart the process via recursion to place the submarine
    
  } else {
    endRow = startRow;
    endCol = startCol + 2;

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {
      for (int j = startCol; j <= endCol; j++) {
	arr[endRow][j] = 'S';
      } //for-loop placing all submarine pieces
    } //if space is available for horizontal placement

    else {
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

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {
      for (int i = startRow; i <= endRow; i++) {
	arr[i][endCol] = 'P';
      } //for-loop placing all patrol pieces
    } //if space is available for vertical placement

    else {
      setPatrol(arr);
      return;
    } //restart the process via recursion to place the patrol boat
    
  } else {
    endRow = startRow;
    endCol = startCol + 1;

    if (occupiedSpace(startRow, startCol, endRow, endCol, isVertical, arr) == false) {
      for (int j = startCol; j <= endCol; j++) {
	arr[endRow][j] = 'P';
      } //for-loop placing all patrol pieces
    } //if space is available for horizontal placement

    else {
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
    rowCoor = std::atoi(rowStr.c_str());
    if (rowCoor >= 0 && rowCoor <= 9) {
      rowGood = true;
    } else {
      cout << "ERROR: Row out of range" << endl;
    } //if-else
  } //while validating row input

  while (!colGood) {
    cout << "Column: ";
    cin >> colStr;
    colCoor = std::atoi(colStr.c_str());
    if (colCoor >= 0 && colCoor <= 9) {
      colGood = true;
    } else {
      cout << "ERROR: Column out of range" << endl;
    } //of-else
  } //while validating col input
  playerFire(rowCoor, colCoor);
} //prompt

/*
  This function takes in the player's row and column firing coordinates and "fires"
  on the computer's fleet. The results of the firing are reported to the player's sonar,
  be it a hit or a miss.
*/

void playerFire(int row, int col) {

  cout << "Provided Coordinates: " << row << " " << col << endl;
  
  sleep(1.5); //timed delay
  
  if (playerSonar[row][col] != '~') {
    cout << "Provided coordinates already used. Try again." << endl;
    prompt();
    return;
  } //if user enters coordinates already used

  if (compBoard[row][col] != '~') {
    compBoard[row][col] = '!';
    playerSonar[row][col] = 'X';
    cout << "HIT! Enemy Sustained Damage" << endl;
  } //if user scores a hit against the computer

  else {
    playerSonar[row][col] = 'O';
    cout << "MISS! Enemy Evaded Attack" << endl;
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
  if (target != '~') {
    pushCoordinates(row, col, target);
    printLoggedCoordinates();
    playerBoard[row][col] = '!';
    cout << "HIT! You've sustained damage at (" << row << ", " << col << ")" << endl;
    compSonar[row][col] = 'X';
    
    if (oriPrevRow == -1 && oriPrevCol == -1) {
      oriPrevRow = row;
      oriPrevCol = col;
    } //if original coordinates not yet set
    
    compPrevRow = row;
    compPrevCol = col;
  } //if player takes a hit
  
  else {
    cout << "MISS! You've evaded damage at (" << row << ", " << col << ")" << endl;
    compSonar[row][col] = 'O';
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
    cCsunk = checkVessel(compBoard, 'C');
    if (cCsunk)
      cout << "You've sunk the enemy \033[4;31mCarrier!\033[0m" << endl;
  } //if comp carrier not yet reported as sunk

  if (!cDsunk) {
    cDsunk = checkVessel(compBoard, 'D');
    if (cDsunk)
      cout << "You've sunk the enemy \033[4;34mDestroyer!\033[0m" << endl;
  } //if comp destroyer not yet reported as sunk

  if (!cBsunk) {
    cBsunk = checkVessel(compBoard, 'B');
    if (cBsunk)
      cout << "You've sunk the enemy \033[4;32mBattleship!\033[0m" << endl;
  } //if comp battleship not yet reported as sunk

  if (!cSsunk) {
    cSsunk = checkVessel(compBoard, 'S');
    if (cSsunk)
      cout << "You've sunk the enemy \033[4;33mSubmarine!\033[0m" << endl;
  } //if comp submarine not yet reported as sunk

  if (!cPsunk) {
    cPsunk = checkVessel(compBoard, 'P');
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
    pCsunk = checkVessel(playerBoard, 'C');
    if (pCsunk) {
      cout << "Your \033[4;31mCarrier\033[0m has been sunk!" << endl;
      scrubLoggedCoordinates('C');
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player carrier now sunk
  } //if player carrier not yet reported as sunk

  if (!pDsunk) {
    pDsunk = checkVessel(playerBoard, 'D');
    if (pDsunk) {
      cout << "Your \033[4;34mDestroyer\033[0m has been sunk!" << endl;
      scrubLoggedCoordinates('D');
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player destroyer now sunk
  } //if player destroyer not yet reported as sunk

  if (!pBsunk) {
    pBsunk = checkVessel(playerBoard, 'B');
    if (pBsunk) {
      cout << "Your \033[4;32mBattleship\033[0m has been sunk!" << endl;
      scrubLoggedCoordinates('B');
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player battleship now sunk
  } //if player battleship not yet reported as sunk

  if (!pSsunk) {
    pSsunk = checkVessel(playerBoard, 'S');
    if (pSsunk) {
      cout << "Your \033[4;33mSubmarine\033[0m has been sunk!" << endl;
      scrubLoggedCoordinates('S');
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player sub now sunk
  } //if player sub not yet reported as sunk

  if (!pPsunk) {
    pPsunk = checkVessel(playerBoard, 'P');
    if (pPsunk) {
      cout << "Your \033[4;36mPatrol Boat\033[0m has been sunk!" << endl;
      scrubLoggedCoordinates('P');
      assignNewCoordinates();
      oriPrevRow = -1;
      oriPrevCol = -1;
    } //if player patrol now sunk
  } //if player patrol boat not yet reported as sunk
  sleep(1.5);  
} //checkAfterComp
