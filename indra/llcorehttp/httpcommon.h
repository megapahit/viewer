/**
 * @file httpcommon.h
 * @brief Public-facing declarations and definitions of common types
 *
 * $LicenseInfo:firstyear=2012&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2012, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef	_LLCORE_HTTP_COMMON_H_
#define	_LLCORE_HTTP_COMMON_H_

/// @package LLCore::HTTP
///
/// This library implements a high-level, Indra-code-free client interface to
/// HTTP services based on actual patterns found in the viewer and simulator.
/// Interfaces are similar to those supplied by the legacy classes
/// LLCurlRequest and LLHTTPClient.  To that is added a policy scheme that
/// allows an application to specify connection behaviors:  limits on
/// connections, HTTP keepalive, HTTP pipelining, retry-on-error limits, etc.
///
/// Features of the library include:
/// - Single, private working thread where all transport and processing occurs.
/// - Support for multiple consumers running in multiple threads.
/// - Scatter/gather (a.k.a. buffer array) model for bulk data movement.
/// - Reference counting used for many object instance lifetimes.
/// - Minimal data sharing across threads for correctness and low latency.
/// 
/// The public interface is declared in a few key header files:
/// - <core-http/bufferarray.h>
/// - <core-http/httpcommon.h>
/// - <core-http/httphandler.h>
/// - <core-http/httpheaders.h>
/// - <core-http/httpoptions.h>
/// - <core-http/httprequest.h>
/// - <core-http/httpresponse.h>
///
/// The library is still under early development and particular users
/// may need access to internal implementation details that are found
/// in the _*.h header files.  But this is a crutch to be avoided if at
/// all possible and probably indicates some interface work is neeeded.
///
/// Using the library is fairly easy.  Global setup needs a few
/// steps:
///
/// - libcurl initialization with thread-safely callbacks for c-ares
///   DNS lookups.
/// - HttpRequest::createService() called to instantiate singletons
///   and support objects.
///
/// An HTTP consumer in an application, and an application may have many
/// consumers, does a few things:
///
/// - Instantiate and retain an object based on HttpRequest.  This
///   object becomes the portal into runtime services for the consumer.
/// - Derive or mixin the HttpHandler class if you want notification
///   when requests succeed or fail.  This object's onCompleted()
///   method is invoked and an instance can be shared across
///   requests.
///
/// Issuing a request is straightforward:
/// - Construct a suitable URL.
/// - Configure HTTP options for the request.  (optional)
/// - Build a list of additional headers.  (optional)
/// - Invoke one of the requestXXXX() methods (requestGetByteRange,
///   requestPost, etc.) on the HttpRequest instance supplying the
///   above along with a policy class, a priority and an optional
///   pointer to an HttpHandler instance.  Work is then queued to
///   the worker thread and occurs asynchronously.
/// - Periodically invoke the update() method on the HttpRequest
///   instance which performs completion notification to HttpHandler
///   objects.
/// - Do completion processing in your onCompletion() method.
///
/// Code fragments:
/// <TBD>
///

#include <string>


namespace LLCore
{


/// All queued requests are represented by an HttpHandle value.
/// The invalid value is returned when a request failed to queue.
/// The actual status for these failures is then fetched with
/// HttpRequest::getStatus().
///
/// The handle is valid only for the life of a request.  On
/// return from any HttpHandler notification, the handle immediately
/// becomes invalid and may be recycled for other queued requests.

typedef void * HttpHandle;
#define LLCORE_HTTP_HANDLE_INVALID		(NULL)


/// Error codes defined by the library itself as distinct from
/// libcurl (or any other transport provider).
enum HttpError
{
	// Successful value compatible with the libcurl codes.
	HE_SUCCESS = 0,

	// Service is shutting down and requested operation will
	// not be queued or performed.
	HE_SHUTTING_DOWN = 1,
	
	// Operation was canceled by request.
	HE_OP_CANCELED = 2,
	
	// Invalid content range header received.
	HE_INV_CONTENT_RANGE_HDR = 3
	
}; // end enum HttpError


/// HttpStatus encapsulates errors from libcurl (easy, multi) as well as
/// internal errors.  The encapsulation isn't expected to completely
/// isolate the caller from libcurl but basic operational tests (success
/// or failure) are provided.
struct HttpStatus
{
	typedef unsigned short type_enum_t;
	
	HttpStatus()
		: mType(LLCORE),
		  mStatus(HE_SUCCESS)
		{}

	HttpStatus(type_enum_t type, short status)
		: mType(type),
		  mStatus(status)
		{}
	
	HttpStatus(const HttpStatus & rhs)
		: mType(rhs.mType),
		  mStatus(rhs.mStatus)
		{}

	HttpStatus & operator=(const HttpStatus & rhs)
		{
			// Don't care if lhs & rhs are the same object

			mType = rhs.mType;
			mStatus = rhs.mStatus;
			return *this;
		}
	
	static const type_enum_t EXT_CURL_EASY = 0;
	static const type_enum_t EXT_CURL_MULTI = 1;
	static const type_enum_t LLCORE = 2;
	
	type_enum_t			mType;
	short				mStatus;

	/// Test for successful status in the code regardless
	/// of error source (internal, libcurl).
	///
	/// @return			'true' when status is successful.
	///
	operator bool() const
	{
		return 0 == mStatus;
	}

	/// Inverse of previous operator.
	///
	/// @return			'true' on any error condition
	bool operator !() const
	{
		return 0 != mStatus;
	}

	/// Convert status to a string representation.  For
	/// success, returns an empty string.  For failure
	/// statuses, a string as appropriate for the source of
	/// the error code (libcurl easy, libcurl multi, or
	/// LLCore itself).
	std::string toString() const;
	
}; // end struct HttpStatus

}  // end namespace LLCore

#endif	// _LLCORE_HTTP_COMMON_H_
