#ifndef GT_LOGSTREAM_H
#define GT_LOGSTREAM_H

#include "gt_logging_exports.h"

#include <sstream>
#include <iostream>

namespace gt
{

namespace log
{

//! Enum for verbosity log levels
enum Verbosity
{
    Silent = 0,
    Medium = 5,
    Everything = 9
};

//! Enum for setting certain flags of a stream
enum StreamFlag
{
    LogSpace = 2, // 0b0010 else no space
    LogQuote = 4  // 0b0100 else no quote
};

class Stream;
//! Helper class to restore state of a stream object once destroyed.
//! Stream object may not go out of scope before state saver does
class StreamStateSaver
{
public:

    //! ctor (copies states)
    explicit StreamStateSaver(Stream& s);
    //! dtor (reapplies states)
    ~StreamStateSaver();

private:
    Stream* stream;
    std::ios_base::fmtflags iosflags;
    int flags;
    int vlevel;
};

//! Main stream class for logging most types. May work with Qt symbols as well.
class Stream
{
    friend class StreamStateSaver;

public:

    Stream() {
        // manipulators
        m_stream << std::boolalpha;
    };
    ~Stream() = default;

    Stream(Stream const&) = delete;
    Stream(Stream&&) = delete;
    Stream& operator=(Stream const&) = delete;
    Stream& operator=(Stream&&) = delete;

    inline Stream& space()   { m_flags |=  LogSpace; return *this; }
    inline Stream& nospace() { m_flags &= ~LogSpace; return *this; }
    inline Stream& quote()   { m_flags |=  LogQuote; return *this; }
    inline Stream& noquote() { m_flags &= ~LogQuote; return *this; }

    Stream& medium() { return verbose(gt::log::Medium); }
    Stream& verbose(int level = gt::log::Everything)
    {
        m_vlevel = level;
        return *this;
    }

    std::string str() { return m_stream.str(); }

    GT_LOGGING_EXPORT static bool mayLog(int level);
    bool mayLog() const { return mayLog(m_vlevel); }
    bool mayLogSpace() const { return m_flags & LogSpace; }
    bool mayLogQuote() const { return m_flags & LogQuote; }

    // pod
    inline Stream& operator<<(std::nullptr_t) { return doLog("(nullptr)"); }
    inline Stream& operator<<(void const* t)
    {
        // format pointers
        StreamStateSaver s{*this};
        return nospace().
                doLog(std::hex).
                doLog("0x").doLog(reinterpret_cast<std::uint64_t>(t));
    }

    inline Stream& operator<<(bool t) { return doLog(t); }

    // chars
    inline Stream& operator<<(char t) { return doLog(t); }
    // indicate wide chars
    inline Stream& operator<<(char16_t t)
    {
        { // block for state saver
            StreamStateSaver s{*this};
            nospace().operator<<("u'").doLog(t);
        }
        return operator<<('\'');
    }
    inline Stream& operator<<(char32_t t)
    {
        { // block for state saver
            StreamStateSaver s{*this};
            nospace().operator<<("U'").doLog(t);
        }
        return operator<<('\'');
    }

    // ints
    inline Stream& operator<<(short t) { return doLog(t); }
    inline Stream& operator<<(unsigned short t) { return doLog(t); }
    inline Stream& operator<<(int t) { return doLog(t); }
    inline Stream& operator<<(unsigned int  t) { return doLog(t); }
    inline Stream& operator<<(long t) { return doLog(t); }
    inline Stream& operator<<(unsigned long t) { return doLog(t); }
    inline Stream& operator<<(long long t) { return doLog(t); }
    inline Stream& operator<<(unsigned long long t) { return doLog(t); }

    // floats
    inline Stream& operator<<(float t) { return doLog(t); }
    inline Stream& operator<<(double t) { return doLog(t); }

    // strings
    inline Stream& operator<<(const char* t) { return doLog(t); }
    inline Stream& operator<<(std::string const& t) { return doLog(t); }

    //! Default logging method
    template <typename T>
    inline Stream& doLog(T const& t)
    {
        if (mayLog())
        {
            m_stream << t;
            logSpace();
        }
        return *this;
    }

    //! Logging with quotes (if enabled)
    template <typename T>
    inline Stream& doLogQuoted(T const& t)
    {
        if (mayLog())
        {
            logQuote();
            m_stream << t;
            logQuote().logSpace();
        }
        return *this;
    }

private:

    /// flags
    int m_flags{gt::log::LogSpace};
    /// verbosity level
    int m_vlevel{gt::log::Silent};
    /// ostream
    std::ostringstream m_stream;

    //! Helper function to log ' '
    inline Stream& logSpace() noexcept
    {
        if (mayLogSpace()) m_stream << ' ';
        return *this;
    };

    //! Helper function to log '"'
    inline Stream& logQuote() noexcept
    {
        if (mayLogQuote()) m_stream << '"';
        return *this;
    };
};

} // namespace log

} // namespace gt

inline gt::log::StreamStateSaver::StreamStateSaver(Stream& s)
    : stream{&s}
    , iosflags{s.m_stream.flags()}
    , flags{s.m_flags}
    , vlevel{s.m_vlevel}
{}

inline gt::log::StreamStateSaver::~StreamStateSaver()
{
    stream->m_stream.setf(iosflags);
    stream->m_flags = flags;
    stream->m_vlevel = vlevel;
}

#endif // GT_LOGSTREAM_H