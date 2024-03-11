#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <queue>

using namespace std;

class WebCrawler {
    private:
        queue<string> url_list;
        vector<thread> thread_list;
        mutex m;
        condition_variable cv;
        bool stop_crawler;

        void crawl() {
            while(true) {
                auto exit_from_wait = [this]()
                {
                    return stop_crawler || !url_list.empty();
                };
                unique_lock<mutex> lock(m);
                cv.wait(lock, exit_from_wait);

                if(stop_crawler) {
                    cout<<"Received stop request"<<endl;
                    return;
                }
                
                string url = url_list.front();
                url_list.pop();
                
                lock.unlock();
                cv.notify_all();
                
                cout<<"Parsing URL: "<<url<<endl;
                this_thread::sleep_for(chrono::seconds(2));
                cout<<"Finished parsing: "<<url<<endl;
            }
        }
    public:
        WebCrawler(int thread_count): stop_crawler(false) {
            for(int i = 0; i < thread_count; i++) {
                thread t(&WebCrawler::crawl, this);
                thread_list.push_back(std::move(t));
            }
        }

        void enqueue(string url) {
            unique_lock<mutex> lock(m);
            url_list.emplace(url);
            lock.unlock();
        }

        ~WebCrawler() {
            {
                unique_lock<mutex> lock(m);
                stop_crawler = true;
            }
            cv.notify_all();
            for(auto &t: thread_list) {
                t.join();
            }
        }
};

void test_WebCrawler(int thread_count) {
    WebCrawler webcrawler(2);
    webcrawler.enqueue("www.google.com");
    webcrawler.enqueue("www.github.com");
    webcrawler.enqueue("www.leetcode.com");
    this_thread::sleep_for(chrono::seconds(5));
}

int main() {
    test_WebCrawler(2);
    return 0;
}