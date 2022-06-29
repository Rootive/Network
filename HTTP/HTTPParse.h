#ifndef ROOTIVE_HTTPPARSE_H
#define ROOTIVE_HTTPPARSE_H

#include <list>
#include <functional>


namespace Rootive
{
struct HTTPParse
{
    enum EState
    {
        StartLine, Headers, EntityBody, Finished, Paused
    };
    enum EError
    {
        StartLineParseError, ContentLengthInvalid, HeaderParseError, HeaderKeyLost, HeaderValueLost
    };
    enum EType
    {
        Request, Response, HeadResponse
    };
    typedef std::pair<int, int> Offset;
    typedef std::function<void(const HTTPParse *)> StartLineFinishedCallback;               // Omitted
    typedef std::function<void(const HTTPParse *)> HeadersChangedCallback;                  // mean the back of headersList_ is new; will not call when blank line
    typedef std::function<void(const HTTPParse *)> HeadersFinishedCallback;                 // call when blank line
    typedef std::function<void(const HTTPParse *)> FinishedCallback;                        // mean entitybody finished
    typedef std::function<void(const HTTPParse *)> UnparsedLengthIncreaseCallback;          // have got new data but no crlf or entitybody.size < content-length
    typedef std::function<void(const HTTPParse *, EError, int, int)> ErrorOccurredCallback; // error line will be ignored if continued
    Offset method_;
    Offset url_;
    Offset version_;
    Offset status_;
    Offset reasonPhrase_;
    std::list<std::pair<Offset, Offset> > headersList_;

    HTTPParse(EType type) :
    state_(StartLine), type_(type), contentLength_(0), offset_(0), unparsedOffset_(0)
    {
        clearOffset(method_);
        clearOffset(url_);
        clearOffset(version_);
        clearOffset(status_);
        clearOffset(reasonPhrase_);
        clearOffset(entityBody_);
    }

    static inline void clearOffset(Offset &offset) { offset.first = 0; offset.second = 0; }
    static inline void setOffset(Offset &offset, int first, int second) { offset.first = first; offset.second = second; }

    inline bool bFinished() const { return state_ == Finished; }
    inline bool bPaused() const { return state_ == Paused; }
    inline EState getState() const { return state_; }
    inline EType getType() const { return type_; }
    inline int getContentLength() const { return contentLength_; }
    inline int getOffset() const { return offset_; }
    inline int getUnparsedOffset() const { return unparsedOffset_; }
    inline const Offset &getEntityBody() const { return entityBody_; }

    inline void setStartLineFinishedCallback(const StartLineFinishedCallback &callback) { startLineFinishedCallback_ = callback; }
    inline void setHeadersChangedCallback(const HeadersChangedCallback &callback) { headersChangedCallback_ = callback; }
    inline void setHeadersFinishedCallback(const HeadersFinishedCallback &callback) { headersFinishedCallback_ = callback; }
    inline void setFinishedCallback(const FinishedCallback &callback) { finishedCallback_ = callback; }
    inline void setUnparsedLengthIncreaseCallback(const UnparsedLengthIncreaseCallback &callback) { unparsedLengthIncreaseCallback_ = callback; }
    inline void setErrorOccurredCallback(const ErrorOccurredCallback &callback) { errorOccurredCallback_ = callback; }

    inline int unparsedLength() const { return unparsedOffset_ - offset_; }

    inline void pause() { state_ = Paused; }
    void update(const char *data, int size);
    void clear(EType type);
private:
    EState state_;
    EType type_;
    int contentLength_;
    int offset_;
    int unparsedOffset_;
    Offset entityBody_;

    StartLineFinishedCallback startLineFinishedCallback_;
    HeadersChangedCallback headersChangedCallback_;
    HeadersFinishedCallback headersFinishedCallback_;
    FinishedCallback finishedCallback_;
    UnparsedLengthIncreaseCallback unparsedLengthIncreaseCallback_;
    ErrorOccurredCallback errorOccurredCallback_;

    static inline int indexOf(char ch, const char *data, const int begin, const int end)
    {
        int ret;
        for (ret = begin; ret < end; ++ret)
        {
            if (data[ret] == ch) { break; }
        }
        return ret;
    }
    static inline int indexOfNot(char ch, const char *data, const int begin, const int end)
    {
        int ret;
        for (ret = begin; ret < end; ++ret)
        {
            if (data[ret] != ch) { break; }
        }
        return ret;
    }
    static inline int lastIndexOfNot(char ch, const char *data, const int begin, const int end)
    {
        int ret;
        for (ret = end; ret >= begin; --ret)
        {
            if (data[ret] != ch) { break; }
        }
        return ret;
    }
    void updateStartLine(const char *data, int end);
    void updateHeader(const char *data, int end);
};
}

#endif