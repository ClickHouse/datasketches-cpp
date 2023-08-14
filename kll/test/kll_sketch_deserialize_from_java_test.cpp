/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <catch2/catch.hpp>
#include <fstream>
#include <kll_sketch.hpp>

namespace datasketches {

// assume the binary sketches for this test have been generated by datasketches-java code
// in the subdirectory called "java" in the root directory of this project
static std::string testBinaryInputPath = std::string(TEST_BINARY_INPUT_PATH) + "../../java/";

TEST_CASE("kll float", "[serde_compat]") {
  unsigned n_arr[] = {0, 1, 10, 100, 1000, 10000, 100000, 1000000};
  for (const unsigned n: n_arr) {
    std::ifstream is;
    is.exceptions(std::ios::failbit | std::ios::badbit);
    is.open(testBinaryInputPath + "kll_float_n" + std::to_string(n) + ".sk", std::ios::binary);
    auto sketch = kll_sketch<float>::deserialize(is);
    REQUIRE(sketch.is_empty() == (n == 0));
    REQUIRE(sketch.is_estimation_mode() == (n > kll_constants::DEFAULT_K));
    REQUIRE(sketch.get_n() == n);
    if (n > 0) {
      REQUIRE(sketch.get_min_item() == 1.0f);
      REQUIRE(sketch.get_max_item() == static_cast<float>(n));
      uint64_t weight = 0;
      for (const auto pair: sketch) {
        REQUIRE(pair.first >= sketch.get_min_item());
        REQUIRE(pair.first <= sketch.get_max_item());
        weight += pair.second;
      }
      REQUIRE(weight == sketch.get_n());
    }
  }
}

TEST_CASE("kll double", "[serde_compat]") {
  unsigned n_arr[] = {0, 1, 10, 100, 1000, 10000, 100000, 1000000};
  for (const unsigned n: n_arr) {
    std::ifstream is;
    is.exceptions(std::ios::failbit | std::ios::badbit);
    is.open(testBinaryInputPath + "kll_double_n" + std::to_string(n) + ".sk", std::ios::binary);
    auto sketch = kll_sketch<double>::deserialize(is);
    REQUIRE(sketch.is_empty() == (n == 0));
    REQUIRE(sketch.is_estimation_mode() == (n > kll_constants::DEFAULT_K));
    REQUIRE(sketch.get_n() == n);
    if (n > 0) {
      REQUIRE(sketch.get_min_item() == 1.0);
      REQUIRE(sketch.get_max_item() == static_cast<double>(n));
      uint64_t weight = 0;
      for (const auto pair: sketch) {
        REQUIRE(pair.first >= sketch.get_min_item());
        REQUIRE(pair.first <= sketch.get_max_item());
        weight += pair.second;
      }
      REQUIRE(weight == sketch.get_n());
    }
  }
}

} /* namespace datasketches */
