//
//  Copyright (c) 2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NOWIDE_CSTDIO_H_INCLUDED
#define NOWIDE_CSTDIO_H_INCLUDED

#include <cstdio>
#include <stdio.h>
#include <nowide/config.hpp>
#include <nowide/convert.hpp>
#include <nowide/stackstring.hpp>
#include <errno.h>

#ifdef NOWIDE_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif



namespace nowide {
#if !defined(NOWIDE_WINDOWS) && !defined(NOWIDE_DOXYGEN)
    using std::fopen;
    using std::freopen;
    using std::remove;
    using std::rename;
#else

///
/// \brief Same as freopen but file_name and mode are UTF-8 strings
///
inline FILE *freopen(char const *file_name,char const *mode,FILE *stream)
{
    wstackstring wname(file_name);
    wshort_stackstring wmode(mode);
    return _wfreopen(wname.c_str(),wmode.c_str(),stream);
}
///
/// \brief Same as fopen but file_name and mode are UTF-8 strings
///
inline FILE *fopen(char const *file_name,char const *mode)
{
    wstackstring wname(file_name);
    wshort_stackstring wmode(mode);
    return _wfopen(wname.c_str(),wmode.c_str());
}
///
/// \brief Same as rename but old_name and new_name are UTF-8 strings
///
inline int rename(char const *old_name,char const *new_name)
{
    wstackstring wold(old_name),wnew(new_name);
    return _wrename(wold.c_str(),wnew.c_str());
}
///
/// \brief Same as rename but name is UTF-8 string
///
inline int remove(char const *name)
{
    wstackstring wname(name);
    return _wremove(wname.c_str());
}
#endif
} // nowide


#ifdef NOWIDE_MSVC
#pragma warning(pop)
#endif

#endif
///
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
