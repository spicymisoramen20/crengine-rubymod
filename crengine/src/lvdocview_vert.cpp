// =============================================================================
// Fork-only LVDocView vertical-rl helpers.
//
// #included at the end of lvdocview.cpp so these methods belong to the
// same translation unit as LVDocView's other definitions and can access
// private members (m_doc, m_pages, m_dx, m_pageMargins).
//
// Created during the upstream-merge-friendliness pass to keep lvdocview.cpp
// closer to upstream.  Future upstream changes to LVDocView won't conflict
// with these methods since they live in a separate fork-only file.
// =============================================================================

/// Returns the screen-X anchor for the right edge of a vertical-rl page.
/// In vertical-rl mode, columns are anchored at clip.right = page_right.
/// page.height = N × strut ≤ page_width, so the remainder would otherwise
/// accumulate on the left and make the left visual gap wider than the right
/// (very noticeable on narrow phone screens).  Distribute it equally by
/// shifting the anchor inward by half the gap.  All three callers
/// (drawPageTo, docToWindowPoint, windowToDocPoint) use this function so
/// draw and hit-testing stay aligned.
///
/// First-column ruby overhangs into the physical right margin (JLReq before
/// side).  Body-only centering treats that whole margin as empty, so the
/// left gap still looks larger than the ink-to-edge gap on the right.  Add
/// half an em (capped by the right margin) to the centering offset so the
/// body+ruby block balances.
///
/// Trade-off vs right-flush short pages: a chapter's final few columns are
/// centered in the usable width rather than pinned to the right margin.
int LVDocView::vertPageRight( const lvRect & pageRect, int page_content_height ) const {
    int page_right = pageRect.right - m_pageMargins.right;
    int page_width = pageRect.width() - m_pageMargins.left - m_pageMargins.right;
    // Mono-ruby overhang is typically ~0.5em into the right margin.
    int ruby_reserve = m_font_size / 2;
    if ( ruby_reserve > m_pageMargins.right )
        ruby_reserve = m_pageMargins.right;
    if ( ruby_reserve < 0 )
        ruby_reserve = 0;
    int centering_offset = (page_width - page_content_height + ruby_reserve) / 2;
    if ( centering_offset < 0 )
        centering_offset = 0;
    return page_right - centering_offset;
}

/// Returns true if the document is laid out vertically (vertical-rl/-lr).
///
/// Primary signal: the per-page writing mode recorded during rendering.  Each
/// page carries the css writing-mode of its content (stamped from its first
/// line in the page splitter), so a document with a horizontal cover followed
/// by vertical body text is correctly detected, while a purely horizontal
/// document is not — even if it contains a few short pages (title page, part
/// divider, cover).
///
/// The previous implementation only inspected the FIRST <body> (usually the
/// horizontal cover, whose writing-mode is unset) and then fell back to a
/// page-HEIGHT heuristic that returned true if ANY page was short.  That
/// false-positived on horizontal rtl-spine EPUBs with no writing-mode CSS
/// (e.g. calibre conversions): a short title page made isVerticalText() true,
/// and ReaderRolling forced RTL page-turn on horizontal text (horizontal-RTL).
bool LVDocView::isVerticalText() const {
    if (m_pages.length() > 0) {
        for (int i = 0; i < m_pages.length(); i++) {
            if (css_wm_is_vertical(m_pages[i]->writing_mode))
                return true;
        }
        return false; // pages rendered, none vertical → horizontal document
    }
    // Pre-render fallback (no pages yet): inspect the body element's style.
    if (m_doc) {
        ldomNode * root = m_doc->getRootNode();
        if (root) {
            // Walk shallowly to find <body> (typically root → DocFragment → body).
            for (int depth = 0; depth < 4; depth++) {
                if (!root) break;
                if (root->isElement() && root->getNodeId() == el_body) {
                    css_style_ref_t style = root->getStyle();
                    if (!style.isNull() && css_wm_is_vertical(style->writing_mode))
                        return true;
                    break;
                }
                // Descend to first element child.
                ldomNode * next = NULL;
                int cnt = root->getChildCount();
                for (int i = 0; i < cnt; i++) {
                    ldomNode * c = root->getChildNode(i);
                    if (c && c->isElement()) { next = c; break; }
                }
                root = next;
            }
        }
    }
    return false;
}

/// Vertical-rl doc → window for SCROLL view mode.
///   doc.y (column position) → screen.x = page_right − (doc.y − _pos)
///   doc.x (in-column pos)   → screen.y = doc.x
/// _pos is the doc-y at the viewport's right edge (entry point for forward
/// reading), kept in lockstep with corner-scroll's `_gotoPos(currentPos +
/// screen_w/3)` advances.  Mirrors PAGE-mode docToWindowPoint's vertical-rl
/// branch but with _pos in place of m_pages[page]->start and a single global
/// page_right anchor instead of per-page m_pageRects.
bool LVDocView::docToWindowPointScrollVert( lvPoint & pt ) const {
    int page_right = m_dx - m_pageMargins.right;
    int screen_x = page_right - (pt.y - _pos);
    int screen_y = pt.x;
    pt.x = screen_x;
    pt.y = screen_y;
    return true;
}

/// Reverse of docToWindowPointScrollVert.  pullInPageArea clamps screen.x to
/// the visible horizontal band (margin.left … page_right − 1) so taps in the
/// outer margins still resolve to the nearest in-page column, matching the
/// upstream SCROLL-mode pullInPageArea behaviour for horizontal text.
bool LVDocView::windowToDocPointScrollVert( lvPoint & pt, bool pullInPageArea ) const {
    int page_right = m_dx - m_pageMargins.right;
    if ( pullInPageArea ) {
        if ( pt.x < m_pageMargins.left )
            pt.x = m_pageMargins.left;
        if ( pt.x >= page_right )
            pt.x = page_right - 1;
    }
    int doc_y = page_right - pt.x + _pos;
    int doc_x = pt.y;
    pt.x = doc_x;
    pt.y = doc_y;
    return true;
}
