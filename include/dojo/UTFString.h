// utf8string.h
// Copyright (c) 2013, Dominque A Douglas
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
//    in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//
// squaredprogramming.blogspot.com
//

// Modifications by Tommaso Checchi (2015)
// 1. reworked all functions taking and returning a size_t index to use a const_iterator to avoid linear searches.
// 2. exposed the internal std::string for easier interop.
// 3. added ends_with, starts_with and utf::to_string utility methods.


#pragma once

#ifndef UTF8STRINGHEADER
#define UTF8STRINGHEADER

#define SAFE_SIGNED_STRINGS

#include <memory.h>
#include <string.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <limits>

namespace utf
{
	// To make things easier I'll define some types that I can use that can be forced to the same size on all platforms.
	// You'll have to define WCHAR32BITS on platforms where wchar_t is 4 bytes.

	// #define WCHAR32BITS

#ifndef WCHAR32BITS

	// because signed or unsigned is not mandated for char or wchar_t in the standard,
	// always use the char and wchar_t types or we may get compiler errors when using
	// some standaard function

	typedef char _char8bit; // for ASCII and UTF8
	typedef unsigned char _uchar8bit; // for ASCII and UTF8
	typedef wchar_t _char16bit; // for USC2
	typedef std::uint32_t _char32bit; // for UTF32
#else
	typedef char __char8bit; // for ASCII and UTF8
	typedef unsigned char __uchar8bit; // for ASCII and UTF8
	typedef std::uint16_t __char16bit; // for USC2
	typedef wchar_t __char32bit; // for UTF32
#endif

	typedef _char32bit character;
	typedef _uchar8bit utf8_encoding[4];

	/// \brief Generates a UTF8 encoding
	/// This function generates a UTF-8 encoding from a 32 bit UCS-4 character.
	/// This is being provided as a static method so it can be used with normal std::string objects
	/// default_order is true when the uint8_t order matches the system
	inline void GetUTF8Encoding(_char32bit in_char, utf8_encoding &out_encoding, size_t &out_size, bool default_order = true)
	{
		// check the order uint8_t order and reorder if neccessary
		if (default_order == false)
		{
			in_char = ((in_char & 0x000000ff) << 24) + ((in_char & 0x0000ff00) << 8) + ((in_char & 0x00ff0000) >> 8) + ((in_char & 0xff000000) >> 24);
		}

		if (in_char < 0x80)
		{
			// 1 uint8_t encoding
			out_encoding[0] = (char)in_char;
			out_size = 1;
		}
		else if (in_char < 0x800)
		{
			// 2 uint8_t encoding
			out_encoding[0] = 0xC0 + (_uchar8bit)((in_char & 0x7C0) >> 6);
			out_encoding[1] = 0x80 + (_uchar8bit)(in_char & 0x3F);
			out_size = 2;
		}
		else if (in_char < 0x10000)
		{
			// 3 uint8_t encoding
			out_encoding[0] = 0xE0 + (_uchar8bit)((in_char & 0xF000) >> 12);
			out_encoding[1] = 0x80 + (_uchar8bit)((in_char & 0xFC0) >> 6);
			out_encoding[2] = 0x80 + (_uchar8bit)(in_char & 0x3F);
			out_size = 3;
		}
		else
		{
			// 4 uint8_t encoding
			out_encoding[0] = 0xF8 + (_uchar8bit)((in_char & 0x1C0000) >> 18);
			out_encoding[1] = 0x80 + (_uchar8bit)((in_char & 0x3F000) >> 12);
			out_encoding[2] = 0x80 + (_uchar8bit)((in_char & 0xFC0) >> 6);
			out_encoding[3] = 0x80 + (_uchar8bit)(in_char & 0x3F);
			out_size = 4;
		}
	}

	/// \brief Generates a UTF8 encoding
	/// This function generates a UTF-8 encoding from a 16 bit UCS-2 character.
	/// This is being provided as a static method so it can be used with normal std::string objects
	/// default_order is true when the uint8_t order matches the system
	inline void GetUTF8Encoding(_char16bit in_char, utf8_encoding &out_encoding, size_t &out_size, bool default_order = true)
	{
		// check the order uint8_t order and reorder if neccessary
		if (default_order == false)
		{
			in_char = ((in_char & 0x00ff) << 8) + ((in_char & 0xff00) >> 8);
		}

		// to reduce redundant code and possible bugs from typingg errors, use 32bit version
		GetUTF8Encoding((_char32bit)in_char, out_encoding, out_size, true);
	}

