#ifndef __EVENTPROCESS_H__
#define __EVENTPROCESS_H__


#include <vector>
#include <queue>
#include <mutex>
#include <thread>

#include <Fragment.h>

class EventProcess {
  public:
    virtual ~EventProcess(); 
    static EventProcess *Get();

    //void push(std::unique_ptr<Fragment> frag);
    //bool pop(std::vector<Fragment> &Builtfrags);  
    //bool peek(Fragment*& out);

    void loop(); // monitor the queue and decide when to do useful things.

    void Stop() { fStop = true; }

    bool     Running() const { return !fStop.load(); }
    uint32_t Pushed() const { return fPushed.load(); }
    uint32_t Popped() const { return fPopped.load(); }


  private:
    EventProcess();

  private:
    static EventProcess *fEventProcess;

    std::mutex fMutex;
    //std::priority_queue<std::unique_ptr<Fragment>,std::vector<std::unique_ptr<Fragment> > > fQueue;

    std::atomic<uint32_t> fPushed{0};
    std::atomic<uint32_t> fPopped{0};

    std::atomic_bool fStop{false};
    std::thread fWorker;
};




#endif
