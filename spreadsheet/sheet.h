#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>

class Sheet : public SheetInterface {
public:
    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void UpdateCache(Position pos, double value) const;
    std::optional<double> GetFromCache(Position pos) const;
    // обнуляет кэш позиции и всех позиций, которы прямо или косвенно зависят от неё
    void InvalidateCache(Position pos) const;

    bool IsCellEmpty(Position pos) const;
private:
    struct PositionHasher {
        size_t operator()(const Position& pos) const {
          return pos.row + pos.col * 37;
        }  
    };

    struct FormulaInfo {
        // те клетки, от которых зависит данная клетка (нужны для отслеживания циклов в графе зависимостей)
        std::vector<Position> dependencies;
        // те клетки, которые зависят от данной клетки (нужны для инвалидации кэша)
        std::unordered_set<Position, PositionHasher> dependent_сells;
        std::optional<double> cache;
    };

    bool HasCycleInGraph(Position start, 
        std::unordered_set<Position, PositionHasher>& path,
        std::unordered_set<Position, PositionHasher>& processed_cells) const;

    // Структура вектора векторов выбразана из подсказки от авторов тренажёра в уроке (9 таблица)
    std::vector<std::vector<Cell>> sheet_;
    Size printable_size_;
    mutable std::unordered_map<Position, FormulaInfo, PositionHasher> formula_graph_and_cache_;
};