	/// \brief Converts a UTF8 encoded character into a 32 bit single code
	/// The input should represent a single unicode character and does not need to be null terminated.
	inline _char32bit UTF8CharToUnicode(const _char8bit *utf8data_s)
	{
        auto utf8data = (const _uchar8bit*)utf8data_s;
        
		if (utf8data[0] < 0x80)
		{
			return (_char32bit)utf8data[0];
		}
		else if (utf8data[0] < 0xE0)
		{
			// 2 bytes
			return ((utf8data[0] & 0x1F) << 6) + (utf8data[1] & 0x3F);
		}
		else if (utf8data[0] < 0xF0)
		{
			// 3 bytes
			return ((utf8data[0] & 0xF) << 12) + ((utf8data[1] & 0x3F) << 6) + (utf8data[2] & 0x3F);
		}
		else
		{
			// 4 bytes
			return ((utf8data[0] & 0x7) << 18) + ((utf8data[1] & 0x3F) << 12) + ((utf8data[2] & 0x3F) << 6) + (utf8data[3] & 0x3F);
		}
	}

	// increments a pointer to a UTF-8 encoded string to the next character
	// undefined behavior if pointer doesn't point to valid UTF-8 data
	inline void IncToNextCharacter(const _char8bit *&s_utf8data)
	{
		auto utf8data = (const _uchar8bit*)s_utf8data;
		// increments the iterator by one
		// result in undefined behavior (crashes) if already at the end 
		if (*utf8data < 0x80) ++s_utf8data;
		else if (*utf8data < 0xE0)	s_utf8data += 2;		// 2 bytes
		else if (*utf8data < 0xF0) s_utf8data += 3;		// 3 bytes
		else s_utf8data += 4;		// 4 bytes
	}

	// decrements a pointer to a UTF-8 encoded string to the previous character
	// undefined behavior if pointer doesn't point to valid UTF-8 data or is
	// already at the beginning of the string
	inline void DecToNextCharacter(const _char8bit *&utf8data)
	{
		// decrements the iterator by one
		// result in undefined behavior (crashes) if already at the beginning
		do
		{
			// first go back one
			--utf8data;
			// keep looking until the tops bits are not 10
		} while ((*(const _uchar8bit*)utf8data & 0xc0) == 0x80);
	}


	// increments a utf-8 string pointer to a position
	// sets the pointer to the null-terminator if the position is off the string
	// Behavior is undefined is string doesn't point to a properly formated UTF-8 string.
	inline void IncrementToPosition(const _char8bit *&utf8data, size_t pos)
	{
		for (size_t cur_index = 0; (*utf8data != 0) && (cur_index < pos); )
		{
			IncToNextCharacter(utf8data);

			++cur_index;
		}
	}

	// Get's the character's position from the buffer position
	inline const _char8bit* GetCharPtrFromBufferPosition(const _char8bit *string, size_t buffer_pos)
	{
		//TODO this could not be O(n)?
		const auto *end_addr = &string[buffer_pos];
		while (string < end_addr)
		{
			IncToNextCharacter(string);
		}
		return string;
	}

	// reads a non-encoded unicode string to get the minium buffer size needed to encoded it in UTF-8
	template <class T>
	inline size_t GetMinimumBufferSize(const T *string)
	{
		size_t min_size = 0;
		while (*string != 0)
		{
			if (*string < 0x80) min_size += 1;
			else if (string < 2048) min_size += 2;
			else if (string < 65536) min_size += 3;
			else min_size += 4;

			++string;
		}
	}

	// use a template method because the 16bit and 32 bit implementations are identical
	// except for the type
	template <typename char_type, typename Alloc>
	inline void MakeUTF8StringImpl(const char_type* instring, std::basic_string<_uchar8bit, Alloc> &out, bool appendToOut)
	{
		// first empty the string
		if (!appendToOut) out.clear();

		bool default_order = true; // the string uses the same uint8_t order as the system
		int start = 0; // index of the first real character in the string

					   // check for uint8_t order mark
		if (instring[0] == 0xfffe)
		{
			default_order = false;
			++start;
		}
		else if (instring[0] == 0xfeff)
		{
			// jump past the uint8_t order mark
			++start;
		}

		// loop until null terminator is reached
		for (int i = start; instring[i] != 0; ++i)
		{
			utf8_encoding cur_encoding;
			size_t encoding_size;

			// convert to UTF-8
			GetUTF8Encoding(instring[i], cur_encoding, encoding_size, default_order);

			// add to the std::string
			for (size_t j = 0; j < encoding_size; ++j)
			{
				out += cur_encoding[j];
			}
		}
	}

