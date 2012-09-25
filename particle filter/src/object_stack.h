#ifndef OBJECT_STACK_H
#define OBJECT_STACK_H

#define MAX_COUNT 5

#include <iostream>

using namespace std;

template <class T> class ObjectStack {
private:
    class ObjectStackItem {
    public:
        T item;
        ObjectStackItem *next;
        ObjectStackItem *prev;
        
        ObjectStackItem() { next = 0; prev = 0;  }
        ObjectStackItem(T it) { item = it; next = 0; prev = 0;  }
        //~ObjectStackItem() { if(item != 0) delete item; }
    };
    
public:    
    ObjectStackItem *top;
    ObjectStackItem *bottom;
    
    int count;
    
    void init(){
        top = 0;
        bottom = 0;
        count = 0;
    }
    
    ObjectStack() {
        init();
    }
    
    void insert(T item){
        ObjectStackItem *i = new ObjectStackItem(item);
        if(count == 0){
            //cout << "vkladam prvni prvek: " << item << endl; 
            top = i;
            bottom = i;
        }
        else {
            //cout << "vkladam nasledujici prvek: " << item << endl;
            i->next = top;
            top->prev = i;
            top = i;
        }
        count++;
        if(count > MAX_COUNT){
            //cout << "mazu pres limit " << endl;
            remove();
        }
    }
    
    void remove() {
        if(bottom != 0){
            bottom = bottom->prev;
            delete(bottom->next);
            count--;
        }
    }
    
    T operator[](int index){
        ObjectStackItem *actual = top;
        while(index > 0 && actual->next != 0){
            actual = actual->next;
            index--;
        }
        return actual->item;
    }
};

#endif //OBJECT_STACK_H