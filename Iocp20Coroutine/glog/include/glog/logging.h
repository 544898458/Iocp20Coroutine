// Copyright (c) 2024, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Ray Sidney
//
// This file contains #include information about logging-related stuff.
// Pretty much everybody needs to #include this file so that they can
// log various happenings.
//
#ifndef GLOG_LOGGING_H
#define GLOG_LOGGING_H

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iosfwd>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#if defined(GLOG_USE_GLOG_EXPORT)
#  include "glog/export.h"
#endif

#if !defined(GLOG_EXPORT) || !defined(GLOG_NO_EXPORT)
#  error <glog/logging.h> was not included correctly. See the documention for how to consume the library.
#endif

#include "glog/flags.h"
#include "glog/platform.h"
#include "glog/types.h"

#if defined(__has_attribute)
#  if __has_attribute(used)
#    define GLOG_USED __attribute__((used))
#  endif  // __has_attribute(used)
#endif    // defined(__has_attribute)

#if !defined(GLOG_USED)
#  define GLOG_USED
#endif  // !defined(GLOG_USED)

#include "glog/log_severity.h"
#include "glog/vlog_is_on.h"

namespace google {

struct GLOG_EXPORT LogMessageTime {
  LogMessageTime();
  explicit LogMessageTime(std::chrono::system_clock::time_point now);

  [[deprecated("Use LogMessageTime::when() instead.")]] std::time_t timestamp()
      const noexcept {
    return std::chrono::system_clock::to_time_t(when());
  }
  const std::chrono::system_clock::time_point& when() const noexcept {
    return timestamp_;
  }
  int sec() const noexcept { return tm_.tm_sec; }
  long usec() const noexcept { return usecs_.count(); }
  int(min)() const noexcept { return tm_.tm_min; }
  int hour() const noexcept { return tm_.tm_hour; }
  int day() const noexcept { return tm_.tm_mday; }
  int month() const noexcept { return tm_.tm_mon; }
  int year() const noexcept { return tm_.tm_year; }
  int dayOfWeek() const noexcept { return tm_.tm_wday; }
  int dayInYear() const noexcept { return tm_.tm_yday; }
  int dst() const noexcept { return tm_.tm_isdst; }
  [[deprecated("Use LogMessageTime::gmtoffset() instead.")]] long gmtoff()
      const noexcept {
    return gmtoffset_.count();
  }
  std::chrono::seconds gmtoffset() const noexcept { return gmtoffset_; }
  const std::tm& tm() const noexcept { return tm_; }

 private:
  std::tm tm_{};  // Time of creation of LogMessage
  std::chrono::system_clock::time_point
      timestamp_;  // Time of creation of LogMessage in seconds
  std::chrono::microseconds usecs_;
  std::chrono::seconds gmtoffset_;
};

struct [[deprecated("Use LogMessage instead.")]] LogMessageInfo {
  explicit LogMessageInfo(const char* const severity_,
                          const char* const filename_, const int& line_number_,
                          std::thread::id thread_id_,
                          const LogMessageTime& time_)
      : severity(severity_),
        filename(filename_),
        line_number(line_number_),
        thread_id(thread_id_),
        time(time_) {}

  const char* const severity;
  const char* const filename;
  const int& line_number;
  std::thread::id thread_id;
  const LogMessageTime& time;
};

}  // namespace google

// The global value of GOOGLE_STRIP_LOG. All the messages logged to
// LOG(XXX) with severity less than GOOGLE_STRIP_LOG will not be displayed.
// If it can be determined at compile time that the message will not be
// printed, the statement will be compiled out.
//
// Example: to strip out all INFO and WARNING messages, use the value
// of 2 below. To make an exception for WARNING messages from a single
// file, add "#define GOOGLE_STRIP_LOG 1" to that file _before_ including
// base/logging.h
#ifndef GOOGLE_STRIP_LOG
#  define GOOGLE_STRIP_LOG 0
#endif

// GCC can be told that a certain branch is not likely to be taken (for
// instance, a CHECK failure), and use that information in static analysis.
// Giving it this information can help it optimize for the common case in
// the absence of better information (ie. -fprofile-arcs).
//
#if defined(__has_builtin)
#  if __has_builtin(__builtin_expect)
#    define GLOG_BUILTIN_EXPECT_PRESENT
#  endif
#endif

#if !defined(GLOG_BUILTIN_EXPECT_PRESENT) && defined(__GNUG__)
// __has_builtin is not available prior to GCC 10
#  define GLOG_BUILTIN_EXPECT_PRESENT
#endif

#if defined(GLOG_BUILTIN_EXPECT_PRESENT)

#  ifndef GOOGLE_PREDICT_BRANCH_NOT_TAKEN
#    define GOOGLE_PREDICT_BRANCH_NOT_TAKEN(x) (__builtin_expect(x, 0))
#  endif

#  ifndef GOOGLE_PREDICT_FALSE
#    define GOOGLE_PREDICT_FALSE(x) (__builtin_expect(x, 0))
#  endif

#  ifndef GOOGLE_PREDICT_TRUE
#    define GOOGLE_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#  endif

#else

#  ifndef GOOGLE_PREDICT_BRANCH_NOT_TAKEN
#    define GOOGLE_PREDICT_BRANCH_NOT_TAKEN(x) x
#  endif

#  ifndef GOOGLE_PREDICT_TRUE
#    define GOOGLE_PREDICT_FALSE(x) x
#  endif

#  ifndef GOOGLE_PREDICT_TRUE
#    define GOOGLE_PREDICT_TRUE(x) x
#  endif

#endif

#undef GLOG_BUILTIN_EXPECT_PRESENT

// Make a bunch of macros for logging.  The way to log things is to stream
// things to LOG(<a particular severity level>).  E.g.,
//
//   LOG(INFO) << "Found " << num_cookies << " cookies";
//
// You can capture log messages in a string, rather than reporting them
// immediately:
//
//   vector<string> errors;
//   LOG_STRING(ERROR, &errors) << "Couldn't parse cookie #" << cookie_num;
//
// This pushes back the new error onto 'errors'; if given a nullptr pointer,
// it reports the error via LOG(ERROR).
//
// You can also do conditional logging:
//
//   LOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//
// You can also do occasional logging (log every n'th occurrence of an
// event):
//
//   LOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";
//
// The above will cause log messages to be output on the 1st, 11th, 21st, ...
// times it is executed.  Note that the special google::COUNTER value is used
// to identify which repetition is happening.
//
// You can also do occasional conditional logging (log every n'th
// occurrence of an event, when condition is satisfied):
//
//   LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << google::COUNTER
//                                           << "th big cookie";
//
// You can log messages the first N times your code executes a line. E.g.
//
//   LOG_FIRST_N(INFO, 20) << "Got the " << google::COUNTER << "th cookie";
//
// Outputs log messages for the first 20 times it is executed.
//
// Analogous SYSLOG, SYSLOG_IF, and SYSLOG_EVERY_N macros are available.
// These log to syslog as well as to the normal logs.  If you use these at
// all, you need to be aware that syslog can drastically reduce performance,
// especially if it is configured for remote logging!  Don't use these
// unless you fully understand this and have a concrete need to use them.
// Even then, try to minimize your use of them.
//
// There are also "debug mode" logging macros like the ones above:
//
//   DLOG(INFO) << "Found cookies";
//
//   DLOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//
//   DLOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";
//
// All "debug mode" logging is compiled away to nothing for non-debug mode
// compiles.
//
// We also have
//
//   LOG_ASSERT(assertion);
//   DLOG_ASSERT(assertion);
//
// which is syntactic sugar for {,D}LOG_IF(FATAL, assert fails) << assertion;
//
// There are "verbose level" logging macros.  They look like
//
//   VLOG(1) << "I'm printed when you run the program with --v=1 or more";
//   VLOG(2) << "I'm printed when you run the program with --v=2 or more";
//
// These always log at the INFO log level (when they log at all).
// The verbose logging can also be turned on module-by-module.  For instance,
//    --vmodule=mapreduce=2,file=1,gfs*=3 --v=0
// will cause:
//   a. VLOG(2) and lower messages to be printed from mapreduce.{h,cc}
//   b. VLOG(1) and lower messages to be printed from file.{h,cc}
//   c. VLOG(3) and lower messages to be printed from files prefixed with "gfs"
//   d. VLOG(0) and lower messages to be printed from elsewhere
//
// The wildcarding functionality shown by (c) supports both '*' (match
// 0 or more characters) and '?' (match any single character) wildcards.
//
// There's also VLOG_IS_ON(n) "verbose level" condition macro. To be used as
//
//   if (VLOG_IS_ON(2)) {
//     // do some logging preparation and logging
//     // that can't be accomplished with just VLOG(2) << ...;
//   }
//
// There are also VLOG_IF, VLOG_EVERY_N and VLOG_IF_EVERY_N "verbose level"
// condition macros for sample cases, when some extra computation and
// preparation for logs is not needed.
//   VLOG_IF(1, (size > 1024))
//      << "I'm printed when size is more than 1024 and when you run the "
//         "program with --v=1 or more";
//   VLOG_EVERY_N(1, 10)
//      << "I'm printed every 10th occurrence, and when you run the program "
//         "with --v=1 or more. Present occurrence is " << google::COUNTER;
//   VLOG_IF_EVERY_N(1, (size > 1024), 10)
//      << "I'm printed on every 10th occurrence of case when size is more "
//         " than 1024, when you run the program with --v=1 or more. ";
//         "Present occurrence is " << google::COUNTER;
//
// The supported severity levels for macros that allow you to specify one
// are (in increasing order of severity) INFO, WARNING, ERROR, and FATAL.
// Note that messages of a given severity are logged not only in the
// logfile for that severity, but also in all logfiles of lower severity.
// E.g., a message of severity FATAL will be logged to the logfiles of
// severity FATAL, ERROR, WARNING, and INFO.
//
// There is also the special severity of DFATAL, which logs FATAL in
// debug mode, ERROR in normal mode.
//
// Very important: logging a message at the FATAL severity level causes
// the program to terminate (after the message is logged).
//
// Unless otherwise specified, logs will be written to the filename
// "<program name>.<hostname>.<user name>.log.<severity level>.", followed
// by the date, time, and pid (you can't prevent the date, time, and pid
// from being in the filename).
//
// The logging code takes two flags:
//     --v=#           set the verbose level
//     --logtostderr   log all the messages to stderr instead of to logfiles