	// make this method private to restrict which types can be called
	// use a template method because the 16bit and 32 bit implementations are identical
	// except for the type
	// out should point to a buffer large enough to hold the data
	template <typename char_type>
	inline void MakeUTF8StringImpl(const char_type* instring, _uchar8bit *out)
	{
		bool default_order = true; // the string uses the same uint8_t order as the system
		int start = 0; // index of the first real character in the string

					   // check for uint8_t order mark
		if (instring[0] == 0xfffe)
		{
			default_order = false;
			++start;
		}
		else if (instring[0] == 0xfeff)
		{
			// jump past the uint8_t order mark
			++start;
		}

		// loop until null terminator is reached
		for (int i = start; instring[i] != 0; ++i)
		{
			utf8_encoding cur_encoding;
			int encoding_size;

			// convert to UTF-8
			GetUTF8Encoding(instring[i], cur_encoding, encoding_size, default_order);

			// add to the std::string
			for (int j = 0; j < encoding_size; ++j)
			{
				*out = cur_encoding[j];
				++out;
			}
		}
	}

	/// \brief Converts a null-terminated string to UTF-8
	/// This function generates a UTF-8 encoding from a 16 bit null terminated string and places
	/// it inside a std::string.
	/// This is being provided so it can be used with normal std::string objects
	template <typename Alloc>
	inline void MakeUTF8String(const _char16bit* instring_UCS2, std::basic_string<_uchar8bit, std::char_traits<unsigned char>, Alloc> &out, bool appendToOut = false)
	{
		// call the template method
		MakeUTF8StringImpl(instring_UCS2, out, appendToOut);
	}

	/// \brief Converts a null-terminated string to UTF-8
	/// This function generates a UTF-8 encoding from a 16 bit null terminated string and places
	/// it inside a std::string.
	/// This is being provided as a static method so it can be used with normal std::string objects
	template <typename Alloc>
	inline void MakeUTF8String(const _char32bit* instring_UCS4, std::basic_string<_uchar8bit, std::char_traits<unsigned char>, Alloc> &out, bool appendToOut = false)
	{
		// call the template method
		MakeUTF8StringImpl(instring_UCS4, out, appendToOut);
	}

	inline size_t GetNumCharactersInUTF8String(const _char8bit *utf8data)
	{
		size_t count = 0;
		while (*utf8data != 0)
		{
			++count;
			IncToNextCharacter(utf8data);
		}
		return count;
	}

	template <class Alloc = std::allocator<_uchar8bit>>
	class _utf8string
	{
	public:
		typedef _utf8string<Alloc> _utf8stringImpl;

		// some types that we need to define to make this work like an stl object
		// internally this is an std string, but outwardly, it returns __char32bit
		typedef _char32bit			value_type;
		typedef _char32bit			*pointer;
		typedef const _char32bit	*const_pointer;
		typedef _char32bit			&reference;
		typedef const _char32bit	&const_reference;
		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;

		// declare our iterator
		// declare as template class so we don't have to write everything twice for the const_iterator
		template <class TBaseIterator>
		class value_reverse_iterator : public std::iterator<std::bidirectional_iterator_tag, value_type>
		{
		public:
			TBaseIterator forward_iterator;

		public:
			// copy constructor
			value_reverse_iterator(const value_reverse_iterator &other)
				:forward_iterator(other.forward_iterator)
			{
			}

			// create from forward iterator
			value_reverse_iterator(const TBaseIterator &iterator)
				:forward_iterator(iterator)
			{

			}

			value_type operator*() const
			{
				TBaseIterator temp = forward_iterator;
				return *(--temp);
			}

			// does not check to see if it goes past the end
			// iterating past the end is undefined
			value_reverse_iterator &operator++()
			{
				--forward_iterator;

				return (*this);
			}

			// does not check to see if it goes past the end
			// iterating past the end is undefined
			value_reverse_iterator operator++(int)
			{
				value_reverse_iterator copy((*this));

				// increment
				--forward_iterator;

				return copy;
			}

			// does not check to see if it goes past the end
			// iterating past begin is undefined
			value_reverse_iterator &operator--()
			{
				++forward_iterator;
				return (*this);
			}

			// does not check to see if it goes past the end
			// iterating past begin is undefined
			value_reverse_iterator operator--(int)
			{
				value_reverse_iterator copy((*this));

				++forward_iterator;

				return copy;
			}

			bool operator == (const value_reverse_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return forward_iterator == other.forward_iterator;
			}

			bool operator != (const value_reverse_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return forward_iterator != other.forward_iterator;
			}

			bool operator < (const value_reverse_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return forward_iterator > other.forward_iterator;
			}

			bool operator > (const value_reverse_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return forward_iterator < other.forward_iterator;
			}

			bool operator <= (const value_reverse_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return forward_iterator >= other.forward_iterator;
			}

			bool operator >= (const value_reverse_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return forward_iterator <= other.forward_iterator;
			}

		};

		template <class Ty>
		class utf8string_iterator : public std::iterator<std::bidirectional_iterator_tag, Ty>
		{
		private:
			const _char8bit *utf8string_buf;

			void inc()
			{
				// increments the iterator by one
				// result in undefined behavior (crashes) if already at the end 
				IncToNextCharacter(utf8string_buf);
			}

