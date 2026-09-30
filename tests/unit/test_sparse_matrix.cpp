#include "vajra/sparse_matrix.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <random>
#include <vector>

namespace vajra {
namespace {

// ============================================================================
// Reference Dense Matrix implementation for differential testing
// (purely internal to test suite, zero external solver dependencies)
// ============================================================================
class DenseMatrixRef {
  public:
    DenseMatrixRef(Int rows, Int cols)
        : rows_(rows), cols_(cols), data_(static_cast<std::size_t>(rows * cols), 0.0) {}

    void set(Int r, Int c, Real val) { data_[static_cast<std::size_t>(r * cols_ + c)] = val; }

    [[nodiscard]] Real get(Int r, Int c) const {
        return data_[static_cast<std::size_t>(r * cols_ + c)];
    }

    [[nodiscard]] std::vector<Real> multiply(const std::vector<Real>& x) const {
        std::vector<Real> y(static_cast<std::size_t>(rows_), 0.0);
        for (Int r = 0; r < rows_; ++r) {
            Real sum = 0.0;
            for (Int c = 0; c < cols_; ++c) {
                sum += get(r, c) * x[static_cast<std::size_t>(c)];
            }
            y[static_cast<std::size_t>(r)] = sum;
        }
        return y;
    }

    [[nodiscard]] std::vector<Real> multiply_transpose(const std::vector<Real>& x) const {
        std::vector<Real> y(static_cast<std::size_t>(cols_), 0.0);
        for (Int c = 0; c < cols_; ++c) {
            Real sum = 0.0;
            for (Int r = 0; r < rows_; ++r) {
                sum += get(r, c) * x[static_cast<std::size_t>(r)];
            }
            y[static_cast<std::size_t>(c)] = sum;
        }
        return y;
    }

