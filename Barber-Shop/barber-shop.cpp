#include<iostream>
#include<chrono>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<thread>

using namespace std;

class BarberShop {
    private:
        thread barber;
        int maximum_chairs_available;
        int current_available_chairs;
        mutex m;
        condition_variable cv;
        bool stop;
        chrono::seconds hair_cut_time;
        int current_waiting_customer;
        int current_serving_customer;

        void run() {
            auto readyToWork = [this]() -> bool {
                return stop || current_available_chairs > 0;
            };
            while(true) {
                cout<<"Barber is ready to work!!!"<<endl;
                unique_lock<mutex> lock(m);
                cv.wait(lock, readyToWork);

                if(stop) {
                    break;
                }

                current_available_chairs--;
                current_serving_customer++;
                cout<<"Cutting hair for customer "<<current_serving_customer<<endl;
                this_thread::sleep_for(hair_cut_time);
                cout<<"Finished cutting hair for customer "<<current_serving_customer<<endl;
            }
        }
    public:
        BarberShop(int max_chairs, int serving_time): maximum_chairs_available(max_chairs), current_available_chairs(0), current_waiting_customer(0), current_serving_customer(0), hair_cut_time(serving_time), stop(false) {
            barber = thread(&BarberShop::run, this);
        }
        ~BarberShop() {
            {
                unique_lock<mutex> lock(m);
                stop = true;
            }
            cv.notify_all();
            barber.join();
        }
        void wait() {
            unique_lock<mutex> lock(m);
            cout<<"Customer entered the shop"<<endl;
            if(current_available_chairs >= maximum_chairs_available) {
                cout<<"No available chairs found. Customer is leaving the shop"<<endl;
            } else {
                current_waiting_customer++;
                current_available_chairs++;
                cout<<"Customer "<<current_waiting_customer<<" is waiting for their turn"<<endl;
            }
            lock.unlock();
            cv.notify_all();
        }
};

void test_barber_shop(int max_chairs, int customer_count, int serving_time) {
    BarberShop barber_shop(max_chairs, serving_time);
    vector<thread> customer_list;
    auto wait_for_hair_cut = [&]() -> void {
        thread t(&BarberShop::wait, &barber_shop);
        customer_list.emplace_back(std::move(t));
    };

    for(int i = 0; i < customer_count; i++) {
        wait_for_hair_cut();
    }

    for(auto &t: customer_list) {
        t.join();
    }
    this_thread::sleep_for(chrono::seconds(20));
}

int main() {
    test_barber_shop(4, 20, 2);
    return 0;
}