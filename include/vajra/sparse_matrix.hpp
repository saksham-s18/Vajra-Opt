#pragma once

#include "vajra/types.hpp"

#include <string>
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
///
/// Storage invariants:
/// - col_ptr has size num_cols + 1, with col_ptr[0] == 0.
/// - col_ptr[j] <= col_ptr[j + 1] for all j in [0, num_cols).
/// - Column j entries are stored in [col_ptr[j], col_ptr[j + 1]).
/// - Within each column, row indices are strictly increasing (no duplicates).
/// - row_indices and values have size col_ptr.back() == num_nonzeros().
class SparseMatrix {
  public:
    /// Default constructor creates a 0x0 empty matrix.
    SparseMatrix() = default;

    /// Construct an empty m x n matrix with no non-zero entries.
    SparseMatrix(Int num_rows, Int num_cols);

    /// Construct directly from raw CSC arrays.
    SparseMatrix(Int num_rows, Int num_cols, std::vector<Int> col_ptr, std::vector<Int> row_indices,
                 std::vector<Real> values);

    /// Construct a canonical CSC matrix from a list of coordinate triplets.
    /// - Triplet entries for the same (row, col) have their values summed.
    /// - Entries within each column are sorted by row index in strictly increasing order.
    /// - Throws std::invalid_argument if dimensions are negative or triplet indices are out of
    /// bounds.
    [[nodiscard]] static SparseMatrix from_triplets(Int num_rows, Int num_cols,
                                                    const std::vector<Triplet>& triplets);

    // Dimension queries
    [[nodiscard]] Int rows() const noexcept { return num_rows_; }
    [[nodiscard]] Int cols() const noexcept { return num_cols_; }
    [[nodiscard]] Int nonzeros() const noexcept { return static_cast<Int>(values_.size()); }

    // Compatibility aliases for Phase 0 API
    [[nodiscard]] Int num_rows() const noexcept { return num_rows_; }
    [[nodiscard]] Int num_cols() const noexcept { return num_cols_; }
    [[nodiscard]] Int num_nonzeros() const noexcept { return nonzeros(); }
    [[nodiscard]] bool empty() const noexcept { return num_rows_ == 0 || num_cols_ == 0; }

    // Read-only access to canonical CSC arrays
    [[nodiscard]] const std::vector<Int>& col_ptr() const noexcept { return col_ptr_; }
    [[nodiscard]] const std::vector<Int>& row_indices() const noexcept { return row_indices_; }
    [[nodiscard]] const std::vector<Real>& values() const noexcept { return values_; }

    /// Element value lookup by (row, col).
    /// Uses binary search within the column: O(log(nnz_in_col)).
    /// Returns 0.0 if not present or if indices are out of bounds.
    [[nodiscard]] Real get(Int row, Int col) const noexcept;

    /// Verify structural and numerical validity of the CSC matrix.
    /// Validates col_ptr monotonicity, index ranges, strictly increasing row indices,
    /// and absence of NaN / Inf values.
    [[nodiscard]] bool is_valid() const noexcept;
    [[nodiscard]] bool is_valid(std::string* error_message) const;

    /// Matrix-vector multiplication: y = A * x
    /// Allocates and returns a vector y of size num_rows.
    /// Throws std::invalid_argument if x.size() != num_cols.
    [[nodiscard]] std::vector<Real> multiply(const std::vector<Real>& x) const;

    /// Matrix-vector multiplication into existing vector: y = A * x
    /// Resizes y to num_rows if necessary and zeroes it before accumulating.
    /// Throws std::invalid_argument if x.size() != num_cols.
    void multiply(const std::vector<Real>& x, std::vector<Real>& y) const;

    /// Matrix-vector multiply and accumulate: y += A * x
    /// Throws std::invalid_argument if x.size() != num_cols or y.size() != num_rows.
    void multiply_add(const std::vector<Real>& x, std::vector<Real>& y) const;

    /// Transpose matrix-vector multiplication: y = A^T * x
    /// Allocates and returns a vector y of size num_cols.
    /// Throws std::invalid_argument if x.size() != num_rows.
    [[nodiscard]] std::vector<Real> multiply_transpose(const std::vector<Real>& x) const;

    /// Transpose matrix-vector multiplication into existing vector: y = A^T * x
    /// Resizes y to num_cols if necessary and zeroes it before accumulating.
    /// Throws std::invalid_argument if x.size() != num_rows.
    void multiply_transpose(const std::vector<Real>& x, std::vector<Real>& y) const;

    /// Transpose matrix-vector multiply and accumulate: y += A^T * x
    /// Throws std::invalid_argument if x.size() != num_rows or y.size() != num_cols.
    void multiply_transpose_add(const std::vector<Real>& x, std::vector<Real>& y) const;

    /// Construct A^T as a new canonical CSC matrix in O(m + n + nnz) time.
    /// Uses a two-pass counting sort transpose that guarantees strictly increasing row indices.
    [[nodiscard]] SparseMatrix transpose() const;

    /// Compare two matrices for structural equality and numerical value equivalence within
    /// tolerance.
    [[nodiscard]] bool equals(const SparseMatrix& other, Real tolerance = 1e-12) const noexcept;

    /// Reset matrix to 0x0 empty state.
    void clear() noexcept;

    /// Resize matrix dimensions (clears all non-zero entries).
    void resize(Int num_rows, Int num_cols);

  private:
    Int num_rows_{0};
    Int num_cols_{0};
    std::vector<Int> col_ptr_{0};
    std::vector<Int> row_indices_{};
    std::vector<Real> values_{};
};

} // namespace vajra
