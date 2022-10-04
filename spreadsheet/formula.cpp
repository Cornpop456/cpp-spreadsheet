#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try : ast_{ParseFormulaAST(std::move(expression))} {
    } catch (std::exception& error) {
        throw FormulaException("Wrong expression");
    }
        
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        } catch (FormulaError& err) {
            return err;
        } 
    }

    std::string GetExpression() const override  {
        std::stringstream res;
        ast_.PrintFormula(res);
        return res.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        auto poses = ast_.GetCells();

        std::set<Position> uniq{poses.begin(), poses.end()};

        return std::vector<Position>{uniq.begin(), uniq.end()};
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}