// LOG LINE PREFIX FORMAT
//
// Log lines have this form:
//
//     Lyyyymmdd hh:mm:ss.uuuuuu threadid file:line] msg...
//
// where the fields are defined as follows:
//
//   L                A single character, representing the log level
//                    (eg 'I' for INFO)
//   yyyy             The year
//   mm               The month (zero padded; ie May is '05')
//   dd               The day (zero padded)
//   hh:mm:ss.uuuuuu  Time in hours, minutes and fractional seconds
//   threadid         The m_space-padded thread ID as returned by GetTID()
//                    (this matches the PID on Linux)
//   file             The file name
//   line             The line number
//   msg              The user-supplied message
//
// Example:
//
//   I1103 11:57:31.739339 24395 google.cc:2341] Command line: ./some_prog
//   I1103 11:57:31.739403 24395 google.cc:2342] Process id 24395
//
// NOTE: although the microseconds are useful for comparing events on
// a single machine, clocks on different machines may not be well
// synchronized.  Hence, use caution when comparing the low bits of
// timestamps from different machines.

// Log messages below the GOOGLE_STRIP_LOG level will be compiled away for
// security reasons. See LOG(severity) below.

// A few definitions of macros that don't generate much code.  Since
// LOG(INFO) and its ilk are used all over our code, it's
// better to have compact code for these operations.

#if GOOGLE_STRIP_LOG == 0
#  define COMPACT_GOOGLE_LOG_INFO google::LogMessage(__FILE__, __LINE__)
#  define LOG_TO_STRING_INFO(message) \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_INFO, message)
#else
#  define COMPACT_GOOGLE_LOG_INFO google::NullStream()
#  define LOG_TO_STRING_INFO(message) google::NullStream()
#endif

#if GOOGLE_STRIP_LOG <= 1
#  define COMPACT_GOOGLE_LOG_WARNING \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_WARNING)
#  define LOG_TO_STRING_WARNING(message) \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_WARNING, message)
#else
#  define COMPACT_GOOGLE_LOG_WARNING google::NullStream()
#  define LOG_TO_STRING_WARNING(message) google::NullStream()
#endif

#if GOOGLE_STRIP_LOG <= 2
#  define COMPACT_GOOGLE_LOG_ERROR \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_ERROR)
#  define LOG_TO_STRING_ERROR(message) \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_ERROR, message)
#else
#  define COMPACT_GOOGLE_LOG_ERROR google::NullStream()
#  define LOG_TO_STRING_ERROR(message) google::NullStream()
#endif

#if GOOGLE_STRIP_LOG <= 3
#  define COMPACT_GOOGLE_LOG_FATAL google::LogMessageFatal(__FILE__, __LINE__)
#  define LOG_TO_STRING_FATAL(message) \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_FATAL, message)
#else
#  define COMPACT_GOOGLE_LOG_FATAL google::NullStreamFatal()
#  define LOG_TO_STRING_FATAL(message) google::NullStreamFatal()
#endif

#if defined(NDEBUG) && !defined(DCHECK_ALWAYS_ON)
#  define DCHECK_IS_ON() 0
#else
#  define DCHECK_IS_ON() 1
#endif

// For DFATAL, we want to use LogMessage (as opposed to
// LogMessageFatal), to be consistent with the original behavior.
#if !DCHECK_IS_ON()
#  define COMPACT_GOOGLE_LOG_DFATAL COMPACT_GOOGLE_LOG_ERROR
#elif GOOGLE_STRIP_LOG <= 3
#  define COMPACT_GOOGLE_LOG_DFATAL \
    google::LogMessage(__FILE__, __LINE__, google::GLOG_FATAL)
#else
#  define COMPACT_GOOGLE_LOG_DFATAL google::NullStreamFatal()
#endif

#define GOOGLE_LOG_INFO(counter)                                     \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_INFO, counter, \
                     &google::LogMessage::SendToLog)
#define SYSLOG_INFO(counter)                                         \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_INFO, counter, \
                     &google::LogMessage::SendToSyslogAndLog)
#define GOOGLE_LOG_WARNING(counter)                                     \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_WARNING, counter, \
                     &google::LogMessage::SendToLog)
#define SYSLOG_WARNING(counter)                                         \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_WARNING, counter, \
                     &google::LogMessage::SendToSyslogAndLog)
#define GOOGLE_LOG_ERROR(counter)                                     \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_ERROR, counter, \
                     &google::LogMessage::SendToLog)
#define SYSLOG_ERROR(counter)                                         \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_ERROR, counter, \
                     &google::LogMessage::SendToSyslogAndLog)
#define GOOGLE_LOG_FATAL(counter)                                     \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_FATAL, counter, \
                     &google::LogMessage::SendToLog)
#define SYSLOG_FATAL(counter)                                         \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_FATAL, counter, \
                     &google::LogMessage::SendToSyslogAndLog)
#define GOOGLE_LOG_DFATAL(counter)                                      \
  google::LogMessage(__FILE__, __LINE__, google::DFATAL_LEVEL, counter, \
                     &google::LogMessage::SendToLog)
#define SYSLOG_DFATAL(counter)                                          \
  google::LogMessage(__FILE__, __LINE__, google::DFATAL_LEVEL, counter, \
                     &google::LogMessage::SendToSyslogAndLog)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
    defined(__CYGWIN__) || defined(__CYGWIN32__)
// A very useful logging macro to log windows errors:
#  define LOG_SYSRESULT(result)                                                \
    if (FAILED(HRESULT_FROM_WIN32(result))) {                                  \
      LPSTR message = nullptr;                                                 \
      LPSTR msg = reinterpret_cast<LPSTR>(&message);                           \
      DWORD message_length = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |   \
                                                FORMAT_MESSAGE_FROM_SYSTEM |   \
                                                FORMAT_MESSAGE_IGNORE_INSERTS, \
                                            0, result, 0, msg, 100, nullptr);  \
      std::unique_ptr<char, decltype(&LocalFree)> release{message,             \
                                                          &LocalFree};         \
      if (message_length > 0) {                                                \
        google::LogMessage(__FILE__, __LINE__, google::GLOG_ERROR, 0,          \
                           &google::LogMessage::SendToLog)                     \
                .stream()                                                      \
            << reinterpret_cast<const char*>(message);                         \
      }                                                                        \
    }
#endif

// We use the preprocessor's merging operator, "##", so that, e.g.,
// LOG(INFO) becomes the token GOOGLE_LOG_INFO.  There's some funny
// subtle difference between ostream member streaming functions (e.g.,
// ostream::operator<<(int) and ostream non-member streaming functions
// (e.g., ::operator<<(ostream&, string&): it turns out that it's
// impossible to stream something like a string directly to an unnamed
// ostream. We employ a neat hack by calling the stream() member
// function of LogMessage which seems to avoid the problem.
#define LOG(severity) COMPACT_GOOGLE_LOG_##severity.stream()
#define SYSLOG(severity) SYSLOG_##severity(0).stream()

