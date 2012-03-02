#ifndef _SOAEntry_M_H_
#define _SOAEntry_M_H_

#include <omnetpp.h>


class SOAEntry : public cPolymorphic
{
  protected:
    int inPort_var;
    int inLambda_var;
    int outPort_var;
    int outLambda_var;

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const SOAEntry&);

  public:
      bool valid;

  public:
    SOAEntry(){this->valid=false;}
    SOAEntry(int ip, int il, int op, int ol);
    virtual ~SOAEntry() {}
    virtual std::string info() const;

    // field getter/setter methods
    int getInPort() const {return inPort_var;};
    void setInPort(int inPort_var){this->inPort_var = inPort_var;};
    int getInLambda() const {return inLambda_var;};
    void setInLambda(int inLambda_var){this->inLambda_var = inLambda_var;};
    int getOutPort() const {return outPort_var;};
    void setOutPort(int outPort_var){this->outPort_var = outPort_var;};
    int getOutLambda() const {return outLambda_var;};
    void setOutLambda(int outLambda_var){this->outLambda_var = outLambda_var;};
};

#endif // _LABELINFORMATIONBASEENTRY_M_H_
