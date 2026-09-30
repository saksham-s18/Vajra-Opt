#include "vajra/sparse_matrix.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace vajra {

SparseMatrix::SparseMatrix(Int num_rows, Int num_cols)
    : num_rows_(std::max<Int>(0, num_rows)), num_cols_(std::max<Int>(0, num_cols)),
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
    num_rows_ = std::max<Int>(0, num_rows);
    num_cols_ = std::max<Int>(0, num_cols);
    col_ptr_.assign(static_cast<std::size_t>(num_cols_ + 1), 0);
    row_indices_.clear();
    values_.clear();
}

SparseMatrix SparseMatrix::from_triplets(Int num_rows, Int num_cols,
                                         const std::vector<Triplet>& triplets) {
    if (num_rows < 0 || num_cols < 0) {
        throw std::invalid_argument(
            "Matrix dimensions cannot be negative: rows=" + std::to_string(num_rows) +
            ", cols=" + std::to_string(num_cols));
    }

    if (num_rows == 0 || num_cols == 0 || triplets.empty()) {
        return SparseMatrix(num_rows, num_cols);
    }

    // Validate triplet bounds and finite values
    for (std::size_t k = 0; k < triplets.size(); ++k) {
        const auto& t = triplets[k];
        if (t.row < 0 || t.row >= num_rows || t.col < 0 || t.col >= num_cols) {
            throw std::invalid_argument("Triplet at index " + std::to_string(k) +
                                        " out of bounds: (" + std::to_string(t.row) + ", " +
                                        std::to_string(t.col) + ") for matrix " +
                                        std::to_string(num_rows) + "x" + std::to_string(num_cols));
        }
        if (std::isnan(t.value)) {
            throw std::invalid_argument("Triplet at index " + std::to_string(k) +
                                        " contains NaN value");
        }
        if (std::isinf(t.value)) {
            throw std::invalid_argument("Triplet at index " + std::to_string(k) +
                                        " contains infinite value");
        }
    }

    // Sort triplets by column index, then row index
    std::vector<Triplet> sorted = triplets;
    std::sort(sorted.begin(), sorted.end(), [](const Triplet& a, const Triplet& b) noexcept {
        if (a.col != b.col) {
            return a.col < b.col;
        }
        return a.row < b.row;
    });

    // Sum duplicates with identical (col, row)
    std::vector<Int> row_indices;
    std::vector<Real> values;
    row_indices.reserve(sorted.size());
    values.reserve(sorted.size());

    std::vector<Int> col_counts(static_cast<std::size_t>(num_cols), 0);

    Int prev_col = -1;
    Int prev_row = -1;

    for (const auto& t : sorted) {
        if (t.col == prev_col && t.row == prev_row) {
            // Duplicate entry in column: sum value
            values.back() += t.value;
        } else {
            row_indices.push_back(t.row);
            values.push_back(t.value);
            col_counts[static_cast<std::size_t>(t.col)]++;
            prev_col = t.col;
            prev_row = t.row;
        }
    }

    // Build column pointers via prefix sums
    std::vector<Int> col_ptr(static_cast<std::size_t>(num_cols + 1), 0);
    for (std::size_t c = 0; c < static_cast<std::size_t>(num_cols); ++c) {
        col_ptr[c + 1] = col_ptr[c] + col_counts[c];
    }

    return SparseMatrix(num_rows, num_cols, std::move(col_ptr), std::move(row_indices),
                        std::move(values));
}

Real SparseMatrix::get(Int row, Int col) const noexcept {
    if (row < 0 || row >= num_rows_ || col < 0 || col >= num_cols_) {
        return 0.0;
    }
    const Int start = col_ptr_[static_cast<std::size_t>(col)];
    const Int end = col_ptr_[static_cast<std::size_t>(col + 1)];
    const auto it = std::lower_bound(row_indices_.begin() + start, row_indices_.begin() + end, row);
    if (it != row_indices_.begin() + end && *it == row) {
        const auto offset = std::distance(row_indices_.begin(), it);
        return values_[static_cast<std::size_t>(offset)];
    }
    return 0.0;
}

bool SparseMatrix::is_valid() const noexcept {
    return is_valid(nullptr);
}

