#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
        Cell();

        void Set(const std::string& text, const SheetInterface& sheet);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool IsReferenced() const;
        bool IsEmpty() const;

private:
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
  virtual bool IsReferenced() const = 0;
  virtual bool IsEmpty() const = 0;
  virtual ~Impl() = default;
};
