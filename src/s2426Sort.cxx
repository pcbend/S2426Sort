
#include<cstdio>
#include<chrono>

#include <TMidasBanks.h>
#include <TMidasFile.h>
#include <TMidasEvent.h>

#include <Fragment.h>
#include <EventBuilder.h>
#include <EventProcess.h>

#include <Histogramer.h>
#include <Channel.h>

#include <utils.h>

#include <coreMap.h>

void MakeTigressFragments(uint32_t*,int);
void MakeEmmaADC(uint32_t*,int);
void MakeEmmaTDC(uint32_t*,int);


void doStatus(TMidasFile&,bool forcePrint=false);
auto start     = std::chrono::steady_clock::now();
auto lastPrint = std::chrono::steady_clock::now();
auto timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);
const std::chrono::seconds interval(1); // 1 second interval


int main(int argc, char **argv) {
  TMidasFile infile(argv[1]);
  TMidasEvent event;

  Histogramer *gHist = Histogramer::Get();

  int run,subrun;
  getRunNumber(argv[1],run,subrun);
  gHist->SetRun(run,subrun);

  printf(" sorting \t %s\n",argv[1]);
  printf(" \trun:    %i\n",run);
  printf(" \tsubrun: %i\n",subrun);

  Channel::Read("cal/CalibrationFile_Nov182025.cal"); 

  //start event builder;
  EventBuilder::Get();
  //start event process;
  EventProcess::Get();

  std::map<std::string,int> banksFound;
  std::map<std::string,int> banksFound2;
  std::map<int,int>         typeFound;

  int counter = 0;
  while(infile.Read(event)) {
    //if(!event.GetEventId()&0xf000)
    //event.Print();
    switch(event.GetEventId()) {
      case 1:  //trigger
        Histogramer::Fill("EventTrigger",10,0,10,event.GetTriggerMask());
        event.SetBankList();
        //if(event.GetTriggerMask()==0) event.Print("all");
        void *ptr;
        int banksize;
        if((banksize = event.LocateBank(nullptr, "GRF4", &ptr)) > 0) {
          banksFound["GRIF4"]++;
          MakeTigressFragments((uint32_t*)ptr,banksize); 
        } else if((banksize = event.LocateBank(nullptr, "MADC", &ptr)) > 0) {
          //banksFound["MADC"]++;     // adc
          //printf(" ------------------------------------------ \n");
          MakeEmmaADC((uint32_t*)ptr,banksize); 
          if((banksize = event.LocateBank(nullptr, "EMMT", &ptr)) > 0) {   // MADC and EMMT are nested.
            MakeEmmaTDC((uint32_t*)ptr,banksize); 
            //banksFound["EMMT"]++;   // tdc
          }
          //event.Print("all");
          //printf("\n------------------------------------------ \n\n");
        }
      case 2:  //scalar
      case 5:  //epics
        break;
      case 0x8000: //BOR (odb)
      case 0x8001: //EOR
        event.Print();
      case 0x8002: //message Event;
        break;
    };
    typeFound[event.GetEventId()]++;
    counter++;
    doStatus(infile);

  }
  doStatus(infile,true);
  //printf("Event info: %zu types\n",typeFound.size());
  //for(auto it : typeFound) { 
  //  printf("\t0x%08x: \t%i\n",it.first,it.second);
  //}
  //printf("Bank info:\n");
  //for(auto it : banksFound) { 
  //  printf("\t%s: \t%i\n",it.first.c_str(),it.second);
  //}

  //std::ofstream ofile("banks.txt");
  //for(auto it : banksFound) { 
  //  ofile << "Bank info:" << std::endl;
  //  ofile << "bank:  " << it.first.c_str() << std::endl;
  //  ofile << "count: " << it.second        << std::endl;
  //}
  //ofile.close();
  gHist->Close();
  return 0;  
}






