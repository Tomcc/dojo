#ifndef FASTVECTOR_H
#define FASTVECTOR_H

#define FV_INLINE inline

#include "dojo_config.h"

namespace Dojo
{
		template <typename T>
		class Array
		{
		public:

			///Costruttore
			/**
			E' opzionale specificare la grandezza delle pagine di memoria.
			*/
			Array(size_t firstPageSize = 0, const size_t newPageSize = 0)
			{
				pageSize = (newPageSize == 0) ? 64 : newPageSize;
				firstPageSize = ( firstPageSize == 0) ? pageSize : firstPageSize;

				elements = 0;
				arraySize = firstPageSize;

				//allocate the first memory page
				vectorArray = (T*)malloc( sizeof(T) * arraySize );
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
			FV_INLINE void addElement(const T& element)
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
			\remark E' sensibilmente piu' lento di addElement(). Se index e' fuori del vettore
			l'elemento sara' aggiunto alla fine.
			*/
			FV_INLINE void addElement(const T& element, const size_t& index)
			{
				//se l'index e' fuori dei limiti aggiungilo alla fine
				if(index > size())
					addElement(element);
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
				vectorArray = malloc( sizeof(T) * arraySize );
				//e poi copiala
				memcpy( vectorArray, fv._getArrayPointer(), sizeof(T) * elements);
			}
			
			///Rimuove un elemento dal vettore all'indirizzo dato
			/**
			Non chiama delete in caso di pointers.
			*/
			FV_INLINE void removeElement(uint index)
			{
				DOJO_ASSERT( size() > index );
				
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
			///Rimuove un elemento dal vettore.
			/**
			Chiama semplicemente getElementIndex seguito da removeElement(unsigned int).
			Non chiama delte in caso di pointers.
			*/
			FV_INLINE bool removeElement( T& e)
			{
				int i = getElementIndex(e);
				if(i != -1)
				{
					removeElement(i);
					return true;
				}
				else return false;
			}

			///Rimuove e restituisce l'elemento in coda del vettore
			FV_INLINE T popElement()
			{
				DOJO_ASSERT( size() );
				
				T* elem = vectorArray+elements-1;
				elements--;
				return *elem;
			}

			///Rimuove e restituisce un elemento qualsiasi del vettore
			FV_INLINE T popElement( unsigned int index )
			{
				T elem = at( index );
				removeElement( index );

				return elem;
			}

			///Rimuove e restituisce un elemento qualsiasi del vettore
			FV_INLINE T popElement( T& elem )
			{
				int i = getElementIndex(elem);

				return popElement( i );
			}

			///Rimuove qualsiasi elemento dal vettore
			/**
			Non chiama delete.
			\param newPageSize la nuova grandezza della page per il vettore. Lasciare a 0 per non 
			modificare.
			*/
			FV_INLINE void clear(const size_t newPageSize = 0)
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
			FV_INLINE T& operator[] (const size_t index) const
			{
				DOJO_ASSERT( index < size() );
				
				return vectorArray[ index ];
			}
			///Metodo identico a [] utile se si ha un pointer al vettore
			FV_INLINE T& at( const size_t index) const
			{
				DOJO_ASSERT( index < size() );
				
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
				for(size_t i = 0; i < size(); ++i)
				{
					if(vectorArray[i] == element)
						return i;
				}
				return -1;
			}

			///Numero di elementi
			FV_INLINE size_t size() const	{	return elements;	}
			///Numero di bytes
			FV_INLINE size_t byteSize()		const {	return elements * sizeof(T) + sizeof(vectorArray);	}
			///Numero massimo di elementi prima di un nuovo realloc
			FV_INLINE size_t getArraySize()	const {	return arraySize;	}
			///Ottieni il numero massimo di elementi accettabili prima di riallocare
			FV_INLINE size_t getPageSize()	const {	return pageSize;	}


			///Posizione del vettore in memoria -uso avanzato-
			FV_INLINE T* _getArrayPointer()	const {	return vectorArray;	}

		protected:

			size_t elements, arraySize, pageSize;

			//puntatore C-style alla memoria che contiene gli elementi.
			T* vectorArray;

			FV_INLINE void _allocatePage()
			{
				//add a page to the array
				arraySize += pageSize;
				//reallocate the memory
				vectorArray = (T*)realloc( vectorArray, sizeof(T) * arraySize );
			}

		};
}

#endif
