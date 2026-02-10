/*
CppOpenScraper - C++ Web Scraping Library
Copyright (C) 2025 Adrian Maire (escain@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONTENT_EXTRACTOR_H
#define CONTENT_EXTRACTOR_H

#include "ScrapedPage.hpp"
#include "LexborDocument.hpp"

#include <string>
#include <vector>

namespace CppScrap
{

/// @brief Extracted content from an HTML page: text, images and links.
struct ExtractedContent
{
	std::string text;                   // Main text content, whitespace-normalized.
	std::vector<ScrapedImage> images;   // Content images found on the page.
	std::vector<ScrapedLink> links;     // Hyperlinks found on the page.
};

/// @brief Extracts main content from a parsed HTML document.
class ContentExtractor final
{
public:
	/// @brief Clean HTML and extract main text, images and links.
	auto extract(LexborDocument& doc, const std::string& baseUrl) const -> ExtractedContent;

private:
	/// @brief Strip script, style, form and other non-content tags.
	auto removeUnwantedElements(LexborDocument& doc) const -> void;
	/// @brief Remove nav, header, footer, sidebar and boilerplate elements.
	auto removeBoilerplate(LexborDocument& doc) const -> void;
	/// @brief Score block elements to find the main content container.
	auto findMainContent(LexborDocument& doc) const -> LexborDocument::Node;
	/// @brief Heuristically score a node by text density and semantic hints.
	auto scoreNode(LexborDocument& doc, LexborDocument::Node node) const -> int;
	/// @brief Extract and normalize whitespace in text content.
	auto extractText(LexborDocument& doc, LexborDocument::Node node) const -> std::string;
	/// @brief Collect content images, filtering icons, logos and tracking pixels.
	auto extractImages(LexborDocument& doc, LexborDocument::Node node,
		const std::string& baseUrl) const -> std::vector<ScrapedImage>;
	/// @brief Collect hyperlinks, skipping fragment-only and javascript: URLs.
	auto extractLinks(LexborDocument& doc, LexborDocument::Node node,
		const std::string& baseUrl) const -> std::vector<ScrapedLink>;
	/// @brief Resolve a relative URL against a base URL.
	auto resolveUrl(const std::string& relative, const std::string& base) const -> std::string;
	/// @brief Check if a class or id string matches boilerplate keywords.
	auto matchesBoilerplatePattern(const std::string& classOrId) const -> bool;
};

}

#endif // CONTENT_EXTRACTOR_H