			void dec()
			{
				// decrements the iterator by one
				// result in undefined behavior (crashes) if already at the beginning
				DecToNextCharacter(utf8string_buf);
			}

		public:
			utf8string_iterator() : utf8string_buf(nullptr) {}
			// b should be a null terminated string in UTF-8
			// if this is the end start_pos should be the index of the null terminator
			// start_pos should be the valid start of a character
			utf8string_iterator(const _char8bit *b, size_type start_pos)
				:utf8string_buf(b)
			{
				IncrementToPosition(utf8string_buf, start_pos);
			}

			// copy constructor
			utf8string_iterator(const utf8string_iterator &other)
				:utf8string_buf(other.utf8string_buf)
			{
			}

			// b should already point to the correct position in the string
			explicit utf8string_iterator(const _char8bit *b)
				:utf8string_buf(b)
			{
			}

			value_type operator*() const
			{
				// returns the character currently being pointed to
				return UTF8CharToUnicode(utf8string_buf);
			}

			// does not check to see if it goes past the end
			// iterating past the end is undefined
			utf8string_iterator &operator++()
			{
				inc();

				return (*this);
			}

			// does not check to see if it goes past the end
			// iterating past the end is undefined
			utf8string_iterator operator++(int)
			{
				utf8string_iterator copy((*this));

				// increment
				inc();

				return copy;
			}

			// does not check to see if it goes past the end
			// iterating past begin is undefined
			utf8string_iterator &operator--()
			{
				dec();
				return (*this);
			}

			// does not check to see if it goes past the end
			// iterating past begin is undefined
			utf8string_iterator operator--(int)
			{
				utf8string_iterator copy((*this));

				dec();

				return copy;
			}

			utf8string_iterator operator + (size_t n) const {
				auto copy = (*this);
				for (size_t i = 0; i < n; ++i, copy.inc());
				return copy;
			}

			bool operator == (const utf8string_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return utf8string_buf == other.utf8string_buf;
			}

			bool operator != (const utf8string_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return utf8string_buf != other.utf8string_buf;
			}

			bool operator < (const utf8string_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return utf8string_buf < other.utf8string_buf;
			}

			bool operator > (const utf8string_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return utf8string_buf > other.utf8string_buf;
			}

			bool operator <= (const utf8string_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return utf8string_buf <= other.utf8string_buf;
			}

			bool operator >= (const utf8string_iterator &other) const
			{
				// just compare pointers
				// the programmer is responsible for making both iterators are for the same set of data
				return utf8string_buf >= other.utf8string_buf;
			}

			auto get_ptr() const {
				return utf8string_buf;
			}
		};

		// make our iterator types here
		typedef utf8string_iterator<value_type>			iterator;
		typedef utf8string_iterator<const value_type>	const_iterator;
		typedef value_reverse_iterator<iterator>		reverse_iterator;
		typedef value_reverse_iterator<const_iterator>	const_reverse_iterator;

	private:
		std::string raw_bytes;

	public:
		// default constructor
		_utf8string()
		{
		}

		// build from a c string
		// undefined (ie crashes) if str is NULL
		_utf8string(const _char8bit *str)
			: raw_bytes(str)
		{
		}

		_utf8string(const _char8bit *str, size_t n)
			: raw_bytes(str, n)
		{
		}

		// build from a c string
		// undefined (ie crashes) if str is NULL
		_utf8string(const _uchar8bit *str)
			: _utf8string((const _char8bit*)str)
		{
		}

		// construct from an unsigned char
		_utf8string(size_t n, _char32bit c)
		{
			utf8_encoding encoding;
			size_t encoding_size;

			GetUTF8Encoding(c, encoding, encoding_size);

			raw_bytes.reserve(encoding_size * n);

			for (size_t j = 0; j < n; j++)
			{
				for (size_t i = 0; i < encoding_size; ++i)
				{
					raw_bytes += encoding[i];
				}
			}
		}

		// construct from a normal char
		_utf8string(_uchar8bit c)
			: raw_bytes(1, c)
		{
		}

		// construct from a normal char
		_utf8string(_char8bit c)
			: raw_bytes(1, (_uchar8bit)c)
		{
		}

		// construct from a normal char
		_utf8string(_char16bit c)
		{
			utf8_encoding encoding;
			size_t encoding_size;

			GetUTF8Encoding(c, encoding, encoding_size);

			for (size_t i = 0; i < encoding_size; ++i)
			{
				raw_bytes += encoding[i];
			}
		}

		// construct from a normal char
		_utf8string(_char32bit c)
		{
			utf8_encoding encoding;
			size_t encoding_size;

			GetUTF8Encoding(c, encoding, encoding_size);

			for (size_t i = 0; i < encoding_size; ++i)
			{
				raw_bytes += encoding[i];
			}
		}

