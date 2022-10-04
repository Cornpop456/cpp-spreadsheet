#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

bool Sheet::IsCellEmpty(Position pos) const {
    if (sheet_.size() <= static_cast<size_t>(pos.row) 
        || sheet_[pos.row].size() <= static_cast<size_t>(pos.col)) {
        
        return true;
    }
    return sheet_[pos.row][pos.col].IsEmpty();
}

std::optional<double> Sheet::GetFromCache(Position pos) const {
    return formula_graph_and_cache_[pos].cache;
}

void Sheet::UpdateCache(Position pos, double value) const {
    formula_graph_and_cache_[pos].cache = value;
}

void Sheet::InvalidateCache(Position pos) const {
    formula_graph_and_cache_[pos].cache = std::nullopt;

    for (auto p : formula_graph_and_cache_[pos].dependent_сells) {
        if (formula_graph_and_cache_[p].cache != std::nullopt) {
            InvalidateCache(p);
        }
    }
}

bool Sheet::HasCycleInGraph(Position start, 
    std::unordered_set<Position, PositionHasher>& path,
    std::unordered_set<Position, PositionHasher>& processed_cells) const {

    path.insert(start);

    for (auto p : formula_graph_and_cache_[start].dependencies) {
        if (path.count(p) != 0) {
            return true;
        }

        if (processed_cells.count(p) == 0) {
            bool cycle = HasCycleInGraph(p, path, processed_cells);

            if (cycle) {
                return true;
            }
        }
    }

    processed_cells.insert(start);
    path.erase(start);

    return false;
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException{"Invalid position"};
    }

    bool cell_behind_window = false;

    if (static_cast<size_t>(pos.row) >= sheet_.size()) {
        sheet_.resize(pos.row + 1);
        cell_behind_window = true;

        if (sheet_.size() > Position::MAX_ROWS) {
            throw InvalidPositionException{"Max row number exceeded"};
        }

        printable_size_.rows = sheet_.size();
    }

    if (static_cast<size_t>(pos.col) >= sheet_[pos.row].size()) {
        sheet_[pos.row].resize(pos.col + 1);
        cell_behind_window = true;

        if (sheet_.size() > Position::MAX_COLS) {
            throw InvalidPositionException{"Max cols number exceeded"};
        }

        if (sheet_[pos.row].size() > static_cast<size_t>(printable_size_.cols)) {
            printable_size_.cols = sheet_[pos.row].size();
        }
    }

    auto old_cell_text = sheet_[pos.row][pos.col].GetText();

    sheet_[pos.row][pos.col].Set(text, *this);
    formula_graph_and_cache_[pos].dependencies = sheet_[pos.row][pos.col].GetReferencedCells();

    std::unordered_set<Position, PositionHasher> path, processed_cells;

    bool cycle = HasCycleInGraph(pos, path, processed_cells);

    if (cycle) {
        if (cell_behind_window) {
            ClearCell(pos);
            formula_graph_and_cache_[pos].dependencies.clear();
        } else {
            sheet_[pos.row][pos.col].Set(old_cell_text, *this);
            formula_graph_and_cache_[pos].dependencies = sheet_[pos.row][pos.col].GetReferencedCells();
        }

        throw CircularDependencyException{"Formula makes cycle"};
    }

    for (auto p : formula_graph_and_cache_[pos].dependencies) {
        formula_graph_and_cache_[p].dependent_сells.insert(pos);
    }

    InvalidateCache(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException{"Invalid position"};
    }

    if (pos.row >= printable_size_.rows || pos.col >= printable_size_.cols) {
        return nullptr;
    }

    return &sheet_[pos.row][pos.col];
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException{"Invalid position"};
    }

    if (pos.row >= printable_size_.rows || pos.col >= printable_size_.cols) {
        return nullptr;
    }

    if (static_cast<size_t>(pos.col) >= sheet_[pos.row].size()) {
        sheet_[pos.row].resize(printable_size_.cols);
    }

    return &sheet_[pos.row][pos.col];
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException{"Invalid position"};
    }

    if (pos.row >= printable_size_.rows 
        || pos.col >= printable_size_.cols 
        || sheet_[pos.row ].size() <= static_cast<size_t>(pos.col)) {
        
        return;
    }

    sheet_[pos.row][pos.col].Set("", *this);

    if (pos.row == printable_size_.rows - 1) {
        for (size_t i = 0; i < sheet_.size(); ++i) {
            int els = 0;
            
            for (size_t j = 0; j < sheet_[pos.row - i].size(); ++j) {
                if (sheet_[pos.row - i][j].GetText() != "") {
                    ++els;
                }
            }

            if (els == 0) {
                sheet_.pop_back();
                printable_size_.rows -= 1;
            } else {
                break;
            }

        }
    }

    if (pos.col == printable_size_.cols - 1) {
        for (int j = 0; j < printable_size_.cols; ++j) {
            int els = 0;
            
            for (size_t i = 0; i < sheet_.size(); ++i) {
                if (sheet_[i].size() > static_cast<size_t>(pos.col - j) && sheet_[i][pos.col - j].GetText() != "") {
                    ++els;
                }
            }

            if (els == 0) {
                printable_size_.cols -= 1;
            } else {
                break;
            }
        }

        for (size_t i = 0; i < sheet_.size(); ++i) {
            if (sheet_[i].size() > static_cast<size_t>(printable_size_.cols)) {
                sheet_[i].resize(printable_size_.cols);
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < printable_size_.rows; ++i) {
        for (int j = 0; j < printable_size_.cols; ++j) {
            if (sheet_[i].size() <= static_cast<size_t>(j)) {
                output << "";
            } else {
                auto value = sheet_[i][j].GetValue();

                if (std::holds_alternative<double>(value)) {
                    output << std::get<double>(value);
                } else if (std::holds_alternative<FormulaError>(value)) {
                    output << std::get<FormulaError>(value);
                } else {
                    output << std::get<std::string>(value);
                }
            }

            if (j < printable_size_.cols - 1) {
                output << '\t';
            }
        }

        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < printable_size_.rows; ++i) {
        for (int j = 0; j < printable_size_.cols; ++j) {
            if (sheet_[i].size() <= static_cast<size_t>(j)) {
                output << "";
            } else {
                output << sheet_[i][j].GetText();
            }

            if (j < printable_size_.cols - 1) {
                output << '\t';
            }
        }

        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}