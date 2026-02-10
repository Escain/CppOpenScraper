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

#include "Scraper.hpp"
#include "LexborDocument.hpp"
#include "ContentExtractor.hpp"

namespace CppScrap
{

auto Scraper::scrape(const std::string& url) -> ScrapedPage
{
	ScrapedPage page;
	page.url = url;

	auto fetchResult = m_fetcher.fetch(url);
	if (!fetchResult.ok())
	{
		page.error = fetchResult.error;
		return page;
	}

	page.url = fetchResult.finalUrl;

	LexborDocument doc;
	if (!doc.parse(fetchResult.body))
	{
		page.error = "Failed to parse HTML";
		return page;
	}

	page.title = doc.title();

	ContentExtractor extractor;
	auto content = extractor.extract(doc, page.url);

	page.text = std::move(content.text);
	page.images = std::move(content.images);
	page.links = std::move(content.links);

	return page;
}

}
