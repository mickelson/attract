//
// SqratGlobalMethods: Global Methods
//

//
// Copyright (c) 2009 Brandon Jones
// Copyirght 2011 Li-Cheng (Andy) Tai
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

#if !defined(_SCRAT_GLOBAL_METHODS_H_)
#define _SCRAT_GLOBAL_METHODS_H_

#include <squirrel.h>
#include "sqratTypes.h"

namespace Sqrat {

//
// Squirrel Global Functions
//

template <class R>
class SqGlobal {
public:
    // Arg Count 0
    template <bool overloaded  /* = false */ >
    static SQInteger Func0(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != 2) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)();
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        R ret = (*method)();

        PushVar(vm, ret);
        return 1;
    }



    // Arg Count 1
    template <class A1, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func1(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 1) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 2
    template <class A1, class A2, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func2(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 2) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 3
    template <class A1, class A2, class A3, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func3(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 3) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 4
    template <class A1, class A2, class A3, class A4, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func4(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 4) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 5
    template <class A1, class A2, class A3, class A4, class A5, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func5(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 5) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 6
    template <class A1, class A2, class A3, class A4, class A5, class A6, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func6(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 6) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 7
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func7(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 7) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 8
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func8(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 8) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 9
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func9(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 9) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 10
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func10(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 10) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 11
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func11(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 11) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 12
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func12(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 12) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value,
                    a12.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 13
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func13(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 13) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        Var<A13> a13(vm, startIdx + 12);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value,
                    a12.value,
                    a13.value
                );

        PushVar(vm, ret);
        return 1;
    }

    // Arg Count 14
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func14(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 14) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        Var<A13> a13(vm, startIdx + 12);
        Var<A14> a14(vm, startIdx + 13);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value,
                    a12.value,
                    a13.value,
                    a14.value
                );

        PushVar(vm, ret);
        return 1;
    }
};

//
// reference return specialization
//

template <class R>
class SqGlobal<R& > {
public:
    // Arg Count 0
    template <bool overloaded  /* = false */ >
    static SQInteger Func0(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != 2) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)();
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        R & ret = (*method)();

        PushVarR(vm, ret);
        return 1;
    }



    // Arg Count 1
    template <class A1, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func1(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 1) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 2
    template <class A1, class A2, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func2(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 2) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 3
    template <class A1, class A2, class A3, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func3(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 3) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 4
    template <class A1, class A2, class A3, class A4, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func4(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 4) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 5
    template <class A1, class A2, class A3, class A4, class A5, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func5(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 5) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 6
    template <class A1, class A2, class A3, class A4, class A5, class A6, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func6(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 6) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 7
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func7(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 7) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 8
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func8(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 8) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 9
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func9(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 9) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 10
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func10(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 10) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 11
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func11(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 11) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 12
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func12(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 12) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value,
                    a12.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 13
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func13(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 13) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        Var<A13> a13(vm, startIdx + 12);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value,
                    a12.value,
                    a13.value
                );

        PushVarR(vm, ret);
        return 1;
    }

    // Arg Count 14
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func14(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 14) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef R & (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        Var<A13> a13(vm, startIdx + 12);
        Var<A14> a14(vm, startIdx + 13);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        R & ret = (*method)(
                    a1.value,
                    a2.value,
                    a3.value,
                    a4.value,
                    a5.value,
                    a6.value,
                    a7.value,
                    a8.value,
                    a9.value,
                    a10.value,
                    a11.value,
                    a12.value,
                    a13.value,
                    a14.value
                );

        PushVarR(vm, ret);
        return 1;
    }
};



//
// void return specialization
//

template <>
class SqGlobal<void> {
public:
    // Arg Count 0
    template <bool overloaded  /* = false */>
    static SQInteger Func0(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != 2) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)();
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);
        (*method)();
        return 0;
    }

    // Arg Count 1
    template <class A1, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func1(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 1) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value
        );
        return 0;
    }

    // Arg Count 2
    template <class A1, class A2, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func2(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 2) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value
        );
        return 0;
    }

    // Arg Count 3
    template <class A1, class A2, class A3, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func3(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 3) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value
        );
        return 0;
    }

    // Arg Count 4
    template <class A1, class A2, class A3, class A4, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func4(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 4) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value
        );
        return 0;
    }

    // Arg Count 5
    template <class A1, class A2, class A3, class A4, class A5, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func5(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 5) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value
        );
        return 0;
    }

    // Arg Count 6
    template <class A1, class A2, class A3, class A4, class A5, class A6, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func6(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 6) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value
        );
        return 0;
    }

    // Arg Count 7
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func7(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 7) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value
        );
        return 0;
    }

    // Arg Count 8
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func8(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 8) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value
        );
        return 0;
    }

    // Arg Count 9
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func9(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 9) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value
        );
        return 0;
    }

    // Arg Count 10
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func10(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 10) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value,
            a10.value
        );
        return 0;
    }

    // Arg Count 11
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func11(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 11) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value,
            a10.value,
            a11.value
        );
        return 0;
    }

    // Arg Count 12
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func12(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 12) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value,
            a10.value,
            a11.value,
            a12.value
        );
        return 0;
    }

    // Arg Count 13
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func13(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 13) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        Var<A13> a13(vm, startIdx + 12);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value,
            a10.value,
            a11.value,
            a12.value,
            a13.value
        );
        return 0;
    }

    // Arg Count 14
    template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, SQInteger startIdx, bool overloaded /*= false*/>
    static SQInteger Func14(HSQUIRRELVM vm) {
        if (!overloaded && sq_gettop(vm) != startIdx + 14) {
            return sq_throwerror(vm, _SC("wrong number of parameters"));
        }
        typedef void (*M)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
        M* method;
        sq_getuserdata(vm, -1, (SQUserPointer*)&method, NULL);

        Var<A1> a1(vm, startIdx);
        Var<A2> a2(vm, startIdx + 1);
        Var<A3> a3(vm, startIdx + 2);
        Var<A4> a4(vm, startIdx + 3);
        Var<A5> a5(vm, startIdx + 4);
        Var<A6> a6(vm, startIdx + 5);
        Var<A7> a7(vm, startIdx + 6);
        Var<A8> a8(vm, startIdx + 7);
        Var<A9> a9(vm, startIdx + 8);
        Var<A10> a10(vm, startIdx + 9);
        Var<A11> a11(vm, startIdx + 10);
        Var<A12> a12(vm, startIdx + 11);
        Var<A13> a13(vm, startIdx + 12);
        Var<A14> a14(vm, startIdx + 13);
        if (Error::Instance().Occurred(vm)) {
            if (overloaded)
                return 0;
            else
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }
        (*method)(
            a1.value,
            a2.value,
            a3.value,
            a4.value,
            a5.value,
            a6.value,
            a7.value,
            a8.value,
            a9.value,
            a10.value,
            a11.value,
            a12.value,
            a13.value,
            a14.value
        );
        return 0;
    }
};