void doStatus(TMidasFile &infile,bool forcePrint) {
  
  if((std::chrono::steady_clock::now()-lastPrint) > interval) forcePrint=true;
  if(!forcePrint) return; 
  lastPrint = std::chrono::steady_clock::now();
  timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);
  printf(CLEAR_LINE);
  printf(" %lld \t read %.02f / %.02f  MB\r",
          timeEllapsed.count(),infile.GetBytesRead()/1024./1024.,
          infile.GetFileSize()/1024./1024.);
  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventBuilder[%s] Q[%i] pushed[%i]  pop[%i]\r",
           EventBuilder::Get()->Running() ?  "on" : "off",
           EventBuilder::Get()->Size(),EventBuilder::Get()->Pushed(),EventBuilder::Get()->Popped());
  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventProcess[%s] pushed[%i]\r",
           EventBuilder::Get()->Running() ?  "on" : "off",
           EventProcess::Get()->Pushed());

  if(infile.GetBytesRead()>=infile.GetFileSize()) {
    printf("\ndone!\n");
  } else {
    printf(CURSOR_UP);
    printf(CURSOR_UP);
  }
  fflush(stdout);
  //if(timeEllapsed.count()>20) break;
}


void MakeEmmaADC(uint32_t* pdata,int size) {
  //printf("MADC, size = %i:\n",size);
  long timestamp=0;
  int  channel=0;
  int  charge;
  while(size>0) { 
    size--;
    uint32_t datum = *(pdata+size);
    switch(datum&0xf0000000) { 
      case 0xc0000000:
      case 0xd0000000:
      case 0xe0000000:
      case 0xf0000000:
        timestamp=datum&0x3fffffff;
        break;   
      case 0x40000000:
        break;   
      case 0x00000000:
        if(datum & 0x00800000) {//
          timestamp += ((long(datum&0x0000ffff))<<30);
        } else if(datum & 0x04000000) {//
          channel = (datum>>16)&0x1F; // ADC Channel Number
          charge  = (datum & 0xfff); // ADC Charge

          std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
          frag.get()->AddCharge(charge);
          frag.get()->AddInt(5);
          frag.get()->SetAddress(0x800000 + channel);
          frag.get()->SetTimestamp(timestamp);
          frag.get()->SetCfd(0);             
          frag.get()->SetFilterPattern(0);   
          frag.get()->SetPileup(0);          
          frag.get()->SetDetType(12);

          //frag.get()->Print();

          EventBuilder::Get()->push(std::move(frag));
        }
        break;   
      default:  
        break;
    }
    //printf("0x%08x ",*(pdata+i));
    //if(i && (i%8)==0) printf("\n");
  }
  //printf("\n");
}

// kludge stolen form  GH                0xffffffff 
static uint32_t wraparoundcounter = 0; //0xffffffff; // Needed for bad data at start of run before GRIFFIN starts
static uint32_t lasttimestamp = 0;     // "last" time stamp for simple wraparound algorightm 
static uint32_t countsbetweenwraps; // number of counts between wraparounds