bool SparseMatrix::is_valid(std::string* error_message) const {
    if (num_rows_ < 0 || num_cols_ < 0) {
        if (error_message) {
            *error_message = "Matrix dimensions are negative: rows=" + std::to_string(num_rows_) +
                             ", cols=" + std::to_string(num_cols_);
        }
        return false;
    }

    if (col_ptr_.size() != static_cast<std::size_t>(num_cols_ + 1)) {
        if (error_message) {
            *error_message = "col_ptr size (" + std::to_string(col_ptr_.size()) +
                             ") does not match num_cols + 1 (" + std::to_string(num_cols_ + 1) +
                             ")";
        }
        return false;
    }

    if (col_ptr_[0] != 0) {
        if (error_message) {
            *error_message = "col_ptr[0] must be 0, got " + std::to_string(col_ptr_[0]);
        }
        return false;
    }

    if (col_ptr_.back() != static_cast<Int>(values_.size())) {
        if (error_message) {
            *error_message = "col_ptr.back() (" + std::to_string(col_ptr_.back()) +
                             ") does not match values count (" + std::to_string(values_.size()) +
                             ")";
        }
        return false;
    }

    if (row_indices_.size() != values_.size()) {
        if (error_message) {
            *error_message = "row_indices count (" + std::to_string(row_indices_.size()) +
                             ") does not match values count (" + std::to_string(values_.size()) +
                             ")";
        }
        return false;
    }

    for (Int c = 0; c < num_cols_; ++c) {
        const Int start = col_ptr_[static_cast<std::size_t>(c)];
        const Int end = col_ptr_[static_cast<std::size_t>(c + 1)];

        if (start > end) {
            if (error_message) {
                *error_message = "Non-monotonic col_ptr at column " + std::to_string(c);
            }
            return false;
        }

        Int prev_row = -1;
        for (Int idx = start; idx < end; ++idx) {
            const Int r = row_indices_[static_cast<std::size_t>(idx)];
            const Real val = values_[static_cast<std::size_t>(idx)];

            if (r < 0 || r >= num_rows_) {
                if (error_message) {
                    *error_message = "Column " + std::to_string(c) +
                                     " has out-of-bounds row index " + std::to_string(r);
                }
                return false;
            }

            if (r <= prev_row) {
                if (error_message) {
                    *error_message = "Column " + std::to_string(c) +
                                     " has non-increasing or duplicate row index " +
                                     std::to_string(r);
                }
                return false;
            }
            prev_row = r;

            if (std::isnan(val)) {
                if (error_message) {
                    *error_message = "Matrix entry at (" + std::to_string(r) + ", " +
                                     std::to_string(c) + ") is NaN";
                }
                return false;
            }
            if (std::isinf(val)) {
                if (error_message) {
                    *error_message = "Matrix entry at (" + std::to_string(r) + ", " +
                                     std::to_string(c) + ") is infinite";
                }
                return false;
            }
        }
    }

    return true;
}

std::vector<Real> SparseMatrix::multiply(const std::vector<Real>& x) const {
    if (static_cast<Int>(x.size()) != num_cols_) {
        throw std::invalid_argument("Vector dimension mismatch for A*x: expected size " +
                                    std::to_string(num_cols_) + ", got " +
                                    std::to_string(x.size()));
    }
    std::vector<Real> y(static_cast<std::size_t>(num_rows_), 0.0);
    multiply_add(x, y);
    return y;
}

void SparseMatrix::multiply(const std::vector<Real>& x, std::vector<Real>& y) const {
    if (static_cast<Int>(x.size()) != num_cols_) {
        throw std::invalid_argument("Vector dimension mismatch for A*x: expected size " +
                                    std::to_string(num_cols_) + ", got " +
                                    std::to_string(x.size()));
    }
    y.assign(static_cast<std::size_t>(num_rows_), 0.0);
    multiply_add(x, y);
}

void SparseMatrix::multiply_add(const std::vector<Real>& x, std::vector<Real>& y) const {
    if (static_cast<Int>(x.size()) != num_cols_) {
        throw std::invalid_argument("Vector dimension mismatch for y += A*x: x size " +
                                    std::to_string(x.size()) + " != cols " +
                                    std::to_string(num_cols_));
    }
    if (static_cast<Int>(y.size()) != num_rows_) {
        throw std::invalid_argument("Vector dimension mismatch for y += A*x: y size " +
                                    std::to_string(y.size()) + " != rows " +
                                    std::to_string(num_rows_));
    }
    if (empty() || nonzeros() == 0) {
        return;
    }

    for (Int j = 0; j < num_cols_; ++j) {
        const Real xj = x[static_cast<std::size_t>(j)];
        if (xj == 0.0) {
            continue;
        }
        const Int start = col_ptr_[static_cast<std::size_t>(j)];
        const Int end = col_ptr_[static_cast<std::size_t>(j + 1)];
        for (Int idx = start; idx < end; ++idx) {
            const Int r = row_indices_[static_cast<std::size_t>(idx)];
            y[static_cast<std::size_t>(r)] += values_[static_cast<std::size_t>(idx)] * xj;
        }
    }
}

