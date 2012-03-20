#ifndef _MACEntry_M_H_
#define _MACEntry_M_H_

#include <omnetpp.h>


class MACEntry : public cPolymorphic
{
  protected:
    int outPort_var;
    int outLambda_var;
    simtime_t from_var;
    simtime_t to_var;


    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MACEntry&);

  public:
      bool valid;

  public:
    MACEntry(){}
    MACEntry(int op, int ol, simtime_t from, simtime_t to);
    virtual ~MACEntry() {}
    virtual std::string info() const;

    // field getter/setter methods
    int getOutPort() const {return outPort_var;};
    void setOutPort(int outPort_var){this->outPort_var = outPort_var;};
    int getOutLambda() const {return outLambda_var;};
    void setOutLambda(int outLambda_var){this->outLambda_var = outLambda_var;};
    simtime_t getTo()const {return to_var;};
};

#endif // _LABELINFORMATIONBASEENTRY_M_H_