void MakeEmmaTDC(uint32_t* pdata ,int size) {
  //printf("MTDC, size = %i:\n",size);
  //long timestamp=0;
  //std::vector<uint32_t> addresses;
  //std::vector<uint32_t> charges;
  //uint32_t tmpAddress=0;

  uint32_t tmpTimestamp = 0;
  uint32_t tmpAddress   = 0;
  Long64_t ts           = 0;

  std::vector<uint32_t> addresses;
  std::vector<uint32_t> charges;

  //for(int i=0;i<size;i++) {
  //  printf("0x%08x ",*(pdata+i));
  //  if(i && (i%8)==0) printf("\n");
  //}
  //printf("\n\n");


  for(int x=0;x<size;x++) {  
    uint32_t datum = *(pdata+x);  
    int type = (datum>>27) & 0x1f; //&0xf8000000) >>27;
                                   //printf("datum: %p\n",datum);
    switch(type) {
      case 0x8:  //event header
        break;
      case 0x1:  //tdc header
        tmpAddress = (datum>>16)&0x300; 
        break;
      case 0x0:  //tdc measurement
         { 
           int c = ((datum >> 19) & 0xff ); 
           //printf("tdc: 0x%08x\t%i\n",c,c);
         }  
        addresses.push_back(0x900000 + ((datum >> 19) & 0xff ) );  
        charges.push_back(datum & 0x7ffff);
        break;
      case 0x3:  //tdc trailer
        break;
      case 0x4:  //tdc error
        break;
      case 0x11: //extended trigger time
        tmpTimestamp = (datum & 0x7FFFFFFU) << 5;
        break;
      case 0x10: //trailer
        tmpTimestamp |= ((datum) & 0x1fU);
        if(tmpTimestamp < lasttimestamp) {
          wraparoundcounter++;
          countsbetweenwraps=0;
        }
        lasttimestamp = tmpTimestamp;
        ts = static_cast<Long64_t>(lasttimestamp) +
          static_cast<Long64_t>(0x100000000ULL) * static_cast<Long64_t>(wraparoundcounter);
        ts = (ts * 5) >> 1;

        for (size_t i = 0; i < addresses.size(); ++i) {
          // optional duplicate check, mirroring GH:
          bool duped = false;
          //for (size_t j = 0; j < i; ++j) {
          //  if (addresses[i] == addresses[j]) {
          //    duped = true;
          //    break;
          //  }
          //}
          //if (duped) continue;

          std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
          frag.get()->AddCharge(charges.at(i));
          frag.get()->AddInt(5);
          frag.get()->SetAddress(addresses.at(i));
          frag.get()->SetTimestamp(ts);
          frag.get()->SetCfd(0);             
          frag.get()->SetFilterPattern(0);
          frag.get()->SetPileup(0);
          frag.get()->SetDetType(13);


           int c     = frag.get()->Address()&0xff;
           float chg = frag.get()->Charge(); 
           Histogramer::Get()->Fill("emmaChannels",4000,0,64000,chg,
                                                    1000,0,1000,c);

          //frag.get()->Print();
          EventBuilder::Get()->push(std::move(frag));
        }
        addresses.clear();
        charges.clear();
        tmpTimestamp = 0;
        tmpAddress   = 0;
        break;
      default:
        break;
    }
  }
}

void MakeTigressFragments(uint32_t *pdata,int size) { 
  int words=0;
  int counter=0;
  int good=0;
  int bad=0;
  uint32_t *pStart = 0; 
  uint32_t *pEnd   = 0;
  while(words<size) {
    //printf("%p \n",*(pdata+words));
    if((*(pdata+words)&0xf0000000)==0x80000000)
      pStart = pdata+words;
    if((*(pdata+words)&0xf0000000)==0xe0000000)
      pEnd   = pdata+words;
    if(pStart!=0 && pEnd!=0) {
      counter++;
      int nwords = int(pEnd-pStart);
      //processGRF4(pStart,nwords);
      std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
      //printf("%p \t %i\n",pStart,nwords);
      int i=0;
      //while(i<nwords) {
      //  printf("0x%08x\t",*(pStart+i));
      //  i++;
      //  if(i!=0)
      //    if((i%8)==0)
      //      printf("\n");
      //}
      //printf("\n");
      if(frag.get()->Unpack(pStart,nwords)) {
        good++;
        //for(number oif pileupes)
        //  Histogramer::Fill("something",70,0,70,frag.get()->GetNumber(i),8000,0,64000,frag.get()->GetCharge(i)
        frag.get()->Print();
        EventBuilder::Get()->push(std::move(frag));
      } else {
        bad++;
      }
      pStart = 0;
      pEnd   = 0;
    }
    words+=1;
  }
  //printf("found %i frags\n",counter);
  //printf("\tgood: %i frags\n",good);
  //printf("\tbad: %i frags\n",bad);
}





































