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
    simtime_t start_var;
    simtime_t stop_var;
    bool aggregation_var;
    bool disaggregation_var;
    bool buffer_var;
    bool buffer_in_var; //True= to buffer, False= from buffer

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const SOAEntry&);

  public:
      bool valid;

      /*
       * only used with buffer mode of carobs
       * makes reference between in and out of buffer SOAEntry such that
       * from to buffer can be reached to buffer one ..
       *
       * This is result of many tries to avoid this but it is only one workig sollution
       * Buffered OpticalLayer packet is marked my SOAEntry which is used for buffering
       * and then MAC obtains its out of buffer SOAEntry and resolves proper waiting time..
       * In real systems this can be implemented with some external shared memory for MAC, SOA and SOAm
       */
      SOAEntry *bound;

  public:
    SOAEntry(){this->valid=false;}
    SOAEntry(int ip, int il, int op, int ol);
    SOAEntry(int op, int ol, bool aggregation);
    virtual ~SOAEntry() {}
    virtual std::string info() const;
    virtual SOAEntry *dup() const;

    // field getter/setter methods
    int getInPort() const {return inPort_var;};
    void setInPort(int inPort_var){this->inPort_var = inPort_var;};
    int getInLambda() const {return inLambda_var;};
    void setInLambda(int inLambda_var){this->inLambda_var = inLambda_var;};
    int getOutPort() const {return outPort_var;};
    void setOutPort(int outPort_var){this->outPort_var = outPort_var;};
    int getOutLambda() const {return outLambda_var;};
    void setOutLambda(int outLambda_var){this->outLambda_var = outLambda_var;};
    simtime_t getStart() const {return start_var;};
    void setStart(simtime_t start_var){this->start_var = start_var;};
    simtime_t getStop() const {return stop_var;};
    void setStop(simtime_t stop_var){this->stop_var = stop_var;};
    bool isAggregation() const {return aggregation_var;};
    void setAggregation(bool aggregation_var){this->aggregation_var = aggregation_var;};
    bool isDisaggregation() const {return disaggregation_var;};
    void setDisaggregation(bool disaggregation_var){this->disaggregation_var = disaggregation_var;};
    bool getBuffer() const {return buffer_var;};
    void setBuffer(bool buffer_var){this->buffer_var = buffer_var;};
    void setInBuffer(){this->buffer_in_var = true;};
    void setOutBuffer(){this->buffer_in_var = false;};
    bool getBufferDirection() const { return buffer_in_var;};
};

#endif // _LABELINFORMATIONBASEENTRY_M_H_