  private:
    Int rows_{0};
    Int cols_{0};
    std::vector<Real> data_;
};

// ============================================================================
// Unit Tests
// ============================================================================

TEST(SparseMatrixTest, EmptyMatrixProperties) {
    const SparseMatrix empty_default;
    EXPECT_EQ(empty_default.rows(), 0);
    EXPECT_EQ(empty_default.cols(), 0);
    EXPECT_EQ(empty_default.nonzeros(), 0);
    EXPECT_TRUE(empty_default.empty());
    EXPECT_TRUE(empty_default.is_valid());

    const SparseMatrix empty_rect(0, 5);
    EXPECT_EQ(empty_rect.rows(), 0);
    EXPECT_EQ(empty_rect.cols(), 5);
    EXPECT_EQ(empty_rect.nonzeros(), 0);
    EXPECT_TRUE(empty_rect.empty());
    EXPECT_TRUE(empty_rect.is_valid());
    EXPECT_EQ(empty_rect.col_ptr().size(), 6);

    const std::vector<Real> x5 = {1.0, 2.0, 3.0, 4.0, 5.0};
    const auto y_empty = empty_rect.multiply(x5);
    EXPECT_TRUE(y_empty.empty());

    const SparseMatrix empty_trans = empty_rect.transpose();
    EXPECT_EQ(empty_trans.rows(), 5);
    EXPECT_EQ(empty_trans.cols(), 0);
    EXPECT_TRUE(empty_trans.empty());
}

TEST(SparseMatrixTest, SingleElementMatrix) {
    const std::vector<Triplet> triplets = {
        {0, 0, 4.5}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(1, 1, triplets);

    EXPECT_EQ(A.rows(), 1);
    EXPECT_EQ(A.cols(), 1);
    EXPECT_EQ(A.nonzeros(), 1);
    EXPECT_FALSE(A.empty());
    EXPECT_TRUE(A.is_valid());

    EXPECT_DOUBLE_EQ(A.get(0, 0), 4.5);
    EXPECT_DOUBLE_EQ(A.get(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(A.get(1, 0), 0.0);

    const std::vector<Real> x = {2.0};
    const auto y = A.multiply(x);
    ASSERT_EQ(y.size(), 1);
    EXPECT_DOUBLE_EQ(y[0], 9.0);

    const auto yt = A.multiply_transpose(x);
    ASSERT_EQ(yt.size(), 1);
    EXPECT_DOUBLE_EQ(yt[0], 9.0);

    const SparseMatrix At = A.transpose();
    EXPECT_TRUE(A.equals(At));
}

TEST(SparseMatrixTest, SmallDenseLookingMatrix) {
    // 3x3 matrix:
    // [ 1.0  2.0  0.0 ]
    // [ 3.0  0.0  4.0 ]
    // [ 0.0  5.0  6.0 ]
    const std::vector<Triplet> triplets = {
        {0, 0, 1.0},
        {0, 1, 2.0},
        {1, 0, 3.0},
        {1, 2, 4.0},
        {2, 1, 5.0},
        {2, 2, 6.0}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(3, 3, triplets);
    EXPECT_EQ(A.rows(), 3);
    EXPECT_EQ(A.cols(), 3);
    EXPECT_EQ(A.nonzeros(), 6);
    EXPECT_TRUE(A.is_valid());

    const std::vector<Real> x = {1.0, 2.0, 3.0};
    // y = A * x:
    // [ 1*1 + 2*2 + 0*3 ] = [ 5.0 ]
    // [ 3*1 + 0*2 + 4*3 ] = [ 15.0 ]
    // [ 0*1 + 5*2 + 6*3 ] = [ 28.0 ]
    const auto y = A.multiply(x);
    ASSERT_EQ(y.size(), 3);
    EXPECT_DOUBLE_EQ(y[0], 5.0);
    EXPECT_DOUBLE_EQ(y[1], 15.0);
    EXPECT_DOUBLE_EQ(y[2], 28.0);

    // yt = A^T * x:
    // [ 1*1 + 3*2 + 0*3 ] = [ 7.0 ]
    // [ 2*1 + 0*2 + 5*3 ] = [ 17.0 ]
    // [ 0*1 + 4*2 + 6*3 ] = [ 26.0 ]
    const auto yt = A.multiply_transpose(x);
    ASSERT_EQ(yt.size(), 3);
    EXPECT_DOUBLE_EQ(yt[0], 7.0);
    EXPECT_DOUBLE_EQ(yt[1], 17.0);
    EXPECT_DOUBLE_EQ(yt[2], 26.0);
}

TEST(SparseMatrixTest, RectangularMatrixMultiplications) {
    // 2x4 matrix:
    // [ 1.0  0.0  2.0  0.0 ]
    // [ 0.0  3.0  0.0  4.0 ]
    const std::vector<Triplet> triplets = {
        {0, 0, 1.0},
        {0, 2, 2.0},
        {1, 1, 3.0},
        {1, 3, 4.0}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(2, 4, triplets);
    EXPECT_EQ(A.rows(), 2);
    EXPECT_EQ(A.cols(), 4);
    EXPECT_TRUE(A.is_valid());

    const std::vector<Real> x4 = {10.0, 20.0, 30.0, 40.0};
    const auto y2 = A.multiply(x4);
    ASSERT_EQ(y2.size(), 2);
    EXPECT_DOUBLE_EQ(y2[0], 1.0 * 10.0 + 2.0 * 30.0); // 70.0
    EXPECT_DOUBLE_EQ(y2[1], 3.0 * 20.0 + 4.0 * 40.0); // 220.0

    const std::vector<Real> x2 = {5.0, 7.0};
    const auto y4 = A.multiply_transpose(x2);
    ASSERT_EQ(y4.size(), 4);
    EXPECT_DOUBLE_EQ(y4[0], 1.0 * 5.0); // 5.0
    EXPECT_DOUBLE_EQ(y4[1], 3.0 * 7.0); // 21.0
    EXPECT_DOUBLE_EQ(y4[2], 2.0 * 5.0); // 10.0
    EXPECT_DOUBLE_EQ(y4[3], 4.0 * 7.0); // 28.0

    const SparseMatrix At = A.transpose();
    EXPECT_EQ(At.rows(), 4);
    EXPECT_EQ(At.cols(), 2);
    EXPECT_TRUE(At.is_valid());

    const auto y4_via_At = At.multiply(x2);
    EXPECT_EQ(y4, y4_via_At);
}

TEST(SparseMatrixTest, DuplicateTripletSumming) {
    // Duplicate triplets must be summed: (0, 1, 2.0) and (0, 1, 3.0) -> (0, 1, 5.0)
    const std::vector<Triplet> triplets = {
        {0, 1, 2.0 },
        {0, 1, 3.0 },
        {1, 0, 4.0 },
        {1, 0, -1.0},
        {0, 1, 1.5 }
    };
    const SparseMatrix A = SparseMatrix::from_triplets(2, 2, triplets);

    EXPECT_EQ(A.rows(), 2);
    EXPECT_EQ(A.cols(), 2);
    EXPECT_EQ(A.nonzeros(), 2);
    EXPECT_TRUE(A.is_valid());

    EXPECT_DOUBLE_EQ(A.get(0, 1), 6.5); // 2.0 + 3.0 + 1.5
    EXPECT_DOUBLE_EQ(A.get(1, 0), 3.0); // 4.0 - 1.0
    EXPECT_DOUBLE_EQ(A.get(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(A.get(1, 1), 0.0);
}

TEST(SparseMatrixTest, ZeroValuedEntriesAndEmptyNonzeros) {
    // Explicit zero value
    const std::vector<Triplet> triplets = {
        {0, 0, 0.0},
        {1, 1, 2.5}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(2, 2, triplets);
    EXPECT_EQ(A.nonzeros(), 2);
    EXPECT_DOUBLE_EQ(A.get(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(A.get(1, 1), 2.5);
    EXPECT_TRUE(A.is_valid());

    // Matrix with no nonzeros
    const SparseMatrix A_zero = SparseMatrix::from_triplets(3, 3, {});
    EXPECT_EQ(A_zero.nonzeros(), 0);
    EXPECT_TRUE(A_zero.is_valid());
    const std::vector<Real> x = {1.0, 2.0, 3.0};
    const auto y = A_zero.multiply(x);
    for (Real v : y) {
        EXPECT_DOUBLE_EQ(v, 0.0);
    }
}

TEST(SparseMatrixTest, EmptyRowsAndEmptyColumns) {
    // 4x4 matrix with empty column 1 and empty row 2:
    // [ 1.0  0.0  2.0  0.0 ]
    // [ 0.0  0.0  3.0  0.0 ]
    // [ 0.0  0.0  0.0  0.0 ]  <- empty row
    // [ 4.0  0.0  0.0  5.0 ]
    //        ^
    //    empty col
    const std::vector<Triplet> triplets = {
        {0, 0, 1.0},
        {0, 2, 2.0},
        {1, 2, 3.0},
        {3, 0, 4.0},
        {3, 3, 5.0}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(4, 4, triplets);
    EXPECT_TRUE(A.is_valid());

    // Check that column 1 has 0 entries (col_ptr[1] == col_ptr[2])
    EXPECT_EQ(A.col_ptr()[1], A.col_ptr()[2]);

    const std::vector<Real> x = {1.0, 10.0, 2.0, 3.0};
    const auto y = A.multiply(x);
    ASSERT_EQ(y.size(), 4);
    EXPECT_DOUBLE_EQ(y[0], 1.0 * 1.0 + 2.0 * 2.0); // 5.0
    EXPECT_DOUBLE_EQ(y[1], 3.0 * 2.0);             // 6.0
    EXPECT_DOUBLE_EQ(y[2], 0.0);                   // row 2 empty -> 0.0
    EXPECT_DOUBLE_EQ(y[3], 4.0 * 1.0 + 5.0 * 3.0); // 19.0

    // Transpose and verify
    const SparseMatrix At = A.transpose();
    EXPECT_TRUE(At.is_valid());
    EXPECT_DOUBLE_EQ(At.get(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(At.get(2, 0), 2.0);
}

TEST(SparseMatrixTest, MultiplyAddInPlace) {
    const std::vector<Triplet> triplets = {
        {0, 0, 2.0},
        {1, 1, 3.0}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(2, 2, triplets);

    const std::vector<Real> x = {4.0, 5.0};
    std::vector<Real> y = {10.0, 20.0};

    A.multiply_add(x, y);
    EXPECT_DOUBLE_EQ(y[0], 10.0 + 2.0 * 4.0); // 18.0
    EXPECT_DOUBLE_EQ(y[1], 20.0 + 3.0 * 5.0); // 35.0

    std::vector<Real> yt = {100.0, 200.0};
    A.multiply_transpose_add(x, yt);
    EXPECT_DOUBLE_EQ(yt[0], 100.0 + 2.0 * 4.0); // 108.0
    EXPECT_DOUBLE_EQ(yt[1], 200.0 + 3.0 * 5.0); // 215.0
}

TEST(SparseMatrixTest, TransposeDoubleInversion) {
    // (A^T)^T == A must hold exactly for arbitrary rectangular matrices
    const std::vector<Triplet> triplets = {
        {0, 1, 1.2 },
        {0, 3, 3.4 },
        {1, 0, -2.1},
        {1, 2, 4.5 },
        {2, 1, 6.7 },
        {2, 3, -8.9}
    };
    const SparseMatrix A = SparseMatrix::from_triplets(3, 4, triplets);
    const SparseMatrix At = A.transpose();
    const SparseMatrix Att = At.transpose();

    EXPECT_TRUE(A.is_valid());
    EXPECT_TRUE(At.is_valid());
    EXPECT_TRUE(Att.is_valid());

    EXPECT_EQ(Att.rows(), A.rows());
    EXPECT_EQ(Att.cols(), A.cols());
    EXPECT_EQ(Att.nonzeros(), A.nonzeros());
    EXPECT_TRUE(A.equals(Att));
}

TEST(SparseMatrixTest, DenseDifferentialTesting) {
    // Generate deterministic pseudo-random sparse matrices with varying shapes
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<Real> val_dist(-10.0, 10.0);
    std::uniform_real_distribution<Real> prob_dist(0.0, 1.0);

    const std::vector<std::pair<Int, Int>> dimensions = {
        {5,  5 },
        {8,  12},
        {15, 6 },
        {20, 20}
    };

    const std::vector<Real> densities = {0.10, 0.25, 0.50};

    for (const auto& [m, n] : dimensions) {
        for (const Real density : densities) {
            DenseMatrixRef dense(m, n);
            std::vector<Triplet> triplets;

            for (Int r = 0; r < m; ++r) {
                for (Int c = 0; c < n; ++c) {
                    if (prob_dist(rng) < density) {
                        const Real val = std::round(val_dist(rng) * 100.0) / 100.0;
                        dense.set(r, c, val);
                        triplets.push_back({r, c, val});
                    }
                }
            }

            const SparseMatrix sparse = SparseMatrix::from_triplets(m, n, triplets);
            ASSERT_TRUE(sparse.is_valid());

            // Generate test vector x for A*x
            std::vector<Real> x(static_cast<std::size_t>(n));
            for (auto& v : x) {
                v = val_dist(rng);
            }

            const auto y_sparse = sparse.multiply(x);
            const auto y_dense = dense.multiply(x);

            ASSERT_EQ(y_sparse.size(), y_dense.size());
            for (std::size_t i = 0; i < y_sparse.size(); ++i) {
                EXPECT_NEAR(y_sparse[i], y_dense[i], 1e-11);
            }

            // Generate test vector xt for A^T*xt
            std::vector<Real> xt(static_cast<std::size_t>(m));
            for (auto& v : xt) {
                v = val_dist(rng);
            }

            const auto yt_sparse = sparse.multiply_transpose(xt);
            const auto yt_dense = dense.multiply_transpose(xt);

            ASSERT_EQ(yt_sparse.size(), yt_dense.size());
            for (std::size_t i = 0; i < yt_sparse.size(); ++i) {
                EXPECT_NEAR(yt_sparse[i], yt_dense[i], 1e-11);
            }

            // Verify transpose element-by-element
            const SparseMatrix sparse_T = sparse.transpose();
            ASSERT_TRUE(sparse_T.is_valid());
            for (Int r = 0; r < m; ++r) {
                for (Int c = 0; c < n; ++c) {
                    EXPECT_NEAR(sparse_T.get(c, r), dense.get(r, c), 1e-12);
                }
            }
        }
    }
}

TEST(SparseMatrixTest, PerformanceSanityModeratelySized) {
    // 1000x1000 matrix with 10,000 nonzeros (~1% density)
    // Ensures algorithm is genuinely sparse and does not allocate O(m*n) dense arrays
    const Int dim = 1000;
    const Int target_nnz = 10000;

    std::vector<Triplet> triplets;
    triplets.reserve(static_cast<std::size_t>(target_nnz));

    std::mt19937 rng(12345);
    std::uniform_int_distribution<Int> idx_dist(0, dim - 1);
    std::uniform_real_distribution<Real> val_dist(0.1, 5.0);

    for (Int k = 0; k < target_nnz; ++k) {
        triplets.push_back({idx_dist(rng), idx_dist(rng), val_dist(rng)});
    }

    const SparseMatrix A = SparseMatrix::from_triplets(dim, dim, triplets);
    EXPECT_TRUE(A.is_valid());
    EXPECT_LE(A.nonzeros(), target_nnz);
    EXPECT_GT(A.nonzeros(), 0);

    std::vector<Real> x(static_cast<std::size_t>(dim), 1.0);
    const auto y = A.multiply(x);
    EXPECT_EQ(y.size(), static_cast<std::size_t>(dim));

    const auto yt = A.multiply_transpose(x);
    EXPECT_EQ(yt.size(), static_cast<std::size_t>(dim));

    const SparseMatrix At = A.transpose();
    EXPECT_TRUE(At.is_valid());
    EXPECT_EQ(At.rows(), dim);
    EXPECT_EQ(At.cols(), dim);
    EXPECT_EQ(At.nonzeros(), A.nonzeros());
}

TEST(SparseMatrixTest, DimensionMismatchExceptions) {
    const SparseMatrix A(3, 2);

    const std::vector<Real> x_wrong = {1.0, 2.0, 3.0}; // length 3 != cols 2
    EXPECT_THROW([[maybe_unused]] auto res = A.multiply(x_wrong), std::invalid_argument);

    const std::vector<Real> xt_wrong = {1.0, 2.0}; // length 2 != rows 3
    EXPECT_THROW([[maybe_unused]] auto res = A.multiply_transpose(xt_wrong), std::invalid_argument);

    std::vector<Real> y_wrong(5);
    const std::vector<Real> x_correct = {1.0, 2.0};
    EXPECT_THROW(A.multiply_add(x_correct, y_wrong), std::invalid_argument);
}

TEST(SparseMatrixTest, InvalidConstructionExceptions) {
    // Negative dimensions
    EXPECT_THROW([[maybe_unused]] auto A = SparseMatrix::from_triplets(-1, 5, {}),
                 std::invalid_argument);
    EXPECT_THROW([[maybe_unused]] auto A = SparseMatrix::from_triplets(5, -1, {}),
                 std::invalid_argument);

    // Out-of-bounds triplet index
    const std::vector<Triplet> oob_row = {
        {5, 0, 1.0}
    };
    EXPECT_THROW([[maybe_unused]] auto A = SparseMatrix::from_triplets(2, 2, oob_row),
                 std::invalid_argument);

    const std::vector<Triplet> oob_col = {
        {0, 5, 1.0}
    };
    EXPECT_THROW([[maybe_unused]] auto A = SparseMatrix::from_triplets(2, 2, oob_col),
                 std::invalid_argument);

    // NaN value
    const double qnan = std::numeric_limits<double>::quiet_NaN();
    const std::vector<Triplet> nan_triplet = {
        {0, 0, qnan}
    };
    EXPECT_THROW([[maybe_unused]] auto A = SparseMatrix::from_triplets(2, 2, nan_triplet),
                 std::invalid_argument);
}

TEST(SparseMatrixTest, ValidationDiagnosticReporting) {
    // Test is_valid diagnostics on manually formed invalid matrices
    std::string err;

    // 1. col_ptr.back() mismatch
    SparseMatrix bad_col_back(2, 2, {0, 1, 5}, {0, 1}, {1.0, 2.0});
    EXPECT_FALSE(bad_col_back.is_valid(&err));
    EXPECT_NE(err.find("does not match values count"), std::string::npos);

    // 2. Non-monotonic col_ptr
    err.clear();
    SparseMatrix bad_col_ptr(2, 3, {0, 2, 1, 2}, {0, 1}, {1.0, 2.0});
    EXPECT_FALSE(bad_col_ptr.is_valid(&err));
    EXPECT_NE(err.find("Non-monotonic col_ptr"), std::string::npos);

    // 3. Out-of-bounds row index
    err.clear();
    SparseMatrix bad_row(2, 1, {0, 1}, {5}, {1.0});
    EXPECT_FALSE(bad_row.is_valid(&err));
    EXPECT_NE(err.find("out-of-bounds row index"), std::string::npos);

    // 4. Duplicate or non-increasing row index
    err.clear();
    SparseMatrix dup_row(3, 1, {0, 2}, {1, 1}, {1.0, 2.0});
    EXPECT_FALSE(dup_row.is_valid(&err));
    EXPECT_NE(err.find("non-increasing or duplicate row index"), std::string::npos);

    // 5. NaN entry
    err.clear();
    const double qnan = std::numeric_limits<double>::quiet_NaN();
    SparseMatrix nan_entry(2, 1, {0, 1}, {0}, {qnan});
    EXPECT_FALSE(nan_entry.is_valid(&err));
    EXPECT_NE(err.find("is NaN"), std::string::npos);
}

} // namespace
} // namespace vajra
