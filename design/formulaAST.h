class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(const SheetInterface& sheet) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::forward_list<Position> GetCells() const;

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::forward_list<Position> cells_;
};


class CellExpr final : public Expr {
public:
    // тут я не знаю, как будет приходить, как я понял, это будет в заготовке, как строка или как структура позиции, предварительно поставил строкой
    explicit CellExpr(std::string  cell_pos); 
    void Print(std::ostream& out) const override;

    void DoPrintFormula(std::ostream& out, ExprPrecedence /* precedence */) const override;

    ExprPrecedence GetPrecedence() const override;

    double Evaluate(const SheetInterface& sheet) const override;

private:
    std::string cell_pos_;
};