		// copy constructor
		_utf8string(const _utf8stringImpl &str)
			: raw_bytes(str.raw_bytes)
		{
		}

		//move constructor
		_utf8string(_utf8stringImpl&& str)
			: raw_bytes(std::move(str.raw_bytes))
		{
		}

		/// \brief copy constructor from basic std::string
		_utf8string(const std::string &instring)
			: raw_bytes(instring)
		{
		}

		_utf8string(std::string&& instring) 
			: raw_bytes(std::move(instring)) {
			
		}

		// destructor
		~_utf8string()
		{
		}



		// assignment operator
		// we can define assignment operators for all possible types such as char, const char *, etc,
		// but this is not neccessary. Because those constructors were provided, the compiler will be
		// able to build a _utf8stringImpl for those types and then call this overloaded operator.
		// if performance becomes an issue, the additional variations to this operator can be created
		_utf8stringImpl& operator= (const _utf8stringImpl &rvalue)
		{
			raw_bytes = rvalue.raw_bytes;

			return (*this);
		}

		// move assignment operator
		// should move the data to this object and remove it from the old one
		_utf8stringImpl& operator= (_utf8stringImpl &&rvalue)
		{
			raw_bytes = std::move(rvalue.raw_bytes);

			return (*this);
		}

		// capacity ------------------------------------------------------------
		// request a new buffer size
		// this will resize the buffer, but it will not shrink the buffer is new_size < reserve_size
		// not useful unless the actual size of the string is known
		void reserve(size_type new_size)
		{
			raw_bytes.reserve(new_size);
		}

		// returns the size of the string in characters
		// synonomous with length()
		size_type size() const
		{
			return GetNumCharactersInUTF8String(raw_bytes.c_str());
		}

		// returns the size of the string in characters
		// synonomous with size()
		size_type length() const
		{
			return size();
		}

		// resizes the length of the string, padding the string with c
		// if the size is greater than the current size
		void resize(size_type n, value_type c)
		{
			if (n < size())
			{
				auto end = cbegin() + n;

				// set the null terminator
				raw_bytes.resize(get_byte_position(end));
			}
			else if (n > size())
			{
				size_type diff = n - size();

				// convert c to UTF-8 to get the size
				utf8_encoding c_utf8;
				size_type c_real_size;

				GetUTF8Encoding(c, c_utf8, c_real_size);

				// total size to add
				size_type additional_space = c_real_size * diff;

				size_type cur_size = raw_bytes.length();

				// make sure the buffer is big enough
				// we must do this now because if an exception is thrown because of
				// memory, the value of the string shouldn't change
				raw_bytes.resize(additional_space + cur_size);

				for (size_type i = cur_size; i < (additional_space + cur_size); i += c_real_size)
				{
					for (size_type j = 0; j < c_real_size; ++j) raw_bytes[i + j] = c_utf8[j];
				}
			}
		}

		// resizes the length of the string, padding the string with null
		// if the size is greater than the current size
		void resize(size_type n)
		{
			resize(n, value_type());
		}

		// returns the size of the allocated buffer
		size_type capacity() const
		{
			return raw_bytes.capacity();
		}

		// clears the string, setting the size to 0
		void clear()
		{
			raw_bytes.clear();
		}

		// checks to see if the string is empty
		bool empty() const
		{
			return raw_bytes.empty();
		}

		bool not_empty() const {
			return !empty();
		}

		bool starts_with(const _utf8stringImpl& string) const {
			if (string.bytes().size() > raw_bytes.size() || string.empty()) {
				return false;
			}

			return strncmp(
				string.raw_bytes.data(),
				raw_bytes.data(), 
				string.raw_bytes.size()) == 0;
		}

		bool ends_with(const _utf8stringImpl& string) const {
			if (string.bytes().size() > raw_bytes.size() || string.empty()) {
				return false;
			}

			return strncmp(
				string.raw_bytes.data(), 
				raw_bytes.data() + (raw_bytes.length() - string.raw_bytes.length()),
				string.raw_bytes.size()) == 0;
		}

		void shrink_to_fit()
		{
			raw_bytes.shrink_to_fit();
		}

		// iterators ----------------------------------------------------------------------

		// return iterator to the beginning of the list
		iterator begin()
		{
			return iterator(raw_bytes.c_str());
		}

		// return const_iterator to the beginning of the list
		const_iterator cbegin() const
		{
			// actually all iterators are const in this implementation
			return const_iterator(raw_bytes.c_str());
		}

		// return const_iterator to the beginning of the list
		const_iterator begin() const
		{
			return cbegin();
		}

		// return iterator to just after the last element of the list
		iterator end()
		{
			// this works because end will point to just after the end of the string which is the null terminator
			// and the basic_string that holds the data's length is the length of the buffer since sizeof(unsigned char) == 1
			return iterator(raw_bytes.c_str() + raw_bytes.length());
		}

