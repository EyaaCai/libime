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

} // namespace libime
