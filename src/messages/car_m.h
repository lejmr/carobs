//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/car.msg.
//

#ifndef _CAR_M_H_
#define _CAR_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0402
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Class generated from <tt>messages/car.msg</tt> by opp_msgc.
 * <pre>
 * packet Car {
 *     cQueue payload;
 * }
 * </pre>
 */
class Car : public ::cPacket
{
  protected:
    cQueue payload_var;

  private:
    void copy(const Car& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Car&);

  public:
    Car(const char *name=NULL, int kind=0);
    Car(const Car& other);
    virtual ~Car();
    Car& operator=(const Car& other);
    virtual Car *dup() const {return new Car(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual cQueue& getPayload();
    virtual const cQueue& getPayload() const {return const_cast<Car*>(this)->getPayload();}
    virtual void setPayload(const cQueue& payload);
};

inline void doPacking(cCommBuffer *b, Car& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, Car& obj) {obj.parsimUnpack(b);}


#endif // _CAR_M_H_