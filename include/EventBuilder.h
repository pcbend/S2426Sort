#ifndef __EVENTBUILDER_H__
#define __EVENTBUILDER_H__

#include <vector>
#include <queue>
#include <mutex>
#include <thread>

#include <Fragment.h>


struct CompareFragmentPtrs {
    bool operator()(const std::unique_ptr<Fragment>& a, const std::unique_ptr<Fragment>& b) const {
      if(!a.get() && !b.get()) return false;
      if (!a.get()) return true;  
      if (!b.get()) return false; 

      //a.get()->Print();
      //b.get()->Print();
      return a.get()->Timestamp() > b.get()->Timestamp();
    }
};




class EventBuilder {
  public:
    virtual ~EventBuilder(); 
    static EventBuilder *Get();

    void push(std::unique_ptr<Fragment> frag);
    bool pop(std::vector<std::unique_ptr<Fragment> > &Builtfrags);  
    //bool push(Fragment *frag);
    //bool pop(std::vector<Fragment*> &Builtfrags);  
    //bool peek(Fragment*& out);

    void loop(); // monitor the queue and decide when to do useful things.

    
    void Stop() { fStop = true; }

    bool     Running() const { return !fStop.load(); }
    uint32_t Size()    const { std::lock_guard lk(fMutex); return fQueue.size(); }
    uint32_t Pushed()  const {  return fPushed.load(); }
    uint32_t Popped()  const {  return fPopped.load(); }


  private:
    EventBuilder();

  private:
    static EventBuilder *fEventBuilder;

    long fLastTimestamp{-1};
    mutable std::mutex fMutex;
    std::priority_queue<std::unique_ptr<Fragment>,
                        std::vector<std::unique_ptr<Fragment> >,
                        CompareFragmentPtrs > fQueue;
    //std::priority_queue<Fragment*,
    //                    std::vector<Fragment*> > fQueue;

    std::atomic<uint32_t> fPushed{0};
    std::atomic<uint32_t> fPopped{0};

    std::atomic_bool fStop{false};
    std::thread fWorker;
};


#endif
