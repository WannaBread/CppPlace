#pragma once

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <functional>
#include <cstddef>

namespace cppplace {

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    template <typename F>
    void post(F&& task) {
        boost::asio::post(pool_, std::forward<F>(task));
    }

    void stop();
    size_t size() const { return num_threads_; }

private:
    size_t num_threads_;
    boost::asio::thread_pool pool_;
};

} // namespace cppplace
