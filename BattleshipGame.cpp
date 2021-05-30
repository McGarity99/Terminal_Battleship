#include <cstdlib>
#include <string>
#include <iostream>
#include <time.h>
#include <chrono>
#include <thread>
#include <unistd.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;


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

char compSonar[10][10];
char compBoard[10][10];

int compPrevRow = -1;
int compPrevCol = -1;

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
void compFire();
bool compSmartFire();
bool checkVessel(char arr[10][10], char code);
void checkAfterPlayer();
void checkAfterComp();

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
    
    compFire();
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
*/

void printWelcome() {
  cout << "Welcome to Command-Line Battleship!" << endl;
  cout << "Play against a random-select AI for supremacy on the high seas." << endl;
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
  This function allows the computer to "fire" on the player's fleet. If the compPrevRow and compPrevCol
  variables are currently unset (value of -1), then this function calls the compSmartFire function.
  Should the compSmartFire function fail to yield a hit, then this function will continue with
  "normal" firing procedures. It will generate random row and col coordinates until an unused space is found.
  It will also mark the results of the firing on the computer's sonar (in the interest of devMode), as well as
  alert the player to whether the shot was hit or miss.
*/

void compFire() {
  sleep(2);
  cout << "Computer is Firing" << endl;
  sleep(1.5);
  bool smartFireSuccess = false;
  if (compPrevRow != -1 && compPrevCol != -1) {
    smartFireSuccess = compSmartFire();
  } //if previous turn yielded a hit (smart fire)

  if (!smartFireSuccess) {
    int rowCoor = rand() % 10;
    int colCoor = rand() % 10;
    while (compSonar[rowCoor][colCoor] != '~') {
      rowCoor = rand() % 10;
      if (compSonar[rowCoor][colCoor] == '~')
	break;
      else colCoor = rand() % 10;
    } //while validating the coordinates

    if (playerBoard[rowCoor][colCoor] != '~') {
      playerBoard[rowCoor][colCoor] = '!';
      compSonar[rowCoor][colCoor] = 'X';
      cout << "HIT! You've taken damage" << endl;
      compPrevRow = rowCoor;
      compPrevCol = colCoor;
    } //if comp scored a hit against user
    else {
      compSonar[rowCoor][colCoor] = 'O';
      cout << "MISS! You've evaded damage" << endl;
    } //else (miss)
  } //regular firing process if smart fire yielded no hit
  sleep(2);
} //compFire

/*
  This function allows the computer to make 'smarter' firing decisions
  so long as the previous turn yielded a hit.
*/

bool compSmartFire() {
  cout << "smart fire" << endl;
  if ((compPrevCol + 1) <= 9) {
    if (compSonar[compPrevRow][compPrevCol + 1] != '~') {
      cout << "smartfire right used" << endl;
    } //if right point is already used (do nothing)
    else if (playerBoard[compPrevRow][compPrevCol + 1] != '~') {
      cout << "smartfire right hit" << endl;
      playerBoard[compPrevRow][compPrevCol + 1] = '!';
      compSonar[compPrevRow][compPrevCol + 1] = 'X';
      cout << "HIT! You've taken damage" << endl;
      compPrevCol++;
      return true;
    } //if comp scored hit against user
    else {
      cout << "smartfire right miss" << endl;
      //compSonar[compPrevRow][compPrevCol + 1] = 'O';
      //cout << "MISS! You've evaded damage" << endl;
    } //else (miss)
  } //check right

  if ((compPrevCol - 1) >= 0) {
    if (compSonar[compPrevRow][compPrevCol - 1] != '~') {
      cout << "smartfire left used" << endl;
    } //if left point is already used (do nothing)
    else if (playerBoard[compPrevRow][compPrevCol - 1] != '~') {
      cout << "smartfire left hit" << endl;
      playerBoard[compPrevRow][compPrevCol - 1] = '!';
      compSonar[compPrevRow][compPrevCol - 1] = 'X';
      cout << "HIT! You've taken damage" << endl;
      compPrevCol--;
      return true;
    } //if comp scored hit against user
    else {
      cout << "smartfire left missed" << endl;
      //compSonar[compPrevRow][compPrevCol - 1] = 'O';
      //cout << "MISS! You've evaded damage" << endl;
    } //else (miss)
  } //check left

  if ((compPrevRow + 1) <= 9) {
    if (compSonar[compPrevRow + 1][compPrevCol] != '~') {
      cout << "smartfire down used" << endl;
    } //if below point is already used (do nothing)
    else if (playerBoard[compPrevRow + 1][compPrevCol] != '~') {
      cout << "smartfire down hit" << endl;
      playerBoard[compPrevRow + 1][compPrevCol] = '!';
      compSonar[compPrevRow + 1][compPrevCol] = 'X';
      cout << "HIT! You've taken damage" << endl;
      compPrevRow++;
      return true;
    } //if comp scored hit against user
    else {
      cout << "smartfire down miss" << endl;
      //compSonar[compPrevRow + 1][compPrevCol] = 'O';
      //cout << "MISS! You've evaded damage" << endl;
    } //else (miss)
  } //check down

  if ((compPrevRow - 1) >= 0) {
    if (compSonar[compPrevRow - 1][compPrevCol] != '~') {
      cout << "smartfire up used" << endl;
    } //if above point is already used (do nothing)
    else if (playerBoard[compPrevRow - 1][compPrevCol] != '~') {
      cout << "smartfire up hit" << endl;
      playerBoard[compPrevRow - 1][compPrevCol] = '!';
      compSonar[compPrevRow - 1][compPrevCol] = 'X';
      cout << "HIT! You've taken damage" << endl;
      compPrevRow--;
      return true;
    } //if comp scored hit against user
    else {
      cout << "smartfire up miss" << endl;
      //compSonar[compPrevRow - 1][compPrevCol] = 'O';
      //cout << "MISS! You've evaded damage" << endl;
    } //else (miss)
  } //check up
  compPrevRow = -1;
  compPrevCol = -1;
  return false;
} //compSmartFire


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

void checkAfterPlayer() {
  if (!cCsunk) {
    cCsunk = checkVessel(compBoard, 'C');
    if (cCsunk)
      cout << "You've sunk the enemy Carrier!" << endl;
  } //if comp carrier not yet reported as sunk

  if (!cDsunk) {
    cDsunk = checkVessel(compBoard, 'D');
    if (cDsunk)
      cout << "You've sunk the enemy Destroyer!" << endl;
  } //if comp destroyer not yet reported as sunk

  if (!cBsunk) {
    cBsunk = checkVessel(compBoard, 'B');
    if (cBsunk)
      cout << "You've sunk the enemy Battleship!" << endl;
  } //if comp battleship not yet reported as sunk

  if (!cSsunk) {
    cSsunk = checkVessel(compBoard, 'S');
    if (cSsunk)
      cout << "You've sunk the enemy Submarine!" << endl;
  } //if comp submarine not yet reported as sunk

  if (!cPsunk) {
    cPsunk = checkVessel(compBoard, 'P');
    if (cPsunk)
      cout << "You've sunk the enemy Patrol Boat!" << endl;
  } //if comp patrol boat not yet reported as sunk

  if (cCsunk && cDsunk && cBsunk && cSsunk && cPsunk)
    playerWon = true;
} //checkAfterPlayer

void checkAfterComp() {

  if (!pCsunk) {
    pCsunk = checkVessel(playerBoard, 'C');
    if (pCsunk) {
      cout << "Your Carrier has been sunk!" << endl;
      compPrevRow = -1;
      compPrevCol = -1;
    } //if player carrier now sunk
  } //if player carrier not yet reported as sunk

  if (!pDsunk) {
    pDsunk = checkVessel(playerBoard, 'D');
    if (pDsunk) {
      cout << "Your Destroyer has been sunk!" << endl;
      compPrevRow = -1;
      compPrevCol = -1;
    } //if player destroyer now sunk
  } //if player destroyer not yet reported as sunk

  if (!pBsunk) {
    pBsunk = checkVessel(playerBoard, 'B');
    if (pBsunk) {
      cout << "Your Battleship has been sunk!" << endl;
      compPrevRow = -1;
      compPrevCol = -1;
    } //if player battleship now sunk
  } //if player battleship not yet reported as sunk

  if (!pSsunk) {
    pSsunk = checkVessel(playerBoard, 'S');
    if (pSsunk) {
      cout << "Your Submarine has been sunk!" << endl;
      compPrevRow = -1;
      compPrevCol = -1;
    } //if player sub now sunk
  } //if player sub not yet reported as sunk

  if (!pPsunk) {
    pPsunk = checkVessel(playerBoard, 'P');
    if (pPsunk) {
      cout << "Your Patrol Boat has been sunk!" << endl;
      compPrevRow = -1;
      compPrevCol = -1;
    } //if player patrol now sunk
  } //if player patrol boat not yet reported as sunk
  
} //checkAfterComp
