
//
// SqratClass: Class Binding
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

#if !defined(_SCRAT_CLASS_H_)
#define _SCRAT_CLASS_H_

#include <typeinfo>
#include <squirrel.h>
#include <string.h>

#include "sqratObject.h"
#include "sqratClassType.h"
#include "sqratMemberMethods.h"
#include "sqratAllocator.h"

namespace Sqrat
{

/**
    @tparam    C    class type to expose
    @tparam A    allocator to use when instantiating and destroying class instances in Squirrel

    @remarks
    DefaultAllocator<C> is used if no allocator is specified. This should be sufficent for most classes
    but if specific behavior is desired it can be overridden. If the class should not be instantiated from
    Squirrel the NoConstructor allocator may be used.
*/
/// Exposes a C++ class to Squirrel
template<class C, class A = DefaultAllocator<C> >
class Class : public Object
{
    static SQInteger cleanup_hook(SQUserPointer p, SQInteger size)
    {
        HSQUIRRELVM v = *(HSQUIRRELVM *) p;
        ClassTypeData<C, void>* ptr = static_cast<ClassTypeData<C, void>*>(ClassType<C>::getClassTypeData(v));
        delete ptr;
        ClassType<C>::deleteClassTypeData(v);
        return 1;
    }
public:
    /**
        @param v    Squirrel virtual machine to bind to
    */
    /// Constructor
    Class(HSQUIRRELVM v = DefaultVM::Get(), const string& className = string() , bool createClass = true) : Object(v, false) {
        if(createClass && !ClassType<C>::hasClassTypeData(v)) {
            ClassType<C>::getClassTypeData(v) = new ClassTypeData<C, void>;
            HSQOBJECT& classObj = ClassType<C>::ClassObject(v);
            sq_resetobject(&classObj);

            sq_newclass(vm, false);
            sq_getstackobj(vm, -1, &classObj);
            sq_addref(vm, &classObj); // must addref before the pop!
            sq_pop(vm, 1);
            if (className.empty()) 
#ifdef SQUNICODE                
                InitClass(string_to_wstring(typeid(*this).name()));
#else
                InitClass(typeid(*this).name());
#endif
            else InitClass(className);

            // install cleanup hook
            HSQUIRRELVM *p = (HSQUIRRELVM *) sq_newuserdata(v, sizeof(v));
            *p = v;
            
            sq_setreleasehook(v, -1, cleanup_hook);
            // finish install cleanup hook
        }
    }

    ~Class() {
        /*ClassType<C>::deleteClassTypeData(vm);*/
        /* done in cleanup_hook */
    }

    /// Get the Squirrel Object for this Class (const)
    virtual HSQOBJECT GetObject() const {
        return ClassType<C>::ClassObject(vm);
    }

    /// Get the Squirrel Object for this Class (ref)
    virtual HSQOBJECT& GetObject() {
        return ClassType<C>::ClassObject(vm);
    }

public:
    //
    // Variable Binding
    //

    /**
        @param name    name of the static slot
        @param var    value to assign
    */
    /// Assign a static class slot a value
    template<class V>
    Class& SetStaticValue(const SQChar* name, const V& val) {
        BindValue<V>(name, val, true);
        return *this;
    }

    Class& SetStaticValue(const SQChar* name, int val) {
        BindValue<int>(name, val, true);
        return *this;
    }

    /**
        @param name    name of the slot
        @param var    value to assign
    */
    /// Assign a class slot a value
    template<class V>
    Class& SetValue(const SQChar* name, const V& val) {
        BindValue<V>(name, val, false);
        return *this;
    }

    /**
        @param name    name of the variable as it will appear in Squirrel
        @param var    variable to bind
    */
    /// Bind a class variable
    template<class V>
    Class& Var(const SQChar* name, V C::* var) {
        // Add the getter
        BindAccessor(name, &var, sizeof(var), &sqDefaultGet<C, V>, ClassType<C>::GetTable(vm));

        // Add the setter
        BindAccessor(name, &var, sizeof(var), &sqDefaultSet<C, V>, ClassType<C>::SetTable(vm));

        return *this;
    }

