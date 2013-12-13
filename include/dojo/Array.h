#ifndef FASTVECTOR_H
#define FASTVECTOR_H

#define FV_INLINE inline

#include "dojo_common_header.h"

namespace Dojo
{
	template <typename T>
	class Array 
	{
	public:

		typedef Array< T > ArrayImpl;

		class iterator;

		///A range is the natural iterable view of an Array, to use in foreachs
		class Range
		{
		public:

			///creates a new range over base starting at start and ending at end (not included), or the natural end
			Range( const ArrayImpl& base, int start, int end = -1 ) :
			mStart( start ),
			mEnd( end ),
			mBase( base )
			{
				if( mEnd < 0 )
					mEnd = mBase.size();

				DEBUG_ASSERT( mStart >= 0, "Range start was negative" );
				DEBUG_ASSERT( mEnd >= 0, "Range end was negative" );
				DEBUG_ASSERT( mStart <= mEnd, "Flipped range start and end (end < start)" );
			}
			
			iterator begin() const;

			///returns a C++11-foreach-compliant vector past the back element
			iterator end() const;

			const ArrayImpl& mBase;
			int mStart, mEnd;
		};

		class iterator
		{
		public:
			iterator( Range a ) :
			pos( a.mStart ),
			range( a )
			{

			}

			// these three methods form the basis of an iterator for use with
			// a range-based for loop
			bool operator!= (const iterator& other) const
			{
				return pos != other.pos;
			}

			const T& operator* () const
			{
				return range.mBase[ pos ];
			}

			const iterator& operator++ ()
			{
				//don't allow to increase after the end
				DEBUG_ASSERT( pos < range.mBase.size(), "Iterator was increased past the end of the range" );

				++pos;
				// although not strictly necessary for a range-based for loop
				// following the normal convention of returning a value from
				// operator++ is a good idea.
				return *this;
			}

		private:
			int pos;
			Range range;
		};

		
		///Constructor
		/**
		 \param firstPageSize the size of the first page, in element count (not bytes)
		 \param newPageSize the size of all the newly created pages, in element count
		 */
		Array(int firstPageSize = 64, int newPageSize = 64, int validElements = 0)
		{
			DEBUG_ASSERT( validElements <= firstPageSize, "The first page must contain at least 'validElements' elements" );
			
			pageSize = newPageSize;			
			elements = validElements;
			arraySize = firstPageSize;
			
			//allocate the first memory page
			vectorArray = (T*)malloc( sizeof(T) * arraySize );
		}
		
		///assigment constructor - the memory is assigned to this vector
		/**
		 WARNING - buffer has to be created with malloc()
		 */
		Array( T* buffer, int size ) :
		vectorArray( buffer ),
		arraySize( size )
		{
			DEBUG_ASSERT( size <= 0, "Array constructor: size is negative" );
			
			elements = arraySize / sizeof( T );
			
			pageSize = 64;
		}
		
		///copy constructor
		Array( const Array<T>& fv ) :
		elements( fv.elements ),
		arraySize( fv.arraySize ),
		pageSize( fv.pageSize )
		{
			//allocae copia la memoria necessaria
			vectorArray = (T*)malloc( sizeof(T) * arraySize );
			memcpy( vectorArray, fv.vectorArray, sizeof(T) * elements);
		}
		
		~Array()
		{
			//libera la memoria puntata
			if(vectorArray)	free(vectorArray);
		}
		
		///adds an element to the back of the vector
		FV_INLINE void add(const T& element)
		{
			//se nel vettore non entra un'altra pagina di memoria
			if(elements >= arraySize) _allocatePage();
			
			vectorArray[ elements ] = element;
			elements++;
		}
		///adds an element to a given vector index, moving the rest
		/**
		 \param element element to insert
		 \param index new element index
		 
		 All the elements with index > i will be shifted (copied) to make room for the new one.
		 \remark it is O(n) except in the case where index == size, where it is O(1)
		 */
		FV_INLINE void add(const T& element, int index)
		{
			//se l'index e' fuori dei limiti aggiungilo alla fine
			if(index > size())
				add(element);
			else
			{
				//se nel vettore non entra un'altro elemento
				if(arraySize == elements) _allocatePage();
				
				//spostiamo tutto quanto di 1 per fare spazio al nuovo elemento
				memmove( vectorArray + index+1, vectorArray +index, (size() - index)*sizeof(T));
				
				//ora infiliamo l'elemento
				vectorArray[ index ] = element;
				elements++;	
			}
		}
		
		///Copy operator
		FV_INLINE void operator= (const Array<T>& fv)
		{
			//libera la memoria che non serve piu'
			free( vectorArray );
			
			//copia i parametri del vettore Src
			elements = fv.size();
			arraySize = fv.getArraySize();
			pageSize = fv.getPageSize();
			
			//rialloca la memoria necessaria
			vectorArray = (T*)malloc( sizeof(T) * arraySize );
			//e poi copiala
			memcpy( vectorArray, fv._getArrayPointer(), sizeof(T) * elements);
		}
		
