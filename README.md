# CppOpenScraper

C++ web scraping library that fetches web pages, cleans the HTML (removes scripts, ads, navigation, boilerplate), and converts the result to **Markdown** preserving structure (tables, headings, links, images).

## Architecture

```
Raw HTML (from HTTP or headless browser)
    |
    v
LexborDocument::parse()          -- HTML5 DOM tree
    |
    v
ContentExtractor::extract()      -- Clean DOM:
    |  1. removeUnwantedElements  (script, style, noscript, iframe)
    |  2. removeBoilerplate       (nav, header, footer, aside, menu)
    |  3. removeAdElements        (rel="sponsored", adsbygoogle, data-ad*, dialogs, cookie/consent)
    |
    v
LexborDocument::serializeHtml()  -- Cleaned HTML string
    |
    v
html2md::Converter               -- Markdown with tables, headings, links
```

### Design decisions

**Why not text-density scoring?** Earlier versions used heuristic scoring to pick a single "main content" container. This failed on data-rich pages (financial tables, anime databases) where content spans multiple containers. The current approach removes only unambiguous noise and keeps everything else.

**Why not class/id pattern matching?** Substring matching on CSS class names (e.g. removing elements with "nav" in class) was too aggressive -- `class="row-nav-main"` matched and removed entire content sections. Only semantic HTML tags and standard attributes (rel, role, data-ad) are used for filtering.

**Why Markdown?** Markdown preserves document structure (tables, headings, links, emphasis) while being compact and readable by both humans and LLMs. Plain text extraction loses all formatting.

## Dependencies

| Dependency | Type | License | Purpose |
|-----------|------|---------|---------|
| [Qt6](https://www.qt.io/) (Core, Network) | System | LGPL-3 | HTTP networking, URL handling |
| [lexbor](https://lexbor.com/) | Submodule | Apache-2.0 | HTML5 parsing and DOM |
| [html2md](https://github.com/tim-gromeyer/html2md) | Submodule | MIT | HTML to Markdown conversion |
| [poppler-cpp](https://poppler.freedesktop.org/) | Optional system | GPL-2 | PDF text extraction |

### For JavaScript-rendered pages

CppOpenScraper processes **HTML strings** -- it does not run a browser. For pages that require JavaScript rendering (SPAs, dynamically loaded content), the caller must provide the rendered HTML. A common approach is headless Chromium via the Chrome DevTools Protocol (CDP):

1. Launch `chromium --headless --remote-debugging-port=9222`
2. Navigate and wait for `networkAlmostIdle` lifecycle event
3. Extract `document.documentElement.outerHTML` via `Runtime.evaluate`
4. Pass the HTML string to CppOpenScraper

## Build

```bash
git submodule update --init --recursive
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)
```

### CMake options

| Option | Default | Effect |
|--------|---------|--------|
| `CPPSCRAPER_BUILD_EXAMPLES` | `OFF` | Build example programs |
| `CPPSCRAPER_BUILD_TESTS` | `OFF` | Build unit tests |

### Running tests

```bash
cmake .. -DCPPSCRAPER_BUILD_TESTS=ON
cmake --build . -j$(nproc)
ctest --output-on-failure
```

## Usage

```cpp
#include <Scraper.hpp>

CppScrap::Scraper scraper;
auto page = scraper.scrape("https://example.com");

if (page.ok())
{
    // page.text contains Markdown
    std::cout << page.text << std::endl;
}
```

## License

LGPL-3.0-or-later
