#include<chrono>
#include<thread>
#include<iostream>
#include<condition_variable>
#include<mutex>
#include<vector>

using namespace std;

class RateLimiter {
    private:
        mutex m;
        condition_variable cv;
        int max_allowed_requests;
        int current_request_in_queue;
        chrono::seconds max_request_processing_time;
    public:
        RateLimiter(int max_requests, int processing_time): max_allowed_requests(max_requests), max_request_processing_time(processing_time), current_request_in_queue(0) {}
        void run() {
            cout<<"Entered run block"<<endl;
            auto rateAllowed = [this]() -> bool {
                return max_allowed_requests >= current_request_in_queue;
            };
            unique_lock<mutex> lock(m);
            cv.wait(lock, rateAllowed);

            current_request_in_queue++;
            cout<<"Processing Request"<<endl;
            this_thread::sleep_for(max_request_processing_time);
            current_request_in_queue--;
            cout<<"Finished Processing Request"<<endl;
            cv.notify_one();
        }
};

void test_rate_limiter(int max_requests, int processing_time, int number_of_threads) {
    RateLimiter rate_limiter(max_requests, processing_time);
    auto send_rate_limiting_requests = [&rate_limiter, max_requests]() -> void {
        for(int i = 0; i < 2 * max_requests; i++) {
            rate_limiter.run();
        }
    };
    vector<thread> thread_list;
    for(int i = 0; i < number_of_threads; i++) {
        thread t(send_rate_limiting_requests);
        thread_list.push_back(std::move(t));
    }

    for(auto &t: thread_list)
        t.join();
}

int main() {
    test_rate_limiter(2, 1, 10);
    return 0;
}