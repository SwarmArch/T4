/** $lic$
 * Copyright (C) 2014-2020 by Massachusetts Institute of Technology
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 * This software was developed and released to accompany the T4 ISCA 2020 paper
 * ("T4: Compiling Sequential Code for Effective Speculative Parallelization in
 * Hardware", Ying et al., ISCA-47, June 2020).  If you use this software in
 * your research, we request that you reference the T4 paper, and we ask that
 * you send us a citation of your work.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */

/**
 * Shamelessly copied from ordspecsim/tests/common.h. Will it become stale?
 * We'll see.
 */
#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <numeric>

namespace tests {

/**
 * This interface borrows from googletest's, albeit it has poor handling if
 * these methods are invoked multiple times in the same test. For example,
 *   assert_true(true);
 *   assert_true(false);
 * would print out
 *   Verify OK
 *   Verify Incorrect
 * which is confusing. Scripts that read the output should account for this.
 */

__attribute__((noswarmify))
static inline void assert_true(bool verify) {
    std::cout << "Verify: " << (verify ? "OK" : "Incorrect") << std::endl;
    if (!verify) std::abort();
}


// Check for equality of expected and actual.
// Works for primative types as well as containers.
// The overlads of assert_eq below use SFINAE:
// For any given call of assert_eq only one
// of the overloads below should suceed in substituting
// in the expressions in the return type.

template <typename T>
__attribute__((noswarmify))
static inline auto assert_eq(T expected, T actual)
        -> decltype(expected == actual, std::cout << expected, void()) {
    bool verify = expected == actual;
    if (!verify) {
        std::cout << "Expected " << expected
                  << " actual " << actual << std::endl;
    }
    tests::assert_true(verify);
}

template <typename Iterator>
__attribute__((noswarmify))
static inline std::string __to_string(Iterator begin, Iterator end) {
    using T = typename std::iterator_traits<Iterator>::value_type;
    // From http://en.cppreference.com/w/cpp/algorithm/accumulate
    std::string str = std::accumulate(
            begin, end, std::string{},
            [](const std::string& a, const T& t) {
                using std::to_string;  // enable ADL
                return a.empty() ? to_string(t)
                       : a + ", " + to_string(t);
            });
    return str;
}

template <typename C1, typename C2>
__attribute__((noswarmify))
static inline auto assert_eq(const C1& expected, const C2& actual)
        -> decltype(std::begin(expected), std::begin(actual), void()) {
    auto begin1 = std::begin(expected);
    auto end1 = std::end(expected);
    auto begin2 = std::begin(actual);
    auto end2 = std::end(actual);
    bool verify = std::equal(begin1, end1, begin2, end2);
    if (!verify) {
        std::cout << "expected "
                  << __to_string(begin1, end1)
                  << "\n"
                  << "actual   "
                  << __to_string(begin2, end2)
                  << std::endl;
    }
    tests::assert_true(verify);
}


// Like assert_eq, but does not check full length of one input

template <typename C1, typename C2>
__attribute__((noswarmify))
static inline auto assert_startswith(const C1& actual, const C2& expected_prefix)
        -> decltype(std::begin(expected_prefix), std::begin(actual), void()) {
    auto begin_prefix = std::begin(expected_prefix);
    auto end_prefix = std::end(expected_prefix);
    auto begin = std::begin(actual);
    bool verify = std::equal(begin_prefix, end_prefix, begin);
    if (!verify) {
        std::cout << "expected " << __to_string(begin_prefix, end_prefix) << "\n"
                  << "actual   " << __to_string(begin, begin + std::distance(begin_prefix, end_prefix)) << std::endl;
    }
    tests::assert_true(verify);
}

}
