//
// SqratGlobalMethods: Global Methods
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

#if !defined(_SQRAT_OVERLOAD_METHODS_H_)
#define _SQRAT_OVERLOAD_METHODS_H_

#include <squirrel.h>
#include <sqstdaux.h>
#include <sstream>
#include "sqratTypes.h"
#include "sqratUtil.h"
#include "sqratGlobalMethods.h"
#include "sqratMemberMethods.h"

namespace Sqrat {

//
// Overload name generator
//
class SqOverloadName {
public:
    static string Get(const SQChar* name, int args) {
        std::basic_stringstream<SQChar> overloadName;
        overloadName << _SC("__sqrat_ol_ ") << name << _SC("_") << args;

        return overloadName.str();
    }
};

//
// Squirrel Overload Functions
//

template <class R>
class SqOverload {
public:
    static SQInteger Func(HSQUIRRELVM vm) {
        // Get the arg count
        int argCount = sq_gettop(vm) - 2;

        const SQChar* funcName;
        sq_getstring(vm, -1, &funcName); // get the function name (free variable)

        string overloadName = SqOverloadName::Get(funcName, argCount);

        sq_pushstring(vm, overloadName.c_str(), -1);
        if(SQ_FAILED(sq_get(vm, 1))) { // Lookup the proper overload
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }

        // Push the args again
        for(int i = 1; i <= argCount + 1; ++i) {
            sq_push(vm, i);
        }

        sq_call(vm, argCount + 1, true, ErrorHandling::IsEnabled());
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }

        return 1;
    }
};

//
// void return specialization
//

template <>
class SqOverload<void> {
public:
    static SQInteger Func(HSQUIRRELVM vm) {
        // Get the arg count
        int argCount = sq_gettop(vm) - 2;

        const SQChar* funcName;
        sq_getstring(vm, -1, &funcName); // get the function name (free variable)

        string overloadName = SqOverloadName::Get(funcName, argCount);

        sq_pushstring(vm, overloadName.c_str(), -1);
        if(SQ_FAILED(sq_get(vm, 1))) { // Lookup the proper overload
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }

        // Push the args again
        for(int i = 1; i <= argCount + 1; ++i) {
            sq_push(vm, i);
        }

        sq_call(vm, argCount + 1, false, ErrorHandling::IsEnabled());
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }

        return 0;
    }
};
//
// Global Overloaded Function Resolvers
//

// Arg Count 0
template <class R>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)()) {
    return &SqGlobal<R>::template Func0<true>;
}

template <class R>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)()) {
    return &SqGlobal<R&>::template Func0<true>;
}

// Arg Count 1
template <class R, class A1>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1)) {
    return &SqGlobal<R>::template Func1<A1, 2, true>;
}

template <class R, class A1>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1)) {
    return &SqGlobal<R&>::template Func1<A1, 2, true>;
}

// Arg Count 2
template <class R, class A1, class A2>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2)) {
    return &SqGlobal<R>::template Func2<A1, A2, 2, true>;
}

template <class R, class A1, class A2>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2)) {
    return &SqGlobal<R&>::template Func2<A1, A2, 2, true>;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3)) {
    return &SqGlobal<R>::template Func3<A1, A2, A3, 2, true>;
}

template <class R, class A1, class A2, class A3>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3)) {
    return &SqGlobal<R&>::template Func3<A1, A2, A3, 2, true>;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R>::template Func4<A1, A2, A3, A4, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R& >::template Func4<A1, A2, A3, A4, 2, true>;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R>::template Func5<A1, A2, A3, A4, A5, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R&>::template Func5<A1, A2, A3, A4, A5, 2, true>;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R>::template Func6<A1, A2, A3, A4, A5, A6, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R&>::template Func6<A1, A2, A3, A4, A5, A6, 2, true>;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R>::template Func7<A1, A2, A3, A4, A5, A6, A7, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R&>::template Func7<A1, A2, A3, A4, A5, A6, A7, 2, true>;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R&>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 2, true>;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R&>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 2, true>;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R&>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 2, true>;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R&>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 2, true>;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R&>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 2, true>;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R&>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 2, true>;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 2, true>;
}
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R&>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 2, true>;
}

//
// Member Global Overloaded Function Resolvers
//