namespace google {

// Initialize google's logging library. You will see the program name
// specified by argv0 in log outputs.
GLOG_EXPORT void InitGoogleLogging(const char* argv0);

class LogMessage;

#if defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif  // __GNUG__
using CustomPrefixCallback
    [[deprecated("Use PrefixFormatterCallback instead.")]] =
        void (*)(std::ostream&, const LogMessageInfo&, void*);
#if defined(__GNUG__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif  // __GNUG__
//[[deprecated("Use InstallPrefixFormatter instead.")]] GLOG_EXPORT void
//InitGoogleLogging(const char* argv0, CustomPrefixCallback prefix_callback,
//                  void* prefix_callback_data = nullptr);

// Check if google's logging library has been initialized.
GLOG_EXPORT bool IsGoogleLoggingInitialized();

// Shutdown google's logging library.
GLOG_EXPORT void ShutdownGoogleLogging();

#if defined(__GNUC__)
typedef void (*logging_fail_func_t)() __attribute__((noreturn));
#else
typedef void (*logging_fail_func_t)();
#endif

using PrefixFormatterCallback = void (*)(std::ostream&, const LogMessage&,
                                         void*);

GLOG_EXPORT void InstallPrefixFormatter(PrefixFormatterCallback callback,
                                        void* data = nullptr);

// Install a function which will be called after LOG(FATAL).
GLOG_EXPORT void InstallFailureFunction(logging_fail_func_t fail_func);

[[deprecated(
    "Use the type-safe std::chrono::minutes EnableLogCleaner overload "
    "instead.")]] GLOG_EXPORT void
EnableLogCleaner(unsigned int overdue_days);
// Enable/Disable old log cleaner.
GLOG_EXPORT void EnableLogCleaner(const std::chrono::minutes& overdue);
GLOG_EXPORT void DisableLogCleaner();
GLOG_EXPORT void SetApplicationFingerprint(const std::string& fingerprint);

class LogSink;  // defined below

// If a non-nullptr sink pointer is given, we push this message to that sink.
// For LOG_TO_SINK we then do normal LOG(severity) logging as well.
// This is useful for capturing messages and passing/storing them
// somewhere more specific than the global log of the process.
// Argument types:
//   LogSink* sink;
//   LogSeverity severity;
// The cast is to disambiguate nullptr arguments.
#define LOG_TO_SINK(sink, severity)                               \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity, \
                     static_cast<google::LogSink*>(sink), true)   \
      .stream()
#define LOG_TO_SINK_BUT_NOT_TO_LOGFILE(sink, severity)            \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity, \
                     static_cast<google::LogSink*>(sink), false)  \
      .stream()

// If a non-nullptr string pointer is given, we write this message to that
// string. We then do normal LOG(severity) logging as well. This is useful for
// capturing messages and storing them somewhere more specific than the global
// log of the process. Argument types:
//   string* message;
//   LogSeverity severity;
// The cast is to disambiguate nullptr arguments.
// NOTE: LOG(severity) expands to LogMessage().stream() for the specified
// severity.
#define LOG_TO_STRING(severity, message) \
  LOG_TO_STRING_##severity(static_cast<std::string*>(message)).stream()

// If a non-nullptr pointer is given, we push the message onto the end
// of a vector of strings; otherwise, we report it with LOG(severity).
// This is handy for capturing messages and perhaps passing them back
// to the caller, rather than reporting them immediately.
// Argument types:
//   LogSeverity severity;
//   vector<string> *outvec;
// The cast is to disambiguate nullptr arguments.
#define LOG_STRING(severity, outvec)                                       \
  LOG_TO_STRING_##severity(static_cast<std::vector<std::string>*>(outvec)) \
      .stream()

#define LOG_IF(severity, condition) \
  static_cast<void>(0),             \
      !(condition)                  \
          ? (void)0                 \
          : google::logging::internal::LogMessageVoidify() & LOG(severity)
#define SYSLOG_IF(severity, condition) \
  static_cast<void>(0),                \
      !(condition)                     \
          ? (void)0                    \
          : google::logging::internal::LogMessageVoidify() & SYSLOG(severity)

#define LOG_ASSERT(condition) \
  LOG_IF(FATAL, !(condition)) << "Assert failed: " #condition
#define SYSLOG_ASSERT(condition) \
  SYSLOG_IF(FATAL, !(condition)) << "Assert failed: " #condition

// CHECK dies with a fatal error if condition is not true.  It is *not*
// controlled by DCHECK_IS_ON(), so the check will be executed regardless of
// compilation mode.  Therefore, it is safe to do things like:
//    CHECK(fp->Write(x) == 4)
#define CHECK(condition)                                       \
  LOG_IF(FATAL, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(condition))) \
      << "Check failed: " #condition " "

namespace logging {
namespace internal {

// A container for a string pointer which can be evaluated to a bool -
// true iff the pointer is nullptr.
struct CheckOpString {
  CheckOpString(std::string* str) : str_(str) {}
  // No destructor: if str_ is non-nullptr, we're about to LOG(FATAL),
  // so there's no point in cleaning up str_.
  explicit operator bool() const noexcept {
    return GOOGLE_PREDICT_BRANCH_NOT_TAKEN(str_ != nullptr);
  }
  std::string* str_;
};

// Function is overloaded for integral types to allow static const
// integrals declared in classes and not defined to be used as arguments to
// CHECK* macros. It's not encouraged though.
template <class T>
inline const T& GetReferenceableValue(const T& t) {
  return t;
}
inline char GetReferenceableValue(char t) { return t; }
inline unsigned char GetReferenceableValue(unsigned char t) { return t; }
inline signed char GetReferenceableValue(signed char t) { return t; }
inline short GetReferenceableValue(short t) { return t; }
inline unsigned short GetReferenceableValue(unsigned short t) { return t; }
inline int GetReferenceableValue(int t) { return t; }
inline unsigned int GetReferenceableValue(unsigned int t) { return t; }
inline long GetReferenceableValue(long t) { return t; }
inline unsigned long GetReferenceableValue(unsigned long t) { return t; }
inline long long GetReferenceableValue(long long t) { return t; }
inline unsigned long long GetReferenceableValue(unsigned long long t) {
  return t;
}

// This is a dummy class to define the following operator.
struct DummyClassToDefineOperator {};

// Define global operator<< to declare using ::operator<<.
// This declaration will allow use to use CHECK macros for user
// defined classes which have operator<< (e.g., stl_logging.h).
inline std::ostream& operator<<(std::ostream& out,
                                const DummyClassToDefineOperator&) {
  return out;
}

// This formats a value for a failing CHECK_XX statement.  Ordinarily,
// it uses the definition for operator<<, with a few special cases below.
template <typename T>
inline void MakeCheckOpValueString(std::ostream* os, const T& v) {
  (*os) << v;
}

// Overrides for char types provide readable values for unprintable
// characters.
template <>
GLOG_EXPORT void MakeCheckOpValueString(std::ostream* os, const char& v);
template <>
GLOG_EXPORT void MakeCheckOpValueString(std::ostream* os, const signed char& v);
template <>
GLOG_EXPORT void MakeCheckOpValueString(std::ostream* os,
                                        const unsigned char& v);

// Provide printable value for nullptr_t
template <>
GLOG_EXPORT void MakeCheckOpValueString(std::ostream* os,
                                        const std::nullptr_t& v);

// Build the error message string. Specify no inlining for code size.
template <typename T1, typename T2>
std::string* MakeCheckOpString(const T1& v1, const T2& v2, const char* exprtext)
#if defined(__has_attribute)
#  if __has_attribute(used)
    __attribute__((noinline))
#  endif
#endif
    ;

// A helper class for formatting "expr (V1 vs. V2)" in a CHECK_XX
// statement.  See MakeCheckOpString for sample usage.  Other
// approaches were considered: use of a template method (e.g.,
// base::BuildCheckOpString(exprtext, base::Print<T1>, &v1,
// base::Print<T2>, &v2), however this approach has complications
// related to volatile arguments and function-pointer arguments).
class GLOG_EXPORT CheckOpMessageBuilder {
 public:
  // Inserts "exprtext" and " (" to the stream.
  explicit CheckOpMessageBuilder(const char* exprtext);
  // Deletes "stream_".
  ~CheckOpMessageBuilder();
  // For inserting the first variable.
  std::ostream* ForVar1() { return stream_; }
  // For inserting the second variable (adds an intermediate " vs. ").
  std::ostream* ForVar2();
  // Get the result (inserts the closing ")").
  std::string* NewString();

