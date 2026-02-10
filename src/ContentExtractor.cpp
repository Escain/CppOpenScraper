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

#include "ContentExtractor.hpp"

#include <QUrl>

#include <algorithm>
#include <array>
#include <cctype>
#include <numeric>
#include <sstream>

namespace CppScrap
{

namespace
{

auto toLower(const std::string& s) -> std::string
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return result;
}

auto normalizeWhitespace(const std::string& text) -> std::string
{
	std::string result;
	result.reserve(text.size());

	bool lastWasSpace = true;
	int newlineCount = 0;

	for (char c : text)
	{
		if (c == '\n' || c == '\r')
		{
			if (!lastWasSpace)
			{
				++newlineCount;
				if (newlineCount <= 2)
				{
					result += '\n';
				}
				lastWasSpace = true;
			}
		}
		else if (c == ' ' || c == '\t')
		{
			if (!lastWasSpace)
			{
				result += ' ';
				lastWasSpace = true;
			}
			newlineCount = 0;
		}
		else
		{
			result += c;
			lastWasSpace = false;
			newlineCount = 0;
		}
	}

	// Trim trailing whitespace
	while (!result.empty() && (result.back() == ' ' || result.back() == '\n'))
	{
		result.pop_back();
	}

	// Trim leading whitespace
	size_t start = 0;
	while (start < result.size() && (result[start] == ' ' || result[start] == '\n'))
	{
		++start;
	}

	return result.substr(start);
}

constexpr auto isBlockTag(std::string_view tag) -> bool
{
	constexpr std::array blocks = {
		std::string_view("div"),
		std::string_view("section"),
		std::string_view("article"),
		std::string_view("main"),
		std::string_view("td"),
		std::string_view("body"),
	};

	for (const auto& b : blocks)
	{
		if (tag == b) return true;
	}
	return false;
}

auto isFilteredImage(const std::string& src, const std::string& cls,
	const std::string& alt, const std::string& width, const std::string& height) -> bool
{
	static const std::array filterWords = {
		std::string("logo"), std::string("icon"), std::string("ad"),
		std::string("tracking"), std::string("pixel"),
		std::string("avatar"), std::string("badge"),
	};

	auto srcLower = toLower(src);
	auto clsLower = toLower(cls);
	auto altLower = toLower(alt);

	for (const auto& word : filterWords)
	{
		if (srcLower.find(word) != std::string::npos) return true;
		if (clsLower.find(word) != std::string::npos) return true;
		if (altLower.find(word) != std::string::npos) return true;
	}

	auto parseSize = [](const std::string& s) -> int
	{
		if (s.empty()) return -1;
		try { return std::stoi(s); }
		catch (...) { return -1; }
	};

	int w = parseSize(width);
	int h = parseSize(height);
	if ((w >= 0 && w < 50) || (h >= 0 && h < 50))
	{
		return true;
	}

	return false;
}

auto linkTextLength(LexborDocument& doc, LexborDocument::Node node) -> int
{
	auto links = doc.findByTag(node, "a");
	int total = 0;
	for (auto* link : links)
	{
		total += static_cast<int>(doc.textContent(link).size());
	}
	return total;
}

auto countChildElements(LexborDocument& doc, LexborDocument::Node node) -> int
{
	int count = 0;
	for (auto* child = doc.firstChild(node); child; child = doc.nextSibling(child))
	{
		if (doc.isElement(child))
		{
			++count;
		}
	}
	return count;
}

} // anonymous namespace


auto ContentExtractor::extract(LexborDocument& doc, const std::string& baseUrl) const -> ExtractedContent
{
	ExtractedContent result;

	auto* body = doc.body();
	if (!body)
	{
		return result;
	}

	removeUnwantedElements(doc);
	removeBoilerplate(doc);

	auto* mainNode = findMainContent(doc);
	if (!mainNode)
	{
		mainNode = body;
	}

	result.text = extractText(doc, mainNode);
	result.images = extractImages(doc, mainNode, baseUrl);
	result.links = extractLinks(doc, mainNode, baseUrl);

	return result;
}

auto ContentExtractor::removeUnwantedElements(LexborDocument& doc) const -> void
{
	auto* body = doc.body();
	if (!body)
	{
		return;
	}

	static const std::array unwantedTags = {
		std::string("script"), std::string("style"), std::string("noscript"),
		std::string("iframe"), std::string("svg"), std::string("form"),
		std::string("button"), std::string("input"), std::string("select"),
		std::string("textarea"),
	};

	for (const auto& tag : unwantedTags)
	{
		// Collect first, then remove (modifying tree while iterating is unsafe)
		auto nodes = doc.findByTag(body, tag);
		for (auto* node : nodes)
		{
			doc.removeNode(node);
		}
	}
}