    /**
        @param name    name of the variable as it will appear in Squirrel
        @param var    variable to bind
    */
    /// Bind a class static variable
    template<class V>
    Class& StaticVar(const SQChar* name, V * var) {
        // Add the getter
        BindAccessor(name, &var, sizeof(var), &sqStaticGet<C, V>, ClassType<C>::GetTable(vm));

        // Add the setter
        BindAccessor(name, &var, sizeof(var), &sqStaticSet<C, V>, ClassType<C>::SetTable(vm));

        return *this;
    }

    /// Bind a class property (variable accessed via a setter and getter)
    template<class V>
    Class& Prop(const SQChar* name, V (C::*getMethod)() const, void (C::*setMethod)(const V&)) {
        if(getMethod != NULL) {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));
        }

        if(setMethod != NULL) {
            // Add the setter
            BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberOverloadedFunc(setMethod), ClassType<C>::SetTable(vm));
        }

        return *this;
    }
    
    /// Bind a class property (variable accessed via a setter and getter)
    template<class V>
    Class& Prop(const SQChar* name, V (C::*getMethod)() const, void (C::*setMethod)(V)) {
        if(getMethod != NULL) {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));
        }

        if(setMethod != NULL) {
            // Add the setter
            BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberOverloadedFunc(setMethod), ClassType<C>::SetTable(vm));
        }

        return *this;
    }

    /// Bind a class property (variable accessed via a setter and getter)
    template<class V>
    Class& Prop(const SQChar* name, V (C::*getMethod)(), void (C::*setMethod)(V)) {
        if(getMethod != NULL) {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));
        }

        if(setMethod != NULL) {
            // Add the setter
            BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberOverloadedFunc(setMethod), ClassType<C>::SetTable(vm));
        }

        return *this;
    }
    template<class F1, class F2>
    Class& GlobalProp(const SQChar* name, F1 getMethod, F2 setMethod) {
        if(getMethod != NULL) {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberGlobalOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));
        }

        if(setMethod != NULL) {
            // Add the setter
            BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberGlobalOverloadedFunc(setMethod), ClassType<C>::SetTable(vm));
        }

        return *this;
    }

    /// Bind a read only class property (variable accessed via a getter)
    template<class V>
    Class& Prop(const SQChar* name, V (C::*getMethod)() const) {
        // Add the getter
        BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));

        return *this;
    }

    /// Bind a read only class property (variable accessed via a getter)
    template<class V>
    Class& Prop(const SQChar* name, V (C::*getMethod)()) {
        // Add the getter
        BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));

        return *this;
    }
    template<class F>
    Class& GlobalProp(const SQChar* name, F getMethod) {
        // Add the getter
        BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberGlobalOverloadedFunc(getMethod), ClassType<C>::GetTable(vm));

        return *this;
    }

    // TODO: Handle static instance vars

    //
    // Function Binding
    //

    template<class F>
    Class& Func(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberFunc(method));
        return *this;
    }

    template<class F>
    Class& Overload(const SQChar* name, F method) {
        BindOverload(name, &method, sizeof(method), SqMemberOverloadedFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
        return *this;
    }

    template<class F>
    Class& GlobalFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberGlobalFunc(method));
        return *this;
    }

    template<class F>
    Class& StaticFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqGlobalFunc(method));
        return *this;
    }

    template<class F>
    Class& GlobalOverload(const SQChar* name, F method) {
        BindOverload(name, &method, sizeof(method), SqMemberGlobalOverloadedFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
        return *this;
    }

    template<class F>
    Class& StaticOverload(const SQChar* name, F method) {
        BindOverload(name, &method, sizeof(method), SqGlobalOverloadedFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
        return *this;
    }

    Class& SquirrelFunc(const SQChar* name, SQFUNCTION func) {
        sq_pushobject(vm, ClassType<C>::ClassObject(vm));
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, func, 0);
        sq_newslot(vm, -3, false);
        sq_pop(vm, 1); // pop table

        return *this;
    }

    //
    // Function Calls
    //

    Function GetFunction(const SQChar* name) {
        HSQOBJECT funcObj;
        sq_pushobject(vm, ClassType<C>::ClassObject(vm));
        sq_pushstring(vm, name, -1);
        if(SQ_FAILED(sq_get(vm, -2))) {
            sq_pushnull(vm);
        }
        sq_getstackobj(vm, -1, &funcObj);
        sq_pop(vm, 2);

        return Function(vm, ClassType<C>::ClassObject(vm), funcObj);
    }

protected:
    static SQInteger ClassWeakref(HSQUIRRELVM vm) {
        sq_weakref(vm, -1);
        return 1;
    }

    static SQInteger ClassTypeof(HSQUIRRELVM vm) {
        sq_pushstring(vm, ClassType<C>::ClassName(vm).c_str(), -1);
        return 1;
    }

    // Initialize the required data structure for the class
    void InitClass(const string& className) {
        ClassType<C>::CopyFunc(vm) = &A::Copy;
        ClassType<C>::ClassName(vm) = className;
        ClassType<C>::BaseClass(vm) = NULL;

        // push the class
        sq_pushobject(vm, ClassType<C>::ClassObject(vm));

        // set the typetag of the class
        sq_settypetag(vm, -1, ClassType<C>::getClassTypeData(vm));

        // add the default constructor
        sq_pushstring(vm,_SC("constructor"), -1);
        sq_newclosure(vm, &A::New, 0);
        sq_newslot(vm, -3, false);

        // add the set table (static)
        HSQOBJECT& setTable = ClassType<C>::SetTable(vm);
        sq_resetobject(&setTable);
        sq_pushstring(vm,_SC("__setTable"), -1);
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &setTable);
        sq_addref(vm, &setTable);
        sq_newslot(vm, -3, true);

        // add the get table (static)
        HSQOBJECT& getTable = ClassType<C>::GetTable(vm);
        sq_resetobject(&getTable);
        sq_pushstring(vm,_SC("__getTable"), -1);
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &getTable);
        sq_addref(vm, &getTable);
        sq_newslot(vm, -3, true);

        // override _set
        sq_pushstring(vm, _SC("_set"), -1);
        sq_pushobject(vm, setTable); // Push the set table as a free variable
        sq_newclosure(vm, &sqVarSet, 1);
        sq_newslot(vm, -3, false);

        // override _get
        sq_pushstring(vm, _SC("_get"), -1);
        sq_pushobject(vm, getTable); // Push the get table as a free variable
        sq_newclosure(vm, &sqVarGet, 1);
        sq_newslot(vm, -3, false);

        // add weakref (apparently not provided by default)
        sq_pushstring(vm, _SC("weakref"), -1);
        sq_newclosure(vm, &Class::ClassWeakref, 0);
        sq_newslot(vm, -3, false);

        // add _typeof
        sq_pushstring(vm, _SC("_typeof"), -1);
        sq_newclosure(vm, &Class::ClassTypeof, 0);
        sq_newslot(vm, -3, false);

        // pop the class
        sq_pop(vm, 1);
    }

    // Helper function used to bind getters and setters
    inline void BindAccessor(const SQChar* name, void* var, size_t varSize, SQFUNCTION func, HSQOBJECT table) {
        // Push the get or set table
        sq_pushobject(vm, table);
        sq_pushstring(vm, name, -1);

        // Push the variable offset as a free variable
        SQUserPointer varPtr = sq_newuserdata(vm, static_cast<SQUnsignedInteger>(varSize));
        memcpy(varPtr, var, varSize);

        // Create the accessor function
        sq_newclosure(vm, func, 1);

        // Add the accessor to the table
        sq_newslot(vm, -3, false);

        // Pop get/set table
        sq_pop(vm, 1);
    }

    
    // constructor binding

    Class& BindConstructor(SQFUNCTION method, SQInteger nParams, const SQChar *name = 0){
        SQFUNCTION overload = SqOverloadFunc(method);
        bool alternative_global = false;
        if (name == 0)
            name = _SC("constructor");
        else alternative_global = true;
        string overloadName = SqOverloadName::Get(name, nParams);

        if (!alternative_global )
        {
            // push the class
            sq_pushobject(vm, ClassType<C>::ClassObject(vm));
        }
        else
        {  // the containing environment is the root table??
            sq_pushroottable(vm);            
        }
        // Bind overload handler
        sq_pushstring(vm, name, -1);
        sq_pushstring(vm, name, -1); // function name is passed as a free variable
        sq_newclosure(vm, overload, 1);
        sq_newslot(vm, -3, false);
        
        // Bind overloaded allocator function
        
        sq_pushstring(vm, overloadName.c_str(), -1);
        sq_newclosure(vm, method, 0);
        sq_setparamscheck(vm,nParams + 1,NULL);
        sq_newslot(vm, -3, false);
        sq_pop(vm, 1);
        return *this;
    }
    
    void setDefaultCtor() 
    {
        ClassTypeDataBase *type_data = ClassType<C>::getClassTypeData(vm);
        if (type_data->ctorCalled == false)
        {
            Ctor(); // set up default constructor with the 'constructor' override
            type_data->ctorCalled = true;
        }
    }