 private:
  std::ostringstream* stream_;
};

template <typename T1, typename T2>
std::string* MakeCheckOpString(const T1& v1, const T2& v2,
                               const char* exprtext) {
  CheckOpMessageBuilder comb(exprtext);
  MakeCheckOpValueString(comb.ForVar1(), v1);
  MakeCheckOpValueString(comb.ForVar2(), v2);
  return comb.NewString();
}

// Helper functions for CHECK_OP macro.
// The (int, int) specialization works around the issue that the compiler
// will not instantiate the template version of the function on values of
// unnamed enum type - see comment below.
#define DEFINE_CHECK_OP_IMPL(name, op)                                   \
  template <typename T1, typename T2>                                    \
  inline std::string* name##Impl(const T1& v1, const T2& v2,             \
                                 const char* exprtext) {                 \
    if (GOOGLE_PREDICT_TRUE(v1 op v2))                                   \
      return nullptr;                                                    \
    else                                                                 \
      return MakeCheckOpString(v1, v2, exprtext);                        \
  }                                                                      \
  inline std::string* name##Impl(int v1, int v2, const char* exprtext) { \
    return name##Impl<int, int>(v1, v2, exprtext);                       \
  }

// We use the full name Check_EQ, Check_NE, etc. in case the file including
// base/logging.h provides its own #defines for the simpler names EQ, NE, etc.
// This happens if, for example, those are used as token names in a
// yacc grammar.
DEFINE_CHECK_OP_IMPL(Check_EQ, ==)
DEFINE_CHECK_OP_IMPL(Check_NE, !=)
DEFINE_CHECK_OP_IMPL(Check_LE, <=)
DEFINE_CHECK_OP_IMPL(Check_LT, <)
DEFINE_CHECK_OP_IMPL(Check_GE, >=)
DEFINE_CHECK_OP_IMPL(Check_GT, >)

#undef DEFINE_CHECK_OP_IMPL

// Helper macro for binary operators.
// Don't use this macro directly in your code, use CHECK_EQ et al below.

#if defined(STATIC_ANALYSIS)
// Only for static analysis tool to know that it is equivalent to assert
#  define CHECK_OP_LOG(name, op, val1, val2, log) CHECK((val1)op(val2))
#elif DCHECK_IS_ON()
// In debug mode, avoid constructing CheckOpStrings if possible,
// to reduce the overhead of CHECK statements by 2x.
// Real DCHECK-heavy tests have seen 1.5x speedups.

// The meaning of "string" might be different between now and
// when this macro gets invoked (e.g., if someone is experimenting
// with other string implementations that get defined after this
// file is included).  Save the current meaning now and use it
// in the macro.
typedef std::string _Check_string;
#  define CHECK_OP_LOG(name, op, val1, val2, log)                              \
    while (google::logging::internal::_Check_string* _result =                 \
               google::logging::internal::Check##name##Impl(                   \
                   google::logging::internal::GetReferenceableValue(val1),     \
                   google::logging::internal::GetReferenceableValue(val2),     \
                   #val1 " " #op " " #val2))                                   \
    log(__FILE__, __LINE__, google::logging::internal::CheckOpString(_result)) \
        .stream()
#else
// In optimized mode, use CheckOpString to hint to compiler that
// the while condition is unlikely.
#  define CHECK_OP_LOG(name, op, val1, val2, log)                          \
    while (google::logging::internal::CheckOpString _result =              \
               google::logging::internal::Check##name##Impl(               \
                   google::logging::internal::GetReferenceableValue(val1), \
                   google::logging::internal::GetReferenceableValue(val2), \
                   #val1 " " #op " " #val2))                               \
    log(__FILE__, __LINE__, _result).stream()
#endif  // STATIC_ANALYSIS, DCHECK_IS_ON()

#if GOOGLE_STRIP_LOG <= 3
#  define CHECK_OP(name, op, val1, val2) \
    CHECK_OP_LOG(name, op, val1, val2, google::LogMessageFatal)
#else
#  define CHECK_OP(name, op, val1, val2) \
    CHECK_OP_LOG(name, op, val1, val2, google::NullStreamFatal)
#endif  // STRIP_LOG <= 3

// Equality/Inequality checks - compare two values, and log a FATAL message
// including the two values when the result is not as expected.  The values
// must have operator<<(ostream, ...) defined.
//
// You may append to the error message like so:
//   CHECK_NE(1, 2) << ": The world must be ending!";
//
// We are very careful to ensure that each argument is evaluated exactly
// once, and that anything which is legal to pass as a function argument is
// legal here.  In particular, the arguments may be temporary expressions
// which will end up being destroyed at the end of the apparent statement,
// for example:
//   CHECK_EQ(string("abc")[1], 'b');
//
// WARNING: These don't compile correctly if one of the arguments is a pointer
// and the other is nullptr. To work around this, simply static_cast nullptr to
// the type of the desired pointer.

#define CHECK_EQ(val1, val2) CHECK_OP(_EQ, ==, val1, val2)
#define CHECK_NE(val1, val2) CHECK_OP(_NE, !=, val1, val2)
#define CHECK_LE(val1, val2) CHECK_OP(_LE, <=, val1, val2)
#define CHECK_LT(val1, val2) CHECK_OP(_LT, <, val1, val2)
#define CHECK_GE(val1, val2) CHECK_OP(_GE, >=, val1, val2)
#define CHECK_GT(val1, val2) CHECK_OP(_GT, >, val1, val2)

// Check that the input is non nullptr.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val)                 \
  google::logging::internal::CheckNotNull( \
      __FILE__, __LINE__, "'" #val "' Must be non nullptr", (val))

// Helper functions for string comparisons.
// To avoid bloat, the definitions are in logging.cc.
#define DECLARE_CHECK_STROP_IMPL(func, expected)        \
  GLOG_EXPORT std::string* Check##func##expected##Impl( \
      const char* s1, const char* s2, const char* names);

DECLARE_CHECK_STROP_IMPL(strcmp, true)
DECLARE_CHECK_STROP_IMPL(strcmp, false)
DECLARE_CHECK_STROP_IMPL(strcasecmp, true)
DECLARE_CHECK_STROP_IMPL(strcasecmp, false)

}  // namespace internal
}  // namespace logging

#undef DECLARE_CHECK_STROP_IMPL

// Helper macro for string comparisons.
// Don't use this macro directly in your code, use CHECK_STREQ et al below.
#define CHECK_STROP(func, op, expected, s1, s2)                      \
  while (google::logging::internal::CheckOpString _result =          \
             google::logging::internal::Check##func##expected##Impl( \
                 (s1), (s2), #s1 " " #op " " #s2))                   \
  LOG(FATAL) << *_result.str_

// String (char*) equality/inequality checks.
// CASE versions are case-insensitive.
//
// Note that "s1" and "s2" may be temporary strings which are destroyed
// by the compiler at the end of the current "full expression"
// (e.g. CHECK_STREQ(Foo().c_str(), Bar().c_str())).

#define CHECK_STREQ(s1, s2) CHECK_STROP(strcmp, ==, true, s1, s2)
#define CHECK_STRNE(s1, s2) CHECK_STROP(strcmp, !=, false, s1, s2)
#define CHECK_STRCASEEQ(s1, s2) CHECK_STROP(strcasecmp, ==, true, s1, s2)
#define CHECK_STRCASENE(s1, s2) CHECK_STROP(strcasecmp, !=, false, s1, s2)

#define CHECK_INDEX(I, A) CHECK(I < (sizeof(A) / sizeof(A[0])))
#define CHECK_BOUND(B, A) CHECK(B <= (sizeof(A) / sizeof(A[0])))

#define CHECK_DOUBLE_EQ(val1, val2)                \
  do {                                             \
    CHECK_LE((val1), (val2) + 0.000000000000001L); \
    CHECK_GE((val1), (val2)-0.000000000000001L);   \
  } while (0)

#define CHECK_NEAR(val1, val2, margin)   \
  do {                                   \
    CHECK_LE((val1), (val2) + (margin)); \
    CHECK_GE((val1), (val2) - (margin)); \
  } while (0)

// perror()..googly style!
//
// PLOG() and PLOG_IF() and PCHECK() behave exactly like their LOG* and
// CHECK equivalents with the addition that they postpend a description
// of the current state of errno to their output lines.

#define PLOG(severity) GOOGLE_PLOG(severity, 0).stream()

#define GOOGLE_PLOG(severity, counter)                                 \
  google::ErrnoLogMessage(__FILE__, __LINE__, google::GLOG_##severity, \
                          counter, &google::LogMessage::SendToLog)

#define PLOG_IF(severity, condition) \
  static_cast<void>(0),              \
      !(condition)                   \
          ? (void)0                  \
          : google::logging::internal::LogMessageVoidify() & PLOG(severity)

// A CHECK() macro that postpends errno if the condition is false. E.g.
//
// if (poll(fds, nfds, timeout) == -1) { PCHECK(errno == EINTR); ... }
#define PCHECK(condition)                                       \
  PLOG_IF(FATAL, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(condition))) \
      << "Check failed: " #condition " "

