//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/car.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "car_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Car_Base::Car_Base(const char *name, int kind) : cPacket(name,kind)
{
    payload_arraysize = 0;
    this->payload_var = 0;
}

Car_Base::Car_Base(const Car_Base& other) : cPacket(other)
{
    payload_arraysize = 0;
    this->payload_var = 0;
    copy(other);
}

Car_Base::~Car_Base()
{
    delete [] payload_var;
}

Car_Base& Car_Base::operator=(const Car_Base& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void Car_Base::copy(const Car_Base& other)
{
    delete [] this->payload_var;
    this->payload_var = (other.payload_arraysize==0) ? NULL : new Payload[other.payload_arraysize];
    payload_arraysize = other.payload_arraysize;
    for (unsigned int i=0; i<payload_arraysize; i++)
        this->payload_var[i] = other.payload_var[i];
}

void Car_Base::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    b->pack(payload_arraysize);
    doPacking(b,this->payload_var,payload_arraysize);
}

void Car_Base::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    delete [] this->payload_var;
    b->unpack(payload_arraysize);
    if (payload_arraysize==0) {
        this->payload_var = 0;
    } else {
        this->payload_var = new Payload[payload_arraysize];
        doUnpacking(b,this->payload_var,payload_arraysize);
    }
}

void Car_Base::setPayloadArraySize(unsigned int size)
{
    Payload *payload_var2 = (size==0) ? NULL : new Payload[size];
    unsigned int sz = payload_arraysize < size ? payload_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        payload_var2[i] = this->payload_var[i];
    payload_arraysize = size;
    delete [] this->payload_var;
    this->payload_var = payload_var2;
}

unsigned int Car_Base::getPayloadArraySize() const
{
    return payload_arraysize;
}

Payload& Car_Base::getPayload(unsigned int k)
{
    if (k>=payload_arraysize) throw cRuntimeError("Array of size %d indexed by %d", payload_arraysize, k);
    return payload_var[k];
}

void Car_Base::setPayload(unsigned int k, const Payload& payload)
{
    if (k>=payload_arraysize) throw cRuntimeError("Array of size %d indexed by %d", payload_arraysize, k);
    this->payload_var[k] = payload;
}

void Car::insertPayload(Payload *payload)
{
    take(payload);
    unsigned int k = this->getPayloadArraySize();
    this->setPayloadArraySize(k+1);
    if (k>=payload_arraysize) throw cRuntimeError("Array of size %d indexed by %d", payload_arraysize, k);
    this->payload_var[k] = *payload;
    this->setByteLength( this->getByteLength() + payload->getByteLength() );
}

Payload* Car::popPayload(){
    unsigned int k = this->getPayloadArraySize();
    Payload *pl = &payload_var[k-1];
    this->setPayloadArraySize(k-1);
    //drop(pl);
    return pl;
}

class CarDescriptor : public cClassDescriptor
{
  public:
    CarDescriptor();
    virtual ~CarDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(CarDescriptor);

CarDescriptor::CarDescriptor() : cClassDescriptor("Car", "cPacket")
{
}

CarDescriptor::~CarDescriptor()
{
}

bool CarDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<Car_Base *>(obj)!=NULL;
}

const char *CarDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"customize")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int CarDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int CarDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISCOMPOUND,
    };
    return (field>=0 && field<1) ? fieldTypeFlags[field] : 0;
}

const char *CarDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "payload",
    };
    return (field>=0 && field<1) ? fieldNames[field] : NULL;
}

int CarDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "payload")==0) return base+0;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *CarDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "Payload",
    };
    return (field>=0 && field<1) ? fieldTypeStrings[field] : NULL;
}

const char *CarDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int CarDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    Car_Base *pp = (Car_Base *)object; (void)pp;
    switch (field) {
        case 0: return pp->getPayloadArraySize();
        default: return 0;
    }
}

std::string CarDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    Car_Base *pp = (Car_Base *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getPayload(i); return out.str();}
        default: return "";
    }
}

bool CarDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    Car_Base *pp = (Car_Base *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *CarDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        "Payload",
    };
    return (field>=0 && field<1) ? fieldStructNames[field] : NULL;
}

void *CarDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    Car_Base *pp = (Car_Base *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getPayload(i)); break;
        default: return NULL;
    }
}


