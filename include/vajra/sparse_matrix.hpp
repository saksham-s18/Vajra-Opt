#pragma once

#include "vajra/types.hpp"

#include <vector>

namespace vajra {

/// Coordinate format triplet entry (row, column, value).
struct Triplet {
    Int row{0};
    Int col{0};
    Real value{0.0};

    [[nodiscard]] constexpr bool operator==(const Triplet& other) const noexcept {
        return row == other.row && col == other.col && value == other.value;
    }
};

/// Compressed Sparse Column (CSC) matrix representation.
/// Used for constraint matrices and numerical linear algebra operations.
class SparseMatrix {
  public:
    /// Default constructor creates a 0x0 empty matrix.
    SparseMatrix() = default;

    /// Construct an empty m x n matrix with no non-zero entries.
    SparseMatrix(Int num_rows, Int num_cols);

    /// Construct directly from CSC arrays.
    SparseMatrix(Int num_rows, Int num_cols, std::vector<Int> col_ptr, std::vector<Int> row_indices,
                 std::vector<Real> values);

    /// Construct a CSC matrix from a list of coordinate triplets.
    /// Duplicate (row, col) entries have their values summed together.
    /// Entries are sorted by column, then by row.
    [[nodiscard]] static SparseMatrix from_triplets(Int num_rows, Int num_cols,
                                                    const std::vector<Triplet>& triplets);

    // Dimension queries
    [[nodiscard]] Int num_rows() const noexcept { return num_rows_; }
    [[nodiscard]] Int num_cols() const noexcept { return num_cols_; }
    [[nodiscard]] Int num_nonzeros() const noexcept { return static_cast<Int>(values_.size()); }
    [[nodiscard]] bool empty() const noexcept { return num_rows_ == 0 || num_cols_ == 0; }

    // Direct access to CSC arrays
    [[nodiscard]] const std::vector<Int>& col_ptr() const noexcept { return col_ptr_; }
    [[nodiscard]] const std::vector<Int>& row_indices() const noexcept { return row_indices_; }
    [[nodiscard]] const std::vector<Real>& values() const noexcept { return values_; }

    // Mutable access for assembly
    [[nodiscard]] std::vector<Int>& col_ptr() noexcept { return col_ptr_; }
    [[nodiscard]] std::vector<Int>& row_indices() noexcept { return row_indices_; }
    [[nodiscard]] std::vector<Real>& values() noexcept { return values_; }

    /// Clear the matrix to 0x0 with no non-zero entries.
    void clear() noexcept;

    /// Resize matrix dimensions (resets entries if dimensions change).
    void resize(Int num_rows, Int num_cols);

  private:
    Int num_rows_{0};
    Int num_cols_{0};
    std::vector<Int> col_ptr_{0};
    std::vector<Int> row_indices_{};
    std::vector<Real> values_{};
};

} // namespace vajra
