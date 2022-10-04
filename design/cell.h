class Cell : public CellInterface {
public:
    Cell();

    // Дополнительно устанавливает ссылку на таблицу, для того, чтобы вычислять ячейки, на которые возможно будет ссылаться ячейка
    void Set(const std::string& text, const SheetInterface& sheet);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    
    std::unique_ptr<Impl> impl_;
};

class Cell::Impl {
public:
  virtual Cell::Value GetValue() const = 0;
  virtual std::string GetText() const  = 0;
  virtual std::vector<Position> GetReferencedCells() const = 0;
  virtual ~Impl() = default;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    FormulaImpl(std::string f_text, const SheetInterface& sheet);
    Cell::Value GetValue() const override;
    std::string GetText() const override;
private:
    std::unique_ptr<FormulaInterface> value_;

    // ссылка на таблицу
    const SheetInterface& sheet_;
};


class Cell::EmptyImpl : public Cell::Impl {
public:
    Cell::Value GetValue() const override;
    std::string GetText() const override;
};

class Cell::TextImpl : public Cell::Impl {
public:
    TextImpl(std::string text);
    Cell::Value GetValue()  const override;
    std::string GetText()  const override;
private:
    std::string text_;
};
