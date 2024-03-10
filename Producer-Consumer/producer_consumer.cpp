#include <iostream>
#include <chrono>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>

using namespace std;

class BoundedBuffer {
    int buffer_size;
    queue<int> buffer;
    mutex m;
    condition_variable cv;

    public:
        BoundedBuffer(int size): buffer_size(size) {}

        void enqueue(int item) {
            auto is_space_available = [this]() -> bool {
                return buffer.size() < buffer_size;
            };
            unique_lock<mutex> lock(m);
            cv.wait(lock, is_space_available);

            buffer.emplace(item);
            cout<<"Inserted: "<< item << " to queue"<<endl;
            cv.notify_all();
        }

        void dequeue() {
            auto is_not_empty = [this]() -> bool {
                return !buffer.empty();
            };
            unique_lock<mutex> lock(m);
            cv.wait(lock, is_not_empty);

            int item = buffer.front();
            buffer.pop();
            cout<<"Popped: "<< item << " from queue"<<endl;
            cv.notify_all();
        }

};

void test_BoundedBuffer(int size) {
    BoundedBuffer bounded_buffer(size);
    auto produce = [&]() -> void {
        for(int i = 0; i < 4 * size; i++) {
            bounded_buffer.enqueue(i + 1);
            this_thread::sleep_for(chrono::seconds(1));
        }
    };
    auto consume = [&]() -> void {
        for(int i = 0; i < 4 * size; i++) {
            bounded_buffer.dequeue();
            this_thread::sleep_for(chrono::seconds(3));
        }
    };
    thread t1(produce);
    thread t2(consume);

    t1.join();
    t2.join();
}

int main() {
    test_BoundedBuffer(10);
    return 0;
}