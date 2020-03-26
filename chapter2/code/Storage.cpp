#include "Storage.h"
#include "StorageType.h"

namespace {
    unsigned int totalCapacity = 1000;

    bool isValid(unsigned int capacity) {
        return capacity < totalCapacity;
    } 
    
       
}

class Storage::Impl {
public:
    Impl() {
        this->type = FILE_STORAGE;
        this->capacity = 0;
    }

    unsigned int getCharge() const {
        if (isValid(this->capacity)) return 100;
        return 0;
    }
private:
    StorageType type;
    unsigned int capacity;     
};

Storage::Storage() : p_impl(new Impl()){
}

Storage::~Storage(){
    if(p_impl) delete p_impl;
}

unsigned int Storage::getCharge() const {
    return p_impl->getCharge();
}
