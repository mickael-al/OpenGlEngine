#ifndef __CHESS_GAME__
#define __CHESS_GAME__
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <string>
#include <unordered_map>

class ChessGame {
public:
    ChessGame(const std::string& gameInfo) {
        parseGameInfo(gameInfo);
        board = initBoard();
        currentMoveIndex = 0;
    }

    void printBoard() const {
        for (const auto& row : board) {
            for (char cell : row) {
                std::cout << cell << ' ';
            }
            std::cout << '\n';
        }
    }

    std::vector<std::vector<char>> getBoard() const {
        return board;
    }

    bool step() 
    {
        if (currentMoveIndex >= moves.size()) 
        {
            return false;
        }

        std::string move = moves[currentMoveIndex++];
        applyMove(move);
        return true;
    }

    std::string getGameID() const { return gameID; }
    bool isRated() const { return rated; }
    std::string getStartTime() const { return startTime; }
    std::string getEndTime() const { return endTime; }
    int getNumberOfTurns() const { return numberOfTurns; }
    std::string getGameStatus() const { return gameStatus; }
    std::string getWinner() const { return winner; }
    std::string getTimeIncrement() const { return timeIncrement; }
    std::string getWhitePlayerID() const { return whitePlayerID; }
    int getWhitePlayerRating() const { return whitePlayerRating; }
    std::string getBlackPlayerID() const { return blackPlayerID; }
    int getBlackPlayerRating() const { return blackPlayerRating; }
    std::string getOpeningEco() const { return openingEco; }
    std::string getOpeningName() const { return openingName; }
    int getOpeningPly() const { return openingPly; }

private:
    std::vector<std::vector<char>> initBoard() const 
    {
        std::vector<std::vector<char>> board(8, std::vector<char>(8, '.'));
      
         board[0] = { 'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r' };
         board[1] = { 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p' };
         board[6] = { 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P' };
         board[7] = { 'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R' };

        return board;
    }

    void parseGameInfo(const std::string& gameInfo) {
        std::stringstream ss(gameInfo);
        std::string field;
        std::vector<std::string> fields;
        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() >= 16) {
            gameID = fields[0];
            rated = (fields[1] == "TRUE");
            startTime = fields[2];
            endTime = fields[3];
            numberOfTurns = std::stoi(fields[4]);
            gameStatus = fields[5];
            winner = fields[6];
            timeIncrement = fields[7];
            whitePlayerID = fields[8];
            whitePlayerRating = std::stoi(fields[9]);
            blackPlayerID = fields[10];
            blackPlayerRating = std::stoi(fields[11]);
            moves = splitMoves(fields[12]);
            openingEco = fields[13];
            openingName = fields[14];
            openingPly = std::stoi(fields[15]);
        }
    }

    std::vector<std::string> splitMoves(const std::string& movesStr) 
    {
        std::vector<std::string> result;
        std::stringstream ss(movesStr);
        std::string s;
        while (getline(ss, s, ' '))
        {
            result.push_back(s);
        }

        return result;
    }

    void applyMove(const std::string& move) {
        // Regular expression to match moves like "e4", "Nf3", "Bb5", etc.
        std::regex movePattern(R"(([PNBRQK]?)([a-h])([1-8]))");
        std::smatch match;

        if (std::regex_match(move, match, movePattern)) {
            char piece = match[1].str().empty() ? 'P' : match[1].str()[0];
            char file = match[2].str()[0];
            int rank = match[3].str()[0] - '0';

            // Convert file and rank to board indices
            int x = file - 'a';
            int y = 8 - rank;

            // Find the piece on the board and move it to the destination
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    if (board[i][j] == piece || (piece == 'P' && board[i][j] == 'P')) {
                        // Move the piece to the new position
                        board[y][x] = board[i][j];
                        board[i][j] = '.';
                        return;
                    }
                }
            }
        }
    }

    std::vector<std::vector<char>> board;
    std::vector<std::string> moves;
    size_t currentMoveIndex;

    std::string gameID;
    bool rated;
    std::string startTime;
    std::string endTime;
    int numberOfTurns;
    std::string gameStatus;
    std::string winner;
    std::string timeIncrement;
    std::string whitePlayerID;
    int whitePlayerRating;
    std::string blackPlayerID;
    int blackPlayerRating;
    std::string openingEco;
    std::string openingName;
    int openingPly;
};

class ChessDataset 
{
public:
    bool loadCSV(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file\n";
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            games.emplace_back(line);
        }

        file.close();
        return true;
    }

    ChessGame& getGame(size_t index) {
        if (index < games.size()) {
            return games[index];
        }
        throw std::out_of_range("Game index out of range");
    }

private:
    std::vector<ChessGame> games;
};

#endif //!__CHESS_GAME__