//
// Global Function Resolvers
//

// Arg Count 0
template <class R>
SQFUNCTION SqGlobalFunc(R (*method)()) {
    return &SqGlobal<R>::template Func0<false>;
}

template <class R>
SQFUNCTION SqGlobalFunc(R & (*method)()) {
    return &SqGlobal<R& >::template Func0<false>;
}

// Arg Count 1
template <class R, class A1>
SQFUNCTION SqGlobalFunc(R (*method)(A1)) {
    return &SqGlobal<R>::template Func1<A1, 2, false>;
}

template <class R, class A1>
SQFUNCTION SqGlobalFunc(R & (*method)(A1)) {
    return &SqGlobal<R&>::template Func1<A1, 2, false>;
}

// Arg Count 2
template <class R, class A1, class A2>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2)) {
    return &SqGlobal<R>::template Func2<A1, A2, 2, false>;
}

template <class R, class A1, class A2>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2)) {
    return &SqGlobal<R& >::template Func2<A1, A2, 2, false>;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3)) {
    return &SqGlobal<R>::template Func3<A1, A2, A3, 2, false>;
}

template <class R, class A1, class A2, class A3>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3)) {
    return &SqGlobal<R& >::template Func3<A1, A2, A3, 2, false>;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R>::template Func4<A1, A2, A3, A4, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R& >::template Func4<A1, A2, A3, A4, 2, false>;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R>::template Func5<A1, A2, A3, A4, A5, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R&>::template Func5<A1, A2, A3, A4, A5, 2, false>;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R>::template Func6<A1, A2, A3, A4, A5, A6, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R& >::template Func6<A1, A2, A3, A4, A5, A6, 2, false>;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R>::template Func7<A1, A2, A3, A4, A5, A6, A7, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R &>::template Func7<A1, A2, A3, A4, A5, A6, A7, 2, false>;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R& >::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 2, false>;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R& >::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 2, false>;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R& >::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 2, false>;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R& >::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 2, false>;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R& >::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 2, false>;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R& >::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 2, false>;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 2, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R& >::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 2, false>;
}

//
// Member Global Function Resolvers
//

// Arg Count 1
template <class R, class A1>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1)) {
    return &SqGlobal<R>::template Func1<A1, 1, false>;
}

template <class R, class A1>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1)) {
    return &SqGlobal<R& >::template Func1<A1, 1, false>;
}

// Arg Count 2
template <class R, class A1, class A2>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2)) {
    return &SqGlobal<R>::template Func2<A1, A2, 1, false>;
}

template <class R, class A1, class A2>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2)) {
    return &SqGlobal<R& >::template Func2<A1, A2, 1, false>;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3)) {
    return &SqGlobal<R>::template Func3<A1, A2, A3, 1, false>;
}

template <class R, class A1, class A2, class A3>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3)) {
    return &SqGlobal<R& >::template Func3<A1, A2, A3, 1, false>;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R>::template Func4<A1, A2, A3, A4, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4)) {
    return &SqGlobal<R& >::template Func4<A1, A2, A3, A4, 1, false>;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R>::template Func5<A1, A2, A3, A4, A5, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5)) {
    return &SqGlobal<R& >::template Func5<A1, A2, A3, A4, A5, 1, false>;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R>::template Func6<A1, A2, A3, A4, A5, A6, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6)) {
    return &SqGlobal<R& >::template Func6<A1, A2, A3, A4, A5, A6, 1, false>;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R>::template Func7<A1, A2, A3, A4, A5, A6, A7, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7)) {
    return &SqGlobal<R& >::template Func7<A1, A2, A3, A4, A5, A6, A7, 1, false>;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8)) {
    return &SqGlobal<R& >::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 1, false>;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) {
    return &SqGlobal<R& >::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 1, false>;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)) {
    return &SqGlobal<R& >::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 1, false>;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)) {
    return &SqGlobal<R& >::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 1, false>;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)) {
    return &SqGlobal<R& >::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 1, false>;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)) {
    return &SqGlobal<R& >::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 1, false>;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqMemberGlobalFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 1, false>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqMemberGlobalFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)) {
    return &SqGlobal<R& >::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 1, false>;
}


}

#endif
