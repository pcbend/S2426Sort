
#include <Fragment.h>
#include <TRandom.h>

#include <globals.h>


Fragment::Fragment() { } 

Fragment::~Fragment() { } 


int Fragment::Unpack(char *data) {
  //unpack its self from the data stream....
  int bytes = 0;
  uint32_t datum;

  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       I
  //printf("datum[%i]:\t%p\n",bytes,datum);
  bytes += sizeof(uint32_t);                                // advance to II
  if((datum&0xf0000000)!=0x80000000)  return bytes;
  int words   = ((datum & 0x01f00000) >> 20);
  SetAddress( (datum & 0x000ffff0) >>  4);
  SetDetType( (datum & 0x0000000f) >>  0); 
  //info from the header is set.

  
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       II
  //printf("datum[%i]:\t%p\n",bytes,datum);
  if((datum&0xf0000000)==0xd0000000) {          // has network packet counter 
    bytes += sizeof(uint32_t);                                // advance to III
    datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       III
    //printf("datum[%i]:\t%p\n",bytes,datum);
  } else { 
                                                              // already on III (?)
  }

  if(datum&0x00008000)  SetHasWave(); 
  SetFilterPattern((datum&0x3fff0000)>>16);
  SetPileup(datum&0x0000001f);

  bytes += sizeof(uint32_t);                                // advance to IV
  //datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       IV

  bytes += sizeof(uint32_t);                                // advance to V
  //datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       V

  bytes += sizeof(uint32_t);                                // advance to VI
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VI
  //printf("datum[%i]:\t%p ts low\n",bytes,datum);
  long timestamp = (datum&0x0fffffff);

  bytes += sizeof(uint32_t);                                // advance to VII
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VII
  //printf("datum[%i]:\t%p ts high\n",bytes,datum);
  timestamp += (long(datum&0x00003fff)<<28);
  SetTimestamp(timestamp);
  
  bytes += sizeof(uint32_t);                                // advance to VIII
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VIII

  if((datum&0xf0000000)==0xc0000000) {  //HasWave should be checked....
    //printf("datum[%i]:\t%p\n",bytes,datum);
    // the first one may contain the number of samples?
    while((datum&0xf0000000)==0xc0000000) {
      bytes += sizeof(uint32_t);                                // advance to VIII (reading waves)
      datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VIII (reading waves)
      //printf("datum[%i]:\t%p\n",bytes,datum);
    }
  }

  //printf("datum[%i]:\t%p charge \n",bytes,datum);
  //AddCharge(datum&0x03ffffff);
  int tempChg = (datum&0x3ffffff);
  int tempInt = (datum&0x7c000000)>>(26-9);

  bytes += sizeof(uint32_t);                                // advance to IX 
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       IX 
  //printf("datum[%i]:\t%p cfd \n",bytes,datum);
  SetCfd(datum&0x003fffff);
  tempInt +=((datum&0x7fc00000)>>22);
  AddInt(tempInt);
  AddCharge(tempChg);
  

  while((datum&0xf0000000)!=0xe0000000) {
    bytes += sizeof(uint32_t);                                // advance to X?? (finding trailer)
    datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       X?? (finding trailer)
    //printf("datum[%i]:\t%p\n",bytes,datum);
  }
   
  bytes += sizeof(uint32_t);                                // move to the next word past the trailer
  return bytes;
}

void Fragment::Print(Option_t *opt) const {

  printf("fragment @ %p\n",fTimestamp);
  printf("\taddress:     %p\n",fAddress);
  printf("\tdetType:     %i\n",fDetType);
  printf("\tcfd:         %i\n",fCfd);
  for(size_t i=0;i<fInt.size();i++) {
    printf("\tcharge[%lu]:   %i\n",i,fCharge.at(i));
    printf("\tint[%lu]:      %i\n",i,fInt.at(i));
  }  
}


float Fragment::Charge()   const { // { return float(fCharge.at(0))/float(fInt.at(0)); }
  if(!fCharge.empty() && !fInt.empty())
    return float(fCharge.at(0))/(float(fInt.at(0)/5.)); 
  bytes += sizeof(uint32_t);                                // advance to IV
  //datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       IV
 
  bytes += sizeof(uint32_t);                                // advance to V
  //datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       V
 
  bytes += sizeof(uint32_t);                                // advance to VI
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VI
  //printf("datum[%i]:\t%p ts low\n",bytes,datum);
  long timestamp = (datum&0x0fffffff);
 
  bytes += sizeof(uint32_t);                                // advance to VII
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VII
  //printf("datum[%i]:\t%p ts high\n",bytes,datum);
  timestamp += (long(datum&0x00003fff)<<28);
  SetTimestamp(timestamp);
  
  bytes += sizeof(uint32_t);                                // advance to VIII
  datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VIII
 
  if((datum&0xf0000000)==0xc0000000) {  //HasWave should be checked....
    //printf("datum[%i]:\t%p\n",bytes,datum);
    // the first one may contain the number of samples?
    while((datum&0xf0000000)==0xc0000000) {
      bytes += sizeof(uint32_t);                                // advance to VIII (reading waves)
      datum = *(reinterpret_cast<uint32_t*>(data+bytes));       // read       VIII (reading waves)
      //printf("datum[%i]:\t%p\n",bytes,datum);
    }
  }
 
  //printf("datum[%i]:\t%p charge \n",bytes,datum);
  //AddCharge(datum&0x03ffffff);
  int tempChg = (datum&0x3ffffff);
  int tempInt = (datum&0x7c000000)>>(26-9);
  return -1;

}

float Fragment::Energy() const {
  if(!fEnergy.empty()) 
    return fEnergy.at(0); 
  return -1;
}

void Fragment::AddCharge(int charge) {
  float chg = float(charge) +  gRandom->Uniform();
  fCharge.push_back(chg);
  int order = 0;
  float eng = 0;
  for(auto i : Channel::Get(fAddress)->CalPars()) {  
    eng += pow(Charge(),order++)*i;
    //if((fAddress&0x000f)==0x0009 && (Channel::Get(fAddress)->Number()<710))
    //  printf("%p  %i: eng: %.2f\t Charge() = %.2f \n",fAddress,order-1,eng,Charge());
  }
  //printf("eng = %.02f \n", eng); 
  fEnergy.push_back(eng);
}