std::vector<Real> SparseMatrix::multiply_transpose(const std::vector<Real>& x) const {
    if (static_cast<Int>(x.size()) != num_rows_) {
        throw std::invalid_argument("Vector dimension mismatch for A^T*x: expected size " +
                                    std::to_string(num_rows_) + ", got " +
                                    std::to_string(x.size()));
    }
    std::vector<Real> y(static_cast<std::size_t>(num_cols_), 0.0);
    multiply_transpose_add(x, y);
    return y;
}

void SparseMatrix::multiply_transpose(const std::vector<Real>& x, std::vector<Real>& y) const {
    if (static_cast<Int>(x.size()) != num_rows_) {
        throw std::invalid_argument("Vector dimension mismatch for A^T*x: expected size " +
                                    std::to_string(num_rows_) + ", got " +
                                    std::to_string(x.size()));
    }
    y.assign(static_cast<std::size_t>(num_cols_), 0.0);
    multiply_transpose_add(x, y);
}

void SparseMatrix::multiply_transpose_add(const std::vector<Real>& x, std::vector<Real>& y) const {
    if (static_cast<Int>(x.size()) != num_rows_) {
        throw std::invalid_argument("Vector dimension mismatch for y += A^T*x: x size " +
                                    std::to_string(x.size()) + " != rows " +
                                    std::to_string(num_rows_));
    }
    if (static_cast<Int>(y.size()) != num_cols_) {
        throw std::invalid_argument("Vector dimension mismatch for y += A^T*x: y size " +
                                    std::to_string(y.size()) + " != cols " +
                                    std::to_string(num_cols_));
    }
    if (empty() || nonzeros() == 0) {
        return;
    }

    for (Int j = 0; j < num_cols_; ++j) {
        const Int start = col_ptr_[static_cast<std::size_t>(j)];
        const Int end = col_ptr_[static_cast<std::size_t>(j + 1)];
        Real dot = 0.0;
        for (Int idx = start; idx < end; ++idx) {
            const Int r = row_indices_[static_cast<std::size_t>(idx)];
            dot += values_[static_cast<std::size_t>(idx)] * x[static_cast<std::size_t>(r)];
        }
        y[static_cast<std::size_t>(j)] += dot;
    }
}

SparseMatrix SparseMatrix::transpose() const {
    if (empty() || nonzeros() == 0) {
        return SparseMatrix(num_cols_, num_rows_);
    }

    // Pass 1: Count row occurrences in A (which become column counts in A^T)
    std::vector<Int> row_counts(static_cast<std::size_t>(num_rows_), 0);
    for (const auto r : row_indices_) {
        row_counts[static_cast<std::size_t>(r)]++;
    }

    // Pass 2: Prefix sums for A^T col_ptr (size num_rows + 1)
    std::vector<Int> at_col_ptr(static_cast<std::size_t>(num_rows_ + 1), 0);
    for (std::size_t r = 0; r < static_cast<std::size_t>(num_rows_); ++r) {
        at_col_ptr[r + 1] = at_col_ptr[r] + row_counts[r];
    }

    std::vector<Int> next_pos = at_col_ptr;
    const std::size_t nnz = values_.size();
    std::vector<Int> at_row_indices(nnz);
    std::vector<Real> at_values(nnz);

    // Pass 3: Distribute entries. Outer loop traverses columns j = 0..n-1 in increasing order,
    // which guarantees that in A^T each column's row indices are placed in strictly increasing
    // order.
    for (Int j = 0; j < num_cols_; ++j) {
        const Int start = col_ptr_[static_cast<std::size_t>(j)];
        const Int end = col_ptr_[static_cast<std::size_t>(j + 1)];
        for (Int idx = start; idx < end; ++idx) {
            const Int r = row_indices_[static_cast<std::size_t>(idx)];
            const std::size_t dest =
                static_cast<std::size_t>(next_pos[static_cast<std::size_t>(r)]++);
            at_row_indices[dest] = j;
            at_values[dest] = values_[static_cast<std::size_t>(idx)];
        }
    }

    return SparseMatrix(num_cols_, num_rows_, std::move(at_col_ptr), std::move(at_row_indices),
                        std::move(at_values));
}

bool SparseMatrix::equals(const SparseMatrix& other, Real tolerance) const noexcept {
    if (num_rows_ != other.num_rows_ || num_cols_ != other.num_cols_) {
        return false;
    }
    if (nonzeros() != other.nonzeros()) {
        return false;
    }
    if (col_ptr_ != other.col_ptr_) {
        return false;
    }
    if (row_indices_ != other.row_indices_) {
        return false;
    }
    for (std::size_t i = 0; i < values_.size(); ++i) {
        if (std::abs(values_[i] - other.values_[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

} // namespace vajra