		// return iterator to just after the last element of the list
		const_iterator cend() const
		{
			const auto *end_ptr = raw_bytes.c_str() + raw_bytes.length();

			return const_iterator(end_ptr);
		}

		const_iterator end() const
		{
			return cend();
		}

		// return iterator to the beginning of the list
		reverse_iterator rbegin()
		{
			return reverse_iterator(end());
		}

		// return const_iterator to the beginning of the list
		const_reverse_iterator crbegin() const
		{
			return const_reverse_iterator(end());
		}

		// return const_iterator to the beginning of the list
		const_reverse_iterator rbegin() const
		{
			return crbegin();
		}

		// return iterator to just after the last element of the list
		reverse_iterator rend()
		{
			return reverse_iterator(begin());
		}

		// return iterator to just after the last element of the list
		const_reverse_iterator crend() const
		{
			return const_reverse_iterator(begin());
		}

		const_reverse_iterator rend() const
		{
			return crend();
		}

		// string operatios ---------------------------------------------------------------

		Alloc get_allocator() const
		{
			return raw_bytes.get_allocator();
		}

		const_iterator find(const _utf8stringImpl& str, const_iterator pos = {}) const
		{
			//TODO rewrite using std::string find, avoid linear search in GetCharPtrFromBufferPosition
			size_type real_pos = get_byte_position( pos.get_ptr() ? pos : begin());
			size_type found_pos = raw_bytes.find(str.raw_bytes, real_pos);

			// return the character position
			return found_pos == std::string::npos ? 
				end() :
				const_iterator(GetCharPtrFromBufferPosition(raw_bytes.c_str(), found_pos));
		}

		const_iterator rfind(const _utf8stringImpl& str, const_iterator pos = {}) const
		{
			size_type real_pos = get_byte_position(pos.get_ptr() ? pos : begin());
			size_type found_pos = raw_bytes.rfind(str.raw_bytes, real_pos);

			// return the character position
			return found_pos == std::string::npos ?
				end() :
				const_iterator(GetCharPtrFromBufferPosition(raw_bytes.c_str(), found_pos));
		}

		const_iterator find_first_of(const _utf8stringImpl& str, const_iterator pos = {}) const
		{
			size_type real_pos = get_byte_position(pos.get_ptr() ? pos : begin());
			size_type found_pos = raw_bytes.find_first_of(str.raw_bytes, real_pos);

			// return the character position
			return found_pos == std::string::npos ?
				end() :
				const_iterator(GetCharPtrFromBufferPosition(raw_bytes.c_str(), found_pos));
		}

		const_iterator find_last_of(const _utf8stringImpl& str, const_iterator pos = {}) const
		{
			if (pos == const_iterator{}) {
				pos = str.end();
			}
			size_type found_pos = raw_bytes.find_last_of(str.raw_bytes, get_byte_position(pos));

			// return the character position
			return found_pos == std::string::npos ?
				end() :
				const_iterator(GetCharPtrFromBufferPosition(raw_bytes.c_str(), found_pos));
		}

		const_iterator find_first_not_of(const _utf8stringImpl& str, const_iterator pos = {}) const
		{
			size_type real_pos = get_byte_position(pos.get_ptr() ? pos : begin());
			size_type found_pos = raw_bytes.find(str.raw_bytes, real_pos);

			// return the character position
			return found_pos == std::string::npos ?
				end() :
				const_iterator(GetCharPtrFromBufferPosition(raw_bytes.c_str(), found_pos));
		}

		const_iterator find_last_not_of(const _utf8stringImpl& str, const_iterator pos = {}) const;
// 		{
// 			size_type real_pos;//
// 			if (pos == npos) real_pos = pos;
// 			else real_pos = GetBufferPosition(raw_bytes.c_str(), pos);
// 
// 			size_type found_pos = raw_bytes.find_last_not_of(str.raw_bytes, real_pos);
// 
// 			// return the character position
// 			return GetCharPtrFromBufferPosition(raw_bytes.c_str(), found_pos);
// 		}

		const std::string& bytes() const {
			return raw_bytes;
		}
		
		std::string& bytes() {
			return raw_bytes;
		}

		// no-throw guarantee on non-empty strings. Undefined behavior on empty strings
		value_type front() const
		{
			return *begin();
		}

		// string operations -----------------------------------------------------------------------------
		_utf8stringImpl substr(const_iterator pos, const_iterator end) const
		{
			return{
				pos.get_ptr(),
				get_byte_position(end) - get_byte_position(pos)
			};
		}

		// modifiers -------------------------------------------------------------------------------------

