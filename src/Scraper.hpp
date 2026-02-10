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

#ifndef SCRAPER_H
#define SCRAPER_H

#include "ScrapedPage.hpp"
#include "Fetcher.hpp"

#include <string>

namespace CppScrap
{

class Scraper final
{
public:
	Scraper() = default;
	~Scraper() = default;

	/// @brief Fetch a URL, parse HTML, and extract content into a ScrapedPage.
	auto scrape(const std::string& url) -> ScrapedPage;

	/// @brief Set the HTTP User-Agent header string.
	auto setUserAgent(const std::string& ua) -> void { m_fetcher.setUserAgent(ua); }
	/// @brief Set the request timeout in milliseconds.
	auto setTimeout(int msec) -> void { m_fetcher.setTimeout(msec); }
	/// @brief Enable or disable SSL certificate verification.
	auto setIgnoreSslErrors(bool ignore) -> void { m_fetcher.setIgnoreSslErrors(ignore); }

private:
	Fetcher m_fetcher;
};

}

#endif // SCRAPER_H
