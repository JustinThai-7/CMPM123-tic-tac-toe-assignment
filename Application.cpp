#include "Application.h"
#include "classes/TicTacToe.h"
#include "imgui/imgui.h"

namespace ClassGame {
//
// our global variables
//
TicTacToe *game = nullptr;
bool gameOver = false;
int gameWinner = -1;

//
// game starting point
// this is called by the main render loop in main.cpp
//
void GameStartUp() {
  game = new TicTacToe();
  game->setUpBoard();
}

//
// game render loop
// this is called by the main render loop in main.cpp
//
void RenderGame() {
  ImGui::DockSpaceOverViewport();

  // ImGui::ShowDemoWindow();

  if (!game)
    return;
  if (!game->getCurrentPlayer())
    return;

  ImGui::Begin("Settings");
  ImGui::Text("Current Player Number: %d",
              game->getCurrentPlayer()->playerNumber());
  ImGui::Text("Current Board State: %s", game->stateString().c_str());

  // AI toggle checkbox
  static bool aiEnabled = false;
  static unsigned int lastAITurn = 0; // Track when AI last moved
  if (ImGui::Checkbox("Play vs AI", &aiEnabled)) {
    game->_gameOptions.AIPlaying = aiEnabled;
    game->_gameOptions.AIPlayer = 1; // AI plays as O (player 1)
    lastAITurn = 0;                  // Reset when toggling
  }

  // If AI is enabled and it's the AI's turn (player 1), make a move
  // Only move if we haven't already moved this turn
  unsigned int currentTurn = game->getCurrentTurnNo();
  if (aiEnabled && !gameOver && game->getCurrentPlayer()->playerNumber() == 1 &&
      lastAITurn != currentTurn) {
    lastAITurn = currentTurn;
    game->updateAI();
    EndOfTurn();
  }

  // Always-visible Reset Game button
  if (ImGui::Button("Reset Game")) {
    game->stopGame();
    game->setUpBoard();
    gameOver = false;
    gameWinner = -1;
    lastAITurn = 0;
  }

  if (gameOver) {
    ImGui::Text("Game Over!");
    if (gameWinner == -1) {
      ImGui::Text("It's a Draw!");
    } else {
      ImGui::Text("Winner: Player %d (%s)", gameWinner,
                  gameWinner == 0 ? "X" : "O");
    }
  }
  ImGui::End();

  ImGui::Begin("GameWindow");
  game->drawFrame();
  ImGui::End();
}

//
// end turn is called by the game code at the end of each turn
// this is where we check for a winner
//
void EndOfTurn() {
  Player *winner = game->checkForWinner();
  if (winner) {
    gameOver = true;
    gameWinner = winner->playerNumber();
  }
  if (game->checkForDraw()) {
    gameOver = true;
    gameWinner = -1;
  }
}
} // namespace ClassGame
