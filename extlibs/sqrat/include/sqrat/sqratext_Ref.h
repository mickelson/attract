

#ifndef __SQRATEXT_REF_H
#define __SQRATEXT_REF_H


namespace Sqrat {
    template<class T>
    class Ref {
        T* _ptr;
        T instance;
    public:
        Ref()                            { _ptr = &instance; }//new T;    }

        T *operator ->()                { return _ptr;    }
        const T *operator ->() const     { return _ptr;    }
        operator T*()                    { return _ptr;    }
        
        //Copies override "our" instance for another
        Ref &operator =(Ref *rhs)        { _ptr = rhs->_ptr; return *this; }
        Ref &operator =(T *rhs)            { _ptr = rhs; return *this; }
    };
    
    template<class T>
    struct Var<Ref<T> > {
        T *value;
        Var(HSQUIRRELVM vm, SQInteger idx) {
            value = ClassType<T>::GetInstance(vm, idx);
        }
        static void push(HSQUIRRELVM vm, Ref<T> &value) {
            ClassType<T>::PushInstance(vm, &(*value));
        }
    };
    
    template<class T>
    struct Var<Ref<T>&> {
        T *value;
        Var(HSQUIRRELVM vm, SQInteger idx) {
            value = ClassType<T>::GetInstance(vm, idx);
        }
        static void push(HSQUIRRELVM vm, Ref<T>& value) {
            ClassType<T>::PushInstance(vm, &(*value));
        }
    };
};

#endif //__SQRATEXT_REF_H