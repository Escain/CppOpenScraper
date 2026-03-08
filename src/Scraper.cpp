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

#ifdef HAS_POPPLER
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#endif

namespace CppScrap
{

auto Scraper::fetchRaw(const std::string& url) -> Fetcher::Result
{
	return m_fetcher.fetch(url);
}

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

#ifdef HAS_POPPLER
	if (fetchResult.contentType.find("application/pdf") != std::string::npos)
	{
		auto doc = poppler::document::load_from_raw_data(fetchResult.body.data(),
			static_cast<int>(fetchResult.body.size()));
		if (!doc)
		{
			page.error = "Failed to parse PDF document";
			return page;
		}

		auto titleBytes = doc->get_title().to_utf8();
		page.title = std::string(titleBytes.data(), titleBytes.size());

		for (int i = 0; i < doc->pages(); i++)
		{
			auto p = doc->create_page(i);
			if (p)
			{
				auto textBytes = p->text().to_utf8();
				page.text += std::string(textBytes.data(), textBytes.size());
				page.text += "\n";
			}
		}

		return page;
	}
#endif

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
