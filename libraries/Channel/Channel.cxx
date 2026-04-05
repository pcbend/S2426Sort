
#include <Channel.h>

#include<fstream>
#include<sstream>

std::unordered_map<int,Channel*> Channel::fChannelMap;

void   Channel::Read(std::string fFile) { 
  std::ifstream file(fFile.c_str());
  std::string line;
  Channel *c = 0;
  if(fChannelMap.size()==0) {
    c = new Channel("dummy 999 0xffff 0 1 0");
    fChannelMap[c->fAddress] = c;
  }
  while(getline(file,line)) {
    c = new Channel(line);
    //cSet(line);
    //c.Print();
    fChannelMap[c->fAddress] = c;
  }
} 

void   Channel::Set(std::string line) { 
  //printf("%s\n",line.c_str());
  std::stringstream ss(line);
  ss >> fName;    // 1 - name
  ss >> fNumber;  // 2 - number
  ss >> std::hex >> fAddress; // 3 - address
  double temp;
  while (ss >> temp) fCalPars.push_back(temp);
}

Channel *Channel::Get(int address) { 
  if(fChannelMap.count(address))
    return fChannelMap[address];
  return fChannelMap[0xffff];

}


void Channel::Print(Option_t *opt) const {
  printf("channel[0x%p]\n",fAddress);
  printf("\tName:     %s\n",fName.c_str());
  printf("\tNumber:   %i\n",fNumber);
  printf("\tCal Pars: "); for(auto i : fCalPars) printf("%.04f  ",i);
  printf("\n");

}


