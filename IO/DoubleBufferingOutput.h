#ifndef ROOTIVE_DOUBLEBUFFERINGOUTPUT_H
#define ROOTIVE_DOUBLEBUFFERINGOUTPUT_H

#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <memory>
#include <condition_variable>
#include <chrono>
#include "IO/OutputFileInterface.h"
#include "IO/OutputInterface.h"

namespace Rootive
{
class DoubleBufferingOutput : public OutputInterface
{
public:

    DoubleBufferingOutput(OutputFileInterface *file = new DefaultOutputFile, const std::chrono::seconds &interval = std::chrono::seconds(3));
    virtual ~DoubleBufferingOutput();
    virtual void write(const void *data, size_t size) override;

private:

    std::unique_ptr<OutputFileInterface> file_;

    std::string *available_;
    std::string *fulled_;
    std::chrono::seconds interval_;

    std::mutex mutexAvailable_;
    std::condition_variable condAvailable_;

    std::atomic<bool> bRun_;
    std::thread *thread_;

    void threadFunc();

};

}

#endif