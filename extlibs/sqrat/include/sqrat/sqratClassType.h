
//
// SqratClassType: Type Translators
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
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source
//    distribution.
//

#if !defined(_SCRAT_CLASSTYPE_H_)
#define _SCRAT_CLASSTYPE_H_

#include <squirrel.h>
#include <map>

#include "sqratUtil.h"
namespace Sqrat
{

//
// ClassType
//

// Get the Copy Function for this Class
typedef SQInteger (*COPYFUNC)(HSQUIRRELVM, SQInteger, const void*);

struct ClassTypeDataBase {
    HSQOBJECT    classObj;
    HSQOBJECT    getTable;
    HSQOBJECT    setTable;
    COPYFUNC    copyFunc;
    string        className;
    ClassTypeDataBase* baseClass;
    bool        ctorCalled;
    virtual ~ClassTypeDataBase() {}
    virtual SQUserPointer Cast(SQUserPointer ptr, SQUserPointer classType) = 0;
    ClassTypeDataBase() : ctorCalled(false){}
};

template<class C, class B>
struct ClassTypeData : public ClassTypeDataBase {
    virtual SQUserPointer Cast(SQUserPointer ptr, SQUserPointer classType) {

        if (classType != this) {
            ptr = baseClass->Cast(static_cast<B*>(static_cast<C*>(ptr)), classType);
        }
        return ptr;
    }
};

template<class C>
struct ClassType {

    static std::map< HSQUIRRELVM, ClassTypeDataBase* > s_classTypeDataMap;

    static inline ClassTypeDataBase*& getClassTypeData(HSQUIRRELVM vm) {
        //TODO: use mutex to lock s_classTypeDataMap in multithreaded environment
        return s_classTypeDataMap[vm];
    }

    static inline bool hasClassTypeData(HSQUIRRELVM vm) {
        //TODO: use mutex to lock s_classTypeDataMap in multithreaded environment
        return (s_classTypeDataMap.find(vm) != s_classTypeDataMap.end());
    }

    static inline void deleteClassTypeData(HSQUIRRELVM vm) {
        //TODO: use mutex to lock s_classTypeDataMap in multithreaded environment
        std::map< HSQUIRRELVM, ClassTypeDataBase* >::iterator it = s_classTypeDataMap.find(vm);
        if(it != s_classTypeDataMap.end()) {
            s_classTypeDataMap.erase(it);
        }
    }

    // Get the Squirrel Object for this Class
    static inline HSQOBJECT& ClassObject(HSQUIRRELVM vm) {
        return getClassTypeData(vm)->classObj;
    }

    // Get the Get Table for this Class
    static inline HSQOBJECT& GetTable(HSQUIRRELVM vm) {
        return getClassTypeData(vm)->getTable;
    }

    // Get the Set Table for this Class
    static inline HSQOBJECT& SetTable(HSQUIRRELVM vm) {
        return getClassTypeData(vm)->setTable;
    }

    static inline COPYFUNC& CopyFunc(HSQUIRRELVM vm) {
        return getClassTypeData(vm)->copyFunc;
    }

    static inline string& ClassName(HSQUIRRELVM vm) {
        return getClassTypeData(vm)->className;
    }

    static inline ClassTypeDataBase*& BaseClass(HSQUIRRELVM vm) {
        return getClassTypeData(vm)->baseClass;
    }

    static void PushInstance(HSQUIRRELVM vm, C* ptr) {
        if (ptr != NULL) {
            sq_pushobject(vm, ClassObject(vm));
            sq_createinstance(vm, -1);
            sq_remove(vm, -2);
            sq_setinstanceup(vm, -1, ptr);
        }
        else 
            sq_pushnull(vm);
    }

    static void PushInstanceCopy(HSQUIRRELVM vm, const C& value) {
        sq_pushobject(vm, ClassObject(vm));
        sq_createinstance(vm, -1);
        sq_remove(vm, -2);
        CopyFunc(vm)(vm, -1, &value);
    }

    static C* GetInstance(HSQUIRRELVM vm, SQInteger idx) {
        SQUserPointer ptr = NULL;
        ClassTypeDataBase* classType = getClassTypeData(vm);
        if (classType != 0) /* type checking only done if the value has type data else it may be enum */
        {
            if (SQ_FAILED(sq_getinstanceup(vm, idx, &ptr, classType))) {
                Error::Instance().Throw(vm, Sqrat::Error::FormatTypeError(vm, idx, ClassName(vm)));
                return NULL;
            }
        }
        else /* value is likely of integral type like enums, cannot return a pointer */
        {
            Error::Instance().Throw(vm, Sqrat::Error::FormatTypeError(vm, idx, _SC("unknown")));
            return NULL;
        }
        ClassTypeDataBase* actualType;
        sq_gettypetag(vm, idx, (SQUserPointer*)&actualType);
        if (actualType == NULL) {
            SQInteger top = sq_gettop(vm);
            sq_getclass(vm, idx);
            while (actualType == NULL) {
                sq_getbase(vm, -1);
                sq_gettypetag(vm, -1, (SQUserPointer*)&actualType);
            }
            sq_settop(vm, top);
        }
        if (classType != actualType) {
            return static_cast<C*>(actualType->Cast(ptr, classType));
        }
        return static_cast<C*>(ptr);
    }
};

template<class C>
std::map< HSQUIRRELVM, ClassTypeDataBase* > ClassType<C>::s_classTypeDataMap;

}

#endif