// A CHECK() macro that lets you assert the success of a function that
// returns -1 and sets errno in case of an error. E.g.
//
// CHECK_ERR(mkdir(path, 0700));
//
// or
//
// int fd = open(filename, flags); CHECK_ERR(fd) << ": open " << filename;
#define CHECK_ERR(invocation)                                         \
  PLOG_IF(FATAL, GOOGLE_PREDICT_BRANCH_NOT_TAKEN((invocation) == -1)) \
      << #invocation

// Use macro expansion to create, for each use of LOG_EVERY_N(), static
// variables with the __LINE__ expansion as part of the variable name.
#define LOG_EVERY_N_VARNAME(base, line) LOG_EVERY_N_VARNAME_CONCAT(base, line)
#define LOG_EVERY_N_VARNAME_CONCAT(base, line) base##line

#define LOG_OCCURRENCES LOG_EVERY_N_VARNAME(occurrences_, __LINE__)
#define LOG_OCCURRENCES_MOD_N LOG_EVERY_N_VARNAME(occurrences_mod_n_, __LINE__)

#define LOG_TIME_PERIOD LOG_EVERY_N_VARNAME(timePeriod_, __LINE__)
#define LOG_PREVIOUS_TIME_RAW LOG_EVERY_N_VARNAME(previousTimeRaw_, __LINE__)
#define LOG_TIME_DELTA LOG_EVERY_N_VARNAME(deltaTime_, __LINE__)
#define LOG_CURRENT_TIME LOG_EVERY_N_VARNAME(currentTime_, __LINE__)
#define LOG_PREVIOUS_TIME LOG_EVERY_N_VARNAME(previousTime_, __LINE__)

}  // namespace google

namespace google {

#define SOME_KIND_OF_LOG_EVERY_T(severity, seconds)                            \
  constexpr std::chrono::nanoseconds LOG_TIME_PERIOD =                         \
      std::chrono::duration_cast<std::chrono::nanoseconds>(                    \
          std::chrono::duration<double>(seconds));                             \
  static std::atomic<google::int64> LOG_PREVIOUS_TIME_RAW;                     \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                         \
      __FILE__, __LINE__, &LOG_TIME_PERIOD, sizeof(google::int64), ""));       \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                         \
      __FILE__, __LINE__, &LOG_PREVIOUS_TIME_RAW, sizeof(google::int64), "")); \
  const auto LOG_CURRENT_TIME =                                                \
      std::chrono::duration_cast<std::chrono::nanoseconds>(                    \
          std::chrono::steady_clock::now().time_since_epoch());                \
  const auto LOG_PREVIOUS_TIME =                                               \
      LOG_PREVIOUS_TIME_RAW.load(std::memory_order_relaxed);                   \
  const auto LOG_TIME_DELTA =                                                  \
      LOG_CURRENT_TIME - std::chrono::nanoseconds(LOG_PREVIOUS_TIME);          \
  if (LOG_TIME_DELTA > LOG_TIME_PERIOD)                                        \
    LOG_PREVIOUS_TIME_RAW.store(                                               \
        std::chrono::duration_cast<std::chrono::nanoseconds>(LOG_CURRENT_TIME) \
            .count(),                                                          \
        std::memory_order_relaxed);                                            \
  if (LOG_TIME_DELTA > LOG_TIME_PERIOD)                                        \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity).stream()

#define SOME_KIND_OF_LOG_EVERY_N(severity, n, what_to_do)               \
  static std::atomic<int> LOG_OCCURRENCES(0), LOG_OCCURRENCES_MOD_N(0); \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                  \
      __FILE__, __LINE__, &LOG_OCCURRENCES, sizeof(int), ""));          \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                  \
      __FILE__, __LINE__, &LOG_OCCURRENCES_MOD_N, sizeof(int), ""));    \
  ++LOG_OCCURRENCES;                                                    \
  if (++LOG_OCCURRENCES_MOD_N > n) LOG_OCCURRENCES_MOD_N -= n;          \
  if (LOG_OCCURRENCES_MOD_N == 1)                                       \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity,       \
                     LOG_OCCURRENCES, &what_to_do)                      \
      .stream()

#define SOME_KIND_OF_LOG_IF_EVERY_N(severity, condition, n, what_to_do)       \
  static std::atomic<int> LOG_OCCURRENCES(0), LOG_OCCURRENCES_MOD_N(0);       \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                        \
      __FILE__, __LINE__, &LOG_OCCURRENCES, sizeof(int), ""));                \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                        \
      __FILE__, __LINE__, &LOG_OCCURRENCES_MOD_N, sizeof(int), ""));          \
  ++LOG_OCCURRENCES;                                                          \
  if ((condition) &&                                                          \
      ((LOG_OCCURRENCES_MOD_N = (LOG_OCCURRENCES_MOD_N + 1) % n) == (1 % n))) \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity,             \
                     LOG_OCCURRENCES, &what_to_do)                            \
      .stream()

#define SOME_KIND_OF_PLOG_EVERY_N(severity, n, what_to_do)              \
  static std::atomic<int> LOG_OCCURRENCES(0), LOG_OCCURRENCES_MOD_N(0); \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                  \
      __FILE__, __LINE__, &LOG_OCCURRENCES, sizeof(int), ""));          \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                  \
      __FILE__, __LINE__, &LOG_OCCURRENCES_MOD_N, sizeof(int), ""));    \
  ++LOG_OCCURRENCES;                                                    \
  if (++LOG_OCCURRENCES_MOD_N > n) LOG_OCCURRENCES_MOD_N -= n;          \
  if (LOG_OCCURRENCES_MOD_N == 1)                                       \
  google::ErrnoLogMessage(__FILE__, __LINE__, google::GLOG_##severity,  \
                          LOG_OCCURRENCES, &what_to_do)                 \
      .stream()

#define SOME_KIND_OF_LOG_FIRST_N(severity, n, what_to_do)         \
  static std::atomic<int> LOG_OCCURRENCES(0);                     \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(            \
      __FILE__, __LINE__, &LOG_OCCURRENCES, sizeof(int), ""));    \
  if (LOG_OCCURRENCES <= n) ++LOG_OCCURRENCES;                    \
  if (LOG_OCCURRENCES <= n)                                       \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity, \
                     LOG_OCCURRENCES, &what_to_do)                \
      .stream()

namespace logging {
namespace internal {
template <bool>
struct CompileAssert {};
struct CrashReason;
}  // namespace internal
}  // namespace logging

#define LOG_EVERY_N(severity, n) \
  SOME_KIND_OF_LOG_EVERY_N(severity, (n), google::LogMessage::SendToLog)

#define LOG_EVERY_T(severity, T) SOME_KIND_OF_LOG_EVERY_T(severity, (T))

#define SYSLOG_EVERY_N(severity, n)       \
  SOME_KIND_OF_LOG_EVERY_N(severity, (n), \
                           google::LogMessage::SendToSyslogAndLog)

#define PLOG_EVERY_N(severity, n) \
  SOME_KIND_OF_PLOG_EVERY_N(severity, (n), google::LogMessage::SendToLog)

#define LOG_FIRST_N(severity, n) \
  SOME_KIND_OF_LOG_FIRST_N(severity, (n), google::LogMessage::SendToLog)

#define LOG_IF_EVERY_N(severity, condition, n)            \
  SOME_KIND_OF_LOG_IF_EVERY_N(severity, (condition), (n), \
                              google::LogMessage::SendToLog)

// We want the special COUNTER value available for LOG_EVERY_X()'ed messages
struct Counter_t {};
GLOG_INLINE_VARIABLE constexpr Counter_t COUNTER{};

#ifdef GLOG_NO_ABBREVIATED_SEVERITIES
// wingdi.h defines ERROR to be 0. When we call LOG(ERROR), it gets
// substituted with 0, and it expands to COMPACT_GOOGLE_LOG_0. To allow us
// to keep using this syntax, we define this macro to do the same thing
// as COMPACT_GOOGLE_LOG_ERROR.
#  define COMPACT_GOOGLE_LOG_0 COMPACT_GOOGLE_LOG_ERROR
#  define SYSLOG_0 SYSLOG_ERROR
#  define LOG_TO_STRING_0 LOG_TO_STRING_ERROR
// Needed for LOG_IS_ON(ERROR).
GLOG_INLINE_VARIABLE
constexpr LogSeverity GLOG_0 = GLOG_ERROR;
#else
// Users may include windows.h after logging.h without
// GLOG_NO_ABBREVIATED_SEVERITIES nor WIN32_LEAN_AND_MEAN.
// For this case, we cannot detect if ERROR is defined before users
// actually use ERROR. Let's make an undefined symbol to warn users.
#  define GLOG_ERROR_MSG \
    ERROR_macro_is_defined_Define_GLOG_NO_ABBREVIATED_SEVERITIES_before_including_logging_h_See_the_document_for_detail
