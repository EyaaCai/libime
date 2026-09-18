/*
 * SPDX-FileCopyrightText: 2026-2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _FCITX_LIBIME_PINYIN_T9ENCODER_H_
#define _FCITX_LIBIME_PINYIN_T9ENCODER_H_

#include <string>
#include <string_view>
#include <vector>
#include <libime/core/segmentgraph.h>
#include <libime/pinyin/libimepinyin_export.h>

namespace libime {

/**
 * Nine key (T9) pinyin input.
 *
 * A nine key keypad sends one digit per key press, so a digit string does not
 * say which letters were meant. This turns such a string into a segment graph
 * where every edge is one syllable and carries every pinyin that shares the
 * digits it covers, and it installs a reading resolver so the ordinary pinyin
 * dictionary and decoder can be used unchanged.
 *
 * Every edge is exactly one syllable, which is what the dictionary matching
 * assumes: it advances one syllable per edge, so a span standing for several
 * syllables would break its accounting. A digit run that reads as a longer
 * syllable therefore simply produces a longer edge.
 */
class LIBIMEPINYIN_EXPORT PinyinT9Encoder {
public:
    /**
     * Build the graph for a digit string.
     *
     * @param digits the pressed digits, each in '2'..'9'. Anything else has no
     *               reading and is left without an edge.
     * @return the graph, with its reading resolver already installed. Every
     *         reachable path spells the whole digit string.
     */
    static SegmentGraph parseUserT9(std::string digits);

    /**
     * Encoded pinyin readings of a digit string, two bytes per syllable.
     *
     * Intended for the reading resolver and for tests. A digit string that is
     * not a legal syllable yields nothing.
     */
    static std::vector<std::string_view> readingsForDigits(std::string_view digits);

    /// Digit signature of a lower case full pinyin, e.g. "nihao" -> "64426".
    static std::string digitSignature(std::string_view pinyin);

    /// Longest digit run a single syllable can occupy. Bounds the graph edges.
    static size_t maxSyllableDigits();
};

} // namespace libime

#endif // _FCITX_LIBIME_PINYIN_T9ENCODER_H_
