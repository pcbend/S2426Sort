#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include<string>
#include<vector>
#include<unordered_map>

#include <TObject.h>

class Channel {
  public:
    virtual ~Channel() { }  

    static void Read(std::string fFile); 
    void Set(std::string fLine); 

    void Print(Option_t *opti="") const;

    static Channel* Get(int address);

    std::string Name()   const { return fName;   }
    int         Number() const { return fNumber; }
    std::vector<double> CalPars() const  { return fCalPars; }


  private:
    Channel(std::string line) {
      Set(line);
    }

  private:
    std::string fName{""};
    int         fNumber{-1};
    int         fAddress{-1};
    std::vector<double> fCalPars;

    static std::unordered_map<int,Channel*> fChannelMap;

};

#endif
