

#include <filesystem>


#include <Histogramer.h>
#include <globals.h>

#include <TFile.h>
#include <TKey.h>
#include <TCutG.h>
#include <TH1.h>
#include <TH2.h>


Histogramer                  *Histogramer::fHistogramer = 0;
std::map<std::string,TList*> *Histogramer::gHistMap     = 0;
std::mutex                    Histogramer::gHistMutex;

Histogramer *Histogramer::Get() { 
  static std::mutex instanceMutex;
  if(!fHistogramer) {
    std::lock_guard<std::mutex> lk(instanceMutex);
    fHistogramer = new Histogramer();
  }
  return fHistogramer;
}

Histogramer::Histogramer() : fRun(0), fSubrun(0), fBlobGates(0) { 
  TH1::AddDirectory(false); // if we create a histogramer, we want to control the memory. 
}

void Histogramer::SetRun(int run,int subrun) {
  fRun    = run;
  fSubrun = subrun;
}

int Histogramer::SetBlobGates(std::string cutfile) {
  if(!fBlobGates)
    fBlobGates = new TList;
 
  int ncuts=0;

  TFile *CF = TFile::Open(cutfile.c_str());
  if(!CF) {
    printf("FAILED to load cuts from %s\n",cutfile.c_str());
    return ncuts;
  }
  TIter iter(CF->GetListOfKeys());
  while(TKey *key = (TKey*)iter.Next()) {
    TCutG *cut = (TCutG*)key->ReadObj();
    fBlobGates->Add(cut);
    ncuts++;
    printf("loaded cut %s\n",cut->GetName());
  }
  return ncuts;
}

void Histogramer::Close() {
  if(fHistogramer)
    delete fHistogramer;
  fHistogramer = 0;
}


Histogramer::~Histogramer() { 
  if(gHistMap) {
    int run,subrun;

    std::string outDir = "histOutput";
    if(!std::filesystem::exists(outDir))
      std::filesystem::create_directory(outDir);
    TFile *outFile = new TFile(Form("%s/hist%i-%02i.root",outDir.c_str(),fRun,fSubrun),"recreate");
    std::map<std::string,TList*>::iterator it;
    int counter = 0;

    printf(DYELLOW "starting tfile write...\n" RESET_COLOR);

    for(it=gHistMap->begin();it!=gHistMap->end();it++) {
      std::string dname = it->first;
      TList *l = it->second;
      if(dname=="noname") {
        //printf("\t %i  i am here: %s\n",counter++,dname.c_str());
        l->Sort();
        //for(int i=0;i<l->GetEntries();i++) 
          //printf("\t\t%i:  %si\t%p\n",i,l->At(i)->GetName(),((TH1*)l->At(i))->GetDirectory());
        l->Write();
      } else {
        //printf("\t %i  i am here: %s\n",counter++,dname.c_str());
        TDirectory *current = gDirectory;

        outFile->mkdir(dname.c_str())->cd();
        //outfile->cd();
        l->Sort();
        printf("list entries = %i\n",l->GetEntries());
        l->Write();
        if(current) 
          current->cd();
      }
        /*
        for(int i=0;i<l->GetEntries();i++) {
          //printf("\t\t%i:  %si\t%p\n",i,l->At(i)->GetName(),((TH1*)l->At(i))->GetDirectory());
          if(!((TH1*)l->At(i))->GetDirectory()) {
            delete l->At(i);
          }
        }
        */
    }
    //if(fBlobGates && fBlobGates->GetEntries()) {
    //  TDirectory *current = gDirectory;
    //  outFile->mkdir("blobs")->cd();
    //  fBlobGates->Write();
  
      //outFile->Close();
    //}
    outFile->Close();
  }
}


void Histogramer::Fill(std::string hname,
          int xbins,double xlow, double xhigh, double xval,
          int ybins,double ylow,double yhigh,double yval) {
  std::string dname = "noname";
  Fill(dname,hname,xbins,xlow,xhigh,xval,ybins,ylow,yhigh,yval);
}

void Histogramer::Fill(std::string dname,std::string hname,
          int xbins,double xlow, double xhigh, double xval,
          int ybins,double ylow,double yhigh,double yval) {
  std::lock_guard<std::mutex> lock(gHistMutex);
  if(!gHistMap) 
    gHistMap = new std::map<std::string, TList*>;
  if(!((*gHistMap)[dname])) {
    (*gHistMap)[dname] = new TList(); //dname.c_str(),dname.c_str());
  }
  TList *clist = (*gHistMap)[dname];
  //TH1 *hist = static_cast<TH1*>(gHistMap->at(dname)->FindObject(hname.c_str()));
  TH1 *hist = (TH1*)clist->FindObject(hname.c_str());
  if(!hist) {
    if(ybins>0) 
      hist = new TH2D(hname.c_str(),hname.c_str(),xbins,xlow,xhigh,ybins,ylow,yhigh);
    else 
      hist = new TH1D(hname.c_str(),hname.c_str(),xbins,xlow,xhigh);

    //printf("creating histogram:  %s  %s\n",  
    clist->Add(hist);
  }
  if(ybins>0) 
    hist->Fill(xval,yval);
  else 
    hist->Fill(xval);
  return;

}




