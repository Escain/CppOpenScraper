/*
CppOpenScraper - C++ Web Scraping Library
Copyright (C) 2025 Adrian Maire (escain@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.
*/

#include "UnitTest.hpp"

#include "ContentExtractor.hpp"
#include "LexborDocument.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace
{

auto readFixture(const std::string& name) -> std::string
{
	std::string path = std::string(FIXTURE_DIR) + "/" + name;
	std::ifstream file(path);
	if (!file)
	{
		throw std::runtime_error("Cannot open fixture: " + path);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

auto extractMarkdown(const std::string& html,
	const std::string& url) -> std::string
{
	CppScrap::LexborDocument doc;
	if (!doc.parse(html))
	{
		throw std::runtime_error("Failed to parse HTML");
	}

	CppScrap::ContentExtractor extractor;
	auto content = extractor.extract(doc, url);
	return content.text;
}

auto contains(const std::string& haystack,
	const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

} // anonymous namespace

// ===== Marketbeat: content preservation =====

UNIT_TEST(Marketbeat_ContainsEPS)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	ASSERT(contains(md, "reporting an EPS"));
}

UNIT_TEST(Marketbeat_ContainsRevenue)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	ASSERT(contains(md, "$40.71"));
}

UNIT_TEST(Marketbeat_ContainsQuarter)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	ASSERT(contains(md, "Q3 2026"));
}

UNIT_TEST(Marketbeat_ContainsTable)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	ASSERT(contains(md, "Reported EPS"));
	ASSERT(contains(md, "|"));
}

// ===== Anime-planet: content preservation =====

UNIT_TEST(AnimePlanet_ContainsTitle)
{
	auto html = readFixture("anime_planet.html");
	auto md = extractMarkdown(html,
		"https://www.anime-planet.com/anime/yona-of-the-dawn");

	ASSERT(contains(md, "Yona of the Dawn"));
}

UNIT_TEST(AnimePlanet_ContainsDescription)
{
	auto html = readFixture("anime_planet.html");
	auto md = extractMarkdown(html,
		"https://www.anime-planet.com/anime/yona-of-the-dawn");

	// The anime description should survive extraction
	ASSERT(contains(md, "Yona"));
	ASSERT(contains(md, "Princess Yona"));
}

// ===== Wikipedia: content preservation =====

UNIT_TEST(Wikipedia_ContainsCpp)
{
	auto html = readFixture("wikipedia_cpp.html");
	auto md = extractMarkdown(html,
		"https://en.wikipedia.org/wiki/C%2B%2B");

	ASSERT(contains(md, "C++"));
}

UNIT_TEST(Wikipedia_ContainsAuthor)
{
	auto html = readFixture("wikipedia_cpp.html");
	auto md = extractMarkdown(html,
		"https://en.wikipedia.org/wiki/C%2B%2B");

	ASSERT(contains(md, "Bjarne Stroustrup"));
}

UNIT_TEST(Wikipedia_ContainsSections)
{
	auto html = readFixture("wikipedia_cpp.html");
	auto md = extractMarkdown(html,
		"https://en.wikipedia.org/wiki/C%2B%2B");

	ASSERT(contains(md, "History"));
	ASSERT(contains(md, "#"));
}

// ===== Yahoo Finance: content preservation =====

UNIT_TEST(YahooFinance_ContainsAAPL)
{
	auto html = readFixture("yahoo_finance_aapl.html");
	auto md = extractMarkdown(html,
		"https://finance.yahoo.com/quote/AAPL/");

	ASSERT(contains(md, "AAPL"));
}

UNIT_TEST(YahooFinance_ContainsPrice)
{
	auto html = readFixture("yahoo_finance_aapl.html");
	auto md = extractMarkdown(html,
		"https://finance.yahoo.com/quote/AAPL/");

	// Should contain some dollar amount
	ASSERT(contains(md, "$"));
}

