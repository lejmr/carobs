//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/Payload.msg.
//

#ifndef _PAYLOAD_M_H_
#define _PAYLOAD_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0402
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Class generated from <tt>messages/Payload.msg</tt> by opp_msgc.
 * <pre>
 * packet Payload {
 *     int src;
 *     int dst;
 * }
 * </pre>
 */
class Payload : public ::cPacket
{
  protected:
    int src_var;
    int dst_var;

  private:
    void copy(const Payload& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Payload&);

  public:
    Payload(const char *name=NULL, int kind=0);
    Payload(const Payload& other);
    virtual ~Payload();
    Payload& operator=(const Payload& other);
    virtual Payload *dup() const {return new Payload(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getSrc() const;
    virtual void setSrc(int src);
    virtual int getDst() const;
    virtual void setDst(int dst);
};

inline void doPacking(cCommBuffer *b, Payload& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, Payload& obj) {obj.parsimUnpack(b);}


#endif // _PAYLOAD_M_H_