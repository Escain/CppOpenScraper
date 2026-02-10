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

#ifndef SCRAPED_PAGE_H
#define SCRAPED_PAGE_H

#include <string>
#include <vector>

namespace CppScrap
{

/// @brief Image extracted from a web page.
struct ScrapedImage
{
	std::string url;  // Resolved absolute URL of the image.
	std::string alt;  // Alt text of the image.
};

/// @brief Hyperlink extracted from a web page.
struct ScrapedLink
{
	std::string url;  // Resolved absolute URL of the link target.
	std::string text; // Visible anchor text of the link.
};

/// @brief Result of scraping a web page: text, images, links and metadata.
struct ScrapedPage
{
	std::string url;                    // Final URL after redirects.
	std::string title;                  // Page title from the HTML <title> tag.
	std::string text;                   // Extracted main text content, whitespace-normalized.
	std::vector<ScrapedImage> images;   // Content images found on the page.
	std::vector<ScrapedLink> links;     // Hyperlinks found on the page.
	std::string error;                  // Error message, empty on success.

	/// @brief Returns true if scraping succeeded (no error).
	auto ok() const -> bool { return error.empty(); }
};

}

#endif // SCRAPED_PAGE_H
