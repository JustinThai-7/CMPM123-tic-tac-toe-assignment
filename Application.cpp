#include "Application.h"
#include "classes/Logger.h"
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
// Helper function to render the Logger window
//
void RenderLoggerWindow() {
  ImGui::Begin("Log Console");

  // Control buttons
  if (ImGui::Button("Clear Logs")) {
    Logger::GetInstance().ClearLogs();
  }
  ImGui::SameLine();
  static bool autoScroll = true;
  ImGui::Checkbox("Auto-scroll", &autoScroll);

  ImGui::Separator();

  // Log entries display
  ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false,
                    ImGuiWindowFlags_HorizontalScrollbar);

  const auto &logs = Logger::GetInstance().GetLogEntries();
  for (const auto &entry : logs) {
    // Color based on log level
    ImVec4 color;
    switch (entry.level) {
    case LogLevel::INFO:
      color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
      break;
    case LogLevel::WARNING:
      color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
      break;
    case LogLevel::ERROR:
      color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // Red
      break;
    case LogLevel::DEBUG:
      color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Gray
      break;
    case LogLevel::GAME_EVENT:
      color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f); // Green
      break;
    default:
      color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(
        ("[" + Logger::GetInstance().GetLogLevelString(entry.level) + "] " +
         entry.timestamp + ": " + entry.message)
            .c_str());
    ImGui::PopStyleColor();
  }

  if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();
  ImGui::End();
}

//
// game starting point
// this is called by the main render loop in main.cpp
//
void GameStartUp() {
  game = new TicTacToe();
  game->setUpBoard();
  Logger::GetInstance().LogInfo("Tic-Tac-Toe game started");
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
    Logger::GetInstance().LogInfo(aiEnabled ? "AI enabled (playing as O)"
                                            : "AI disabled");
  }

  // If AI is enabled and it's the AI's turn (player 1), make a move
  // Only move if we haven't already moved this turn
  unsigned int currentTurn = game->getCurrentTurnNo();
  if (aiEnabled && !gameOver && game->getCurrentPlayer()->playerNumber() == 1 &&
      lastAITurn != currentTurn) {
    lastAITurn = currentTurn;
    game->updateAI();
    Logger::GetInstance().LogGameEvent("AI made a move");
    EndOfTurn();
  }

  // Always-visible Reset Game button
  if (ImGui::Button("Reset Game")) {
    game->stopGame();
    game->setUpBoard();
    gameOver = false;
    gameWinner = -1;
    lastAITurn = 0;
    Logger::GetInstance().LogInfo("Game reset");
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

  // Render the Logger window
  RenderLoggerWindow();
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
    Logger::GetInstance().LogGameEvent("Winner: Player " +
                                       std::to_string(gameWinner) +
                                       (gameWinner == 0 ? " (X)" : " (O)"));
  }
  if (game->checkForDraw()) {
    gameOver = true;
    gameWinner = -1;
    Logger::GetInstance().LogGameEvent("Game ended in a draw");
  }
}
} // namespace ClassGame
