#include<iostream>
#include<thread>
#include<chrono>
#include<mutex>
#include<condition_variable>
#include<vector>

using namespace std;

class ReaderWriter {
    private:
        mutex m;
        condition_variable cv;
        bool writing;
        int readers;
    public:
        ReaderWriter(): writing(false), readers(0) {}
        void read(int id) {
            cout<<"Reader "<<id<<" is trying to read the document"<<endl;
            auto is_read_allowed = [this]() -> bool {
                return !writing;
            };
            unique_lock<mutex> lock(m);
            cv.wait(lock, is_read_allowed);

            cout<<"Reader "<<id<<" reading document"<<endl;
            readers++;
            lock.unlock();
            this_thread::sleep_for(chrono::seconds(2));

            lock.lock();
            cout<<"Reader "<<id<<" finished reading the document"<<endl;
            readers--;
            lock.unlock();
            cv.notify_all();
        }
        void write(int id) {
            cout<<"Writer "<<id<<" is trying to write into the document"<<endl;
            auto is_write_allowed = [this]() -> bool{
                return readers == 0 && !writing;
            };
            unique_lock<mutex> lock(m);
            cv.wait(lock, is_write_allowed);

            cout<<"Writer "<<id<<" writing in document"<<endl;
            writing = true;
            lock.unlock();
            this_thread::sleep_for(chrono::seconds(5));

            lock.lock();
            cout<<"Writer "<<id<<" finished writing to the document"<<endl;
            writing = false;
            lock.unlock();
            cv.notify_all();
        }
};

void testReaderWriter(int readers, int writers) {
    ReaderWriter rw = ReaderWriter();

    vector<thread> reader_list;
    vector<thread> writer_list;

    auto writeToFile = [&](int id) -> void {
        rw.write(id);
    };
    auto readFromFile = [&](int id) -> void {
        rw.read(id);
    };

    for(int i = 0; i < readers; i++) {
        thread t(readFromFile, i);
        reader_list.emplace_back(move(t));
    }
    for(int i = 0; i < writers; i++) {
        thread t(writeToFile, i);
        writer_list.emplace_back(move(t));
    }

    for(auto &t: reader_list) {
        t.join();
    }
    for(auto &t: writer_list) {
        t.join();
    }
}

int main() {
    testReaderWriter(10, 2);
    return 0;
}