		///removes in an *unordered* way an element at the given index
		/**
		 WARNING - DEFAULT BEHAVIOUR IS "REMOVE UNORDERED"
		 the back element is used to replace this one, then the vector is shrink by 1
		 Use removeOrdered() if you need to keep relative ordering between elements
		 */
		FV_INLINE void remove(int index)
		{
			DEBUG_ASSERT( size() > index, "index is past the end of the Array" );
			DEBUG_ASSERT( index >= 0, "index is negative" );
			
			--elements;
			
			if( index != elements )
				vectorArray[ index ] = vectorArray[ elements ];					
		}
		
		///removes in an *unordered* way a given element, if found
		/**
		 \returns true if the remove is successful
		 */
		FV_INLINE bool remove( const T& e)
		{
			int i = getElementIndex(e);
			if(i != -1)
			{
				remove(i);
				return true;
			}
			else return false;
		}
		
		///removes an element at index shifting each other subsequent element by 1
		FV_INLINE void removeOrdered( int index )
		{
			DEBUG_ASSERT( size() > index, "removeOrdered: OOB index" );
			DEBUG_ASSERT( index >= 0, "removeOrdered: index is negative" );
			
			if(index < size()-1)
			{
				//we have already the index
				//remove the element from the vector moving the rest in the empty spot
				memmove( vectorArray + index, vectorArray +index+1, (size() - index-1)*sizeof(T));
				
				elements--;
			}
			//we don't need to move memory for the last element
			else if( index == size()-1 )
				elements--;
		}
		
		///removes an element shifting each other subsequent element by 1
		FV_INLINE void removeOrdered( const T& e )
		{
			int i = getElementIndex(e);
			if(i != -1)
				removeOrdered(i);
		}
		
		///removes the back (top if used like a stack) element of the vector
		FV_INLINE void pop()
		{
			DEBUG_ASSERT( size(), "pop: Vector is empty" );
			
			elements--;
		}
		
		///returns the top/back element of the vector
		FV_INLINE T& top()
		{
			DEBUG_ASSERT( size(), "top: Vector is empty" );
			
			return vectorArray[ elements-1 ];
		}
		
		///removes all the elements from the Array and optionally resizes the page size
		FV_INLINE void clear( int newPageSize = 0)
		{
			elements = 0;
			
			//cancel the array
			free(vectorArray);
			
			//recreate the array
			if(newPageSize != 0)	pageSize = newPageSize;
			
			arraySize = pageSize;
			vectorArray = (T*)malloc(sizeof(T) * arraySize);
		}
		
		FV_INLINE bool operator== (const Array<T>& f)
		{
			if(f.size() != size())
				return false;
			
			return ( memcmp(vectorArray, f._getArrayPointer(), sizeof(T) * elements) == 0);
		}
		
		FV_INLINE bool isEmpty() const	{	return (elements == 0);	}
		
		///returns true if this element is found in the Array - linear search
		FV_INLINE bool exists(T& e)
		{
			return (getElementIndex(e) != -1);
		}	
		
		FV_INLINE T& operator[] (int index) const
		{
			DEBUG_ASSERT( size() > index, "operator[]: OOB index" );
			DEBUG_ASSERT( index >= 0, "operator[]: index is negative" );
			
			return vectorArray[ index ];
		}

		///a wrapper for operator []
		FV_INLINE T& at( int index) const
		{
			return vectorArray[ index ];
		}
		
		///returns the index of the given element or -1 if it isn't found
		FV_INLINE int getElementIndex(const T& element)
		{
			//find the element index
			for(int i = 0; i < size(); ++i)
			{
				if(vectorArray[i] == element)
					return i;
			}
			return -1;
		}
		
		FV_INLINE int size() const	{	return elements;	}
		///total size in bytes, non comprehensive of Array structures size
		FV_INLINE int byteSize()		const {	return elements * sizeof(T) + sizeof(vectorArray);	}
		
		///capacity of the vector
		FV_INLINE int getArraySize()	const {	return arraySize;	}

		///the size of a single page
		FV_INLINE int getPageSize()	const {	return pageSize;	}
		
		///returns a C++11-foreach-compliant vector to the front element
		FV_INLINE iterator begin() const
		{
			return iterator( Range( *this, 0 ) );
		}

		///returns a C++11-foreach-compliant vector past the back element
		FV_INLINE iterator end() const
		{
			return iterator( Range( *this, size() ) );
		}

		///returns an iterable range starting at start and ending at end (not included) or the natural end
		FV_INLINE Range range( int start, int end = -1 ) const
		{
			return Range( *this, start, end );
		}
		
		FV_INLINE T* _getArrayPointer()	const {	return vectorArray;	}
		
	protected:
		
		int elements, arraySize, pageSize;
		
		//puntatore C-style alla memoria che contiene gli elementi.
		T* vectorArray;
		
		FV_INLINE void _allocatePage()
		{
			//add a page to the array
			arraySize += pageSize;
			//reallocate the memory
			vectorArray = (T*)realloc( vectorArray, sizeof(T) * arraySize );

			DEBUG_ASSERT( vectorArray, "Array could not be extended" );
		}
		
	};

	template <typename T>
	typename Array<T>::iterator Array<T>::Range::begin() const
	{
		return iterator( *this );
	}

	template <typename T>
	typename Array<T>::iterator Array<T>::Range::end() const
	{
		return iterator( Range( mBase, mEnd ) );
	}
}

#endif
