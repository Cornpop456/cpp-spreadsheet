#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

class Cell::EmptyImpl : public Cell::Impl {
public:
    Cell::Value GetValue() const override {
        return "";
    }
    
    std::string GetText() const override {
        return "";
    }

    bool IsReferenced() const override {
        return false;
    }

    bool IsEmpty() const override {
        return true;
    }

    std::vector<Position> GetReferencedCells() const override {
        return std::vector<Position>{};
    }
};

class Cell::TextImpl : public Cell::Impl {
public:
    TextImpl(std::string text) : text_(std::move(text)) {
    }
    
    Cell::Value GetValue() const override {
        if (text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);        
        }
        return text_;
    }
    
    std::string GetText() const override {
        return text_;
    }

    
    bool IsReferenced() const override {
        return false;
    }

    bool IsEmpty() const override {
        return false;
    }

    std::vector<Position> GetReferencedCells() const override {
        return std::vector<Position>{};
    }
private:
    std::string text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    FormulaImpl(std::string f_text, const SheetInterface& sheet) 
        : value_(ParseFormula(f_text)), sheet_(sheet) {
    }

    Cell::Value GetValue() const override {
        auto res = value_->Evaluate(sheet_);
        
        if (std::holds_alternative<double>(res)) {
            return std::get<double>(res);
        } else {
            return std::get<FormulaError>(res);
        }
    }
    
    std::string GetText() const override {
        return "=" + value_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() const override {
        return value_->GetReferencedCells();
    }

    bool IsReferenced() const override {
        return value_->GetReferencedCells().size() > 0;
    }

    bool IsEmpty() const override {
        return false;
    }
private:
    std::unique_ptr<FormulaInterface> value_;
    const SheetInterface& sheet_;
};

// Реализуйте следующие методы
Cell::Cell() 
    : impl_(std::make_unique<EmptyImpl>()) {
}

void Cell::Set(const std::string& text, const SheetInterface& sheet) {
    std::unique_ptr<Impl> tmp;
    
    if ((text.size() == 1 && text[0] == FORMULA_SIGN) 
        || (text.size() > 0 && text[0] != FORMULA_SIGN)) {
        tmp = std::make_unique<TextImpl>(text);
    } else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        tmp = std::make_unique<FormulaImpl>(text.substr(1), sheet);
    } else {
        tmp = std::make_unique<EmptyImpl>();
    }
    
    impl_.swap(tmp);
}

void Cell::Clear() {
    impl_.reset();
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const  {
    return impl_->GetValue();
}

std::string Cell::GetText() const  {
    return impl_->GetText();
}

bool Cell::IsReferenced() const {
    return impl_->IsReferenced();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsEmpty() const {
    return impl_->IsEmpty();
}