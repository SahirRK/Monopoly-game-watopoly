#ifndef __TEXTDISPLAY_H_
#define __TEXTDISPLAY_H_

#include "observer.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>

class Board;
class Player;
class TextDisplay: public Observer {
private:
    std::string fname; // board text file name
    Board& board; // refrence to game board
    std::vector<std::shared_ptr<Player>> players;
    std::vector<std::vector<char>> display; // 2d array for textDisplay

public:
    TextDisplay(Board& board, std::vector<std::shared_ptr<Player>> players, std::string fname);
    // Notify for when a player moves
    void notify(std::shared_ptr<Subject> s) override;
    // Prints the board
    void printBoard() const;
};

#endif // TEXTDISPLAY_H
