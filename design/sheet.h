class Sheet : public SheetInterface {
public:
    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void UpdateCache(Position pos);
    std::optional<double> GetFromCache(Position pos);
    // обнуляет кэш позиции и всех позиций, которы прямо или косвенно зависят от неё
    void InvalidateCache(Position pos);
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

    // Проверка графа на цикл
    bool HasCycleInGraph(Position start, 
        std::unordered_set<Position, PositionHasher>& path,
        std::unordered_set<Position, PositionHasher>& processed_cells) const;

    std::vector<std::vector<Cell>> sheet_;
    Size printable_size_;

    // Кэш, граф зависимостей и обратный граф зависимостей, объединённые в одной структуре и ассоциированые с клеткой
    std::unordered_map<Position, FormulaInfo, PositionHasher> formula_graph_and_cache_;
};