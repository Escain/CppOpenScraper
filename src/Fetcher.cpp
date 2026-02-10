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

#include "Fetcher.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QTimer>
#include <QUrl>

namespace CppScrap
{

auto Fetcher::fetch(const std::string& url) -> Result
{
	Result result;

	QUrl qurl(QString::fromStdString(url));
	if (!qurl.isValid())
	{
		result.error = "Invalid URL: " + url;
		return result;
	}

	QNetworkAccessManager manager;
	QNetworkRequest request(qurl);
	request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromStdString(m_userAgent));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
		static_cast<int>(QNetworkRequest::NoLessSafeRedirectPolicy));
	request.setMaximumRedirectsAllowed(10);

	// Explicitly load system CA certificates so SSL verification works
	auto sslConfig = QSslConfiguration::defaultConfiguration();
	sslConfig.setCaCertificates(QSslConfiguration::systemCaCertificates());
	request.setSslConfiguration(sslConfig);

	QEventLoop loop;
	QTimer timer;
	timer.setSingleShot(true);

	auto* reply = manager.get(request);

	if (m_ignoreSslErrors)
	{
		QObject::connect(reply, &QNetworkReply::sslErrors,
			reply, qOverload<const QList<QSslError>&>(&QNetworkReply::ignoreSslErrors));
	}

	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

	timer.start(m_timeout);
	loop.exec();

	if (timer.isActive())
	{
		timer.stop();
	}
	else
	{
		reply->abort();
		result.error = "Request timed out";
		reply->deleteLater();
		return result;
	}

	if (reply->error() != QNetworkReply::NoError)
	{
		result.error = reply->errorString().toStdString();
		result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		reply->deleteLater();
		return result;
	}

	result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	result.finalUrl = reply->url().toString().toStdString();
	result.contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toStdString();

	auto data = reply->readAll();
	result.body = std::string(data.constData(), data.size());

	reply->deleteLater();
	return result;
}

}
