#include "Table.h"
#include "Platform.h"
#include "FileStream.h"

using namespace Dojo;

const Table Table::Empty;
const Table::Data Table::Data::Empty = Data(0, 0);

Table Table::loadFromFile(const std::string& path) {
	DEBUG_ASSERT( path.size(), "Tried to load a Table from an empty path string" );

	auto file = Platform::singleton().getFile(path);

	Table dest;
	if (file->open()) {
		//read the contents directly in a string
		std::string buf;
		buf.resize(file->getSize());

		file->read((byte*)buf.c_str(), buf.size());

		StringReader reader(buf);
		dest.deserialize(reader);
	}

	return dest;
}

bool Table::onLoad() {
	//loads itself from file
	DEBUG_ASSERT( !isLoaded(), "The Table is already loaded" );

	if (!isReloadable())
		return false;

	*this = Platform::singleton().load(filePath);

	return (loaded = !isEmpty());
}

void Table::serialize(std::string& buf, const std::string& indent) const {
	using namespace std;

	Data* data;
	Vector* v;

	//serialize to the Table Format	
	EntryMap::const_iterator itr = map.begin();

	for (; itr != map.end(); ++itr) {
		auto& e = *itr->second;

		if (indent.size())
			buf += indent;

		//write name and equal only if not anonymous and if not managed later
		if (itr->first[0] != '_')
			buf += itr->first + " = ";

		switch (e.type) {
		case FieldType::Float:
			buf += String::fromFloat(*((float*)e.getRawValue()));
			break;
		case FieldType::String:
			buf += '\"' + *((std::string*)e.getRawValue()) + '\"';
			break;
		case FieldType::Vector:
			v = (Vector*)e.getRawValue();
			buf += '(';
			buf += String::fromFloat(v->x);
			buf += ' ';
			buf += String::fromFloat(v->y);
			buf += ' ';
			buf += String::fromFloat(v->z);
			buf += ')';

			break;
		case FieldType::RawData:
			data = (Data*)e.getRawValue();
			buf += '#' + String::fromInt(data->size) + ' ';

			buf.append((const char*)data->ptr, data->size);

			break;
		case FieldType::ChildTable:
			buf += std::string("{\n");
			((Table*)e.getRawValue())->serialize(buf, indent + '\t');

			buf += indent + '}';

			break;
		default: 
			FAIL("Unsupported type");
		}

		buf += '\n';
	}
}

enum class ParseState {
	Table,
	Name,
	NameEnd,
	Equal,
	Comment,
	End,
	Error
};

//TODO shouldn't we reuse the type here?
enum class ParseTarget {
	Undefined,
	Float,
	String,
	RawData,
	Vector,
	Table,
	Int64,
	ImplicitTrue
};

