#include <gtest/gtest.h>
#include "services/CanvasService.hpp"
#include "services/UserStore.hpp"
#include "concurrency/ThreadPool.hpp"
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <set>

using namespace cppplace;

class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        palette = std::make_shared<Palette>(Palette::createDefault());
        session_manager = std::make_shared<SessionManager>();
        cooldown_manager = std::make_shared<CooldownManager>(std::chrono::seconds(0));
        event_bus = std::make_shared<EventBus>();
        service = std::make_unique<CanvasService>(
            100, 100, palette, session_manager, cooldown_manager, event_bus
        );
    }

    std::shared_ptr<Palette> palette;
    std::shared_ptr<SessionManager> session_manager;
    std::shared_ptr<CooldownManager> cooldown_manager;
    std::shared_ptr<EventBus> event_bus;
    std::unique_ptr<CanvasService> service;
};

TEST_F(ThreadSafetyTest, ConcurrentPixelPlacements) {
    const int NUM_THREADS = 8;
    const int PIXELS_PER_THREAD = 100;

    // Create tokens for each thread
    std::vector<std::string> tokens;
    for (int i = 0; i < NUM_THREADS; ++i) {
        std::string username = "user" + std::to_string(i);
        tokens.push_back(session_manager->createSession(username));
    }

    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    {
        ThreadPool pool(NUM_THREADS);

        for (int t = 0; t < NUM_THREADS; ++t) {
            pool.post([&, t]() {
                for (int i = 0; i < PIXELS_PER_THREAD; ++i) {
                    size_t x = (t * PIXELS_PER_THREAD + i) % 100;
                    size_t y = (t * PIXELS_PER_THREAD + i) / 100;
                    uint8_t color = static_cast<uint8_t>(t % 16);

                    auto result = service->placePixel(tokens[t], x, y, color);
                    if (result.ok()) {
                        success_count++;
                    } else {
                        failure_count++;
                    }
                }
            });
        }
        // ThreadPool destructor waits for all tasks
    }

    EXPECT_EQ(success_count + failure_count, NUM_THREADS * PIXELS_PER_THREAD);
    EXPECT_GT(success_count.load(), 0);

    // Canvas should have valid state
    auto state = service->getCanvasState();
    EXPECT_EQ(state.size(), 10000u);
}

TEST_F(ThreadSafetyTest, ConcurrentConnectDisconnect) {
    const int NUM_THREADS = 8;

    std::vector<std::string> tokens;
    for (int i = 0; i < NUM_THREADS; ++i) {
        tokens.push_back(session_manager->createSession("user" + std::to_string(i)));
    }

    {
        ThreadPool pool(NUM_THREADS);

        // Connect all
        for (int t = 0; t < NUM_THREADS; ++t) {
            pool.post([&, t]() {
                service->connectUser(tokens[t]);
            });
        }
    }

    EXPECT_EQ(service->getOnlineCount(), static_cast<size_t>(NUM_THREADS));

    {
        ThreadPool pool(NUM_THREADS);

        // Disconnect all
        for (int t = 0; t < NUM_THREADS; ++t) {
            pool.post([&, t]() {
                service->disconnectUser(tokens[t]);
            });
        }
    }

    EXPECT_EQ(service->getOnlineCount(), 0u);
}

TEST_F(ThreadSafetyTest, ConcurrentUserRegistration) {
    UserStore store;
    const int NUM_THREADS = 8;
    const int USERS_PER_THREAD = 50;

    std::atomic<int> success_count{0};
    std::atomic<int> duplicate_count{0};

    {
        ThreadPool pool(NUM_THREADS);

        for (int t = 0; t < NUM_THREADS; ++t) {
            pool.post([&, t]() {
                for (int i = 0; i < USERS_PER_THREAD; ++i) {
                    // Some usernames will collide across threads
                    std::string username = "user" + std::to_string(i);
                    auto result = store.registerUser(username, "pass" + std::to_string(t));
                    if (result.ok()) {
                        success_count++;
                    } else {
                        duplicate_count++;
                    }
                }
            });
        }
    }

    // Each unique username should be registered exactly once
    EXPECT_EQ(store.userCount(), static_cast<size_t>(USERS_PER_THREAD));
    EXPECT_EQ(success_count.load(), USERS_PER_THREAD);
    EXPECT_EQ(duplicate_count.load(), USERS_PER_THREAD * (NUM_THREADS - 1));
}

TEST_F(ThreadSafetyTest, ConcurrentSessionCreation) {
    const int NUM_THREADS = 8;
    const int SESSIONS_PER_THREAD = 50;

    std::mutex token_mutex;
    std::set<std::string> all_tokens;

    {
        ThreadPool pool(NUM_THREADS);

        for (int t = 0; t < NUM_THREADS; ++t) {
            pool.post([&, t]() {
                for (int i = 0; i < SESSIONS_PER_THREAD; ++i) {
                    auto token = session_manager->createSession("user" + std::to_string(t));
                    std::lock_guard<std::mutex> lock(token_mutex);
                    all_tokens.insert(token);
                }
            });
        }
    }

    // All tokens should be unique
    EXPECT_EQ(all_tokens.size(), static_cast<size_t>(NUM_THREADS * SESSIONS_PER_THREAD));
}

TEST_F(ThreadSafetyTest, ConcurrentEventPublishing) {
    const int NUM_THREADS = 8;
    const int EVENTS_PER_THREAD = 100;

    std::atomic<int> received_count{0};
    event_bus->subscribe([&](const Event&) {
        received_count++;
    });

    {
        ThreadPool pool(NUM_THREADS);

        for (int t = 0; t < NUM_THREADS; ++t) {
            pool.post([&, t]() {
                for (int i = 0; i < EVENTS_PER_THREAD; ++i) {
                    event_bus->publish(PixelPlacedEvent{
                        static_cast<size_t>(i),
                        static_cast<size_t>(t),
                        static_cast<uint8_t>(t % 16),
                        "user" + std::to_string(t)
                    });
                }
            });
        }
    }

    EXPECT_EQ(received_count.load(), NUM_THREADS * EVENTS_PER_THREAD);
}

TEST_F(ThreadSafetyTest, ConcurrentReadWrite) {
    // One thread writing, multiple threads reading
    const int NUM_READERS = 4;
    const int NUM_WRITES = 200;
    const int READS_PER_READER = 200;

    auto writer_token = session_manager->createSession("writer");

    std::atomic<bool> writing_done{false};
    std::atomic<int> read_count{0};

    {
        ThreadPool pool(NUM_READERS + 1);

        // Writer thread
        pool.post([&]() {
            for (int i = 0; i < NUM_WRITES; ++i) {
                size_t x = i % 100;
                size_t y = i / 100;
                service->placePixel(writer_token, x, y, static_cast<uint8_t>(i % 16));
            }
            writing_done = true;
        });

        // Reader threads
        for (int r = 0; r < NUM_READERS; ++r) {
            pool.post([&]() {
                for (int i = 0; i < READS_PER_READER; ++i) {
                    auto state = service->getCanvasState();
                    EXPECT_EQ(state.size(), 10000u);
                    read_count++;
                }
            });
        }
    }

    EXPECT_TRUE(writing_done.load());
    EXPECT_EQ(read_count.load(), NUM_READERS * READS_PER_READER);
}
