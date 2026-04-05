#ifndef __FRAGMENT_H__
#define __FRAGMENT_H__

#include <vector>

//#include <TObject.h>

#include <Channel.h>

class Fragment { 
  public:
    Fragment();
    
    virtual ~Fragment();

    bool Unpack(uint32_t *pdata,int &nwords); 
    void Print(Option_t *opt="") const; 

    bool HasWave() const { return fHasWave; }


    // type == in tigress everything is a grif16 - so i do not care. 
    void SetAddress(int address)      { fAddress = address; }  
    void SetDetType(int detType)      { fDetType = detType; }
    void SetTimestamp(long timestamp) { fTimestamp = timestamp; }
    void SetHasWave()                 { fHasWave = !fHasWave; }
    void SetWaveSamples(int samples)  { fWaveSamples = samples; }
    void SetCfd(int cfd)              { fCfd = cfd; }
    void SetFilterPattern(int fp)     { fFilterPattern = fp; }    
    void SetPileup(int pileup)        { fPileup = pileup; }  

    void AddCharge(int chg); //           { fCharge.push_back(chg); }
    void AddInt(int i)                { fInt.push_back(i); }
  

    int  Address()   const { return fAddress; }
    int  DetType()   const { return fDetType; }

    long Timestamp() const { return fTimestamp; }
    double Time()    const { return double(fTimestamp&0xfffffffffffc0000) + double(fCfd)/16.; } 
    int  Cfd()       const { return fCfd;       }
    int  Filter()    const { return fFilterPattern; }
    int  Pileup()    const { return fPileup;        }

    float Charge()   const; // { return float(fCharge.at(0))/float(fInt.at(0)); }
    float Energy()   const; // { return float(fCharge.at(0))/float(fInt.at(0)); }

    int  Number()    const { return Channel::Get(fAddress)->Number(); } 
    int  DetNumber()    const { std::string name = Channel::Get(fAddress)->Name(); if(name.length()>4) return atoi(name.substr(3,2).c_str()); return 99; }

    std::string Name() const { return   Channel::Get(fAddress)->Name(); }  

    bool operator<(const Fragment& other) const { 
      //if(timestamp != other.timestamp) 
        return fTimestamp>other.fTimestamp;
      //return timestamp;
      //return seq<other.seq; 
    }


  private:
    int fAddress{-1}; 
    int fDetType{-1}; 
  
    long fTimestamp{-1}; 
    int  fCfd{-1};
    int fFilterPattern{-1};
    int fPileup{-1};

    bool fHasWave{false};
    int  fWaveSamples{-1};

    std::vector<int> fInt;
    std::vector<float> fCharge;
    std::vector<float> fEnergy;

  //ClassDef(Fragment,1);
};


#endif
