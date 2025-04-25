#ifndef TIC_TAC_TOE_LOGIC_HPP
#define TIC_TAC_TOE_LOGIC_HPP

#include <string>

/**
 * Представляем поле 3x3 в виде строки из 9 символов.
 * Каждый символ может быть 'X', 'O' или '_'.
 * Например: "X__O_____"
 * Индексы ячеек (0-based): 
 *
 *   0 | 1 | 2
 *  ---+---+---
 *   3 | 4 | 5
 *  ---+---+---
 *   6 | 7 | 8
 */

/**
 * Инициализация пустого поля: 9 символов '_'.
 */
std::string initBoard();

/**
 * Проверка, может ли игрок (X или O) сделать ход в ячейку [cellIndex].
 * Возвращает true, если ячейка пуста.
 */
bool canMakeMove(const std::string& board, int cellIndex);

/**
 * Совершить ход (player='X' или 'O') в указанную ячейку (0..8).
 * Возвращает обновлённое поле.
 */
std::string makeMove(const std::string& board, int cellIndex, char player);

/**
 * Проверка состояния: 
 *  - Возвращает 'X', если выиграли X
 *  - Возвращает 'O', если выиграли O
 *  - Возвращает 'D', если ничья (все ячейки заняты, а победы нет)
 *  - Возвращает '_' (нижнее подчёркивание), если игра не закончена
 */
char checkWin(const std::string& board);

#endif
