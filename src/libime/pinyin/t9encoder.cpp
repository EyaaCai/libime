/*
 * SPDX-FileCopyrightText: 2026-2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "t9encoder.h"
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <libime/core/spanreadingresolver.h>
#include <libime/pinyin/pinyindata.h>
#include <libime/pinyin/pinyinencoder.h>

namespace libime {

namespace {

/// digit run -> encoded forms of every syllable spelling it.
using SyllableTable =
    std::unordered_map<std::string, std::vector<std::string>>;

/// Encode one syllable the way the dictionary stores it: initial, final.
std::string encodeSyllable(const PinyinEntry &entry) {
    std::string encoded;
    encoded.push_back(static_cast<char>(entry.initial()));
    encoded.push_back(static_cast<char>(entry.final()));
    return encoded;
}

/**
 * Build the digit -> syllable table from the static syllable table.
 *
 * The syllable table is compiled into libime and never changes at runtime, so
 * this is built once.
 */
const SyllableTable &syllableTable() {
    static const SyllableTable table = [] {
        SyllableTable result;
        for (const auto &entry : getPinyinMapV2()) {
            const auto signature =
                PinyinT9Encoder::digitSignature(entry.pinyinView());
            if (signature.empty()) {
                continue;
            }
            auto &encoded = result[signature];
            auto form = encodeSyllable(entry);
            if (std::find(encoded.begin(), encoded.end(), form) ==
                encoded.end()) {
                encoded.push_back(std::move(form));
            }
        }
        return result;
    }();
    return table;
}

size_t maxSyllableDigitsValue() {
    static const size_t value = [] {
        size_t longest = 0;
        for (const auto &[signature, encoded] : syllableTable()) {
            longest = std::max(longest, signature.size());
        }
        return longest;
    }();
    return value;
}

/**
 * Resolver for a nine key graph.
 *
 * Stateless: the readings of a span depend only on the digits the span covers,
 * which are read back out of the graph data. That lets one shared instance
 * serve every graph, so the graph never holds a pointer to something that
 * could go out of scope before it.
 */
class T9SpanReadingResolver : public SpanReadingResolver {
public:
    std::vector<std::string_view>
    readings(const SegmentGraphBase &graph, const SegmentGraphNode &from,
             const SegmentGraphNode &to) const override {
        return PinyinT9Encoder::readingsForDigits(
            graph.segment(from, to));
    }
};

} // namespace

std::string PinyinT9Encoder::digitSignature(std::string_view pinyin) {
    std::string signature;
    signature.reserve(pinyin.size());
    for (auto letter : pinyin) {
        if (auto digit = letterToKeypadDigit(letter)) {
            signature.push_back(digit);
        }
    }
    return signature;
}

size_t PinyinT9Encoder::maxSyllableDigits() {
    return maxSyllableDigitsValue();
}

std::vector<std::string_view>
PinyinT9Encoder::readingsForDigits(std::string_view digits) {
    // A syllable occupies at least one digit, so the reading can never be
    // longer than the allowed run, and never shorter than one syllable.
    if (digits.empty() || digits.size() > maxSyllableDigits()) {
        return {};
    }
    const auto &table = syllableTable();
    auto iter = table.find(std::string(digits));
    if (iter == table.end()) {
        return {};
    }
    std::vector<std::string_view> readings;
    readings.reserve(iter->second.size());
    for (const auto &encoded : iter->second) {
        readings.emplace_back(encoded);
    }
    return readings;
}

SegmentGraph PinyinT9Encoder::parseUserT9(std::string digits) {
    SegmentGraph graph{std::move(digits)};
    const auto data = graph.data();
    const auto size = data.size();
    const auto longest = maxSyllableDigits();
    const auto &table = syllableTable();

    // Every node must be reachable from the start AND be able to reach the end,
    // or the graph is rejected outright and the decoder gets nothing at all.
    // With variable length syllables neither direction is automatic: a digit
    // that begins no syllable has nobody reaching it, and a position that is
    // not a syllable boundary has nowhere to go. Both are bridged by an edge
    // that carries no reading, so it can never match a word.
    std::vector<bool> fromStart(size + 1, false);
    fromStart[0] = true;

    // Pass one: edges that stand for a whole syllable, the only kind the
    // dictionary can match. An edge must never cover part of a syllable.
    for (size_t begin = 0; begin < size; begin++) {
        const auto maxEnd = std::min(size, begin + longest);
        for (size_t end = begin + 1; end <= maxEnd; end++) {
            if (table.find(std::string(data.substr(begin, end - begin))) ==
                table.end()) {
                continue;
            }
            graph.addNext(begin, end);
            fromStart[end] = true;
        }
    }

    // Pass two, forwards: reach every position from the start.
    for (size_t position = 1; position <= size; position++) {
        if (fromStart[position]) {
            continue;
        }
        size_t from = position - 1;
        while (from > 0 && !fromStart[from]) {
            from--;
        }
        graph.addNext(from, position);
        fromStart[position] = true;
    }

    // Pass three, backwards: give every position a way to the end. Bridging
    // from a position to the next one that already reaches the end also keeps
    // the reading's syllable count honest, because such an edge spans digits
    // that belong to no syllable.
    std::vector<bool> toEnd(size + 1, false);
    toEnd[size] = true;
    for (size_t position = size; position-- > 0;) {
        if (toEnd[position]) {
            continue;
        }
        size_t to = position + 1;
        while (to < size && !toEnd[to]) {
            to++;
        }
        graph.addNext(position, to);
        toEnd[position] = true;
    }
    // One shared, stateless resolver for every graph. It reads the digits of a
    // span back out of the graph, so it holds no per graph state.
    static const T9SpanReadingResolver resolver;
    graph.setReadingResolver(&resolver);
    return graph;
}

} // namespace libime