#  define COMPACT_GOOGLE_LOG_0 GLOG_ERROR_MSG
#  define SYSLOG_0 GLOG_ERROR_MSG
#  define LOG_TO_STRING_0 GLOG_ERROR_MSG
#  define GLOG_0 GLOG_ERROR_MSG
#endif

// Plus some debug-logging macros that get compiled to nothing for production

#if DCHECK_IS_ON()

#  define DLOG(severity) LOG(severity)
#  define DVLOG(verboselevel) VLOG(verboselevel)
#  define DLOG_IF(severity, condition) LOG_IF(severity, condition)
#  define DLOG_EVERY_N(severity, n) LOG_EVERY_N(severity, n)
#  define DLOG_IF_EVERY_N(severity, condition, n) \
    LOG_IF_EVERY_N(severity, condition, n)
#  define DLOG_FIRST_N(severity, n) LOG_FIRST_N(severity, n)
#  define DLOG_EVERY_T(severity, T) LOG_EVERY_T(severity, T)
#  define DLOG_ASSERT(condition) LOG_ASSERT(condition)

// debug-only checking.  executed if DCHECK_IS_ON().
#  define DCHECK(condition) CHECK(condition)
#  define DCHECK_EQ(val1, val2) CHECK_EQ(val1, val2)
#  define DCHECK_NE(val1, val2) CHECK_NE(val1, val2)
#  define DCHECK_LE(val1, val2) CHECK_LE(val1, val2)
#  define DCHECK_LT(val1, val2) CHECK_LT(val1, val2)
#  define DCHECK_GE(val1, val2) CHECK_GE(val1, val2)
#  define DCHECK_GT(val1, val2) CHECK_GT(val1, val2)
#  define DCHECK_NOTNULL(val) CHECK_NOTNULL(val)
#  define DCHECK_STREQ(str1, str2) CHECK_STREQ(str1, str2)
#  define DCHECK_STRCASEEQ(str1, str2) CHECK_STRCASEEQ(str1, str2)
#  define DCHECK_STRNE(str1, str2) CHECK_STRNE(str1, str2)
#  define DCHECK_STRCASENE(str1, str2) CHECK_STRCASENE(str1, str2)

#else  // !DCHECK_IS_ON()

#  define DLOG(severity)  \
    static_cast<void>(0), \
        true ? (void)0    \
             : google::logging::internal::LogMessageVoidify() & LOG(severity)

#  define DVLOG(verboselevel)               \
    static_cast<void>(0),                   \
        (true || !VLOG_IS_ON(verboselevel)) \
            ? (void)0                       \
            : google::logging::internal::LogMessageVoidify() & LOG(INFO)

#  define DLOG_IF(severity, condition) \
    static_cast<void>(0),              \
        (true || !(condition))         \
            ? (void)0                  \
            : google::logging::internal::LogMessageVoidify() & LOG(severity)

#  define DLOG_EVERY_N(severity, n) \
    static_cast<void>(0),           \
        true ? (void)0              \
             : google::logging::internal::LogMessageVoidify() & LOG(severity)

#  define DLOG_IF_EVERY_N(severity, condition, n) \
    static_cast<void>(0),                         \
        (true || !(condition))                    \
            ? (void)0                             \
            : google::logging::internal::LogMessageVoidify() & LOG(severity)

#  define DLOG_FIRST_N(severity, n) \
    static_cast<void>(0),           \
        true ? (void)0              \
             : google::logging::internal::LogMessageVoidify() & LOG(severity)

#  define DLOG_EVERY_T(severity, T) \
    static_cast<void>(0),           \
        true ? (void)0              \
             : google::logging::internal::LogMessageVoidify() & LOG(severity)

#  define DLOG_ASSERT(condition) \
    static_cast<void>(0), true ? (void)0 : (LOG_ASSERT(condition))

// MSVC warning C4127: conditional expression is constant
#  define DCHECK(condition)              \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK(condition)

#  define DCHECK_EQ(val1, val2)          \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_EQ(val1, val2)

#  define DCHECK_NE(val1, val2)          \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_NE(val1, val2)

#  define DCHECK_LE(val1, val2)          \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_LE(val1, val2)

#  define DCHECK_LT(val1, val2)          \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_LT(val1, val2)

#  define DCHECK_GE(val1, val2)          \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_GE(val1, val2)

#  define DCHECK_GT(val1, val2)          \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_GT(val1, val2)

// You may see warnings in release mode if you don't use the return
// value of DCHECK_NOTNULL. Please just use DCHECK for such cases.
#  define DCHECK_NOTNULL(val) (val)

#  define DCHECK_STREQ(str1, str2)       \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_STREQ(str1, str2)

#  define DCHECK_STRCASEEQ(str1, str2)   \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_STRCASEEQ(str1, str2)

#  define DCHECK_STRNE(str1, str2)       \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_STRNE(str1, str2)

#  define DCHECK_STRCASENE(str1, str2)   \
    GLOG_MSVC_PUSH_DISABLE_WARNING(4127) \
    while (false) GLOG_MSVC_POP_WARNING() CHECK_STRCASENE(str1, str2)

#endif  // DCHECK_IS_ON()

// Log only in verbose mode.

#define VLOG(verboselevel) LOG_IF(INFO, VLOG_IS_ON(verboselevel))

#define VLOG_IF(verboselevel, condition) \
  LOG_IF(INFO, (condition) && VLOG_IS_ON(verboselevel))

#define VLOG_EVERY_N(verboselevel, n) \
  LOG_IF_EVERY_N(INFO, VLOG_IS_ON(verboselevel), n)

#define VLOG_IF_EVERY_N(verboselevel, condition, n) \
  LOG_IF_EVERY_N(INFO, (condition) && VLOG_IS_ON(verboselevel), n)

namespace base_logging {

// LogMessage::LogStream is a std::ostream backed by this streambuf.
// This class ignores overflow and leaves two bytes at the end of the
// buffer to allow for a '\n' and '\0'.
class GLOG_EXPORT LogStreamBuf : public std::streambuf {
 public:
  // REQUIREMENTS: "len" must be >= 2 to account for the '\n' and '\0'.
  LogStreamBuf(char* buf, int len) { setp(buf, buf + len - 2); }

  // This effectively ignores overflow.
  int_type overflow(int_type ch) { return ch; }

  // Legacy public ostrstream method.
  size_t pcount() const { return static_cast<size_t>(pptr() - pbase()); }
  char* pbase() const { return std::streambuf::pbase(); }
};

}  // namespace base_logging

namespace logging {
namespace internal {
struct GLOG_NO_EXPORT LogMessageData;
}  // namespace internal
}  // namespace logging

//
// This class more or less represents a particular log message.  You
// create an instance of LogMessage and then stream stuff to it.
// When you finish streaming to it, ~LogMessage is called and the
// full message gets streamed to the appropriate destination.
//
// You shouldn't actually use LogMessage's constructor to log things,
// though.  You should use the LOG() macro (and variants thereof)
// above.
class GLOG_EXPORT LogMessage {
 public:
  enum {
    // Passing kNoLogPrefix for the line number disables the
    // log-message prefix. Useful for using the LogMessage
    // infrastructure as a printing utility. See also the --log_prefix
    // flag for controlling the log-message prefix on an
    // application-wide basis.
    kNoLogPrefix = -1
  };

  // LogStream inherit from non-DLL-exported class (std::ostrstream)
  // and VC++ produces a warning for this situation.
  // However, MSDN says "C4275 can be ignored in Microsoft Visual C++
  // 2005 if you are deriving from a type in the Standard C++ Library"
  // http://msdn.microsoft.com/en-us/library/3tdb471s(VS.80).aspx
  // Let's just ignore the warning.
  GLOG_MSVC_PUSH_DISABLE_WARNING(4275)
  class GLOG_EXPORT LogStream : public std::ostream {
    GLOG_MSVC_POP_WARNING()
   public:
    // In some cases, like when compiling glog as a static library with GCC and
    // linking against a Clang-built executable, this constructor will be
    // removed by the linker. We use this attribute to prevent the linker from
    // discarding it.
    GLOG_USED
    LogStream(char* buf, int len, int64 ctr)
        : std::ostream(nullptr), streambuf_(buf, len), ctr_(ctr), self_(this) {
      rdbuf(&streambuf_);
    }

