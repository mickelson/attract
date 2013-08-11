//
// SqratAllocator: Custom Class Allocation/Deallocation
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//

#if !defined(_SCRAT_ALLOCATOR_H_)
#define _SCRAT_ALLOCATOR_H_

#include <squirrel.h>
#include <string.h>

#include "sqratObject.h"
#include "sqratTypes.h"

namespace Sqrat {

// utility taken from  http://stackoverflow.com/questions/2733377/is-there-a-way-to-test-whether-a-c-class-has-a-default-constructor-other-than/2770326#2770326
// may be obsolete in C++ 11 
template< class T >
class is_default_constructible {
    template<int x>
    class receive_size{};

    template< class U >
    static int sfinae( receive_size< sizeof U() > * );

    template< class U >
    static char sfinae( ... );

public:
    enum { value = sizeof( sfinae<T>(0) ) == sizeof(int) };
};
//
// DefaultAllocator
//

template<class C>
class DefaultAllocator {

    static SQInteger setInstance(HSQUIRRELVM vm, C* instance)
    {
        sq_setinstanceup(vm, 1, instance);
        sq_setreleasehook(vm, 1, &Delete);
        return 0;
    }
    
    template <class T, bool b> 
    struct NewC
    {
        T* p;
        NewC() 
        {
           p = new T();
        }        
    };

    template <class T>
    struct NewC<T, false>
    {
        T* p;
        NewC() 
        {
           p = 0;
        }        
    };

public:
    static SQInteger New(HSQUIRRELVM vm) {
        C* instance = NewC<C, is_default_constructible<C>::value >().p;
        setInstance(vm, instance);
        return 0;
    }

    template <int count>
    static SQInteger iNew(HSQUIRRELVM vm) {
        return New(vm);
    }
    
   // following New functions are used only if constructors are bound via Ctor() in class    

    template <typename A1>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value
            ));
    }
    template <typename A1,typename A2>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value
            ));
    }
    template <typename A1,typename A2,typename A3>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value
            ));
    }
    template <typename A1,typename A2,typename A3,typename A4>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        Var<A4> a4(vm, 5);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value,
            a4.value
            ));
    }
    template <typename A1,typename A2,typename A3,typename A4,typename A5>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        Var<A4> a4(vm, 5);
        Var<A5> a5(vm, 6);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value
            ));
    }
    template <typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        Var<A4> a4(vm, 5);
        Var<A5> a5(vm, 6);
        Var<A6> a6(vm, 7);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value
            ));
    }
    template <typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        Var<A4> a4(vm, 5);
        Var<A5> a5(vm, 6);
        Var<A6> a6(vm, 7);
        Var<A7> a7(vm, 8);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value
            ));
    }
    template <typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        Var<A4> a4(vm, 5);
        Var<A5> a5(vm, 6);
        Var<A6> a6(vm, 7);
        Var<A7> a7(vm, 8);
        Var<A8> a8(vm, 9);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value
            ));
    }
    template <typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
    static SQInteger iNew(HSQUIRRELVM vm) {
        Var<A1> a1(vm, 2);
        Var<A2> a2(vm, 3);
        Var<A3> a3(vm, 4);
        Var<A4> a4(vm, 5);
        Var<A5> a5(vm, 6);
        Var<A6> a6(vm, 7);
        Var<A7> a7(vm, 8);
        Var<A8> a8(vm, 9);
        Var<A9> a9(vm, 10);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        return setInstance(vm, new C(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value
            ));
    }

public:
    
    static SQInteger Copy(HSQUIRRELVM vm, SQInteger idx, const void* value) {
        C* instance = new C(*static_cast<const C*>(value));
        sq_setinstanceup(vm, idx, instance);
        sq_setreleasehook(vm, idx, &Delete);
        return 0;
    }

    static SQInteger Delete(SQUserPointer ptr, SQInteger size) {
        C* instance = reinterpret_cast<C*>(ptr);
        delete instance;
        return 0;
    }
};

//
// NoConstructorAllocator
//

class NoConstructor {
public:
    static SQInteger New(HSQUIRRELVM) {
        return 0;
    }
    static SQInteger Copy(HSQUIRRELVM, SQInteger, const void*) {
        return 0;
    }
    static SQInteger Delete(SQUserPointer, SQInteger) {
        return 0;
    }
};

//
// CopyOnly
//

template<class C>
class CopyOnly {
public:
    static SQInteger New(HSQUIRRELVM) {
        return 0;
    }
    static SQInteger Copy(HSQUIRRELVM vm, SQInteger idx, const void* value) {
        C* instance = new C(*static_cast<const C*>(value));
        sq_setinstanceup(vm, idx, instance);
        sq_setreleasehook(vm, idx, &Delete);
        return 0;
    }
    static SQInteger Delete(SQUserPointer ptr, SQInteger size) {
        C* instance = reinterpret_cast<C*>(ptr);
        delete instance;
        return 0;
    }
};


//
// NoCopy
//

template<class C>
class NoCopy {
public:
    static SQInteger New(HSQUIRRELVM vm) {
        C* instance = new C();
        sq_setinstanceup(vm, 1, instance);
        sq_setreleasehook(vm, 1, &Delete);
        return 0;
    }

    static SQInteger Copy(HSQUIRRELVM vm, SQInteger idx, const void* value) {
        return 0;
    }

    static SQInteger Delete(SQUserPointer ptr, SQInteger size) {
        C* instance = reinterpret_cast<C*>(ptr);
        delete instance;
        return 0;
    }
};

}

#endif
