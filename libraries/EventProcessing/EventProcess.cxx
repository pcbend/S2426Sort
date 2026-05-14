
#include<EventProcess.h>

#include<EventBuilder.h>
#include<Histogramer.h>

#include<globals.h>

#include <map>
#include <iostream>
#include "TVector3.h"
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
    //printf("\n\n=======================================================\n");
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
    std::vector<std::unique_ptr<Fragment> >                emmaadc;
    std::vector<std::unique_ptr<Fragment> >                emmatdc;

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
        case 8: // EMMA master trigger
        case 12: //tip (based on ODB)
          break;
        case 13: // EMMA ADC
          emmaadc.push_back(std::move(builtfrags.at(i)));
          break;
        case 14: // EMMA TDC
          emmatdc.push_back(std::move(builtfrags.at(i)));
          break;
        default:
          break;
      }
    }

// =================== EMMA ADC ===========================//
    std::map<int,std::vector<double>> ics;
    std::vector<double> si;
    for(auto it = emmaadc.begin(); it!=emmaadc.end(); ++it){
      auto& current = *it;
      int c     = current->Address()&0xff;
      float chg = current->Charge();
      long timestamp = current->Timestamp(); 
      Histogramer::Get()->Fill("Event/EMMA","eADC",4000,0,64000,chg, 1000,0,1000,c);
      if(c>15 && c<20) {
        ics[c-16].push_back(chg);
        //printf("ic%i: %lu\n", c-16,timestamp);
      }
      if(c == 3) {
        si.push_back(chg);
        //printf("si: %lu\n",timestamp);
      }
    }
    double sum = 0;
		std::map<int, double> ic_sum;
    for(const auto& [ic_id, values] : ics) {
      for(double v : values) {
        ic_sum[ic_id] += v;
        sum += v;
      }
    }
		double sic = 0;
		for(double v : si) {sum += v; sic += v;}
		for(int i = 0; i < 4; ++i) {
		  Histogramer::Get()->Fill("Event/EMMA", Form("ic%d_vs_sum",i), 4000, 0, 4000, sum,4000, 0, 4000, ic_sum[i]);
		  Histogramer::Get()->Fill("Event/EMMA", Form("ic%d_vs_si",i),  4000, 0, 4000, sic,4000, 0, 4000, ic_sum[i]);
		  for(int j = i + 1; j < 4; ++j) {
		   	if(ic_sum.count(i) && ic_sum.count(j)) {
		   		Histogramer::Get()->Fill("Event/EMMA",Form("ic%d_vs_ic%d", i, j),4000, 0, 4000, ic_sum[i],4000, 0, 4000, ic_sum[j]);
		   	}
		  }
			
		} 

// ============================================================//

// =================== EMMA TDC ===========================//
    double fLdelay = 40;
		double fRdelay = 20;
		double fTdelay = 10;
		double fBdelay = 20;
		double fXlength = 80.; // Size of X focal plane in mm
		double fYlength = 30.; // Size of Y focal plane in mm
		double left   = -1;
		double right  = -1;
		double top    = -1;
		double bottom = -1;	
		for(auto it = emmatdc.begin(); it!=emmatdc.end(); ++it){
      auto& current = *it;
      int c     = current->Address()&0xff;
      float chg = current->Charge(); 
      long timestamp = current->Timestamp(); 
      Histogramer::Get()->Fill("Event/EMMA","eTDC",4000,0,64000,chg, 1000,0,1000,c);
			if(c==3) right  = chg;
			if(c==4) left   = chg;
			if(c==5) top    = chg;
			if(c==6) bottom = chg;
      //if(c>=0 && c<=2) printf("pgac  anode%i: %lu\n",c,timestamp);
      //if(c>=3 && c<=6) printf("pgac cathod%i: %lu\n",c,timestamp);
    }
		if(left>0 && right>0 && top>0 && bottom>0){
			double Xdiff = (left+fLdelay) - (right+fRdelay);
			double Xsum = (left) + (right);
  		double Ydiff = (bottom+fBdelay) - (top+fTdelay);
  		double Ysum = (bottom) + (top);              
  		double Xpos = ( Xdiff / Xsum )*fXlength;
  		double Ypos = ( Ydiff / Ysum )*fYlength;
			TVector3 pgac(Xpos, Ypos, 1);
			Histogramer::Fill("PGAC","focalplanx_y", 60,-30,30,pgac.X(),60,-30,30,pgac.Y());
		}
// ============================================================//

// =================== TIGRES cores ===========================//
    std::sort(cores.begin(), cores.end(),
      [](const std::unique_ptr<Fragment>& a,
        const std::unique_ptr<Fragment>& b) {
        return a->Energy() > b->Energy();  // decending order.
    });

    for(auto it = cores.begin(); it != cores.end(); ++it) {
      auto& current = *it;
      //std::cout << "frag name = [" << current->Name() << "] [" << current->DetType() << "]" << std::endl;
      int  det   = std::stoi(current->Name().substr(3,2));
      char color = current->Name().at(5);
      int  xtal  = (color == 'B') ? 0 :
                   (color == 'G') ? 1 :
                   (color == 'R') ? 2 :
                   (color == 'W') ? 3 : -1;
      Histogramer::Fill("Event","summary_energy",70,0,70,det*4 +xtal,8000,0,4000,current->Energy());
      Histogramer::Fill("Event","summary_charge",70,0,70,det*4 +xtal,16000,0,16000,current->Charge());
      Histogramer::Fill(Form("Event/x%02i%c",det,color),"time_charge",7200,0,72000,current->Time()/1e8,16000,0,16000,current->Charge());
      auto next = std::next(it);
      if(next == cores.end())
        break;  // no next element
      auto& nextCore = *next;
      if(nextCore->Name().size()<10) continue;
      Histogramer::Fill("Event","dtime",4000,-2000,2000,current->Time() - nextCore->Time(),4000,0,4000,nextCore->Energy());
      Histogramer::Fill("Event","dtimestamp",4000,-2000,2000,current->Timestamp() - nextCore->Timestamp(),4000,0,4000,nextCore->Energy());

    }
// ============================================================//


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