bool isNameStarter(uint32_t c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isNumber(uint32_t c) {
	return (c >= '0' && c <= '9') || c == '-'; //- is part of a number!!!
}

bool isName(uint32_t c) {
	return isNameStarter(c) || isNumber(c) || c == '_';
}

bool isValidFloat(uint32_t c) {
	return isNumber(c) || c == '.';
}

bool isWhiteSpace(uint32_t c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void Table::deserialize(StringReader& buf) {
	ParseState state = ParseState::Table;
	ParseTarget target = ParseTarget::Undefined;

	std::string curName, str;
	float number;
	Vector vec;
	Data data;
	Color col;

	//clear old
	clear();

	//feed one char at a time and do things
	uint32_t c, c2;

	while( state != ParseState::End && state != ParseState::Error) {
		c = buf.get();

		switch (state) {
		case ParseState::Table: //wait for either a name, or an anon value	
			if (c == '}' || c == 0)
				state = ParseState::End;
			else if (isNameStarter(c))
				state = ParseState::Name;

			else if (c == '"')
				target = ParseTarget::String;
			else if (c == '(')
				target = ParseTarget::Vector;
			else if (c == '#')
				target = ParseTarget::RawData;
			else if (c == '{')
				target = ParseTarget::Table;
			else if (isNumber(c))
				target = ParseTarget::Float;

			if (state == ParseState::Name) {
				curName.clear();
				String::append(curName, c);
			}

			break;
		case ParseState::Name:
			if (isName(c))
				String::append(curName, c);
			else if (c == '=')
				state = ParseState::Equal;
			else
				state = ParseState::NameEnd;

			break;

		case ParseState::NameEnd: //wait for an equal; drop whitespace and fall back if other is found
			if (c == '=')
				state = ParseState::Equal;
			else if (!isWhiteSpace(c)) //it is something else - store this as an implicit bool and reset the parser
				target = ParseTarget::ImplicitTrue;

			break;

		case ParseState::Equal: //wait for value start
			if (c == '"')
				target = ParseTarget::String;
			else if (c == '(')
				target = ParseTarget::Vector;
			else if (c == '#')
				target = ParseTarget::RawData;
			else if (c == '{')
				target = ParseTarget::Table;
			else if (isNumber(c))
				target = ParseTarget::Float;

			break;
		default: 
			FAIL("Invalid State");
		}

		switch (target) {
		case ParseTarget::Undefined:
			break; //skip, allowed
		case ParseTarget::ImplicitTrue:
			buf.back();
			set(curName, (int)1);
			break;

		case ParseTarget::Float:

			//check if next char is x, that is, really we have an hex color!
			c2 = buf.get();

			if (c == '0' && c2 == 'x') {
				buf.back();
				buf.back();

				//create a color using the hex
				col.set(buf.readHex());

				set(curName, col);
			}
			else if (c == '-' && c2 == '-') //or, well, a comment! (LIKE A HACK)
			{
				//just skip until newline
				do {
					c = buf.get();
				}
				while (c != 0 && c != '\n');
			}
			else {
				buf.back();
				buf.back();

				number = buf.readFloat();

				set(curName, number);
			}

			break;
		case ParseTarget::String:

			str.clear();
			for (;;) {
				c = buf.get();
				if (c == '"')
					break;
				String::append(str, c);
			}

			set(curName, str);
			break;

		case ParseTarget::Vector:
			vec.x = buf.readFloat();
			vec.y = buf.readFloat();
			vec.z = buf.readFloat();

			set(curName, vec);
			break;

		case ParseTarget::RawData:
			data.size = (int)buf.readFloat();
			data.ptr = malloc(data.size);

			//skip space
			buf.get();

			buf.readBytes(data.ptr, data.size);

			set(curName, data); //always retain deserialized data
			break;

		case ParseTarget::Table: {

			createTable(curName).deserialize(buf);
		}
			break;

		default: 
			FAIL("Invalid case");
		}

		if (target != ParseTarget::Undefined) //read something
		{
			state = ParseState::Table;
			target = ParseTarget::Undefined;
			curName.clear();
		}
	}
}

Table::Table() :
	unnamedMembers(0) {

}

Table::Table(Table&& t) :
	unnamedMembers(t.unnamedMembers),
	map(std::move(t.map)) {
	t.unnamedMembers = 0;
}

Table::Table(const Table& t) :
	unnamedMembers(t.unnamedMembers) {
	//deep copy
	for (auto& pair : t.map)
		map[pair.first] = pair.second->clone();
}

Table::Table(ResourceGroup* creator, const std::string& path) :
	Resource(creator, path),
	unnamedMembers(0) {

}

Table& Table::operator=(Table&& t) {
	unnamedMembers = t.unnamedMembers;
	map = std::move(t.map);
	return *this;
}

Table::~Table() {
	clear();
}

void Table::onUnload(bool soft /*= false */) {
	if (!soft || isReloadable()) {
		clear();

		loaded = false;
	}
}

Table* Table::getParentTable(const std::string& key, std::string& realKey) const {
	size_t dotIdx = 0;
	for (; dotIdx < key.size() && key[dotIdx] != '.'; ++dotIdx);

	if (dotIdx == key.size()) {
		realKey = key;
		return (Table*)this;
	}

	std::string partialKey = key.substr(dotIdx + 1);
	std::string childName = key.substr(0, dotIdx);
	auto& child = getTable(childName);

	return child.getParentTable(partialKey, realKey);
}

Table& Table::createTable(const std::string& key /*= std::string::EMPTY */) {
	std::string name;

	if (key.size() == 0)
		name = autoname();
	else
		name = key;

	set(name, Table());

	return get(name)->getAs<Table>(); //TODO don't do another search
}

void Table::clear() {
	unnamedMembers = 0;

	map.clear();
}

void Table::inherit(Table* t) {
	DEBUG_ASSERT(t != nullptr, "Cannot inherit a null Table");

	//for each map member of the other map
	EntryMap::iterator itr = t->map.begin(),
		end = t->map.end(),
		existing;
	for (; itr != end; ++itr) {
		existing = map.find(itr->first); //look for a local element with the same name

		//element exists - do nothing except if it's a table
		if (existing != map.end()) {
			//if it's a table in both tables, inherit
			if (itr->second->type == FieldType::ChildTable && existing->second->type == FieldType::ChildTable)
				((Table*)existing->second->getRawValue())->inherit((Table*)itr->second->getRawValue());
		}
		else //just clone
			map[itr->first] = itr->second->clone();
	}
}

bool Table::exists(const std::string& key) const {
	DEBUG_ASSERT(key.size(), "exists: key is empty");

	return map.find(key) != map.end();
}

bool Table::existsAs(const std::string& key, FieldType t) const {
	EntryMap::const_iterator itr = map.find(key);

	if (itr != map.end())
		return itr->second->type == t;

	return false;
}

Table::Entry* Table::get(const std::string& key) const {
	std::string actualKey;
	const Table* container = getParentTable(key, actualKey);

	if (!container)
		return nullptr;

	auto elem = container->map.find(actualKey);
	return elem != container->map.end() ? elem->second.get() : nullptr;
}

float Table::getNumber(int idx) const {
	return getNumber(autoMemberName(idx));
}

std::string Table::autoMemberName(int idx) const {
	DEBUG_ASSERT(idx >= 0, "autoMemberName: idx is negative");
	DEBUG_ASSERT_INFO(idx < getArrayLength(), "autoMemberName: idx is OOB", "idx = " + String::fromInt(idx));

	return '_' + String::fromInt(idx);
}

void Table::remove(const std::string& key) {
	map.erase(key);
}

void Table::remove(int idx) {
	map.erase(autoMemberName(idx));
}

std::string Table::toString() const {
	std::string str;
	serialize(str);

	return str;
}

void Table::debugPrint() const {
#ifdef _DEBUG 
	DEBUG_MESSAGE(toString());
#endif
}

std::string Table::autoname() {
	return '_' + String::fromInt(unnamedMembers++);
}
