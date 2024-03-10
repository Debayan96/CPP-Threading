#include<iostream>
#include<thread>
#include<condition_variable>
#include<mutex>
#include<chrono>
#include<queue>

using namespace std;

enum class TrafficLightColor {
    Red,
    Green
};

class Vehicle {
    int vehicle_id;
    public:
        Vehicle(int id): vehicle_id(id) {}
        int get_id() {
            return vehicle_id;
        }
};

class TrafficLight {
    TrafficLightColor color;
    public:
        TrafficLight(TrafficLightColor starting_color=TrafficLightColor::Green): color(starting_color) {}
        TrafficLightColor get_color() {
            return color;
        }
        TrafficLightColor toggle() {
            color = (color == TrafficLightColor::Green? TrafficLightColor::Red : TrafficLightColor::Green);
            return color;
        }
};

class TrafficLightSystem {
    mutex color_check;
    mutex vehicle_queue_lock;
    condition_variable cv;
    queue<Vehicle> vehicle_queue;
    TrafficLight traffic_light;
    public:
        TrafficLightSystem(TrafficLight tl): traffic_light(tl) {}

        void add_to_queue(Vehicle v) {
            cout<<"Vehicle ID: "<< v.get_id()<<" entered the queue"<<endl;
            vehicle_queue.emplace(v);
            cv.notify_all();
        }

        void remove_from_queue() {
            auto is_light_green = [this]() -> bool {
                unique_lock<mutex> lock(color_check);
                return traffic_light.get_color() == TrafficLightColor::Green;
            };
            auto is_vehicle_available = [this]() -> bool {
                return !vehicle_queue.empty();
            };
            unique_lock<mutex> lock(vehicle_queue_lock);
            cv.wait(lock, [this, &is_light_green, &is_vehicle_available]() {
                return is_light_green() && is_vehicle_available();
            });
            Vehicle v = vehicle_queue.front();
            cout<<"Vehicle ID: "<< v.get_id()<<" exited the queue"<<endl;
            vehicle_queue.pop();
            cv.notify_all();
        }

        void change_traffic_signal() {
            while(true) {
                unique_lock<mutex> lock(color_check);
                string current_color = traffic_light.toggle() == TrafficLightColor::Green? "Green": "Red";
                cout<<":::: Light is "<<current_color<<" ::::"<<endl;
                lock.unlock();
                cv.notify_all();
                this_thread::sleep_for(chrono::seconds(5));
            }
        }

};

void test_traffic_light_controller(int vehicle_count, int vehicle_entry_time_in_seconds) {
    TrafficLight traffic_light;
    TrafficLightSystem traffic_light_system(traffic_light);
    auto insert_vehicles = [&traffic_light_system, vehicle_count, vehicle_entry_time_in_seconds]() -> void{
        for(int i = 0; i < vehicle_count; i++) {
            traffic_light_system.add_to_queue(Vehicle(i+1));
            this_thread::sleep_for(chrono::seconds(vehicle_entry_time_in_seconds));
        }
    };
    auto remove_vehicles = [&traffic_light_system, vehicle_count]() -> void{
        for(int i = 0; i < vehicle_count; i++) {
            traffic_light_system.remove_from_queue();
            this_thread::sleep_for(chrono::seconds(1));
        }
    };
    auto toggle_signal = [&traffic_light_system]() -> void{
        traffic_light_system.change_traffic_signal();
    };

    thread t1(insert_vehicles);
    thread t2(remove_vehicles);
    thread t3(toggle_signal);

    t1.join();
    t2.join();
    t3.join();
}

int main() {
    test_traffic_light_controller(50, 1);
    return 0;
}