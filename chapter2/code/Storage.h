#ifndef STORAGE_H
#define STORAGE_H

class Storage {
public:
    Storage();
    unsigned int getCharge() const;
    ~Storage();
private:
    class Impl;
    Impl* p_impl{nullptr}; 
};

#endif