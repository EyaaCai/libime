/*
 * SPDX-FileCopyrightText: 2026-2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "spanreadingresolver.h"
#include "segmentgraph.h"

namespace libime {

SpanReadingResolver::~SpanReadingResolver() = default;

std::vector<std::string_view>
SpanReadingResolver::readings(const SegmentGraphBase &graph,
                              const SegmentGraphNode &from,
                              const SegmentGraphNode &to) const {
    // An ordinary graph: the span stands for its own bytes.
    return {graph.segment(from, to)};
}

char letterToKeypadDigit(char letter) {
    switch (letter) {
    case 'a':
    case 'b':
    case 'c':
        return '2';
    case 'd':
    case 'e':
    case 'f':
        return '3';
    case 'g':
    case 'h':
    case 'i':
        return '4';
    case 'j':
    case 'k':
    case 'l':
        return '5';
    case 'm':
    case 'n':
    case 'o':
        return '6';
    case 'p':
    case 'q':
    case 'r':
    case 's':
        return '7';
    case 't':
    case 'u':
    case 'v':
        return '8';
    case 'w':
    case 'x':
    case 'y':
    case 'z':
        return '9';
    default:
        return '\0';
    }
}

} // namespace libime
