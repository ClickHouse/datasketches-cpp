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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <stdexcept>

#include "kll_sketch.hpp"

namespace py = pybind11;

template<typename T>
void bind_kll_sketch(py::module &m, const char* name) {
  using namespace datasketches;

  py::class_<kll_sketch<T>>(m, name)
    .def(py::init<uint16_t>(), py::arg("k")=kll_constants::DEFAULT_K)
    .def(py::init<const kll_sketch<T>&>())
    .def(
        "update",
        static_cast<void (kll_sketch<T>::*)(const T&)>(&kll_sketch<T>::update),
        py::arg("item"),
        "Updates the sketch with the given value"
    )
    .def(
        "update",
        [](kll_sketch<T>& sk, py::array_t<T, py::array::c_style | py::array::forcecast> items) {
          if (items.ndim() != 1) {
            throw std::invalid_argument("input data must have only one dimension. Found: "
              + std::to_string(items.ndim()));
          }
          auto array = items.template unchecked<1>();
          for (uint32_t i = 0; i < array.size(); ++i) sk.update(array(i));
        },
        py::arg("array"),
        "Updates the sketch with the values in the given array"
    )
    .def("merge", (void (kll_sketch<T>::*)(const kll_sketch<T>&)) &kll_sketch<T>::merge, py::arg("sketch"),
        "Merges the provided sketch into this one")
    .def("__str__", &kll_sketch<T>::to_string, py::arg("print_levels")=false, py::arg("print_items")=false,
        "Produces a string summary of the sketch")
    .def("to_string", &kll_sketch<T>::to_string, py::arg("print_levels")=false, py::arg("print_items")=false,
        "Produces a string summary of the sketch")
    .def("is_empty", &kll_sketch<T>::is_empty,
        "Returns True if the sketch is empty, otherwise False")
    .def("get_k", &kll_sketch<T>::get_k,
        "Returns the configured parameter k")
    .def("get_n", &kll_sketch<T>::get_n,
        "Returns the length of the input stream")
    .def("get_num_retained", &kll_sketch<T>::get_num_retained,
        "Returns the number of retained items (samples) in the sketch")
    .def("is_estimation_mode", &kll_sketch<T>::is_estimation_mode,
        "Returns True if the sketch is in estimation mode, otherwise False")
    .def("get_min_value", &kll_sketch<T>::get_min_item,
        "Returns the minimum value from the stream. If empty, kll_floats_sketch returns nan; kll_ints_sketch throws a RuntimeError")
    .def("get_max_value", &kll_sketch<T>::get_max_item,
        "Returns the maximum value from the stream. If empty, kll_floats_sketch returns nan; kll_ints_sketch throws a RuntimeError")
    .def("get_quantile", &kll_sketch<T>::get_quantile, py::arg("rank"), py::arg("inclusive")=false,
        "Returns an approximation to the data value "
        "associated with the given normalized rank in a hypothetical sorted "
        "version of the input stream so far.\n"
        "For kll_floats_sketch: if the sketch is empty this returns nan. "
        "For kll_ints_sketch: if the sketch is empty this throws a RuntimeError.")
    .def(
        "get_quantiles",
        [](const kll_sketch<T>& sk, const std::vector<double>& ranks, bool inclusive) {
          return sk.get_quantiles(ranks.data(), ranks.size(), inclusive);
        },
        py::arg("ranks"), py::arg("inclusive")=false,
        "This returns an array that could have been generated by using get_quantile() for each "
        "normalized rank separately.\n"
        "If the sketch is empty this returns an empty vector.\n"
        "Deprecated. Will be removed in the next major version. Use get_quantile() instead."
    )
    .def("get_rank", &kll_sketch<T>::get_rank, py::arg("value"), py::arg("inclusive")=false,
         "Returns an approximation to the normalized rank of the given value from 0 to 1, inclusive.\n"
         "The resulting approximation has a probabilistic guarantee that can be obtained from the "
         "get_normalized_rank_error(False) function.\n"
         "With the parameter inclusive=true the weight of the given value is included into the rank."
         "Otherwise the rank equals the sum of the weights of values less than the given value.\n"
         "If the sketch is empty this returns nan.")
    .def(
        "get_pmf",
        [](const kll_sketch<T>& sk, const std::vector<T>& split_points, bool inclusive) {
          return sk.get_PMF(split_points.data(), split_points.size(), inclusive);
        },
        py::arg("split_points"), py::arg("inclusive")=false,
        "Returns an approximation to the Probability Mass Function (PMF) of the input stream "
        "given a set of split points (values).\n"
        "The resulting approximations have a probabilistic guarantee that can be obtained from the "
        "get_normalized_rank_error(True) function.\n"
        "If the sketch is empty this returns an empty vector.\n"
        "split_points is an array of m unique, monotonically increasing float values "
        "that divide the real number line into m+1 consecutive disjoint intervals.\n"
        "If the parameter inclusive=false, the definition of an 'interval' is inclusive of the left split point (or minimum value) and "
        "exclusive of the right split point, with the exception that the last interval will include "
        "the maximum value.\n"
        "If the parameter inclusive=true, the definition of an 'interval' is exclusive of the left split point (or minimum value) and "
        "inclusive of the right split point.\n"
        "It is not necessary to include either the min or max values in these split points."
    )
    .def(
        "get_cdf",
        [](const kll_sketch<T>& sk, const std::vector<T>& split_points, bool inclusive) {
          return sk.get_CDF(split_points.data(), split_points.size(), inclusive);
        },
        py::arg("split_points"), py::arg("inclusive")=false,
        "Returns an approximation to the Cumulative Distribution Function (CDF), which is the "
        "cumulative analog of the PMF, of the input stream given a set of split points (values).\n"
        "The resulting approximations have a probabilistic guarantee that can be obtained from the "
        "get_normalized_rank_error(True) function.\n"
        "If the sketch is empty this returns an empty vector.\n"
        "split_points is an array of m unique, monotonically increasing float values "
        "that divide the real number line into m+1 consecutive disjoint intervals.\n"
        "If the parameter inclusive=false, the definition of an 'interval' is inclusive of the left split point (or minimum value) and "
        "exclusive of the right split point, with the exception that the last interval will include "
        "the maximum value.\n"
        "If the parameter inclusive=true, the definition of an 'interval' is exclusive of the left split point (or minimum value) and "
        "inclusive of the right split point.\n"
        "It is not necessary to include either the min or max values in these split points."
    )
    .def(
        "normalized_rank_error",
        static_cast<double (kll_sketch<T>::*)(bool) const>(&kll_sketch<T>::get_normalized_rank_error),
         py::arg("as_pmf"),
         "Gets the normalized rank error for this sketch.\n"
         "If pmf is True, returns the 'double-sided' normalized rank error for the get_PMF() function.\n"
         "Otherwise, it is the 'single-sided' normalized rank error for all the other queries.\n"
         "Constants were derived as the best fit to 99 percentile empirically measured max error in thousands of trials"
    )
    .def_static(
        "get_normalized_rank_error",
        [](uint16_t k, bool pmf) { return kll_sketch<T>::get_normalized_rank_error(k, pmf); },
         py::arg("k"), py::arg("as_pmf"),
         "Gets the normalized rank error given parameters k and the pmf flag.\n"
         "If pmf is True, returns the 'double-sided' normalized rank error for the get_PMF() function.\n"
         "Otherwise, it is the 'single-sided' normalized rank error for all the other queries.\n"
         "Constants were derived as the best fit to 99 percentile empirically measured max error in thousands of trials"
    )
    .def(
        "serialize",
        [](const kll_sketch<T>& sk) {
          auto bytes = sk.serialize();
          return py::bytes(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        },
        "Serializes the sketch into a bytes object"
    )
    .def_static(
        "deserialize",
        [](const std::string& bytes) { return kll_sketch<T>::deserialize(bytes.data(), bytes.size()); },
        py::arg("bytes"),
        "Deserializes the sketch from a bytes object"
    )
    .def("__iter__", [](const kll_sketch<T>& s) { return py::make_iterator(s.begin(), s.end()); });
}

void init_kll(py::module &m) {
  bind_kll_sketch<int>(m, "kll_ints_sketch");
  bind_kll_sketch<float>(m, "kll_floats_sketch");
  bind_kll_sketch<double>(m, "kll_doubles_sketch");
}