    LogStream(LogStream&& other) noexcept
        : std::ostream(nullptr),
          streambuf_(std::move(other.streambuf_)),
          ctr_(std::exchange(other.ctr_, 0)),
          self_(this) {
      rdbuf(&streambuf_);
    }

    LogStream& operator=(LogStream&& other) noexcept {
      streambuf_ = std::move(other.streambuf_);
      ctr_ = std::exchange(other.ctr_, 0);
      rdbuf(&streambuf_);
      return *this;
    }

    int64 ctr() const { return ctr_; }
    void set_ctr(int64 ctr) { ctr_ = ctr; }
    LogStream* self() const { return self_; }

    // Legacy std::streambuf methods.
    size_t pcount() const { return streambuf_.pcount(); }
    char* pbase() const { return streambuf_.pbase(); }
    char* str() const { return pbase(); }

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

   private:
    base_logging::LogStreamBuf streambuf_;
    int64 ctr_;        // Counter hack (for the LOG_EVERY_X() macro)
    LogStream* self_;  // Consistency check hack
  };

 public:
  // icc 8 requires this typedef to avoid an internal compiler error.
  typedef void (LogMessage::*SendMethod)();

  LogMessage(const char* file, int line, LogSeverity severity, int64 ctr,
             SendMethod send_method);

  // Two special constructors that generate reduced amounts of code at
  // LOG call sites for common cases.

  // Used for LOG(INFO): Implied are:
  // severity = INFO, ctr = 0, send_method = &LogMessage::SendToLog.
  //
  // Using this constructor instead of the more complex constructor above
  // saves 19 bytes per call site.
  LogMessage(const char* file, int line);

  // Used for LOG(severity) where severity != INFO.  Implied
  // are: ctr = 0, send_method = &LogMessage::SendToLog
  //
  // Using this constructor instead of the more complex constructor above
  // saves 17 bytes per call site.
  LogMessage(const char* file, int line, LogSeverity severity);

  // Constructor to log this message to a specified sink (if not nullptr).
  // Implied are: ctr = 0, send_method = &LogMessage::SendToSinkAndLog if
  // also_send_to_log is true, send_method = &LogMessage::SendToSink otherwise.
  LogMessage(const char* file, int line, LogSeverity severity, LogSink* sink,
             bool also_send_to_log);

  // Constructor where we also give a vector<string> pointer
  // for storing the messages (if the pointer is not nullptr).
  // Implied are: ctr = 0, send_method = &LogMessage::SaveOrSendToLog.
  LogMessage(const char* file, int line, LogSeverity severity,
             std::vector<std::string>* outvec);

  // Constructor where we also give a string pointer for storing the
  // message (if the pointer is not nullptr).  Implied are: ctr = 0,
  // send_method = &LogMessage::WriteToStringAndLog.
  LogMessage(const char* file, int line, LogSeverity severity,
             std::string* message);

  // A special constructor used for check failures
  LogMessage(const char* file, int line,
             const logging::internal::CheckOpString& result);

  ~LogMessage();

  // Flush a buffered message to the sink set in the constructor.  Always
  // called by the destructor, it may also be called from elsewhere if
  // needed.  Only the first call is actioned; any later ones are ignored.
  void Flush();

  // An arbitrary limit on the length of a single log message.  This
  // is so that streaming can be done more efficiently.
  static const size_t kMaxLogMessageLen;

  // These should not be called directly outside of logging.*,
  // only passed as SendMethod arguments to other LogMessage methods:
  void SendToLog();           // Actually dispatch to the logs
  void SendToSyslogAndLog();  // Actually dispatch to syslog and the logs

  // Call abort() or similar to perform LOG(FATAL) crash.
  [[noreturn]] static void Fail();

  std::ostream& stream();

  int preserved_errno() const;

  // Must be called without the log_mutex held.  (L < log_mutex)
  static int64 num_messages(int severity);

  [[deprecated("Use LogMessage::time() instead.")]] const LogMessageTime&
  getLogMessageTime() const {
    return time();
  }

  LogSeverity severity() const noexcept;
  int line() const noexcept;
  const std::thread::id& thread_id() const noexcept;
  const char* fullname() const noexcept;
  const char* basename() const noexcept;
  const LogMessageTime& time() const noexcept;

  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;

 private:
  // Fully internal SendMethod cases:
  void SendToSinkAndLog();  // Send to sink if provided and dispatch to the logs
  void SendToSink();        // Send to sink if provided, do nothing otherwise.

  // Write to string if provided and dispatch to the logs.
  void WriteToStringAndLog();

  void SaveOrSendToLog();  // Save to stringvec if provided, else to logs

  void Init(const char* file, int line, LogSeverity severity,
            void (LogMessage::*send_method)());

  // Used to fill in crash information during LOG(FATAL) failures.
  void RecordCrashReason(logging::internal::CrashReason* reason);

  // Counts of messages sent at each priority:
  static int64 num_messages_[NUM_SEVERITIES];  // under log_mutex

  // We keep the data in a separate struct so that each instance of
  // LogMessage uses less stack m_space.
  logging::internal::LogMessageData* allocated_;
  logging::internal::LogMessageData* data_;
  LogMessageTime time_;

  friend class LogDestination;
};

// This class happens to be thread-hostile because all instances share
// a single data buffer, but since it can only be created just before
// the process dies, we don't worry so much.
class GLOG_EXPORT LogMessageFatal : public LogMessage {
 public:
  LogMessageFatal(const char* file, int line);
  LogMessageFatal(const char* file, int line,
                  const logging::internal::CheckOpString& result);
  [[noreturn]] ~LogMessageFatal();
};

// A non-macro interface to the log facility; (useful
// when the logging level is not a compile-time constant).
inline void LogAtLevel(LogSeverity severity, std::string const& msg) {
  LogMessage(__FILE__, __LINE__, severity).stream() << msg;
}

// A macro alternative of LogAtLevel. New code may want to use this
// version since there are two advantages: 1. this version outputs the
// file name and the line number where this macro is put like other
// LOG macros, 2. this macro can be used as C++ stream.
#define LOG_AT_LEVEL(severity) \
  google::LogMessage(__FILE__, __LINE__, severity).stream()

// Allow folks to put a counter in the LOG_EVERY_X()'ed messages. This
// only works if ostream is a LogStream. If the ostream is not a
// LogStream you'll get an assert saying as much at runtime.
GLOG_EXPORT std::ostream& operator<<(std::ostream& os, const Counter_t&);

// Derived class for PLOG*() above.
class GLOG_EXPORT ErrnoLogMessage : public LogMessage {
 public:
  ErrnoLogMessage(const char* file, int line, LogSeverity severity, int64 ctr,
                  void (LogMessage::*send_method)());

  // Postpends ": strerror(errno) [errno]".
  ~ErrnoLogMessage();

 private:
  ErrnoLogMessage(const ErrnoLogMessage&);
  void operator=(const ErrnoLogMessage&);
};

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".

namespace logging {
namespace internal {

// Helper for CHECK_NOTNULL().
//
// In C++11, all cases can be handled by a single function. Since the value
// category of the argument is preserved (also for rvalue references),
// member initializer lists like the one below will compile correctly:
//
//   Foo()
//     : x_(CHECK_NOTNULL(MethodReturningUniquePtr())) {}
template <typename T>
T CheckNotNull(const char* file, int line, const char* names, T&& t) {
  if (t == nullptr) {
    LogMessageFatal(file, line, new std::string(names));
  }
  return std::forward<T>(t);
}

struct LogMessageVoidify {
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&) noexcept {}
};

}  // namespace internal
}  // namespace logging

// Flushes all log files that contains messages that are at least of
// the specified severity level.  Thread-safe.
GLOG_EXPORT void FlushLogFiles(LogSeverity min_severity);

// Flushes all log files that contains messages that are at least of
// the specified severity level. Thread-hostile because it ignores
// locking -- used for catastrophic failures.
GLOG_EXPORT void FlushLogFilesUnsafe(LogSeverity min_severity);

//
// Set the destination to which a particular severity level of log
// messages is sent.  If base_filename is "", it means "don't log this
// severity".  Thread-safe.
//
GLOG_EXPORT void SetLogDestination(LogSeverity severity,
                                   const char* base_filename);

//
// Set the basename of the symlink to the latest log file at a given
// severity.  If symlink_basename is empty, do not make a symlink.  If
// you don't call this function, the symlink basename is the
// invocation name of the program.  Thread-safe.
//
GLOG_EXPORT void SetLogSymlink(LogSeverity severity,
                               const char* symlink_basename);

