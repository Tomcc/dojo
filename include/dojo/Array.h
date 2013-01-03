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

		class iterator
		{
		public:
			iterator( const Array<T>& a, int initialPos ) :
			base( a ),
			pos( initialPos )
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
				return base[ pos ];
			}

			const iterator& operator++ ()
			{
				//don't allow to increase after the end
				DEBUG_ASSERT( pos < base.size() );

				++pos;
				// although not strictly necessary for a range-based for loop
				// following the normal convention of returning a value from
				// operator++ is a good idea.
				return *this;
			}

		private:
			int pos;
			const Array<T>& base;
		};
		
		///Costruttore
		/**
		 E' opzionale specificare la grandezza delle pagine di memoria.
		 */
		Array(int firstPageSize = 0, int newPageSize = 0, int validElements = 0)
		{
			DEBUG_ASSERT( validElements*sizeof(T) <= (size_t)firstPageSize );
			
			pageSize = (newPageSize == 0) ? 64 : newPageSize;
			firstPageSize = ( firstPageSize == 0) ? pageSize : firstPageSize;
			
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
			DEBUG_ASSERT( elements );
			DEBUG_ASSERT( size );
			
			elements = arraySize / sizeof( T );
			
			pageSize = 64;
		}
		
		///Costruttore copia - permette di evitare l'allocazione dovuta al costruttore.
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
		
		///Aggiunge un elemento al termine del vettore
		FV_INLINE void add(const T& element)
		{
			//se nel vettore non entra un'altra pagina di memoria
			if(elements >= arraySize) _allocatePage();
			
			vectorArray[ elements ] = element;
			elements++;
		}
		///Inserisce un elemento in un punto del vettore
		/**
		 \param element Elemento da inserire;
		 \param index Indirizzo del nuovo elemento
		 
		 Tutti gli elementi successivi saranno spostati di 1 per fare spazio al nuovo.
		 \remark E' sensibilmente piu' lento di add(). Se index e' fuori del vettore
		 l'elemento sara' aggiunto alla fine.
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
		
		///Copia in questo vettore un altro vettore
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
		
		///removes an element at the given index
		/**
		 WARNING - DEFAULT BEHAVIOUR IS "REMOVE UNORDERED"
		 Use removeOrdered() if you need to keep relative ordering between elements
		 */
		FV_INLINE void remove(int index)
		{
			DEBUG_ASSERT( size() > index );
			DEBUG_ASSERT( index >= 0 );
			
			--elements;
			
			if( index != elements )
				vectorArray[ index ] = vectorArray[ elements ];					
		}
		
		///removes an element from the vector
		/**
		 returns true if the remove is successful
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
		
		FV_INLINE void removeOrdered( int index )
		{
			DEBUG_ASSERT( size() > index );
			DEBUG_ASSERT( index >= 0 );
			
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
		
		FV_INLINE void removeOrdered( const T& e )
		{
			int i = getElementIndex(e);
			if(i != -1)
				removeOrdered(i);
		}
		
		///Rimuove l'elemento in coda del vettore
		FV_INLINE void pop()
		{
			DEBUG_ASSERT( size() );
			
			elements--;
		}
		
		FV_INLINE T& top()
		{
			DEBUG_ASSERT( size() );
			
			return vectorArray[ elements-1 ];
		}
		
		///Rimuove qualsiasi elemento dal vettore
		/**
		 Non chiama delete.
		 \param newPageSize la nuova grandezza della page per il vettore. Lasciare a 0 per non 
		 modificare.
		 */
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
		
		///Dice se i vettori sono identici
		FV_INLINE bool operator== (const Array<T>& f)
		{
			if(f.size() != size())
				return false;
			
			return ( memcmp(vectorArray, f._getArrayPointer(), sizeof(T) * elements) == 0);
		}
		
		///Dice se il vettore ha 0 elementi
		FV_INLINE bool isEmpty()	{	return (elements == 0);	}
		
		///Dice se questo elemento esiste nel vettore
		FV_INLINE bool exists(T& e)
		{
			return (getElementIndex(e) != -1);
		}	
		
		///restituisce l'elemento all'indice richiesto
		FV_INLINE T& operator[] (int index) const
		{
			DEBUG_ASSERT( index < size() );
			DEBUG_ASSERT( index >= 0 );
			
			return vectorArray[ index ];
		}
		///Metodo identico a [] utile se si ha un pointer al vettore
		FV_INLINE T& at( int index) const
		{
			DEBUG_ASSERT( index < size() );
			DEBUG_ASSERT( index >= 0 );
			
			return vectorArray[ index ];
		}
		
		///Restituisce la posizione nel vettore dell'elemento dato
		/**
		 Non chiama delete.
		 \returns 0...n se l'elemento e' stato trovato; -1 se l'elemento non esiste.
		 */
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
		
		///Numero di elementi
		FV_INLINE int size() const	{	return elements;	}
		///Numero di bytes
		FV_INLINE int byteSize()		const {	return elements * sizeof(T) + sizeof(vectorArray);	}
		///Numero massimo di elementi prima di un nuovo realloc
		FV_INLINE int getArraySize()	const {	return arraySize;	}
		///Ottieni il numero massimo di elementi accettabili prima di riallocare
		FV_INLINE int getPageSize()	const {	return pageSize;	}
		
		FV_INLINE iterator begin()
		{
			return iterator( *this, 0 );
		}

		FV_INLINE iterator end()
		{
			return iterator( *this, size() );
		}
		
		///Posizione del vettore in memoria -uso avanzato-
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

			DEBUG_ASSERT( vectorArray );
		}
		
	};
}

#endif
