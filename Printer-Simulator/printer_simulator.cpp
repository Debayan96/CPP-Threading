#include<iostream>
#include<vector>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<chrono>

using namespace std;

class Printer {
    private:
        vector<mutex> printer_mutex;
        vector<condition_variable> printer_cv;
        vector<bool> in_use;
        int printer_count;
    public:
        Printer(int printers): printer_count(printers), printer_mutex(printers), printer_cv(printers), in_use(vector<bool>(printer_count, false)) {}

        void print_document(int user_id) {
            while(true) {
                // cout<<"User: "<<user_id<<" has submitted a request for printing document"<<endl;
                int printer_id = -1;
                for(int i = 0; i < printer_count; i++) {
                    if(printer_mutex[i].try_lock()) {
                        printer_id = i;
                        in_use[printer_id] = true;
                        break;
                    }
                }

                if(printer_id == -1) {
                    int allocated_printer = user_id % printer_count;
                    cout<<"All printers are busy. User "<< user_id<<" will wait for printer "<<(allocated_printer);
                    unique_lock<mutex> lock(printer_mutex[allocated_printer]);
                    auto retry_now = [&]() -> bool {
                        return !in_use[allocated_printer];
                    };
                    printer_cv[allocated_printer].wait(lock, retry_now);
                } else {
                    cout<<"Printing document for user "<<user_id<<" using printer "<<printer_id<<endl;
                    this_thread::sleep_for(chrono::seconds(1));
                    cout<<"Printed document for user "<<user_id<<" using printer "<<printer_id<<endl;
                    printer_mutex[printer_id].unlock();
                    in_use[printer_id] = false;
                    printer_cv[printer_id].notify_one();
                    cout<<"User "<<user_id<<" exiting queue"<<endl<<endl;
                    break;
                }
            }
        }
};

void testPrinter(int printers, int users) {
    Printer printer(printers);

    vector<thread> user_list;
    auto start_printing = [&](int user_id) -> void {
        printer.print_document(user_id);
    };
    for(int i = 0; i < users; i++) {
        thread t(start_printing, i);
        user_list.emplace_back(move(t));
    }

    for(auto &t: user_list) {
        t.join();
    }
}

int main() {
    testPrinter(5, 20);
    return 0;
}