// Arg Count 1
template <class R, class A1>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1)) {
    return &SqGlobal<R>::template Func1<A1, 1, true>;
}

template <class R, class A1>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1)) {
    return &SqGlobal<R&>::template Func1<A1, 1, true>;
}

// Arg Count 2
template <class R, class A1, class A2>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2)) {
    return &SqGlobal<R>::template Func2<A1, A2, 1, true>;
}

template <class R, class A1, class A2>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2)) {
    return &SqGlobal<R&>::template Func2<A1, A2, 1, true>;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3)) {
    return &SqGlobal<R>::template Func3<A1, A2, A3, 1, true>;
}

template <class R, class A1, class A2, class A3>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3)) {
    return &SqGlobal<R&>::template Func3<A1, A2, A3, 1, true>;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R>::template Func4<A1, A2, A3, A4, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R&>::template Func4<A1, A2, A3, A4, 1, true>;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R>::template Func5<A1, A2, A3, A4, A5, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R&>::template Func5<A1, A2, A3, A4, A5, 1, true>;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R>::template Func6<A1, A2, A3, A4, A5, A6, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R&>::template Func6<A1, A2, A3, A4, A5, A6, 1, true>;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R>::template Func7<A1, A2, A3, A4, A5, A6, A7, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R&>::template Func7<A1, A2, A3, A4, A5, A6, A7, 1, true>;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R&>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 1, true>;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R&>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 1, true>;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R&>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 1, true>;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R&>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 1, true>;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R&>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 1, true>;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R&>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 1, true>;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R&>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 1, true>;
}

//
// Member Overloaded Function Resolvers
//

// Arg Count 0
template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)()) {
    return &SqMember<C, R>::template Func0<true>;
}

template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)() const) {
    return &SqMember<C, R>::template Func0C<true>;
}

template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)()) {
    return &SqMember<C, R&>::template Func0<true>;
}
template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)() const) {
    return &SqMember<C, R&>::template Func0C<true>;
}


// Arg Count 1
template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1)) {
    return &SqMember<C, R>::template Func1<A1, true>;
}

template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1) const) {
    return &SqMember<C, R>::template Func1C<A1, true>;
}

template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1)) {
    return &SqMember<C, R&>::template Func1<A1, true>;
}

template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1) const) {
    return &SqMember<C, R&>::template Func1C<A1, true>;
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2)) {
    return &SqMember<C, R>::template Func2<A1, A2, true>;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2) const) {
    return &SqMember<C, R>::template Func2C<A1, A2, true>;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2)) {
    return &SqMember<C, R&>::template Func2<A1, A2, true>;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2) const) {
    return &SqMember<C, R&>::template Func2C<A1, A2, true>;
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3)) {
    return &SqMember<C, R>::template Func3<A1, A2, A3, true>;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3) const) {
    return &SqMember<C, R>::template Func3C<A1, A2, A3, true>;
}
template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3)) {
    return &SqMember<C, R&>::template Func3<A1, A2, A3, true>;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3) const) {
    return &SqMember<C, R&>::template Func3C<A1, A2, A3, true>;
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4)) {
    return &SqMember<C, R>::template Func4<A1, A2, A3, A4, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4) const) {
    return &SqMember<C, R>::template Func4C<A1, A2, A3, A4, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4)) {
    return &SqMember<C, R&>::template Func4<A1, A2, A3, A4, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4) const) {
    return &SqMember<C, R&>::template Func4C<A1, A2, A3, A4, true>;
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5)) {
    return &SqMember<C, R>::template Func5<A1, A2, A3, A4, A5, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5) const) {
    return &SqMember<C, R>::template Func5C<A1, A2, A3, A4, A5, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5)) {
    return &SqMember<C, R&>::template Func5<A1, A2, A3, A4, A5, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5) const) {
    return &SqMember<C, R&>::template Func5C<A1, A2, A3, A4, A5, true>;
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqMember<C, R>::template Func6<A1, A2, A3, A4, A5, A6, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6) const) {
    return &SqMember<C, R>::template Func6C<A1, A2, A3, A4, A5, A6, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqMember<C, R&>::template Func6<A1, A2, A3, A4, A5, A6, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6) const) {
    return &SqMember<C, R&>::template Func6C<A1, A2, A3, A4, A5, A6, true>;
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqMember<C, R>::template Func7<A1, A2, A3, A4, A5, A6, A7, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const) {
    return &SqMember<C, R>::template Func7C<A1, A2, A3, A4, A5, A6, A7, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqMember<C, R&>::template Func7<A1, A2, A3, A4, A5, A6, A7, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7) const) {
    return &SqMember<C, R&>::template Func7C<A1, A2, A3, A4, A5, A6, A7, true>;
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqMember<C, R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const) {
    return &SqMember<C, R>::template Func8C<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqMember<C, R&>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const) {
    return &SqMember<C, R&>::template Func8C<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqMember<C, R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const) {
    return &SqMember<C, R>::template Func9C<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqMember<C, R&>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const) {
    return &SqMember<C, R&>::template Func9C<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqMember<C, R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const) {
    return &SqMember<C, R>::template Func10C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqMember<C, R&>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const) {
    return &SqMember<C, R&>::template Func10C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqMember<C, R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const) {
    return &SqMember<C, R>::template Func11C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqMember<C, R&>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const) {
    return &SqMember<C, R&>::template Func11C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqMember<C, R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const) {
    return &SqMember<C, R>::template Func12C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqMember<C, R&>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const) {
    return &SqMember<C, R&>::template Func12C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqMember<C, R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const) {
    return &SqMember<C, R>::template Func13C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqMember<C, R&>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const) {
    return &SqMember<C, R&>::template Func13C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}


// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqMember<C, R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const) {
    return &SqMember<C, R>::template Func14C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqMember<C, R&>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const) {
    return &SqMember<C, R&>::template Func14C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}

//
// Overload handler resolver
//

template <class R>
inline SQFUNCTION SqOverloadFunc(R (*method)) {
    return &SqOverload<R>::Func;
}

template <class C, class R>
inline SQFUNCTION SqOverloadFunc(R (C::*method)) {
    return &SqOverload<R>::Func;
}

template <class C, class R>
inline SQFUNCTION SqOverloadFunc(R (C::*method)() const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const ) {
    return &SqOverload<R>::Func;
}

//
// Query argument count
//

// Arg Count 0
template <class R>
inline int SqGetArgCount(R (*method)()) {
    return 0;
}

// Arg Count 1
template <class R, class A1>
inline int SqGetArgCount(R (*method)(A1)) {
    return 1;
}

// Arg Count 2
template <class R, class A1, class A2>
inline int SqGetArgCount(R (*method)(A1, A2)) {
    return 2;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
inline int SqGetArgCount(R (*method)(A1, A2, A3)) {
    return 3;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4)) {
    return 4;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5)) {
    return 5;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6)) {
    return 6;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return 7;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return 8;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return 9;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return 10;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return 11;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return 12;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return 13;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return 14;
}

//
// Query member function argument count
//

// Arg Count 0
template <class C, class R>
inline int SqGetArgCount(R (C::*method)()) {
    return 0;
}

// Arg Count 1
template <class C, class R, class A1>
inline int SqGetArgCount(R (C::*method)(A1)) {
    return 1;
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline int SqGetArgCount(R (C::*method)(A1, A2)) {
    return 2;
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3)) {
    return 3;
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4)) {
    return 4;
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5)) {
    return 5;
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6)) {
    return 6;
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return 7;
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return 8;
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return 9;
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return 10;
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return 11;
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return 12;
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return 13;
}

// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return 14;
}

//
// Query const member function argument count
//

// Arg Count 0
template <class C, class R>
inline int SqGetArgCount(R (C::*method)() const) {
    return 0;
}

// Arg Count 1
template <class C, class R, class A1>
inline int SqGetArgCount(R (C::*method)(A1) const) {
    return 1;
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline int SqGetArgCount(R (C::*method)(A1, A2) const) {
    return 2;
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3) const) {
    return 3;
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4) const) {
    return 4;
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5) const) {
    return 5;
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6) const) {
    return 6;
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const) {
    return 7;
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const) {
    return 8;
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const) {
    return 9;
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const) {
    return 10;
}
// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const) {
    return 11;
}
// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const) {
    return 12;
}
// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const) {
    return 13;
}
// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const) {
    return 14;
}

}

#endif
