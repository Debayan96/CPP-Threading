#include<iostream>
#include<thread>
#include<mutex>
#include<vector>
#include<chrono>

using namespace std;

class Spork {
    public:
        mutex m;
};

class Philosopher {
    private:
        Spork &left_spork;
        Spork &right_spork;
        int id;
        bool work;
    public:
        Philosopher(int philosopher_id, Spork &left, Spork &right): id(philosopher_id), left_spork(left), right_spork(right), work(true) {}

        void think() {
            cout<<"Philosopher "<<id<<" is thinking"<<endl;
        }

        void dine() {
            cout<<"Philosopher "<<id<<" wants to dine"<<endl;

            unique_lock<mutex> left_lock(left_spork.m, defer_lock);
            unique_lock<mutex> right_lock(right_spork.m, defer_lock);

            lock(left_lock, right_lock);
            cout<<"Philosopher "<<id<<" has started eating"<<endl;
            this_thread::sleep_for(chrono::seconds(5));
            cout<<"Philosopher "<<id<<" has finished eating"<<endl;
            left_lock.unlock();
            right_lock.unlock();
        }

        void thinkAndDine() {
            while(work) {
                think();
                dine();
            }
        }
        ~Philosopher() {
            cout<<"entered destructor"<<endl;
            work = false;
        }

};

void testDiningPhilosopher(int n) {
    Spork sp[n];
    vector<thread> philosopher_list;
    
    auto activatePhilosophers = [&](int id) -> void {
        Philosopher p(id, sp[id % n], sp[(id + 1) % n]);
        p.thinkAndDine();
    };

    for(int i = 0; i < n; i++) {
        thread t(activatePhilosophers, i + 1);
        philosopher_list.emplace_back(move(t));
    }
    for(auto &t: philosopher_list) {
        t.join();
    }
    this_thread::sleep_for(chrono::seconds(5));
}

int main() {
    testDiningPhilosopher(5);
    return 0;
}