#include <chrono>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <vector>

using namespace std;

class BuildWater {
private:
    bool stop;
    mutex m;
    condition_variable cv;
    int turn;

public:
    BuildWater() : stop(false), turn(0) {}
    ~BuildWater() {
        {
            unique_lock<mutex> lock(m);
            stop = true;
        }
        cv.notify_all();
    }
    void add_hydrogen() {
        auto is_my_turn = [this]() -> bool {
            return stop || turn != 2;
        };

        unique_lock<mutex> lock(m);
        cv.wait(lock, is_my_turn);

        cout << "H";
        turn++;
        cv.notify_all();
    }

    void add_oxygen() {
        auto is_my_turn = [this]() -> bool {
            return stop || turn == 2;
        };

        unique_lock<mutex> lock(m);
        cv.wait(lock, is_my_turn);

        cout << "O" << endl;
        turn = 0;
        cv.notify_all();
    }
};

void testBuildWater(int water_molecules) {
    BuildWater bw;
    int oxygen = water_molecules;
    int hydrogen = water_molecules * 2;

    vector<thread> thread_list;
    for (int i = 0; i < hydrogen; i++) {
        thread_list.emplace_back(&BuildWater::add_hydrogen, &bw);
    }
    for (int i = 0; i < oxygen; i++) {
        thread_list.emplace_back(&BuildWater::add_oxygen, &bw);
    }

    for (auto& t : thread_list) {
        t.join();
    }

    this_thread::sleep_for(chrono::seconds(10));
}

int main() {
    testBuildWater(5);
    return 0;
}