//
// Used to send logs to some other kind of destination
// Users should subclass LogSink and override send to do whatever they want.
// Implementations must be thread-safe because a shared instance will
// be called from whichever thread ran the LOG(XXX) line.
class GLOG_EXPORT LogSink {
 public:
  virtual ~LogSink();

  // Sink's logging logic (message_len is such as to exclude '\n' at the end).
  // This method can't use LOG() or CHECK() as logging system mutex(s) are held
  // during this call.
  virtual void send(LogSeverity severity, const char* full_filename,
                    const char* base_filename, int line,
                    const LogMessageTime& time, const char* message,
                    size_t message_len);
  // Provide an overload for compatibility purposes
  GLOG_DEPRECATED
  virtual void send(LogSeverity severity, const char* full_filename,
                    const char* base_filename, int line, const std::tm* t,
                    const char* message, size_t message_len);

  // Redefine this to implement waiting for
  // the sink's logging logic to complete.
  // It will be called after each send() returns,
  // but before that LogMessage exits or crashes.
  // By default this function does nothing.
  // Using this function one can implement complex logic for send()
  // that itself involves logging; and do all this w/o causing deadlocks and
  // inconsistent rearrangement of log messages.
  // E.g. if a LogSink has thread-specific actions, the send() method
  // can simply add the message to a queue and wake up another thread that
  // handles real logging while itself making some LOG() calls;
  // WaitTillSent() can be implemented to wait for that logic to complete.
  // See our unittest for an example.
  virtual void WaitTillSent();

  // Returns the normal text output of the log message.
  // Can be useful to implement send().
  static std::string ToString(LogSeverity severity, const char* file, int line,
                              const LogMessageTime& time, const char* message,
                              size_t message_len);
};

// Add or remove a LogSink as a consumer of logging data.  Thread-safe.
GLOG_EXPORT void AddLogSink(LogSink* destination);
GLOG_EXPORT void RemoveLogSink(LogSink* destination);

//
// Specify an "extension" added to the filename specified via
// SetLogDestination.  This applies to all severity levels.  It's
// often used to append the port we're listening on to the logfile
// name.  Thread-safe.
//
GLOG_EXPORT void SetLogFilenameExtension(const char* filename_extension);

//
// Make it so that all log messages of at least a particular severity
// are logged to stderr (in addition to logging to the usual log
// file(s)).  Thread-safe.
//
GLOG_EXPORT void SetStderrLogging(LogSeverity min_severity);

//
// Make it so that all log messages go only to stderr.  Thread-safe.
//
GLOG_EXPORT void LogToStderr();

//
// Make it so that all log messages of at least a particular severity are
// logged via email to a list of addresses (in addition to logging to the
// usual log file(s)).  The list of addresses is just a string containing
// the email addresses to send to (separated by spaces, say).  Thread-safe.
//
GLOG_EXPORT void SetEmailLogging(LogSeverity min_severity,
                                 const char* addresses);

// A simple function that sends email. dest is a comma-separated
// list of addresses.  Thread-safe.
GLOG_EXPORT bool SendEmail(const char* dest, const char* subject,
                           const char* body);

GLOG_EXPORT const std::vector<std::string>& GetLoggingDirectories();

// Print any fatal message again -- useful to call from signal handler
// so that the last thing in the output is the fatal message.
// Thread-hostile, but a race is unlikely.
GLOG_EXPORT void ReprintFatalMessage();

// Truncate a log file that may be the append-only output of multiple
// processes and hence can't simply be renamed/reopened (typically a
// stdout/stderr).  If the file "path" is > "limit" bytes, copy the
// last "keep" bytes to offset 0 and truncate the rest. Since we could
// be racing with other writers, this approach has the potential to
// lose very small amounts of data. For security, only follow symlinks
// if the path is /proc/self/fd/*
GLOG_EXPORT void TruncateLogFile(const char* path, uint64 limit, uint64 keep);

// Truncate stdout and stderr if they are over the value specified by
// --max_log_size; keep the final 1MB.  This function has the same
// race condition as TruncateLogFile.
GLOG_EXPORT void TruncateStdoutStderr();

// Return the string representation of the provided LogSeverity level.
// Thread-safe.
GLOG_EXPORT const char* GetLogSeverityName(LogSeverity severity);

// ---------------------------------------------------------------------
// Implementation details that are not useful to most clients
// ---------------------------------------------------------------------

// A Logger is the interface used by logging modules to emit entries
// to a log.  A typical implementation will dump formatted data to a
// sequence of files.  We also provide interfaces that will forward
// the data to another thread so that the invoker never blocks.
// Implementations should be thread-safe since the logging system
// will write to them from multiple threads.

namespace base {

class GLOG_EXPORT Logger {
 public:
  virtual ~Logger();

  // Writes "message[0,message_len-1]" corresponding to an event that
  // occurred at "timestamp".  If "force_flush" is true, the log file
  // is flushed immediately.
  //
  // The input message has already been formatted as deemed
  // appropriate by the higher level logging facility.  For example,
  // textual log messages already contain timestamps, and the
  // file:linenumber header.
  [[deprecated(
      "Logger::Write accepting a std::time_t timestamp is provided for "
      "compatibility purposes only. New code should implement the "
      "std::chrono::system_clock::time_point overload.")]] virtual void
  Write(bool force_flush, time_t timestamp, const char* message,
        size_t message_len);
  virtual void Write(bool force_flush,
                     const std::chrono::system_clock::time_point& timestamp,
                     const char* message, size_t message_len);

  // Flush any buffered messages
  virtual void Flush() = 0;

  // Get the current LOG file size.
  // The returned value is approximate since some
  // logged data may not have been flushed to disk yet.
  virtual uint32 LogSize() = 0;
};

// Get the logger for the specified severity level.  The logger
// remains the property of the logging module and should not be
// deleted by the caller.  Thread-safe.
extern GLOG_EXPORT Logger* GetLogger(LogSeverity level);

// Set the logger for the specified severity level.  The logger
// becomes the property of the logging module and should not
// be deleted by the caller.  Thread-safe.
extern GLOG_EXPORT void SetLogger(LogSeverity level, Logger* logger);

}  // namespace base

// A class for which we define operator<<, which does nothing.
class GLOG_EXPORT NullStream : public LogMessage::LogStream {
 public:
  // Initialize the LogStream so the messages can be written somewhere
  // (they'll never be actually displayed). This will be needed if a
  // NullStream& is implicitly converted to LogStream&, in which case
  // the overloaded NullStream::operator<< will not be invoked.
  NullStream();
  NullStream(const char* /*file*/, int /*line*/,
             const logging::internal::CheckOpString& /*result*/);
  NullStream& stream();

 private:
  // A very short buffer for messages (which we discard anyway). This
  // will be needed if NullStream& converted to LogStream& (e.g. as a
  // result of a conditional expression).
  char message_buffer_[3];
};

// Do nothing. This operator is inline, allowing the message to be
// compiled away. The message will not be compiled away if we do
// something like (flag ? LOG(INFO) : LOG(ERROR)) << message; when
// SKIP_LOG=WARNING. In those cases, NullStream will be implicitly
// converted to LogStream and the message will be computed and then
// quietly discarded.
template <class T>
inline NullStream& operator<<(NullStream& str, const T&) {
  return str;
}

// Similar to NullStream, but aborts the program (without stack
// trace), like LogMessageFatal.
class GLOG_EXPORT NullStreamFatal : public NullStream {
 public:
  using NullStream::NullStream;
  [[noreturn]]
  // Prevent the linker from discarding the destructor.
  GLOG_USED ~NullStreamFatal();
};

// Install a signal handler that will dump signal information and a stack
// trace when the program crashes on certain signals.  We'll install the
// signal handler for the following signals.
//
// SIGSEGV, SIGILL, SIGFPE, SIGABRT, SIGBUS, and SIGTERM.
//
// By default, the signal handler will write the failure dump to the
// standard error.  You can customize the destination by installing your
// own writer function by InstallFailureWriter() below.
//
// Note on threading:
//
// The function should be called before threads are created, if you want
// to use the failure signal handler for all threads.  The stack trace
// will be shown only for the thread that receives the signal.  In other
// words, stack traces of other threads won't be shown.
GLOG_EXPORT void InstallFailureSignalHandler();

// Returns true if FailureSignalHandler is installed.
GLOG_EXPORT bool IsFailureSignalHandlerInstalled();

// Installs a function that is used for writing the failure dump.  "data"
// is the pointer to the beginning of a message to be written, and "size"
// is the size of the message.  You should not expect the data is
// terminated with '\0'.
GLOG_EXPORT void InstallFailureWriter(void (*writer)(const char* data,
                                                     size_t size));

}  // namespace google

#endif  // GLOG_LOGGING_H
