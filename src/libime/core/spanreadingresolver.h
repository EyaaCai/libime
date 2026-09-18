/*
 * SPDX-FileCopyrightText: 2026-2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _FCITX_LIBIME_CORE_SPANREADINGRESOLVER_H_
#define _FCITX_LIBIME_CORE_SPANREADINGRESOLVER_H_

#include <string_view>
#include <vector>
#include <fcitx-utils/macros.h>
#include <libime/core/libimecore_export.h>

namespace libime {

class SegmentGraphBase;
class SegmentGraphNode;

/**
 * Resolves the reading of one span of a segment graph.
 *
 * A span of an ordinary graph stands for itself: the reading is the substring
 * of the graph data between the two nodes, which is what the default
 * implementation returns. A graph may instead let each span stand for one of
 * several readings, for example a nine key graph where a run of digits can be
 * read as any pinyin that shares those digits.
 *
 * Definitions are owned by the caller. A graph only holds a pointer, so the
 * resolver must outlive the graph that refers to it.
 *
 * @since 1.1.16
 */
class LIBIMECORE_EXPORT SpanReadingResolver {
public:
    virtual ~SpanReadingResolver();

    /**
     * Readings a span may stand for.
     *
     * The default implementation treats the span as standing for itself: it
     * returns the substring of the graph data between the two nodes. A graph
     * whose spans carry several readings overrides this.
     *
     * @param graph the graph the span belongs to.
     * @param from the span start node.
     * @param to the span end node.
     * @return the readings. Each reading is encoded the way the pinyin
     *         dictionary stores it, two bytes per syllable. Returning an empty
     *         vector means the span has no reading at all, so nothing can
     *         match it. Returning several readings means any of them may be
     *         tried.
     */
    virtual std::vector<std::string_view>
    readings(const SegmentGraphBase &graph, const SegmentGraphNode &from,
             const SegmentGraphNode &to) const;
};

/**
 * Keypad digit a latin letter sits on, following the standard phone layout.
 *
 * 2 abc, 3 def, 4 ghi, 5 jkl, 6 mno, 7 pqrs, 8 tuv, 9 wxyz. Letters outside
 * a-z, and the digits 0 and 1 which carry no letters, yield '\0'.
 *
 * This is the mapping every nine key layout uses, and it is a property of the
 * keypad rather than of pinyin, so it lives with the reading abstraction
 * instead of in a pinyin header.
 *
 * @since 1.1.16
 */
LIBIMECORE_EXPORT char letterToKeypadDigit(char letter);

} // namespace libime

#endif // _FCITX_LIBIME_CORE_SPANREADINGRESOLVER_H_
