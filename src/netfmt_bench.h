#ifndef NET_FORMAT_BENCH_H
#define NET_FORMAT_BENCH_H

#include <istream>

class Net;

bool readNetFromBench(std::istream &is, Net &net);

#endif