public:        
    Class& Ctor(const SQChar *name = 0) {
        return BindConstructor(A::template iNew<0>, 0, name);
    }
    
    template<class A1>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1>,1, name);
    }
    template<class A1,class A2>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2>,2, name);
    }
    template<class A1,class A2,class A3>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3>,3, name);
    }
    template<class A1,class A2,class A3,class A4>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3,A4>,4, name);
    }
    template<class A1,class A2,class A3,class A4,class A5>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3,A4,A5>,5, name);
    }
    template<class A1,class A2,class A3,class A4,class A5,class A6>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3,A4,A5,A6>,6, name);
    }
    template<class A1,class A2,class A3,class A4,class A5,class A6,class A7>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3,A4,A5,A6,A7>,7, name);
    }
    template<class A1,class A2,class A3,class A4,class A5,class A6,class A7,class A8>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3,A4,A5,A6,A7,A8>,8, name);
    }
    template<class A1,class A2,class A3,class A4,class A5,class A6,class A7,class A8,class A9>
    Class& Ctor(const SQChar *name = 0) {
        setDefaultCtor();
        return BindConstructor(A::template iNew<A1,A2,A3,A4,A5,A6,A7,A8,A9>,9, name);
    }

    
};


/**
    @tparam    C    class type to expose
    @tparam    B    base class type (must already be bound)
    @tparam A    allocator to use when instantiating and destroying class instances in Squirrel

    @remarks
    DefaultAllocator<C> is used if no allocator is specified. This should be sufficent for most classes
    but if specific behavior is desired it can be overridden. If the class should not be instantiated from
    Squirrel the NoConstructor allocator may be used.

    @remarks
    Classes in Squirrel are single-inheritance only, and as such Sqrat only allows for single inheritance as well
*/
/// Exposes a C++ class with a base class to Squirrel
template<class C, class B, class A = DefaultAllocator<C> >
class DerivedClass : public Class<C, A>
{
    static SQInteger cleanup_hook(SQUserPointer p, SQInteger size)
    {
        HSQUIRRELVM v = *(HSQUIRRELVM *) p;
        ClassTypeData<C, B>* ptr = static_cast<ClassTypeData<C, B>*>(ClassType<C>::getClassTypeData(v));
        delete ptr;
        ClassType<C>::deleteClassTypeData(v);
        return 1;
    }
public:
    DerivedClass(HSQUIRRELVM v = DefaultVM::Get(), const string& className = string()) : Class<C, A>(v, string(), false) {
        if(!ClassType<C>::hasClassTypeData(v)) {
            ClassType<C>::getClassTypeData(v) = new ClassTypeData<C, B>;
            HSQOBJECT& classObj = ClassType<C>::ClassObject(v);
            sq_resetobject(&classObj);

            sq_pushobject(v, ClassType<B>::ClassObject(v));
            sq_newclass(v, true);
            sq_getstackobj(v, -1, &classObj);
            sq_addref(v, &classObj); // must addref before the pop!
            sq_pop(v, 1);
            if (className.empty()) 
#ifdef SQUNICODE                
                InitDerivedClass(v, string_to_wstring(typeid(*this).name()));
#else
                InitDerivedClass(v, typeid(*this).name());
#endif
            else InitDerivedClass(v, className);

            // install cleanup hook
            HSQUIRRELVM *p = (HSQUIRRELVM *) sq_newuserdata(v, sizeof(v));
            *p = v;

            sq_setreleasehook(v, -1, cleanup_hook);
            // finish install cleanup hook
        }
    }

protected:
    void InitDerivedClass(HSQUIRRELVM vm, const string& className) {
        ClassType<C>::CopyFunc(vm) = &A::Copy;
        ClassType<C>::ClassName(vm) = className;
        ClassType<C>::BaseClass(vm) = ClassType<B>::getClassTypeData(vm);

        // push the class
        sq_pushobject(vm, ClassType<C>::ClassObject(vm));

        // set the typetag of the class
        sq_settypetag(vm, -1, ClassType<C>::getClassTypeData(vm));
        // add the default constructor
        sq_pushstring(vm,_SC("constructor"), -1);
        sq_newclosure(vm, &A::New, 0);
        sq_newslot(vm, -3, false);

        // clone the base classes set table (static)
        HSQOBJECT& setTable = ClassType<C>::SetTable(vm);
        sq_resetobject(&setTable);
        sq_pushobject(vm, ClassType<B>::SetTable(vm));
        sq_pushstring(vm,_SC("__setTable"), -1);
        sq_clone(vm, -2);
        sq_remove(vm, -3);
        sq_getstackobj(vm, -1, &setTable);
        sq_addref(vm, &setTable);
        sq_newslot(vm, -3, true);

        // clone the base classes get table (static)
        HSQOBJECT& getTable = ClassType<C>::GetTable(vm);
        sq_resetobject(&getTable);
        sq_pushobject(vm, ClassType<B>::GetTable(vm));
        sq_pushstring(vm,_SC("__getTable"), -1);
        sq_clone(vm, -2);
        sq_remove(vm, -3);
        sq_getstackobj(vm, -1, &getTable);
        sq_addref(vm, &getTable);
        sq_newslot(vm, -3, true);

        // override _set
        sq_pushstring(vm, _SC("_set"), -1);
        sq_pushobject(vm, setTable); // Push the set table as a free variable
        sq_newclosure(vm, sqVarSet, 1);
        sq_newslot(vm, -3, false);

        // override _get
        sq_pushstring(vm, _SC("_get"), -1);
        sq_pushobject(vm, getTable); // Push the get table as a free variable
        sq_newclosure(vm, sqVarGet, 1);
        sq_newslot(vm, -3, false);

        // add weakref (apparently not provided by default)
        sq_pushstring(vm, _SC("weakref"), -1);
        sq_newclosure(vm, &Class<C, A>::ClassWeakref, 0);
        sq_newslot(vm, -3, false);

        // add _typeof
        sq_pushstring(vm, _SC("_typeof"), -1);
        sq_newclosure(vm, &Class<C, A>::ClassTypeof, 0);
        sq_newslot(vm, -3, false);

        // pop the class
        sq_pop(vm, 1);
    }
};

}

#endif
