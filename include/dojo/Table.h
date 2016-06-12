#pragma once

#include "dojo_common_header.h"

#include "Vector.h"
#include "StringReader.h"
#include "dojostring.h"
#include "Resource.h"
#include "Color.h"

namespace Dojo {
	class Entry;

	///Table is the internal representation of the Dojo Script data definition format
	/**
	a Table is a multi-typed Dictionary of Strings and Values, where a value can be one of float, Vector, utf::string, Color, Raw Data and Table itself.

	set( key, value ) sets a value to the given key;
	get*( key, defaultValue ) gets the value of the given key, or returns defaultValue if the key was not found

	Table does support numeric indexing via "auto values", or values which are not bound to a (explicit) name.
	auto values can be queried using
	get*( index, defaultValue )
	*/
	class Table : public Resource {
	public:

		enum class FieldType {
			Undefined,
			Float,
			String,
			RawData,
			Vector,
			ChildTable,
			Int64
		};

		template<typename T>
		struct field_type_for {
			operator FieldType() const {
				FAIL("Invalid type requested");
			}
		};
		class Data {
		public:
			static const Data Empty;

			std::vector<uint8_t> buf;

			template<class T>
			static Data fromRaw(const T* ptr, size_t count) {
				Data data;
				data.buf.resize(count * sizeof(T));
				memcpy(data.buf.data(), ptr, data.buf.size());
				return data;
			}

			template<class T>
			static Data fromVec(const std::vector<T>& vec) {
				return fromRaw(vec.data(), vec.size());
			}

			template<class T>
			std::vector<T> toVec() const {
				DEBUG_ASSERT(buf.size() % sizeof(T) == 0, "Invalid cast");
				std::vector<T> res(buf.size() / sizeof(T));
				memcpy(res.data(), buf.data(), buf.size());
				return res;
			}

		};

		class Entry {
		public:
			const Table::FieldType type;

			explicit Entry(Table::FieldType fieldType) :
				type(fieldType) {
			}

			virtual ~Entry() {

			}

			///returns a raw untyped pointer to the underlying data
			virtual void* getRawValue() = 0;

			template<typename T>
			T& getAs() {
				//DEBUG_ASSERT(type == Table::field_type_for<T>(), "type mismatch while reading from a Table Entry");
				return *(T*)getRawValue();
			}

			virtual Unique<Entry> clone() = 0;
		};

		template <class T>
		class TypedEntry : public Entry {
		public:

			T value;

			TypedEntry(FieldType fieldType, T v) :
				Entry(fieldType),
				value(std::move(v)) {

			}

			virtual ~TypedEntry() {

			}

			///returns the raw value pointer
			virtual void* getRawValue() override {
				return &value;
			}

			virtual Unique<Entry> clone() override {
				return make_unique<TypedEntry<T>>(type, value);
			}
		};

		typedef std::unordered_map<utf::string, Unique<Entry>> EntryMap;

		static const Table Empty;

		static utf::string index(int i) {
			return '_' + utf::to_string(i);
		}

		///loads the file at path
		static Table loadFromFile(const utf::string& path);

		///Creates a new table
		Table();

		Table(Table&& t);

		Table& operator=(Table&& t);

		Table(const Table& t);

		Table& operator=(const Table&) = delete;

		///Constructs a new "Table Resource", or a table bound to a file path in a ResourceGroup
		Table(optional_ref<ResourceGroup> creator, const utf::string& path);

		~Table();

		virtual bool onLoad() override;

		virtual void onUnload(bool soft = false) override;

		///returns the table which contains the given "dot formatted" key
		/** it returns "this" for a normal non-hierarchical key
		returns "A" for a key such as "A.key"
		returns "B" for a key such as "A.B.key" */
		Table* getParentTable(const utf::string& key, utf::string& realKey) const;

		template <class T>
		void setImpl(const utf::string& key, FieldType type, T value) {
			map[key.empty() ? autoname() : key] = make_unique<TypedEntry<T>>(type, std::move(value));
		}

		template <class T>
		void set(const utf::string& key, FieldType type, T value) {
			utf::string actualKey;
			Table* t = getParentTable(key, actualKey);
			DEBUG_ASSERT( t != nullptr, "Cannot add a key to a non-existing table" );

			//actually set the key on the right table
			t->setImpl(actualKey, type, std::move(value));
		}

		template<typename T>
		void set(const utf::string& key, T value) {
			set(key, Table::field_type_for<T>(), std::move(value));
		}

		///creates a new nested table named key
		/**
		nested Tables always have name == key */
		Table& createTable(const utf::string& key = String::Empty);

		///empties the map and deletes every value
		void clear();

		///Inherits all the member in table t
		/**
		After the call, this Table contains a copy of all the field defined in t but not here.
		Nested Tables are an exception as if they're defined in both, the local nested table will
		recursively inherit the other nested table.
		*/
		void inherit(Table* t);

		///total number of entries
		int size() const {
			return (int)map.size();
		}

		///returns the total number of unnamed members
		int getArrayLength() const {
			return unnamedMembers;
		}

		bool isEmpty() const {
			return map.empty();
		}

		operator bool() const {
			return not map.empty();
		}

		///returns true if this Table contains key
		bool exists(const utf::string& key) const;

		///returns true if this Table contains key and the value is of type t
		bool existsAs(const utf::string& key, FieldType t) const;

