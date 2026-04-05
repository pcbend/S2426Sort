#ifndef __HISTOGRAMER_H__
#define __HISTOGRAMER_H__

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include <TList.h>

class Histogramer {
  private:
    Histogramer();
    static Histogramer *fHistogramer;
    static std::map<std::string,TList*> *gHistMap;
    static std::mutex gHistMutex;
  
  public:
    static Histogramer *Get(); 
    ~Histogramer();
    
    void SetRun(int run,int subrun);
    int SetBlobGates(std::string cutfile);

    static void Close();

    static void Fill(std::string hname,
                      int xbins,double xlow, double xhigh, double xval,
                      int ybins=-1,double ylow=-1,double yhigh=-1,double yval=-1);

    static void Fill(std::string dname,std::string name,
                      int xbins,double xlow, double xhigh, double xval,
                      int ybins=-1,double ylow=-1,double yhigh=-1,double yval=-1);

    TList *GetBlobs() const { return fBlobGates; }

  private:
    mutable std::mutex fMutex;
    int fRun;
    int fSubrun;

    TList *fBlobGates;

  ClassDef(Histogramer,0);

};

#endif
