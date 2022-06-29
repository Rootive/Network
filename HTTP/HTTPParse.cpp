#include "HTTPParse.h"
#include <cctype>
#include <sstream>

namespace Rootive
{
void HTTPParse::update(const char *data, int size)
{
    int before = unparsedLength();
    while (true)
    {
        if (state_ == StartLine)
        {
            if (size - unparsedOffset_ > 1)
            {
                int begin;
                int end = size - 1;
                for (begin = unparsedOffset_; begin != end; ++begin)
                {
                    if (data[begin] == '\r' && data[begin + 1] == '\n')
                    {
                        updateStartLine(data, begin);
                        unparsedOffset_ = offset_ = begin + 2;
                        break;
                    }
                }
                if (begin == end)
                {
                    unparsedOffset_ = end;
                }
            }
            else
            {
                break;
            }
        }
        else if (state_ == Headers)
        {
            if (size - unparsedOffset_ > 1)
            {
                int begin;
                int end = size - 1;
                for (begin = unparsedOffset_; begin != end; ++begin)
                {
                    if (data[begin] == '\r' && data[begin + 1] == '\n')
                    {
                        if (begin == offset_)
                        {
                            if (contentLength_)
                            {
                                state_ = EntityBody;
                                unparsedOffset_ = offset_ = entityBody_.first = begin + 2;
                                if (headersFinishedCallback_) { headersFinishedCallback_(this); }
                            }
                            else
                            {
                                state_ = Finished;
                                unparsedOffset_ = offset_ = begin + 2;
                                if (headersFinishedCallback_) { headersFinishedCallback_(this); }
                                if (finishedCallback_) { finishedCallback_(this); }
                            }
                        }
                        else
                        {
                            updateHeader(data, begin);
                            unparsedOffset_ = offset_ = begin + 2;
                        }
                        break;
                    }
                }
                if (begin == end)
                {
                    unparsedOffset_ = end;
                }
            }
            else
            {
                break;
            }
        }
        else if (state_ == EntityBody)
        {
            if (size - unparsedOffset_ > 0)
            {
                int endExpected = entityBody_.first + contentLength_;
                if (endExpected <= size)
                {
                    unparsedOffset_ = offset_ = entityBody_.second = endExpected;
                    state_ = Finished;
                    if (finishedCallback_) { finishedCallback_(this); }
                }
                else
                {
                    unparsedOffset_ = size;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    if (unparsedLength() - before)
    {
        if (unparsedLengthIncreaseCallback_) { unparsedLengthIncreaseCallback_(this); }
    }
}
void HTTPParse::clear(EType type)
{
    state_ = StartLine; 
    type_ = type;
    contentLength_ = 0;
    offset_ = 0;
    unparsedOffset_ = 0;
    clearOffset(method_);
    clearOffset(url_);
    clearOffset(version_);
    clearOffset(status_);
    clearOffset(reasonPhrase_);
    clearOffset(entityBody_);
}
void HTTPParse::updateStartLine(const char *data, int end)
{
    int ll = indexOfNot(' ', data, offset_, end);
    int lr = indexOf(' ', data, ll, end);
    int ml = indexOfNot(' ', data, lr, end);
    int mr = indexOf(' ', data, ml, end);
    int rl = indexOfNot(' ', data, mr, end);
    int rr = lastIndexOfNot(' ', data, offset_, end) + 1;
    if (rl == end)
    {
        if (errorOccurredCallback_) { errorOccurredCallback_(this, EError::StartLineParseError, offset_, end); }
    }
    else
    {
        switch (type_)
        {
        case Request:
            setOffset(method_, ll, lr);
            setOffset(url_, ml, mr);
            setOffset(version_, rl, rr);
        break;
        case Response:
        case HeadResponse:
            setOffset(version_, ll, lr);
            setOffset(status_, ml, mr);
            setOffset(reasonPhrase_, rl, rr);
        break;
        }
        state_ = Headers;
        if (startLineFinishedCallback_) { startLineFinishedCallback_(this); }
    }
}
void HTTPParse::updateHeader(const char *data, int end)
{
    int m = indexOf(':', data, offset_, end);
    if (m == end)
    {
        if (errorOccurredCallback_) 
        { errorOccurredCallback_(this, EError::HeaderParseError, offset_, end); }
    }
    else
    {
        int ll = indexOfNot(' ', data, offset_, end);
        int lr = lastIndexOfNot(' ', data, ll, m - 1) + 1;
        if (ll < m)
        {
            int rl = indexOfNot(' ', data, m + 1, end);
            int rr = lastIndexOfNot(' ', data, rl, end - 1) + 1;
            if (rl < end)
            {
                if (lr - ll == 14)
                {
                    if (std::equal(data + ll, data + lr, "content-length", [](char a, char b)->bool { return ::tolower(a) == b; }))
                    {
                        try
                        {
                            contentLength_ = std::stoul(std::string(data + rl, data + rr));
                        }
                        catch(const std::exception& e)
                        {
                            contentLength_ = 0;
                            if (errorOccurredCallback_)
                            { errorOccurredCallback_(this, EError::ContentLengthInvalid, offset_, end); }
                        }
                    }
                }
                headersList_.push_back(std::pair<Offset, Offset>(Offset(ll, lr), Offset(rl, rr)));
                if (headersChangedCallback_) { headersChangedCallback_(this); }
            }
            else
            {
                if (errorOccurredCallback_)
                { errorOccurredCallback_(this, EError::HeaderValueLost, offset_, end); }
            }
        }
        else
        {
            if (errorOccurredCallback_)
            { errorOccurredCallback_(this, EError::HeaderKeyLost, offset_, end); }
        }
    }
}
}