// ===== Hacker News: content preservation =====

UNIT_TEST(HackerNews_ContainsStories)
{
	auto html = readFixture("hacker_news.html");
	auto md = extractMarkdown(html,
		"https://news.ycombinator.com");

	// Should have story links and points
	ASSERT(contains(md, "points"));
	ASSERT(contains(md, "comments"));
}

// ===== Size regression tests =====
// Lower bound: content must not shrink below threshold (content lost).
// Upper bound: soft check, output should not grow excessively (noise).
// Sizes based on research output ± 30% margin.

UNIT_TEST(Size_Marketbeat)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	size_t expected = 15600;
	size_t minSize = expected * 70 / 100;
	size_t maxSize = expected * 200 / 100;

	ASSERT(md.size() >= minSize);
	ASSERT(md.size() <= maxSize);
}

UNIT_TEST(Size_AnimePlanet)
{
	auto html = readFixture("anime_planet.html");
	auto md = extractMarkdown(html,
		"https://www.anime-planet.com/anime/yona-of-the-dawn");

	size_t expected = 55700;
	size_t minSize = expected * 70 / 100;
	size_t maxSize = expected * 200 / 100;

	ASSERT(md.size() >= minSize);
	ASSERT(md.size() <= maxSize);
}

UNIT_TEST(Size_Wikipedia)
{
	auto html = readFixture("wikipedia_cpp.html");
	auto md = extractMarkdown(html,
		"https://en.wikipedia.org/wiki/C%2B%2B");

	size_t expected = 570000;
	size_t minSize = expected * 70 / 100;
	size_t maxSize = expected * 200 / 100;

	ASSERT(md.size() >= minSize);
	ASSERT(md.size() <= maxSize);
}

UNIT_TEST(Size_YahooFinance)
{
	auto html = readFixture("yahoo_finance_aapl.html");
	auto md = extractMarkdown(html,
		"https://finance.yahoo.com/quote/AAPL/");

	size_t expected = 20600;
	size_t minSize = expected * 70 / 100;
	size_t maxSize = expected * 200 / 100;

	ASSERT(md.size() >= minSize);
	ASSERT(md.size() <= maxSize);
}

UNIT_TEST(Size_HackerNews)
{
	auto html = readFixture("hacker_news.html");
	auto md = extractMarkdown(html,
		"https://news.ycombinator.com");

	size_t expected = 33000;
	size_t minSize = expected * 70 / 100;
	size_t maxSize = expected * 200 / 100;

	ASSERT(md.size() >= minSize);
	ASSERT(md.size() <= maxSize);
}

// ===== Format tests =====

UNIT_TEST(Format_ContainsMarkdownHeadings)
{
	auto html = readFixture("wikipedia_cpp.html");
	auto md = extractMarkdown(html,
		"https://en.wikipedia.org/wiki/C%2B%2B");

	ASSERT(contains(md, "# ") || contains(md, "## "));
}

UNIT_TEST(Format_ContainsMarkdownLinks)
{
	auto html = readFixture("hacker_news.html");
	auto md = extractMarkdown(html,
		"https://news.ycombinator.com");

	ASSERT(contains(md, "]("));
}

UNIT_TEST(Format_ContainsMarkdownTables)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	ASSERT(contains(md, "|"));
	ASSERT(contains(md, "---"));
}

UNIT_TEST(Format_NoScriptTags)
{
	auto html = readFixture("marketbeat_baba.html");
	auto md = extractMarkdown(html,
		"https://www.marketbeat.com/stocks/NYSE/BABA/earnings/");

	ASSERT_FALSE(contains(md, "<script"));
}

UNIT_TEST(Format_NoStyleTags)
{
	auto html = readFixture("wikipedia_cpp.html");
	auto md = extractMarkdown(html,
		"https://en.wikipedia.org/wiki/C%2B%2B");

	ASSERT_FALSE(contains(md, "<style"));
}
