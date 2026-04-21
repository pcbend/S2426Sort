
#include<EventProcess.h>

#include<EventBuilder.h>
#include<Histogramer.h>

#include<globals.h>


#include <map>

EventProcess *EventProcess::fEventProcess = 0;

EventProcess::EventProcess() {
  fWorker = std::thread([this]{ this->loop(); });
  fWorker.detach();
}

EventProcess *EventProcess::Get() {
  if(!fEventProcess)
    fEventProcess = new EventProcess;
  return fEventProcess;
}

EventProcess::~EventProcess() { 
  printf("EventProcess destructor called; fStop[%i]\n",fStop.load());
  if(!fStop)
    fWorker.join();
}


void EventProcess::loop() {
  long lasttime    = -1;
  long currenttime =  0;
  while(1) {
    if(fStop) break;
    
    //printf("\n\n\n\n EVENT PROCESS LOOP  ?!\n\n\n");

    //printf("here 1\n");
    std::vector<std::unique_ptr<Fragment> > builtfrags;
    if(!EventBuilder::Get()->Running()) return;
    if(!EventBuilder::Get()->pop(builtfrags)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));  
      continue;
    }
    fPushed += builtfrags.size(); 
    //printf("here 2\n");
    //printf("frags: %lu\n",builtfrags.size());      
    //continue;

    //std::map<int,std::vector<std::unique_ptr<Fragment> > > cores;
    std::vector<std::unique_ptr<Fragment> >                cores;
    std::map<int,std::vector<std::unique_ptr<Fragment> > > segments;
    std::map<int,std::vector<std::unique_ptr<Fragment> > > suppressors;
    std::vector<std::unique_ptr<Fragment> >                tip;
    std::vector<std::unique_ptr<Fragment> >                emma;

    for(size_t i=0;i<builtfrags.size();i++){
      currenttime = builtfrags.at(i).get()->Timestamp();
      if(currenttime < lasttime) { 
        //printf(RED);
        //printf("%s %ld %lu\n",builtfrags.at(i).get()->Name().c_str(), builtfrags.at(i).get()->Timestamp(), builtfrags.at(i).get()->Timestamp());
        //printf(RESET_COLOR);
        //printf("\n");
      }
      lasttime = currenttime;
    
      //ideal i would move the below code to a "physics-loop" 
      //but this will do for now.
      //do physics...
      switch(builtfrags.at(i).get()->DetType()) {
        case 0: // tigress core
          cores.push_back(std::move(builtfrags.at(i)));
          break;
        case 1: // likely tigress core (nid)
          break;
        case 2: //tigress segment
          break;
        case 3: //tigress suppressor
          break;
        case 8: //emma
          break;
        case 11: //tip
        case 12: //tip
          break;
        default:
          break;
      }
    }

    std::sort(cores.begin(), cores.end(),
      [](const std::unique_ptr<Fragment>& a,
        const std::unique_ptr<Fragment>& b) {
        return a->Energy() > b->Energy();  // decending order.
    });

    for(auto it = cores.begin(); it != cores.end(); ++it) {
      auto& current = *it;
      int  det   = std::stoi(current->Name().substr(3,2));
      char color = current->Name().at(5);
      int  xtal  = (color == 'R') ? 0 :
                   (color == 'G') ? 1 :
                   (color == 'B') ? 2 :
                   (color == 'W') ? 3 : -1;
      Histogramer::Fill("summary",70,0,70,det*4 +xtal,8000,0,4000,current->Energy());
      Histogramer::Fill(Form("x%02i%c",det,color),"gain",3600,0,3600,current->Time()/1e8,4000,0,4000,current->Energy());

      auto next = std::next(it);
      if(next == cores.end())
        break;  // no next element
      auto& nextCore = *next;

      Histogramer::Fill("dtime",4000,-2000,2000,current->Time() - nextCore->Time(),4000,0,4000,nextCore->Energy());
      Histogramer::Fill("dtimestamp",4000,-2000,2000,current->Timestamp() - nextCore->Timestamp(),4000,0,4000,nextCore->Energy());

    }

//   // else process the frags.
//   
//   //switchboard....
//
//   //printf("built frags size: %lu \n",builtfrags.size());
//
//   for(size_t i=0;i<builtfrags.size();i++) 
//     if(builtfrags[i]) delete builtfrags[i];
//
//
//   //we let them go out of scope.  
//   
//   //checks que
//   // - if true; pass built events;
    // - if flase; sleep;
  }
};