		// appends a string to the end of this one
		// we can define this operator for all possible types such as char, const char *, etc,
		// but this is not neccessary. Because those constructors were provided, the compiler will be
		// able to build a _utf8stringImpl for those types and then call this overloaded operator.
		// if performance becomes an issue, the additional variations to this operator can be created
		_utf8stringImpl& operator+= (const _utf8stringImpl& str)
		{
			// just use operator from std::basic_string
			raw_bytes += str.raw_bytes;

			return (*this);
		}

		_utf8stringImpl &assign(const _char8bit *str)
		{
			raw_bytes.assign(str);

			return (*this);
		}

		// assigns a new value from a _utf8string
		_utf8stringImpl &assign(const _utf8stringImpl &str)
		{
			raw_bytes.assign(str.raw_bytes);

			return (*this);
		}

		// assigns a new value from a _utf8string (MOVE(
		_utf8stringImpl &assign(const _utf8stringImpl &&str)
		{
			raw_bytes = std::move(str.raw_bytes);

			return (*this);
		}

		// assigns a new value from a std::string
		_utf8stringImpl &assign(const std::string &instring)
		{
			raw_bytes.assign(instring);

			return (*this);
		}

		template <class InputIterator>
		_utf8stringImpl &assign(InputIterator first, InputIterator last)
		{
			// create a temporary string first so an excpetion won't alter the current value
			_utf8stringImpl temp;
			for (auto it = first; it < last; it++)
			{
				temp += (value_type)*it;
			}
			assign(temp);
		}

		// appends a character to the string
		void push_back(value_type c)
		{
			// just use resize to do the work
			resize(length() + 1, c);
		}

		// deletes the last character
		void pop_back()
		{
			// just use resize to do the work
			resize(length() - 1);
		}

		size_t get_byte_position(const_iterator pos) const {
			return pos.get_ptr() - raw_bytes.data();
		}

		// inserts str right before character at position pos
		_utf8stringImpl& insert(const_iterator pos, const _utf8stringImpl & str)
		{			
			// just use the standard insert
			raw_bytes.insert(get_byte_position(pos), str.raw_bytes);

			return (*this);
		}

// 		_utf8stringImpl& insert(size_type pos, const _utf8stringImpl& str, size_type subpos, size_type sublen)
// 		{
// 			// create substring
// 			_utf8stringImpl temp = str.substr(subpos, sub_len);
// 
// 			return insert(pos, temp);
// 		}

		_utf8stringImpl& erase(const_iterator pos, size_type len = npos);
// 		{
// 			size_type real_pos = get_byte_position(pos);
// 
// 			if (len == npos) raw_bytes.erase(real_pos, len);
// 			else
// 			{
// 				size_type end_pos = pos + len;
// 				if (end_pos > size()) end_pos = size() - pos;
// 
// 				size_type real_end_pos = GetBufferPosition(raw_bytes.c_str(), end_pos);
// 
// 				raw_bytes.erase(real_pos, real_end_pos - real_pos);
// 			}
// 
// 			return (*this);
// 		}

		_utf8stringImpl& replace(const_iterator pos, size_type len, const _utf8stringImpl& str);
//		{
// 			// make copy so exceptions won't change string
// 			_utf8stringImpl temp_copy((*this));
// 
// 			// erase
// 			temp_copy.erase(pos, len);
// 
// 			// insert
// 			temp_copy.insert(pos, str);
// 
// 			assign(temp_copy);

//			return (*this);
//		}

		_utf8stringImpl& replace(size_type pos, size_type len, const _utf8stringImpl& str, size_type subpos, size_type sublen);
// 		{
// 			// make copy so exceptions won't change string
// 			_utf8stringImpl temp_copy((*this));
// 			_utf8stringImpl sub_str = str.substr(subpos, sublen);
// 
// 			// erase
// 			temp_copy.erase(pos, len);
// 
// 			// insert
// 			temp_copy.insert(pos, sub_str);
// 
// 			assign(temp_copy);
// 
// 			return (*this);
// 		}

		_utf8stringImpl& replace(size_type pos, size_type len, size_type n, value_type c);
// 		{
// 			// make copy so exceptions won't change string
// 			_utf8stringImpl temp_copy((*this));
// 			_utf8stringImpl str(n, c);
// 
// 			// erase
// 			temp_copy.erase(pos, len);
// 
// 			// insert
// 			temp_copy.insert(pos, str);
// 
// 			assign(temp_copy);
// 
// 			return (*this);
// 		}

		// swaps the contents of the strings
		void swap(_utf8stringImpl& str)
		{
			std::swap(str, (*this));
		}

		void KillEndingWhiteSpace()
		{
			if (raw_bytes.size())
			{
				size_type new_length = raw_bytes.size();
				unsigned char c = raw_bytes[new_length - 1];
				while (((c == '\r') || (c == '\n') || (c == '\t') || (c == ' ')) && (new_length > 0))
				{
					--new_length;
					c = raw_bytes[new_length - 1];
				}
				raw_bytes.resize(new_length);
			}
		}

