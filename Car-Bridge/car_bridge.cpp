#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<chrono>
#include<vector>

using namespace std;

class Bridge {
    private:
        int max_cars;
        int current_cars;
        mutex m;
        condition_variable cv;

    public:
        Bridge(int max_allowed_cars): max_cars(max_allowed_cars), current_cars(0) {}
        void passThroughBridge(int id) {
            auto canEnter = [this]() -> bool {
                return current_cars < max_cars;
            };
            
            cout<<"Car "<<id<<" in front of the bridge"<<endl;
            unique_lock<mutex> lock(m);
            cv.wait(lock, canEnter);
            current_cars++;
            cout<<"Car "<<id<<" entered the bridge"<<endl;
            lock.unlock();
            
            this_thread::sleep_for(chrono::seconds(1));
            cout<<"Car "<<id<<" exiting the bridge"<<endl;
            lock.lock();
            current_cars--;
            cout<<"Car "<<id<<" exited the bridge"<<endl;
            lock.unlock();
            cv.notify_one();
        }   
};

void testBridge(int max_cars, int passing_cars) {
    Bridge b(max_cars);
    auto useBridge = [&](int id) -> void {
        b.passThroughBridge(id);
    };

    vector<thread> cars;
    for(int i = 1; i <= passing_cars; i++) {
        thread t(useBridge, i);
        cars.emplace_back(move(t));
    }

    for(auto &t: cars) {
        t.join();
    }
}

int main() {
    testBridge(3, 10);
    return 0;
}