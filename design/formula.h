class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression);
        
    Value Evaluate(const SheetInterface& sheet) const override;

    std::string GetExpression() const override;
    
    std::vector<Position> GetReferencedCells() const override;

private:
    FormulaAST ast_;
};