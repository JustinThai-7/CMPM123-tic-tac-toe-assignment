#include "TicTacToe.h"
#include <cstdlib>
#include <ctime>
#include <vector>

// -----------------------------------------------------------------------------
// TicTacToe.cpp
// -----------------------------------------------------------------------------
// This file is intentionally *full of comments* and gentle TODOs that guide you
// through wiring up a complete Tic‑Tac‑Toe implementation using the game
// engine’s Bit / BitHolder grid system.
//
// Rules recap:
//  - Two players place X / O on a 3x3 grid.
//  - Players take turns; you can only place into an empty square.
//  - First player to get three-in-a-row (row, column, or diagonal) wins.
//  - If all 9 squares are filled and nobody wins, it’s a draw.
//
// Notes about the provided engine types you'll use here:
//  - Bit              : a visual piece (sprite) that belongs to a Player
//  - BitHolder        : a square on the board that can hold at most one Bit
//  - Player           : the engine’s player object (you can ask who owns a Bit)
//  - Game options     : let the mouse know the grid is 3x3 (rowX, rowY)
//  - Helpers you’ll see used: setNumberOfPlayers, getPlayerAt, startGame, etc.
//
// I’ve already fully implemented PieceForPlayer() for you. Please leave that
// as‑is. The rest of the routines are written as “comment-first” TODOs for you
// to complete.
// -----------------------------------------------------------------------------

const int AI_PLAYER = 1;    // index of the AI player (O)
const int HUMAN_PLAYER = 0; // index of the human player (X)

TicTacToe::TicTacToe() {}

TicTacToe::~TicTacToe() {}

// -----------------------------------------------------------------------------
// make an X or an O
// -----------------------------------------------------------------------------
// DO NOT CHANGE: This returns a new Bit with the right texture and owner
Bit *TicTacToe::PieceForPlayer(const int playerNumber) {
  // depending on playerNumber load the "x.png" or the "o.png" graphic
  Bit *bit = new Bit();
  bit->LoadTextureFromFile(playerNumber == 0 ? "x.png" : "o.png");
  bit->setOwner(getPlayerAt(playerNumber));
  return bit;
}

//
// setup the game board, this is called once at the start of the game
//
void TicTacToe::setUpBoard() {
  setNumberOfPlayers(2);
  _gameOptions.rowX = 3;
  _gameOptions.rowY = 3;

  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      _grid[y][x].initHolder(ImVec2(100 + x * 100, 100 + y * 100), "square.png",
                             x, y);
      _grid[y][x].setGameTag(0); // 0 for empty
    }
  }

  startGame();
}

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool TicTacToe::actionForEmptyHolder(BitHolder *holder) {
  if (!holder)
    return false;
  if (holder->bit())
    return false;

  Player *p = getCurrentPlayer();
  if (!p)
    return false;

  // Block human input when it's AI's turn
  if (_gameOptions.AIPlaying && p->playerNumber() == _gameOptions.AIPlayer) {
    return false;
  }

  Bit *bit = PieceForPlayer(p->playerNumber());
  if (bit) {
    bit->setPosition(holder->getPosition());
    holder->setBit(bit);
    return true;
  }

  return false;
}

bool TicTacToe::canBitMoveFrom(Bit *bit, BitHolder *src) { return false; }

bool TicTacToe::canBitMoveFromTo(Bit *bit, BitHolder *src, BitHolder *dst) {
  return false;
}

//
// free all the memory used by the game on the heap
//
void TicTacToe::stopGame() {
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      _grid[y][x].destroyBit();
    }
  }
}

//
// helper function for the winner check
//
Player *TicTacToe::ownerAt(int index) const {
  if (index < 0 || index >= 9)
    return nullptr;
  int y = index / 3;
  int x = index % 3;
  if (_grid[y][x].bit()) {
    return _grid[y][x].bit()->getOwner();
  }
  return nullptr;
}

Player *TicTacToe::checkForWinner() {
  int wins[8][3] = {
      {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
      {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // cols
      {0, 4, 8}, {2, 4, 6}             // diagonals
  };

  for (int i = 0; i < 8; i++) {
    Player *p0 = ownerAt(wins[i][0]);
    Player *p1 = ownerAt(wins[i][1]);
    Player *p2 = ownerAt(wins[i][2]);

    if (p0 && p0 == p1 && p1 == p2) {
      return p0;
    }
  }
  return nullptr;
}

bool TicTacToe::checkForDraw() {
  for (int i = 0; i < 9; i++) {
    if (ownerAt(i) == nullptr) {
      return false;
    }
  }
  return true;
}

//
// state strings
//
std::string TicTacToe::initialStateString() { return "000000000"; }

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string TicTacToe::stateString() const {
  std::string s = "";
  for (int i = 0; i < 9; i++) {
    Player *p = ownerAt(i);
    if (p) {
      s += std::to_string(p->playerNumber() + 1);
    } else {
      s += "0";
    }
  }
  return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file
// and set the game state to the last saved state
//
void TicTacToe::setStateString(const std::string &s) {
  for (int i = 0; i < 9 && i < s.length(); i++) {
    int y = i / 3;
    int x = i % 3;
    int playerNum = s[i] - '0';

    _grid[y][x].destroyBit();

    if (playerNum > 0) {
      Bit *bit = PieceForPlayer(playerNum - 1);
      bit->setPosition(_grid[y][x].getPosition());
      _grid[y][x].setBit(bit);
    }
  }
}

//
// this is the function that will be called by the AI
//
void TicTacToe::updateAI() {
  // Simple random AI: pick a random empty square
  if (checkForWinner() || checkForDraw()) {
    return;
  }

  // Collect all empty squares
  std::vector<int> emptySquares;
  for (int i = 0; i < 9; i++) {
    if (ownerAt(i) == nullptr) {
      emptySquares.push_back(i);
    }
  }

  // If there are empty squares, pick one randomly
  if (!emptySquares.empty()) {
    static bool seeded = false;
    if (!seeded) {
      srand(static_cast<unsigned int>(time(nullptr)));
      seeded = true;
    }

    int randomIndex = rand() % emptySquares.size();
    int chosenSquare = emptySquares[randomIndex];

    int y = chosenSquare / 3;
    int x = chosenSquare % 3;

    // Place the AI's piece directly (don't use actionForEmptyHolder as it
    // blocks AI input)
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());
    if (bit) {
      bit->setPosition(_grid[y][x].getPosition());
      _grid[y][x].setBit(bit);
      endTurn();
    }
  }
}