		///generic get
		Entry* get(const utf::string& key) const;

		template<typename T>
		const T& get(const utf::string& key, const T& defaultValue) const {
			auto e = get(key);
			if(e and e->type == Table::field_type_for<T>()) {
				return e->getAs<T>();
			}
			return defaultValue;
		}

		//explicit implementations with defaulted default value
		float getNumber(const utf::string& key, float defaultValue = 0) const  {
			return get(key, defaultValue);
		}

		const utf::string& getString(const utf::string& key, const utf::string& defaultValue = String::Empty) const {
			return get(key, defaultValue);
		}

		const Vector& getVector(const utf::string& key, const Vector& defaultValue = Vector::Zero) const  {
			return get(key, defaultValue);
		}

		Quaternion getQuaternion(const utf::string& key, const Quaternion& defaultValue = {}) const {
			return Quaternion(getVector(key, glm::eulerAngles(defaultValue)));
		}

		const Color getColor(const utf::string& key, const Color& defaultValue = Color::Black) const  {
			auto v = get(key, Vector(defaultValue.r, defaultValue.g, defaultValue.b));
			return{ v.x, v.y, v.z, defaultValue.a };
		}

		const int64_t& getInt64(const utf::string& key, const int64_t& defaultValue = 0) const  {
			return get(key, defaultValue);
		}

		const Table& getTable(const utf::string& key, const Table& defaultValue = Empty) const  {
			return get(key, defaultValue);
		}

		const Data& getData(const utf::string& key, const Data& defaultValue = Data::Empty) const {
			return get(key, defaultValue);
		}

		//special implementations
		int getInt(const utf::string& key, int defaultValue = 0) const {
			return (int)get(key, (float)defaultValue);
		}

		bool getBool(const utf::string& key, bool defaultValue = false) const {
			return get(key, (float)defaultValue) > 0.f;
		}

		utf::string autoMemberName(int idx) const;

		float getNumber(int idx) const;

		int getInt(int idx) const {
			return (int)getNumber(idx);
		}

		bool getBool(int idx) const {
			return getNumber(idx) > 0.f;
		}

		const utf::string& getString(int idx) const {
			return getString(autoMemberName(idx));
		}

		const Vector& getVector(int idx) const {
			return getVector(autoMemberName(idx));
		}

		const Color getColor(int idx, float alpha = 1.f) const {
			return Color(getVector(idx), alpha);
		}

		const Table& getTable(int idx) const {
			return getTable(autoMemberName(idx));
		}

		const Data& getData(int idx) const {
			return getData(autoMemberName(idx));
		}

		///returns a new unique anoymous id for a new "array member"
		utf::string autoname();

		template <typename T>
		void push(const T& t) {
			set(autoname(), t);
		}

		const EntryMap::const_iterator begin() const {
			return map.begin();
		}

		const EntryMap::const_iterator end() const {
			return map.end();
		}

		///removes a member named key
		void remove(const utf::string& key);

		///removes the unnamed member index idx
		void remove(int idx);

		///write the table in string form over buf
		void serialize(utf::string& buf, const utf::string& indent = String::Empty) const;

		void deserialize(StringReader& buf);

		///diagnostic method that serializes the table in a string
		utf::string toString() const;

		void debugPrint() const;

		///returns an iterator to the beginning of the internal dictionary
		EntryMap::iterator begin() {
			return map.begin();
		}

		///returns an iterator to the end of the internal dictionary
		EntryMap::iterator end() {
			return map.end();
		}

	protected:

		EntryMap map;

		int unnamedMembers;
	};
    
    
    //specialization magic
    
    template<>
    void Table::set<Color>(const utf::string& key, Color value) {
        set(key, Vector(value.r, value.g, value.b));
    }
    template<>
    void Table::set<int>(const utf::string& key, int value) {
        set(key, (float)value);
    }
    template<>
    void Table::set<bool>(const utf::string& key, bool value) {
        set(key, value ? 1.f : 0.f);
    }
    
    template<>
    void Table::set<Quaternion>(const utf::string& key, Quaternion value) {
        set(key, Vector(glm::eulerAngles(value)));
    }
    
    
    //specializations to get reflected types out of types
    
    template<> struct Table::field_type_for < float > {
        operator FieldType() const {
            return FieldType::Float;
        }
    };
    template<> struct Table::field_type_for < bool > {
        operator FieldType() const {
            return FieldType::Float;
        }
    };
    template<> struct Table::field_type_for < int > {
        operator FieldType() const {
            return FieldType::Float;
        }
    };
    template<> struct Table::field_type_for < unsigned int > {
        operator FieldType() const {
            return FieldType::Float;
        }
    };
    template<> struct Table::field_type_for < utf::string > {
        operator FieldType() const {
            return FieldType::String;
        }
    };
    template<> struct Table::field_type_for < Table::Data > {
        operator FieldType() const {
            return FieldType::RawData;
        }
    };
    template<> struct Table::field_type_for < Vector > {
        operator FieldType() const {
            return FieldType::Vector;
        }
    };
    template<> struct Table::field_type_for < Table > {
        operator FieldType() const {
            return FieldType::ChildTable;
        }
    };
    
    template<> struct Table::field_type_for < int64_t > {
        operator FieldType() const {
            return FieldType::Int64;
        }
    };


}
