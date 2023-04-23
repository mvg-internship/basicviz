//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#ifndef NET_FORMAT_BENCH_H
#define NET_FORMAT_BENCH_H

#include <istream>

class Net;

bool readNetFromBench(std::istream &is, Net &net);

#endif
