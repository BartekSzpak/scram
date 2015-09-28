/*
 * Copyright (C) 2014-2015 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "risk_analysis_tests.h"

namespace scram {
namespace test {

// Benchmark Tests for Baobab 1 fault tree from XFTA.
// Test Minimal cut sets.
TEST_F(RiskAnalysisTest, Baobab_1_Test) {
  std::vector<std::string> input_files;
  input_files.push_back("./share/scram/input/Baobab/baobab1.xml");
  input_files.push_back("./share/scram/input/Baobab/baobab1-basic-events.xml");
  settings.limit_order(6).probability_analysis(true);
  ASSERT_NO_THROW(ProcessInputFiles(input_files));
  ASSERT_NO_THROW(ran->Analyze());
  EXPECT_NEAR(1.2823e-6, p_total(), 1e-8);  // Probability with BDD.
  // Minimal cut set check.
  EXPECT_EQ(2684, min_cut_sets().size());
  std::vector<int> distr = {0, 0, 1, 1, 70, 400, 2212};
  EXPECT_EQ(distr, McsDistribution());
}

}  // namespace test
}  // namespace scram