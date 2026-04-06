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

#include <html2md.h>

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
	removeAdElements(doc);

	// Serialize cleaned DOM to HTML, then convert to markdown.
	auto cleanedHtml = doc.serializeHtml(body);
	html2md::Options opts;
	opts.splitLines = false;
	html2md::Converter converter(cleanedHtml, &opts);
	result.text = converter.convert();

	result.images = extractImages(doc, body, baseUrl);
	result.links = extractLinks(doc, body, baseUrl);

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
		std::string("iframe"),
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

	// Remove only unambiguous semantic boilerplate tags.
	// Class/id pattern matching is intentionally omitted:
	// substring matching on class names is too unreliable
	// and removes real content on many sites.
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
}

auto ContentExtractor::removeAdElements(LexborDocument& doc) const -> void
{
	auto* body = doc.body();
	if (!body)
	{
		return;
	}

	// Walk the DOM tree and collect elements that are ads/sponsored.
	// Uses standard HTML attributes, not class name heuristics.
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

		bool isAd = false;

		// 1. Links with rel="sponsored"
		auto rel = doc.attribute(node, "rel");
		if (rel.find("sponsored") != std::string::npos)
		{
			isAd = true;
		}

		// 2. Google AdSense: <ins class="adsbygoogle">
		auto tag = doc.tagName(node);
		auto cls = doc.className(node);
		if (tag == "ins" && cls.find("adsbygoogle") != std::string::npos)
		{
			isAd = true;
		}

		// 3. Elements with data-ad* attributes
		if (!isAd && !doc.attribute(node, "data-ad").empty())
		{
			isAd = true;
		}
		if (!isAd && !doc.attribute(node, "data-ad-slot").empty())
		{
			isAd = true;
		}
		if (!isAd && !doc.attribute(node, "data-ad-client").empty())
		{
			isAd = true;
		}

		// 4. Modal dialogs (login, upgrade, share overlays)
		if (!isAd && doc.attribute(node, "role") == "dialog")
		{
			isAd = true;
		}

		// 5. Cookie/consent management overlays
		auto nodeId = doc.id(node);
		if (!isAd && (nodeId.find("cookie") != std::string::npos
			|| nodeId.find("consent") != std::string::npos
			|| nodeId.find("cmp") != std::string::npos))
		{
			isAd = true;
		}

		if (isAd)
		{
			toRemove.push_back(node);
			continue;
		}

		for (auto* child = doc.firstChild(node); child;
			child = doc.nextSibling(child))
		{
			stack.push_back(child);
		}
	}

	for (auto* node : toRemove)
	{
		doc.removeNode(node);
	}
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


}
