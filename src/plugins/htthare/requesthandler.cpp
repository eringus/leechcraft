/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "requesthandler.h"
#include <sys/sendfile.h>
#include <errno.h>
#include <QList>
#include <QString>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <util/util.h>
#include "connection.h"
#include "storagemanager.h"

namespace LeechCraft
{
namespace HttThare
{
	RequestHandler::RequestHandler (const Connection_ptr& conn)
	: Conn_ (conn)
	{
	}

	void RequestHandler::operator() (QByteArray data)
	{
		data.replace ("\r", "");

		auto lines = data.split ('\n');
		for (auto& line : lines)
			line = line.trimmed ();
		lines.removeAll ({});

		if (lines.size () <= 0)
			return ErrorResponse (400, "Bad Request");

		const auto& req = lines.takeAt (0).split (' ');
		if (req.size () < 2)
			return ErrorResponse (400, "Bad Request");

		const auto& verb = req.at (0).toLower ();
		Url_ = QUrl::fromEncoded (req.at (1).mid (1));

		for (const auto& line : lines)
		{
			const auto colonPos = line.indexOf (':');
			if (colonPos <= 0)
				return ErrorResponse (400, "Bad Request");
			Headers_ [line.left (colonPos)] = line.mid (colonPos + 1).trimmed ();
		}

		if (verb == "head")
			HandleHead ();
		else if (verb == "get")
			HandleGet ();
		else
			return ErrorResponse (405, "Method Not Allowed",
					"Method " + verb + " not supported by this server.");
	}

	void RequestHandler::ErrorResponse (int code,
			const QByteArray& reason, const QByteArray& full)
	{
		ResponseLine_ = "HTTP/1.1 " + QByteArray::number (code) + " " + reason + "\r\n";

		ResponseBody_ = QString (R"delim(<html>
				<head><title>%1 %2</title></head>
				<body>
					<h1>%1 %2</h1>
					%3
				</body>
			</html>
			)delim")
				.arg (code)
				.arg (reason.data ())
				.arg (full.data ()).toUtf8 ();

		auto c = Conn_;
		boost::asio::async_write (c->GetSocket (),
				ToBuffers (),
				c->GetStrand ().wrap ([c] (const boost::system::error_code& ec, ulong)
					{
						if (ec)
							qWarning () << Q_FUNC_INFO
									<< ec.message ().c_str ();

						boost::system::error_code iec;
						c->GetSocket ().shutdown (boost::asio::socket_base::shutdown_both, iec);
					}));
	}

	namespace
	{
		QByteArray MakeDirResponse (const QFileInfo& fi, const QString& path)
		{
			QStringList rows;
			for (const auto& item : QDir { path }
					.entryInfoList (QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name))
				rows << QString { "<tr><td>%1</td><td>%2</td><td>%3</td></tr>" }
						.arg (item.fileName ())
						.arg (Util::MakePrettySize (item.size ()))
						.arg (item.created ().toString ());

			return QString (R"delim(<html>
					<head><title>%1</title></head>
					<body>
						<table>
						%2
						</table>
					</body>
				</html>
				)delim")
					.arg (fi.fileName ())
					.arg (rows.join (""))
					.toUtf8 ();
		}
	}

	void RequestHandler::HandleGet ()
	{
		const auto& path = Conn_->GetStorageManager ().ResolvePath (Url_);
		const QFileInfo fi { path };
		if (!fi.exists ())
		{
			ResponseLine_ = "HTTP/1.1 404 Not found\r\n";

			ResponseHeaders_.append ({ "Content-Type", "text/html; charset=utf-8" });
			ResponseBody_ = QString (R"delim(<html>
					<head><title>%1</title></head>
					<body>
						%2
					</body>
				</html>
				)delim")
					.arg (fi.fileName ())
					.arg (QObject::tr ("%1 is not found on this server").arg (path))
					.toUtf8 ();

			auto c = Conn_;
			boost::asio::async_write (c->GetSocket (),
					ToBuffers (),
					c->GetStrand ().wrap ([c] (const boost::system::error_code& ec, ulong)
						{
							if (ec)
								qWarning () << Q_FUNC_INFO
										<< ec.message ().c_str ();

							boost::system::error_code iec;
							c->GetSocket ().shutdown (boost::asio::socket_base::shutdown_both, iec);
						}));
		}
		else if (fi.isDir ())
		{
			ResponseLine_ = "HTTP/1.1 200 OK\r\n";

			ResponseBody_ = MakeDirResponse (fi, path);
			ResponseHeaders_.append ({ "Content-Type", "text/html; charset=utf-8" });

			auto c = Conn_;
			boost::asio::async_write (c->GetSocket (),
					ToBuffers (),
					c->GetStrand ().wrap ([c] (const boost::system::error_code& ec, ulong)
						{
							if (ec)
								qWarning () << Q_FUNC_INFO
										<< ec.message ().c_str ();

							boost::system::error_code iec;
							c->GetSocket ().shutdown (boost::asio::socket_base::shutdown_both, iec);
						}));
		}
		else
		{
			ResponseLine_ = "HTTP/1.1 200 OK\r\n";

			ResponseHeaders_.append ({ "Content-Type", "application/octet-stream" });
			ResponseHeaders_.append ({ "Content-Length", QByteArray::number (fi.size ()) });

			auto c = Conn_;
			boost::asio::async_write (c->GetSocket (),
					ToBuffers (),
					c->GetStrand ().wrap ([c, path] (const boost::system::error_code& ec, ulong)
						{
							if (ec)
								qWarning () << Q_FUNC_INFO
										<< ec.message ().c_str ();

							auto& s = c->GetSocket ();

							QFile file (path);
							file.open (QIODevice::ReadOnly);
							qDebug () << "sendfile()" << s.native_handle () << file.handle ();
							const auto rc = sendfile (s.native_handle (),
									file.handle (), nullptr, file.size ());
							if (rc == -1)
								qWarning () << Q_FUNC_INFO
										<< "sendfile() error:"
										<< errno
										<< "; human-readable:"
										<< strerror (errno);

							boost::system::error_code iec;
							s.shutdown (boost::asio::socket_base::shutdown_both, iec);
						}));
		}
	}

	void RequestHandler::HandleHead ()
	{
	}

	namespace
	{
		boost::asio::const_buffer BA2Buffer (const QByteArray& ba)
		{
			return { ba.constData (), static_cast<size_t> (ba.size ()) };
		}
	}

	std::vector<boost::asio::const_buffer> RequestHandler::ToBuffers ()
	{
		std::vector<boost::asio::const_buffer> result;

		if (std::find_if (ResponseHeaders_.begin (), ResponseHeaders_.end (),
				[] (decltype (ResponseHeaders_.at (0)) pair)
					{ return pair.first.toLower () == "content-length"; }) == ResponseHeaders_.end ())
			ResponseHeaders_.append ({ "Content-Length", QByteArray::number (ResponseBody_.size ()) });

		CookedRH_.clear ();
		for (const auto& pair : ResponseHeaders_)
			CookedRH_ += pair.first + ": " + pair.second + "\r\n";
		CookedRH_ += "\r\n";

		result.push_back (BA2Buffer (ResponseLine_));
		result.push_back (BA2Buffer (CookedRH_));
		result.push_back (BA2Buffer (ResponseBody_));

		return result;
	}
}
}
