#pragma once

#include "vajra/model.hpp"
#include "vajra/options.hpp"
#include "vajra/solution.hpp"

namespace vajra {

/// Central optimization solver seam.
/// Solves the given optimization model under the specified solver options.
///
/// In Phase 0, this validates the model and returns:
/// - SolveStatus::InvalidModel with diagnostic details if model validation fails.
/// - SolveStatus::NotImplemented for valid models (solver engines to be added in Phase 1+).
[[nodiscard]] Solution solve(const Model& model, const Options& options = Options{});

} // namespace vajra
