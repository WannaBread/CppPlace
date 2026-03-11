#include "concurrency/ThreadPool.hpp"

namespace cppplace {

ThreadPool::ThreadPool(size_t num_threads)
    : num_threads_(num_threads), pool_(num_threads) {}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::stop() {
    pool_.join();
}

} // namespace cppplace
