#include "tic_tac_toe_logic.hpp"

#include <vector>

std::string initBoard() {
    return "_________";  // 9 символов '_'
}

bool canMakeMove(const std::string& board, int cellIndex) {
    if (cellIndex < 0 || cellIndex >= 9) return false;
    return board[cellIndex] == '_';
}

std::string makeMove(const std::string& board, int cellIndex, char player) {
    if (cellIndex < 0 || cellIndex >= 9) return board; 
    if (player != 'X' && player != 'O') return board;

    std::string newBoard = board;
    newBoard[cellIndex] = player;
    return newBoard;
}

char checkWin(const std::string& board) {
    // Проверим все возможные линии выигрыша
    static std::vector<std::vector<int>> wins = {
        {0,1,2}, {3,4,5}, {6,7,8}, // строки
        {0,3,6}, {1,4,7}, {2,5,8}, // столбцы
        {0,4,8}, {2,4,6}           // диагонали
    };

    for (auto& line : wins) {
        char a = board[line[0]];
        char b = board[line[1]];
        char c = board[line[2]];
        if (a != '_' && a == b && b == c) {
            return a; // 'X' или 'O'
        }
    }

    // Проверяем, есть ли ещё пустые клетки
    for (char cell : board) {
        if (cell == '_') {
            return '_'; // Игра не закончена
        }
    }

    return 'D'; // Ничья (Draw)
}
