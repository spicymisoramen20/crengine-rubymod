// =============================================================================
// Kanji JLPT / 常用 / 漢検 lookup for furigana stand-in level labels (fork-only).
// =============================================================================
#ifndef __LV_KANJI_LEVEL_H_INCLUDED__
#define __LV_KANJI_LEVEL_H_INCLUDED__

#include "lvtypes.h"
#include "lvstring.h"
#include "lvarray.h"

enum RubyToggleLevelScheme {
    RUBY_TOGGLE_LEVEL_OFF = 0,
    RUBY_TOGGLE_LEVEL_JLPT = 1,
    RUBY_TOGGLE_LEVEL_JOYO = 2,
    RUBY_TOGGLE_LEVEL_KANKEN = 3,
};

// Look up hardest mapped level label for codepoints in `text`.
// Returns empty string if scheme is off or no mapped kanji.
lString32 kanjiLevelLabelForText(const lString32 & text, int scheme);

// Per-kanji labels in document order for compound / multi-char bases.
// One entry per CJK ideograph in `text`; empty string if that char is unmapped.
// Non-ideograph codepoints are skipped (no slot).
void kanjiLevelLabelsForText(const lString32 & text, int scheme,
        LVArray<lString32> & out);

#endif // __LV_KANJI_LEVEL_H_INCLUDED__
