#ifndef _CplexRouteEntry_M_H_
#define _CplexRouteEntry_M_H_

#include <omnetpp.h>


class CplexRouteEntry : public cPolymorphic
{
  protected:

    // Flow information
    int path_id_var;
    int source_var;
    int dest_var;

    // Routing
    int inPort_var;
    int inLambda_var;
    int outPort_var;
    int outLambda_var;

    // Path parameters
    int hops_var;
    bool vis_hops;
    simtime_t OT_var;

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const CplexRouteEntry&);


  public:
    CplexRouteEntry(int pid, int s, int d, bool hops);
    virtual ~CplexRouteEntry() {}
    virtual std::string info() const;
    virtual CplexRouteEntry *dup() const;
    //virtual const char *getName () const;

    // field getter/setter methods
    int getSource() const {return source_var;};
    int getdest() const {return dest_var;};
    int getInPort() const {return inPort_var;};
    int getInLambda() const {return inLambda_var;};
    int getOutPort() const {return outPort_var;};
    int getOutLambda() const {return outLambda_var;};

    void setInput(int port, int wl);
    void setOutput(int port, int wl);

    simtime_t getOT() const {return OT_var;};
    void countOT(simtime_t d_p);

    void addHop(){this->hops_var++;};
};

#endif // _LABELINFORMATIONBASEENTRY_M_H_
