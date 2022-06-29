#include "LogFile.h"

namespace Rootive
{
LogFile::LogFile(const char *filename, int fileSizeExpected) :
fileSizeExpected_(fileSizeExpected)
{
    size_t filenameSize = strlen(filename);
    filenameSize_ = std::min(filenameSize, static_cast<size_t>(constexpr_maxFilenameSize));
    memcpy(filename_, filename, filenameSize);
    _roll();
}
}