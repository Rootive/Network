#include "DoubleBufferingOutput.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace Rootive;

DoubleBufferingOutput::DoubleBufferingOutput(OutputFileInterface *file, const std::chrono::seconds &interval) : 
file_(file), interval_(interval), bRun_(false)
{
    thread_ = new std::thread(&DoubleBufferingOutput::threadFunc, this);
    while (!bRun_);
}

DoubleBufferingOutput::~DoubleBufferingOutput()
{
    bRun_ = false;
    thread_->join();
    delete thread_;
}

void DoubleBufferingOutput::write(const void *data, size_t size)
{
    {
    std::lock_guard<std::mutex> lock(mutexAvailable_);
    available_->append(static_cast<const char *>(data), size);
    }
    condAvailable_.notify_one();
}

void DoubleBufferingOutput::threadFunc()
{
    available_ = new std::string;
    fulled_ = new std::string;
    bRun_ = true;
    do
    {
        {
        std::unique_lock<std::mutex> lock(mutexAvailable_);
        condAvailable_.wait_for(lock, interval_, [this]()-> bool { return available_->size(); } );
        std::swap(available_, fulled_);
        }
        file_->write(fulled_->data(), fulled_->size());
        fulled_->clear();
        file_->flush();
    } while (bRun_);
    file_->write(fulled_->data(), fulled_->size());
    {
    std::lock_guard<std::mutex> lock(mutexAvailable_);
    file_->write(available_->data(), available_->size());
    }
    delete available_;
    delete fulled_;
}