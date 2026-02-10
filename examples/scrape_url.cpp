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

Example: Scrape a URL and print the extracted content.
Usage: scrape_url <URL>
*/

#include "Scraper.hpp"

#include <QCoreApplication>

#include <iostream>

auto main(int argc, char* argv[]) -> int
{
	QCoreApplication app(argc, argv);

	if (argc < 2)
	{
		std::cerr << "Usage: scrape_url <URL>\n";
		return 1;
	}

	std::string url = argv[1];

	CppScrap::Scraper scraper;
	auto page = scraper.scrape(url);

	if (!page.ok())
	{
		std::cerr << "Error: " << page.error << "\n";
		return 1;
	}

	std::cout << "URL: " << page.url << "\n";
	std::cout << "Title: " << page.title << "\n\n";

	std::cout << "--- Text (first 500 chars) ---\n";
	if (page.text.size() > 500)
	{
		std::cout << page.text.substr(0, 500) << "...\n";
	}
	else
	{
		std::cout << page.text << "\n";
	}

	std::cout << "\n--- Images (" << page.images.size() << " total) ---\n";
	size_t imgLimit = std::min(page.images.size(), size_t(5));
	for (size_t i = 0; i < imgLimit; ++i)
	{
		std::cout << "  " << page.images[i].url;
		if (!page.images[i].alt.empty())
		{
			std::cout << "  [" << page.images[i].alt << "]";
		}
		std::cout << "\n";
	}
	if (page.images.size() > 5)
	{
		std::cout << "  ... and " << (page.images.size() - 5) << " more\n";
	}

	std::cout << "\n--- Links (" << page.links.size() << " total) ---\n";
	size_t linkLimit = std::min(page.links.size(), size_t(10));
	for (size_t i = 0; i < linkLimit; ++i)
	{
		std::cout << "  " << page.links[i].url;
		if (!page.links[i].text.empty())
		{
			std::cout << "  [" << page.links[i].text << "]";
		}
		std::cout << "\n";
	}
	if (page.links.size() > 10)
	{
		std::cout << "  ... and " << (page.links.size() - 10) << " more\n";
	}

	return 0;
}
