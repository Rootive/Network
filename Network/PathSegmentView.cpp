#include "PathSegmentView.h"

namespace Rootive
{
bool PathSegmentView::set(const StringView &data)
{
    clear();
    int pIndex = data.indexOf(';');
    int qIndex = data.indexOf('?');
    if (pIndex == data.getLength()) { pIndex = qIndex; }
    else if (qIndex < pIndex) { return false; }
    path_ = data.mid(0, pIndex);

    std::vector<StringView> vector;

    if (pIndex < qIndex)
    {
        vector = data.mid(pIndex + 1, qIndex).split('&');
        for (const StringView &p : vector)
        {
            int eIndex = p.indexOf('=');
            if (eIndex != p.getLength())
            {
                StringView key = p.mid(0, eIndex);
                StringView value = p.mid(eIndex + 1, p.getLength());
                if (!key.bEmpty() && !value.bEmpty()) { paramsMap_[key] = value; }
                else { return false; }
            }
            else { return false; }
        }
    }

    if (qIndex != data.getLength())
    {
        vector = data.mid(qIndex, data.getLength()).split('&');
        for (const StringView &q : vector) 
        {
            int eIndex = q.indexOf('=');
            if (eIndex != q.getLength())
            {
                StringView key = q.mid(0, eIndex);
                StringView value = q.mid(eIndex + 1, q.getLength());
                if (!key.bEmpty() && !value.bEmpty()) { queryMap_[key] = value; }
                else { return false; }
            }
            else { return false; }
        }
    }
    return true;
}
}