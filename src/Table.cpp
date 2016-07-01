#include "Table.h"
#include "Platform.h"
#include "FileStream.h"
#include "Base64.h"

using namespace Dojo;

const Table Table::Empty;
const Table::Data Table::Data::Empty{};

Table Table::loadFromFile(utf::string_view path) {
	DEBUG_ASSERT( path.not_empty(), "Tried to load a Table from an empty path string" );

	auto file = Platform::singleton().getFile(path);

	Table dest;

	if (file->open(Stream::Access::Read)) {
		//read the contents directly in a string
		std::string buf((size_t)file->getSize(), 0);

		file->readToFill(buf);

		StringReader reader(buf);
		dest.deserialize(reader);
	}

	return dest;
}

bool Table::onLoad() {
	//loads itself from file
	DEBUG_ASSERT(not isLoaded(), "The Table is already loaded" );

	if (not isReloadable()) {
		return false;
	}

	self = Platform::singleton().load(filePath);

	return (loaded = not isEmpty());
}

void Table::serialize(utf::string& buf, utf::string_view indent) const {
	using namespace std;

	Data* data;
	Vector* v;

	//serialize to the Table Format
	EntryMap::const_iterator itr = map.begin();

	for (; itr != map.end(); ++itr) {
		auto& e = *itr->second;

		if (indent.not_empty()) {
			buf += indent.to_str();
		}

		//write name and equal only if not anonymous and if not managed later
		if (itr->first.front() != '_') {
			buf += itr->first + " = ";
		}

		switch (e.type) {
		case FieldType::Float:
			buf += utf::to_string(*((float*)e.getRawValue()));
			break;

		case FieldType::String:
			buf += '\"' + *((utf::string*)e.getRawValue()) + '\"';
			break;

		case FieldType::Vector:
			v = (Vector*)e.getRawValue();
			buf += '(';
			buf += utf::to_string(v->x);
			buf += ' ';
			buf += utf::to_string(v->y);
			buf += ' ';
			buf += utf::to_string(v->z);
			buf += ')';

			break;

		case FieldType::RawData:
			data = (Data*)e.getRawValue();
			buf += "raw\"" + Base64::fromVec(data->buf) + '"';

			break;

		case FieldType::ChildTable:
			buf += utf::string("{\n");
			((Table*)e.getRawValue())->serialize(buf, indent + '\t');

			buf += indent + '}';

			break;
		case FieldType::Int64:
			buf += "i64\"" + Base64::fromObject(e.getAs<int64_t>()) + "\"";
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
	return (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z');
}

bool isNumber(uint32_t c) {
	return (c >= '0' and c <= '9') or c == '-'; //- is part of a number!!!
}

bool isName(uint32_t c) {
	return isNameStarter(c) or isNumber(c) or c == '_';
}

bool isValidFloat(uint32_t c) {
	return isNumber(c) or c == '.';
}

bool isWhiteSpace(uint32_t c) {
	return c == ' ' or c == '\t' or c == '\r' or c == '\n';
}

void Table::deserialize(StringReader& buf) {
	ParseState state = ParseState::Table;
	ParseTarget target = ParseTarget::Undefined;

	utf::string_view curName;
	float number;
	Vector vec;
	Data data;
	Color col;
	auto nameStart = buf.getCurrentIndex();
	//clear old
	clear();

	//feed one char at a time and do things
	uint32_t c, c2;

	while ( state != ParseState::End and state != ParseState::Error) {
		auto idx = buf.getCurrentIndex();
		c = buf.get();

		switch (state) {
		case ParseState::Table: //wait for either a name, or an anon value
			if (c == '}' or c == 0) {
				state = ParseState::End;
			}
			else if(buf.startsWith("i64\"")) {
				target = ParseTarget::Int64;
			}
			else if (buf.startsWith("raw\"")) {
				target = ParseTarget::RawData;
			}
			else if (c == '"') {
				target = ParseTarget::String;
			}
			else if (c == '(') {
				target = ParseTarget::Vector;
			}
			else if (c == '{') {
				target = ParseTarget::Table;
			}
			else if (isNumber(c)) {
				target = ParseTarget::Float;
			}
			else if (isNameStarter(c)) {
				state = ParseState::Name;
			}

			if (state == ParseState::Name) {
				nameStart = idx;
			}

			break;

		case ParseState::Name:
			if (c == '=') {
				state = ParseState::Equal;
			}
			else if (not isName(c)) {
				state = ParseState::NameEnd;
				curName = buf.getString().substr(nameStart, idx);
			}

			break;

		case ParseState::NameEnd: //wait for an equal; drop whitespace and fall back if other is found
			if (c == '=') {
				state = ParseState::Equal;
			}
			else if (not isWhiteSpace(c)) { //it is something else - store this as an implicit bool and reset the parser
				target = ParseTarget::ImplicitTrue;
			}

			break;

		case ParseState::Equal: //wait for value start
			if (c == '"') {
				target = ParseTarget::String;
			}
			else if (c == '(') {
				target = ParseTarget::Vector;
			}
			else if (c == '{') {
				target = ParseTarget::Table;
			}
			else if (isNumber(c)) {
				target = ParseTarget::Float;
			}
			else if (buf.startsWith("i64\"")) {
				target = ParseTarget::Int64;
			}
			else if (buf.startsWith("raw\"")) {
				target = ParseTarget::RawData;
			}

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

			if (c == '0' and c2 == 'x') {
				buf.back();
				buf.back();

				//create a color using the hex
				col = Color::fromARGB(buf.readHex());

				set(curName, col);
			}
			else if (c == '-' and c2 == '-') { //or, well, a comment! (LIKE A HACK)
				//just skip until newline
				do {
					c = buf.get();
				}
				while (c != 0 and c != '\n');
			}
			else {
				buf.back();
				buf.back();

				number = buf.readFloat();

				set(curName, number);
			}

			break;

		case ParseTarget::String:
			set(curName, buf.readString());
			break;

		case ParseTarget::Vector:
			vec.x = buf.readFloat();
			vec.y = buf.readFloat();
			vec.z = buf.readFloat();

			set(curName, vec);
			break;

		case ParseTarget::RawData:
			//skip prefix
			buf.get();
			buf.get();
			buf.get();
			buf.get();

			set(curName, Data{ Base64::decode(buf.readString()) });
			break;

		case ParseTarget::Int64: {
			//skip prefix
			buf.get();
			buf.get();
			buf.get();
			buf.get();

			auto bytes = Base64::decode(buf.readString());
			if (bytes.size() == 8) {
				set(curName, *(int64_t*)bytes.data());
			}
			else {
				DEBUG_MESSAGE("Key " + curName + " is not a valid int64");
			}
		}
		break;

		case ParseTarget::Table:
			createTable(curName).deserialize(buf);

			break;

		default:
			FAIL("Invalid case");
		}

		if (target != ParseTarget::Undefined) { //read something
			state = ParseState::Table;
			target = ParseTarget::Undefined;
			curName = {};
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
	for (auto&& pair : t.map) {
		map[pair.first] = pair.second->clone();
	}
}

Table::Table(optional_ref<ResourceGroup> creator, utf::string_view path) :
	Resource(creator, path.to_str()),
	unnamedMembers(0) {

}

Table& Table::operator=(Table&& t) {
	unnamedMembers = t.unnamedMembers;
	map = std::move(t.map);
	return self;
}

Table::~Table() {
	clear();
}

void Table::onUnload(bool soft /*= false */) {
	if (not soft or isReloadable()) {
		clear();

		loaded = false;
	}
}

Table* Table::getParentTable(utf::string_view key, utf::string& realKey) const {
	auto dotIdx = key.begin();
	for (; dotIdx != key.end() and *dotIdx != '.'; ++dotIdx);

	if (dotIdx == key.end()) {
		realKey = key.to_str();
		return (Table*)this;
	}

	auto partialKey = key.substr(dotIdx + 1, key.end());
	auto childName = key.substr(key.begin(), dotIdx);
	auto& child = getTable(childName);

	return child.getParentTable(partialKey, realKey);
}

Table& Table::createTable(utf::string_view key /*= utf::string::EMPTY */) {
	utf::string name;

	if (key.empty()) {
		name = autoname();
	}
	else {
		name = key.to_str();
	}

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
			if (itr->second->type == FieldType::ChildTable and existing->second->type == FieldType::ChildTable) {
				((Table*)existing->second->getRawValue())->inherit((Table*)itr->second->getRawValue());
			}
		}
		else { //just clone
			map[itr->first] = itr->second->clone();
		}
	}
}

bool Table::exists(utf::string_view key) const {
	DEBUG_ASSERT(key.not_empty(), "exists: key is empty");

	return map.find(key) != map.end();
}

bool Table::existsAs(utf::string_view key, FieldType t) const {
	auto itr = map.find(key);

	if (itr != map.end()) {
		return itr->second->type == t;
	}

	return false;
}

Table::Entry* Table::get(utf::string_view key) const {
	utf::string actualKey;
	const Table* container = getParentTable(key, actualKey);

	if (not container) {
		return nullptr;
	}

	auto elem = container->map.find(actualKey);
	return elem != container->map.end() ? elem->second.get() : nullptr;
}

float Table::getNumber(int idx) const {
	return getNumber(autoMemberName(idx));
}

utf::string Table::autoMemberName(int idx) const {
	DEBUG_ASSERT(idx >= 0, "autoMemberName: idx is negative");
	DEBUG_ASSERT_INFO(idx < getArrayLength(), "autoMemberName: idx is OOB", "idx = " + utf::to_string(idx));

	return '_' + utf::to_string(idx);
}

void Table::remove(utf::string_view key) {
	auto elem = map.find(key);
	if (elem != map.end()) {
		map.erase(elem);
	}
}

void Table::remove(int idx) {
	map.erase(autoMemberName(idx));
}

utf::string Table::toString() const {
	utf::string str;
	serialize(str);

	return str;
}

void Table::debugPrint() const {
#ifdef _DEBUG
	DEBUG_MESSAGE(toString());
#endif
}

utf::string Table::autoname() {
	return '_' + utf::to_string(unnamedMembers++);
}
