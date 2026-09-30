#include "vajra/sparse_matrix.hpp"

#include <algorithm>
#include <numeric>

namespace vajra {

SparseMatrix::SparseMatrix(Int num_rows, Int num_cols)
    : num_rows_(num_rows), num_cols_(num_cols),
      col_ptr_(static_cast<std::size_t>(std::max<Int>(0, num_cols) + 1), 0) {}

SparseMatrix::SparseMatrix(Int num_rows, Int num_cols, std::vector<Int> col_ptr,
                           std::vector<Int> row_indices, std::vector<Real> values)
    : num_rows_(num_rows), num_cols_(num_cols), col_ptr_(std::move(col_ptr)),
      row_indices_(std::move(row_indices)), values_(std::move(values)) {}

void SparseMatrix::clear() noexcept {
    num_rows_ = 0;
    num_cols_ = 0;
    col_ptr_ = {0};
    row_indices_.clear();
    values_.clear();
}

void SparseMatrix::resize(Int num_rows, Int num_cols) {
    if (num_rows_ == num_rows && num_cols_ == num_cols) {
        return;
    }
    num_rows_ = num_rows;
    num_cols_ = num_cols;
    col_ptr_.assign(static_cast<std::size_t>(std::max<Int>(0, num_cols) + 1), 0);
    row_indices_.clear();
    values_.clear();
}

SparseMatrix SparseMatrix::from_triplets(Int num_rows, Int num_cols,
                                         const std::vector<Triplet>& triplets) {
    SparseMatrix matrix(num_rows, num_cols);
    if (num_rows <= 0 || num_cols <= 0 || triplets.empty()) {
        return matrix;
    }

    // Filter and sort triplets by column, then row
    std::vector<Triplet> sorted_triplets;
    sorted_triplets.reserve(triplets.size());
    for (const auto& t : triplets) {
        if (t.row >= 0 && t.row < num_rows && t.col >= 0 && t.col < num_cols) {
            sorted_triplets.push_back(t);
        }
    }

    std::sort(sorted_triplets.begin(), sorted_triplets.end(),
              [](const Triplet& a, const Triplet& b) noexcept {
                  if (a.col != b.col) {
                      return a.col < b.col;
                  }
                  return a.row < b.row;
              });

    // Sum duplicate (row, col) entries
    std::vector<Int> row_indices;
    std::vector<Real> values;
    row_indices.reserve(sorted_triplets.size());
    values.reserve(sorted_triplets.size());

    std::vector<Int> col_counts(static_cast<std::size_t>(num_cols), 0);

    for (const auto& t : sorted_triplets) {
        if (!row_indices.empty() && !values.empty() && t.col < num_cols &&
            col_counts[static_cast<std::size_t>(t.col)] > 0 && row_indices.back() == t.row) {
            // Duplicate entry for same row and col: sum value
            values.back() += t.value;
        } else {
            row_indices.push_back(t.row);
            values.push_back(t.value);
            col_counts[static_cast<std::size_t>(t.col)]++;
        }
    }

    // Build col_ptr prefix sums
    std::vector<Int> col_ptr(static_cast<std::size_t>(num_cols + 1), 0);
    for (std::size_t c = 0; c < static_cast<std::size_t>(num_cols); ++c) {
        col_ptr[c + 1] = col_ptr[c] + col_counts[c];
    }

    matrix.col_ptr() = std::move(col_ptr);
    matrix.row_indices() = std::move(row_indices);
    matrix.values() = std::move(values);

    return matrix;
}

} // namespace vajra
