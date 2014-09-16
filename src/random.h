/// @file random.h
/// Contains helpers for randomness simulations.
#ifndef SCRAM_RANDOM_H_
#define SCRAM_RANDOM_H_

#include <vector>

#include <boost/random.hpp>

namespace scram {

/// @class Random
/// This class contains generators for various random distributions.
/// The values passed to the member functions are asserted to be in the
/// correct form. In other words, the user should make sure that the passed
/// parameters are valid. For example, variance cannot be negative.
class Random {
 public:
  /// The constructor for Random distibutions.
  /// @param[in] seed The seed for RNGs.
  explicit Random(int seed);

  ~Random() {}

  /// Rng from uniform distribution.
  /// @param[in] min Lower bound.
  /// @param[in] max Upper bound.
  /// @returns A sampled value.
  double UniformRealGenerator(double min, double max);

  /// Rng from a triangular distribution.
  /// @param[in] lower Lower bound.
  /// @param[in] mode The peak of the distribution.
  /// @param[in] upper Upper bound.
  /// @returns A sampled value.
  double TriangularGenerator(double lower, double mode, double upper);

  /// Rng from a piecewise linear distribution.
  /// @param[in] intervals Interval points for the distribution.
  ///                      The values must be strictly increasing.
  /// @param[in] weights Weights at the boundaries. The number of weights
  ///                    must be equal to the number of points.
  ///                    Extra weights are ignored.
  /// @returns A sampled value.
  double PiecewiseLinearGenerator(const std::vector<double>& intervals,
                                  const std::vector<double>& weights);

  /// Rng from a histogram distribution.
  /// @param[in] intervals Interval points for the distribution.
  ///                      The values must be strictly increasing.
  /// @param[in] weights Weights at the boundaries. The number of weights
  ///                    must be one less than the number of points.
  ///                    Extra weights are ignored.
  /// @returns A sampled value.
  double HistogramGenerator(const std::vector<double>& intervals,
                            const std::vector<double>& weights);

  /// Rng from a discrete distribution.
  /// @param[in] values Discrete values.
  /// @param[in] weights Weights for the corresponding values. The size must
  ///                    be the same as the values vector size.
  /// @returns A sample Value from the value vector.
  template<class T>
  inline T DiscreteGenerator(const std::vector<T>& values,
                             const std::vector<double>& weights) {
    assert(values.size() == weights.size());
    return values[DiscreteGenerator(weights)];
  }

  /// Rng from a normal distribution.
  /// @param[in] mean The mean of the distribution.
  /// @param[in] sigma The variance of the distribution.
  /// @returns A sampled value.
  double NormalGenerator(double mean, double sigma);

  /// Rng from lognormal distribution.
  /// @param[in] mean The mean of the distribution.
  /// @param[in] sigma The variance of the distribution.
  /// @returns A sampled value.
  double LogNormalGenerator(double mean, double sigma);

  /// Rng from Gamma distribution.
  /// @param[in] k Shape parameter of Gamma distribution.
  /// @param[in] theta Scale parameter of Gamma distribution.
  /// @returns A sampled value.
  /// @note The rate parameter is 1/theta, so for alpha/beta system, pass
  /// 1/beta as a second paramter for this generator.
  double GammaGenerator(double k, double theta);

  /// Rng from Beta distribution.
  /// @param[in] alpha Alpha shape parameter of Beta distribution.
  /// @param[in] beta Beta shape parameter of Beta distribution.
  /// @returns A sampled value.
  double BetaGenerator(double alpha, double beta);

  /// Rng from Weibull distribution.
  /// @param[in] k Shape parameter of Weibull distribution.
  /// @param[in] lambda Scale parameter of Weibull distribution.
  /// @returns A sampled value.
  double WeibullGenerator(double k, double lambda);

  /// Rng from Exponential distribution.
  /// @param[in] lambda Rate parameter of Exponential distribution.
  /// @returns A sampled value.
  double ExponentialGenerator(double lambda);

  /// Rng from Poisson distribution.
  /// @param[in] mean The mean value for Poisson distribution.
  /// @returns A sampled value.
  double PoissonGenerator(double mean);

 private:
  /// Rng from a discrete distribution.
  /// @param[in] weights Weights for the range [0, n), where n is the size
  ///                    of the vector.
  /// @returns Integer in the range [0, 1).
  int DiscreteGenerator(const std::vector<double>& weights);

  /// The random number generator.
  boost::mt19937 rng_;
};

}  // namespace scram

#endif  // SCRAM_RANDOM_H_