auto ContentExtractor::removeBoilerplate(LexborDocument& doc) const -> void
{
	auto* body = doc.body();
	if (!body)
	{
		return;
	}

	// Remove boilerplate semantic tags
	static const std::array boilerplateTags = {
		std::string("nav"), std::string("header"), std::string("footer"),
		std::string("aside"), std::string("menu"),
	};

	for (const auto& tag : boilerplateTags)
	{
		auto nodes = doc.findByTag(body, tag);
		for (auto* node : nodes)
		{
			doc.removeNode(node);
		}
	}

	// Remove elements with boilerplate class/id patterns
	// We need to walk the tree and check class/id attributes
	std::vector<LexborDocument::Node> toRemove;

	std::vector<LexborDocument::Node> stack;
	stack.push_back(body);

	while (!stack.empty())
	{
		auto* node = stack.back();
		stack.pop_back();

		if (!doc.isElement(node))
		{
			continue;
		}

		auto tag = doc.tagName(node);

		// Never remove article or main elements
		if (tag == "article" || tag == "main")
		{
			// Still traverse children
			for (auto* child = doc.firstChild(node); child; child = doc.nextSibling(child))
			{
				stack.push_back(child);
			}
			continue;
		}

		auto nodeId = doc.id(node);
		auto nodeClass = doc.className(node);

		if (matchesBoilerplatePattern(nodeId) || matchesBoilerplatePattern(nodeClass))
		{
			toRemove.push_back(node);
			continue; // Don't traverse children of removed nodes
		}

		for (auto* child = doc.firstChild(node); child; child = doc.nextSibling(child))
		{
			stack.push_back(child);
		}
	}

	for (auto* node : toRemove)
	{
		doc.removeNode(node);
	}
}

auto ContentExtractor::findMainContent(LexborDocument& doc) const -> LexborDocument::Node
{
	auto* body = doc.body();
	if (!body)
	{
		return nullptr;
	}

	LexborDocument::Node bestNode = nullptr;
	int bestScore = 0;

	std::vector<LexborDocument::Node> stack;
	stack.push_back(body);

	while (!stack.empty())
	{
		auto* node = stack.back();
		stack.pop_back();

		if (!doc.isElement(node))
		{
			continue;
		}

		auto tag = doc.tagName(node);

		if (isBlockTag(tag))
		{
			int score = scoreNode(doc, node);
			if (score > bestScore)
			{
				bestScore = score;
				bestNode = node;
			}
		}

		for (auto* child = doc.firstChild(node); child; child = doc.nextSibling(child))
		{
			stack.push_back(child);
		}
	}

	return bestNode;
}

auto ContentExtractor::scoreNode(LexborDocument& doc, LexborDocument::Node node) const -> int
{
	auto text = doc.textContent(node);
	int textLen = static_cast<int>(text.size());
	int linkLen = linkTextLength(doc, node);

	int score = textLen - linkLen * 2;

	auto tag = doc.tagName(node);
	if (tag == "article") score += 100;
	else if (tag == "main") score += 80;

	auto nodeId = toLower(doc.id(node));
	auto nodeClass = toLower(doc.className(node));
	auto combined = nodeId + " " + nodeClass;

	static const std::array contentPatterns = {
		std::string("article"), std::string("content"), std::string("post"),
		std::string("entry"), std::string("story"),
	};

	for (const auto& pattern : contentPatterns)
	{
		if (combined.find(pattern) != std::string::npos)
		{
			score += 50;
			break;
		}
	}

	if (countChildElements(doc, node) < 3)
	{
		score -= 30;
	}

	return score;
}

auto ContentExtractor::extractText(LexborDocument& doc, LexborDocument::Node node) const -> std::string
{
	auto raw = doc.textContent(node);
	return normalizeWhitespace(raw);
}

auto ContentExtractor::extractImages(LexborDocument& doc, LexborDocument::Node node,
	const std::string& baseUrl) const -> std::vector<ScrapedImage>
{
	std::vector<ScrapedImage> images;

	auto imgNodes = doc.findByTag(node, "img");
	for (auto* img : imgNodes)
	{
		auto src = doc.attribute(img, "src");
		if (src.empty())
		{
			// Try data-src (lazy loading)
			src = doc.attribute(img, "data-src");
		}
		if (src.empty())
		{
			continue;
		}

		auto alt = doc.attribute(img, "alt");
		auto cls = doc.className(img);
		auto width = doc.attribute(img, "width");
		auto height = doc.attribute(img, "height");

		if (isFilteredImage(src, cls, alt, width, height))
		{
			continue;
		}

		auto resolved = resolveUrl(src, baseUrl);
		images.push_back({resolved, alt});
	}

	return images;
}

auto ContentExtractor::extractLinks(LexborDocument& doc, LexborDocument::Node node,
	const std::string& baseUrl) const -> std::vector<ScrapedLink>
{
	std::vector<ScrapedLink> links;

	auto aNodes = doc.findByTag(node, "a");
	for (auto* a : aNodes)
	{
		auto href = doc.attribute(a, "href");
		if (href.empty())
		{
			continue;
		}

		// Skip fragment-only and javascript links
		if (href[0] == '#' || href.substr(0, 11) == "javascript:")
		{
			continue;
		}

		auto text = normalizeWhitespace(doc.textContent(a));
		auto resolved = resolveUrl(href, baseUrl);
		links.push_back({resolved, text});
	}

	return links;
}

auto ContentExtractor::resolveUrl(const std::string& relative, const std::string& base) const -> std::string
{
	QUrl baseUrl(QString::fromStdString(base));
	QUrl resolved = baseUrl.resolved(QUrl(QString::fromStdString(relative)));
	return resolved.toString().toStdString();
}

auto ContentExtractor::matchesBoilerplatePattern(const std::string& classOrId) const -> bool
{
	if (classOrId.empty())
	{
		return false;
	}

	auto lower = toLower(classOrId);

	static const std::array patterns = {
		std::string("nav"), std::string("menu"), std::string("sidebar"),
		std::string("footer"), std::string("header"), std::string("breadcrumb"),
		std::string("social"), std::string("share"), std::string("comment"),
		std::string("widget"), std::string("advert"), std::string("banner"),
		std::string("cookie"), std::string("popup"), std::string("modal"),
	};

	for (const auto& pattern : patterns)
	{
		if (lower.find(pattern) != std::string::npos)
		{
			return true;
		}
	}

	return false;
}

}
