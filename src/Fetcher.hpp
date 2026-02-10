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

#ifndef FETCHER_H
#define FETCHER_H

#include <string>

namespace CppScrap
{

class Fetcher final
{
public:
	/// @brief Result of an HTTP fetch operation.
	struct Result
	{
		int statusCode = 0;        // HTTP status code (0 if request failed).
		std::string body;          // Response body content.
		std::string finalUrl;      // Final URL after redirects.
		std::string contentType;   // Content-Type header value.
		std::string error;         // Error message, empty on success.

		/// @brief Returns true if no error and status is 2xx or 3xx.
		auto ok() const -> bool { return error.empty() && statusCode >= 200 && statusCode < 400; }
	};

	/// @brief Download a URL via HTTP with redirect following and timeout.
	auto fetch(const std::string& url) -> Result;

	/// @brief Set the HTTP User-Agent header string.
	auto setUserAgent(const std::string& ua) -> void { m_userAgent = ua; }
	/// @brief Set the request timeout in milliseconds.
	auto setTimeout(int msec) -> void { m_timeout = msec; }
	/// @brief Enable or disable SSL certificate verification.
	auto setIgnoreSslErrors(bool ignore) -> void { m_ignoreSslErrors = ignore; }

private:
	std::string m_userAgent = "CppOpenScraper/0.1";
	int m_timeout = 30000;
	bool m_ignoreSslErrors = true;
};

}

#endif // FETCHER_H