		// comparison operators ---------------------------------------------------------------------------
		bool operator == (const _utf8stringImpl &other) const
		{
			// just compare pointers
			// the programmer is responsible for making both iterators are for the same set of data
			return raw_bytes == other.raw_bytes;
		}

		bool operator != (const _utf8stringImpl &other) const
		{
			// just compare pointers
			// the programmer is responsible for making both iterators are for the same set of data
			return raw_bytes != other.raw_bytes;
		}

		bool operator < (const _utf8stringImpl &other) const
		{
			// just compare pointers
			// the programmer is responsible for making both iterators are for the same set of data
			return raw_bytes < other.raw_bytes;
		}

		bool operator > (const _utf8stringImpl &other) const
		{
			// just compare pointers
			// the programmer is responsible for making both iterators are for the same set of data
			return raw_bytes > other.raw_bytes;
		}

		bool operator <= (const _utf8stringImpl &other) const
		{
			// just compare pointers
			// the programmer is responsible for making both iterators are for the same set of data
			return raw_bytes <= other.raw_bytes;
		}

		bool operator >= (const _utf8stringImpl &other) const
		{
			// just compare pointers
			// the programmer is responsible for making both iterators are for the same set of data
			return raw_bytes >= other.raw_bytes;
		}

		static const size_type npos = std::numeric_limits<size_type>::max();
	};

	// overload stream insertion so we can write to streams
	template <class Alloc>
	std::ostream& operator<<(std::ostream& os, const _utf8string<Alloc>& string)
	{
		// use from basic string
		// must cast to char otherwise stream will think it's a number
		os << (const char *)string.c_str();

		return os;
	}

	// overload stream extraction so we can write to streams
	template <class Alloc>
	std::istream& operator>>(std::istream& is, _utf8string<Alloc>& string)
	{
		// for now just use std::string because istream as defined can not read unsigned char
		std::string in;
		is >> in;

		string = in;

		return is;
	}

	// we can define this operator for all possible types such as char, const char *, etc,
	// but this is not neccessary. Because those constructors were provided, the compiler will be
	// able to build a _utf8string<Alloc> for those types and then call this overloaded operator.
	// if performance becomes an issue, the additional variations to this operator can be created
	template <class Alloc>
	_utf8string<Alloc> operator + (const _utf8string<Alloc>& lhs, const _utf8string<Alloc>& rhs)
	{
		_utf8string<Alloc> out(lhs);
		out += rhs;
		return out;
	}

	typedef _utf8string<> string;
	template<class Alloc>
	_utf8string<Alloc> operator+ (_utf8string<Alloc>&&      lhs, const _utf8string<Alloc>& rhs) { 
		return lhs += rhs; //reuse lhs' memory
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (const _utf8string<Alloc>& lhs, _utf8string<Alloc>&&      rhs) { 
		auto copy = lhs;
		copy += rhs;
		return copy;
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (const _utf8string<Alloc>& lhs, const char*   rhs) { 
		auto copy = lhs;
		copy += rhs;
		return copy;
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (_utf8string<Alloc>&&      lhs, const char*   rhs) { 
		return lhs += rhs; //reuse lhs' memory
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (const char*   lhs, const _utf8string<Alloc>& rhs) { 
		_utf8string<Alloc> copy = lhs;
		copy += rhs;
		return copy;
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (const char*   lhs, _utf8string<Alloc>&&      rhs) { 
		return _utf8string<Alloc>(lhs) + rhs;
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (const _utf8string<Alloc>& lhs, char          rhs) { 
		auto copy = lhs;
		copy += rhs;
		return copy;
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (_utf8string<Alloc>&&      lhs, char          rhs) { 
		return std::move(lhs += rhs);
	}

	template<class Alloc>
	_utf8string<Alloc> operator+ (char          lhs, const _utf8string<Alloc>& rhs) { 
		_utf8string<Alloc> copy = lhs;
		copy += rhs;
		return copy;
	}
	template<class Alloc>
	_utf8string<Alloc> operator+ (char          lhs, _utf8string<Alloc>&&      rhs) { 
		string copy = lhs;
		copy += rhs;
		return copy;
	}

	template<class Alloc>
	_utf8string<Alloc> operator+ (_utf8string<Alloc>&&      lhs, _utf8string<Alloc>&&      rhs) {
		return lhs += rhs; //reuse lhs' memory
	}

	template<typename T>
	string to_string(T t) {
		return std::to_string(t);
	}
}

namespace std {
	///hash specialization for unordered_maps
	template <>
	struct hash<utf::string> {
		// hash functor for vector
		size_t operator()(const utf::string& _Keyval) const {
			return std::hash<std::string>()(_Keyval.bytes());
		}
	};
}



#endif 