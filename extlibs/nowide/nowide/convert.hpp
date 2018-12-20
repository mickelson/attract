//
//  Copyright (c) 2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NOWIDE_CONVERT_H_INCLUDED
#define NOWIDE_CONVERT_H_INCLUDED

#include <string>
#include <nowide/encoding_utf.hpp>
#include <nowide/replacement.hpp>


namespace nowide {
    ///
    /// \brief Template function that converts a buffer of UTF sequences in range [source_begin,source_end)
    /// to the output \a buffer of size \a buffer_size.
    ///
    /// In case of success a NULL terminated string is returned (buffer), otherwise 0 is returned.
    ///
    /// If there is not enough room in the buffer 0 is returned, and the contents of the buffer are undefined.
    /// Any illegal sequences are replaced with U+FFFD substutution charracter
    ///
    template<typename CharOut,typename CharIn>
    CharOut *basic_convert(CharOut *buffer,size_t buffer_size,CharIn const *source_begin,CharIn const *source_end)
    {
        CharOut *rv = buffer;
        if(buffer_size == 0)
            return 0;
        buffer_size --;
        while(source_begin!=source_end) {
            using namespace nowide::utf;
            code_point c = utf_traits<CharIn>::template decode<CharIn const *>(source_begin,source_end);
            if(c==illegal || c==incomplete) {
                c = NOWIDE_REPLACEMENT_CHARACTER;
            }
            size_t width = utf_traits<CharOut>::width(c);
            if(buffer_size < width) {
                rv=0;
                break;
            }
            buffer = utf_traits<CharOut>::template encode<CharOut *>(c,buffer);
            buffer_size -= width;
        }
        *buffer++ = 0;
        return rv;
    }

    ///
    /// \brief Template function that converts a buffer of UTF sequences in range [source_begin,source_end) and returns a string containing converted value
    ///
    /// Any illegal sequences are replaced with U+FFFD substutution charracter
    ///
    template<typename CharOut,typename CharIn>
    std::basic_string<CharOut>
    basic_convert(CharIn const *begin,CharIn const *end)
    {
        
        std::basic_string<CharOut> result;
        result.reserve(end-begin);
        typedef std::back_insert_iterator<std::basic_string<CharOut> > inserter_type;
        inserter_type inserter(result);
        using namespace nowide::utf;
        code_point c;
        while(begin!=end) {
            c=utf_traits<CharIn>::template decode<CharIn const *>(begin,end);
            if(c==illegal || c==incomplete) {
                c=NOWIDE_REPLACEMENT_CHARACTER;
            }
            utf_traits<CharOut>::template encode<inserter_type>(c,inserter);
        }
        return result;
    }
    
    /// \cond INTERNAL
    namespace details {
        //
        // wcslen defined only in C99... So we will not use it
        //
        template<typename Char>
        Char const *basic_strend(Char const *s)
        {
            while(*s)
                s++;
            return s;
        }
    }
    /// \endcond
    
    ///
    /// \brief Template function that converts a string \a s from one type of UTF to another UTF and returns a string containing converted value
    ///
    /// Any illegal sequences are replaced with U+FFFD substutution charracter
    ///
    template<typename CharOut,typename CharIn>
    std::basic_string<CharOut>
    basic_convert(std::basic_string<CharIn> const &s)
    {
        return basic_convert<CharOut>(s.c_str(),s.c_str()+s.size());
    }
    ///
    /// \brief Template function that converts a string \a s from one type of UTF to another UTF and returns a string containing converted value
    ///
    /// Any illegal sequences are replaced with U+FFFD substutution charracter
    ///
    template<typename CharOut,typename CharIn>
    std::basic_string<CharOut>
    basic_convert(CharIn const *s)
    {
        return basic_convert<CharOut>(s,details::basic_strend(s));
    }



    ///
    /// Convert NULL terminated UTF source string to NULL terminated \a output string of size at
    /// most output_size (including NULL)
    /// 
    /// In case of success output is returned, if the input sequence is illegal,
    /// or there is not enough room NULL is returned 
    ///
    inline char *narrow(char *output,size_t output_size,wchar_t const *source)
    {
        return basic_convert(output,output_size,source,details::basic_strend(source));
    }
    ///
    /// Convert UTF text in range [begin,end) to NULL terminated \a output string of size at
    /// most output_size (including NULL)
    /// 
    /// In case of success output is returned, if the input sequence is illegal,
    /// or there is not enough room NULL is returned 
    ///
    inline char *narrow(char *output,size_t output_size,wchar_t const *begin,wchar_t const *end)
    {
        return basic_convert(output,output_size,begin,end);
    }
    ///
    /// Convert NULL terminated UTF source string to NULL terminated \a output string of size at
    /// most output_size (including NULL)
    /// 
    /// In case of success output is returned, if the input sequence is illegal,
    /// or there is not enough room NULL is returned 
    ///
    inline wchar_t *widen(wchar_t *output,size_t output_size,char const *source)
    {
        return basic_convert(output,output_size,source,details::basic_strend(source));
    }
    ///
    /// Convert UTF text in range [begin,end) to NULL terminated \a output string of size at
    /// most output_size (including NULL)
    /// 
    /// In case of success output is returned, if the input sequence is illegal,
    /// or there is not enough room NULL is returned 
    ///
    inline wchar_t *widen(wchar_t *output,size_t output_size,char const *begin,char const *end)
    {
        return basic_convert(output,output_size,begin,end);
    }


    ///
    /// Convert between Wide - UTF-16/32 string and UTF-8 string.
    ///
    /// boost::locale::conv::conversion_error is thrown in a case of a error
    ///
    inline std::string narrow(wchar_t const *s)
    {
        return basic_convert<char>(s);
    }
    ///
    /// Convert between UTF-8 and UTF-16 string, implemented only on Windows platform
    ///
    /// boost::locale::conv::conversion_error is thrown in a case of a error
    ///
    inline std::wstring widen(char const *s)
    {
        return basic_convert<wchar_t>(s);
    }
    ///
    /// Convert between Wide - UTF-16/32 string and UTF-8 string
    ///
    /// boost::locale::conv::conversion_error is thrown in a case of a error
    ///
    inline std::string narrow(std::wstring const &s) 
    {
        return basic_convert<char>(s);
    }
    ///
    /// Convert between UTF-8 and UTF-16 string, implemented only on Windows platform
    ///
    /// boost::locale::conv::conversion_error is thrown in a case of a error
    ///
    inline std::wstring widen(std::string const &s) 
    {
        return basic_convert<wchar_t>(s);
    }

} // nowide


#endif
///
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
