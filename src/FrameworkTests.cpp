#include "stdafx.h"

#include "FrameworkTests.h"

#include "TestUnit.h"
#include "Table.h"
#include "StringReader.h"
#include "Platform.h"
#include "dojomath.h"

using namespace Dojo;
using namespace std;


void testTable( TestUnit& u, Table* a, Dojo::byte data[] )
{	
	u.test( a->getAutoMembers() == 10 ) << "auto member number";
	
	for( uint i = 0; i < a->getAutoMembers(); ++i )
		u.test( a->getInt( a->autoMember(i) ) == i ) << "auto member " << i;
	
	u.test( a->getString( "string1" ) == String("hello world") ) << "string 1";
	u.test( a->getString( "string2" ) == String::EMPTY ) << "string 2";
	
	Table::Data readData = a->getData( "data" );
	
	u.test( readData.size == 120 ) << "data size";
	u.test( memcmp( readData.ptr, data, readData.size ) == 0 ) << "data content";
}

void FrameworkTests::staticTest()
{	
	{
		TestUnit u( "Strings", out, false );
		
		String a( "12345" );
		String b( "abcde" );
		String c( "12345abcde" );
		String d( "12345abcde12345abcde12345abcde" );
		
		u.test( a == a ) << "a != a";
		u.test( b == b ) << "b != b";
		u.test( a != b ) << "a == b";
		
		u.test( (a + b) == c ) << "string sum";
		u.test( c + c + c == d ) << "string sum";
		
		u.test( '_' + String( 12 ) == String("_12") ) << "append to char";
		u.test( '_' + String( 0 ) == String("_0") ) << "append to char";
				
		u.test( String( "12345" ) == a ) << "bad conversion from const char*";
		u.test( String( "abcde" ) == b ) << "bad conversion from const char*";
		
		u.test( String(0) == String("0") ) << "wrong number conversion";
		u.test( String(12345) == a ) << "wrong number conversion";
		
		u.test( String( 123.45f ) == String("123.45") ) << "float conversion 1";
		u.test( String( 0.0f) == String("0") ) << "float conversion 2";
						
		d.clear();
		u.test( d.size() == 0 ) << "clear not working";
	}
	
	{
		TestUnit u( "Vectors", out, false );
		
		Vector a( 1,0,0 );
		Vector b( 1.5, 0,0 );
		
		u.test( Vector::ZERO.length() == 0 ) << "lenght 0";
		u.test( Vector::UNIT_Y.length() == 1 ) << "lenght 1";
		
		u.test( a == a ) << "equality";
		u.test( a*1.5 == b ) << "scale";
		u.test( a + a == a*2 ) << "sum";
		u.test( b - b + a - a == Vector::ZERO ) << "sums";
		u.test( Vector::UNIT_X * Vector::UNIT_X == 1 ) << "dot product 1";
		u.test( Vector::UNIT_Y * Vector::UNIT_X == 0 ) << "dot product 0";
		
		a.rotate( Vector( 0,0, 90 ) );
		u.test( a == Vector::UNIT_Y ) << "rotation 1";
		
		a.rotate( Vector( 90, 0, 0 ) );
		u.test( a == Vector::UNIT_Z ) << "rotation 2";				
	}
	
	{
		TestUnit u( "Math", out, false );
				
		for( int i = 2; i < 32; ++i )
			u.test( Math::nextPowerOfTwo( (uint)Math::rangeRandom( pow(2.f,i-1 )+1.f, pow( 2.f, i ) ) ) == pow( 2.f,i ) )
				   << "next power of two " << i;
		
		Vector BIG( 99999,99999,99999 );
		for( int i = 0; i < 100; ++i )
		{
			Vector max1 = Math::randomVector( BIG*-1, BIG );
			Vector min1 = max1 - Math::randomVector( Vector::ZERO, BIG );
			
			Vector off = Math::randomVector( (max1 - min1)* -0.5, (max1 - min1)* 0.5 );
			Vector max2 = max1 + off; 
			Vector min2 = min1 + off;
			
			u.test( Math::AABBsCollide(max1, min1, max2, min2) ) << "collide " << i;
		}
		
		
		for( int i = 0; i < 100; ++i )
		{
			Vector max1 = Math::randomVector( BIG*-1, BIG );
			Vector min1 = max1 - Math::randomVector( Vector::ZERO, BIG );
			
			Vector off = Math::randomVector( max1 - min1, BIG ) * Math::sign( Math::rangeRandom( -1, 1 ) );
			Vector max2 = max1 + off; 
			Vector min2 = min1 + off;
			
			u.testFalse( Math::AABBsCollide(max1, min1, max2, min2) ) << "not collide " << i;
		}
	}
	
	{
		TestUnit u( "Tables", out, false );
		
		Table d;
		
		{
			Table a;
			
			a.set( "a", 1 );
						
			u.test( a.exists( "a" ) ) << "exists";
			u.testFalse( a.exists( "A" ) ) << "!exists";
			
			u.test( a.getInt( "a" ) == 1 )	<< "int get broken";
			u.test( a.getInt( "b" ) == 0 ) << "int get broken";
			
			a.set( "b", 2 );
			a.set( "c", 3 );
			
			u.test( a.getInt( "c" ) == 3 )	<< "table structure broken";
			
			a.set( "d", "lol" );
			a.set( "e", Vector::UNIT_X );
			a.set( "f", true );
			
			u.test( a.getString( "d" ) == String( "lol" ) ) << "string get";
			u.test( a.getVector( "e" ) == Vector::UNIT_X ) << "vector get";
			u.test( a.getBool( "f" ) ) << "bool get";
			
			a.clear();
			
			u.test( a.size() == 0 ) << "clear";
		}
		{
			Table* a = d.createTable();
			a->set( "a", 1 );
			
			u.test( a->getInt( "a" ) == 1 )	<< "int get broken";
			
			a = d.getTable( d.autoMember(0) );
			
			u.test( a->getInt( "a" ) == 1 ) << "table get broken";
		}
	}
	{
		TestUnit u( "Table Parsing", out, false );
		
		String buf;		
		Table a( "testTable1" );
		
		for( int i = 0; i < 10; ++i )
			a.set( "", i );
		
		a.set( "string1", "hello world" );
		a.set( "string2", "" );
		
		a.set( "vector", Vector::UNIT_X );
		
		byte data[120]; //some garbage
		a.set( "data", data, 120 );		
		a.serialize( buf );
		
		u.test( buf.size() > 0 ) << "write anything";
		
		a.clear();
		
		u.test( a.size() == 0 ) << "clear";
		
		StringReader str( buf );
		a.deserialize( str );
		
		testTable( u, &a, data );
		
		///nesting
		{
			Table b( "testTable2" );
			
			StringReader r2( buf );
			b.deserialize( r2 ); //copy
			
			a.set( &b );
			
			buf.clear();
			
			a.serialize( buf );
			
			a.clear();
			
			StringReader r3( buf );
			a.deserialize( r3 );
			
			Table* child = a.getTable( "testTable2" );
			
			testTable( u, child, data );
		}
		
		///from user file
		{
			Platform::getSingleton()->save( &a );
			Platform::getSingleton()->load( &a );
			
			testTable( u, &a, data );
		}
	}
}

void FrameworkTests::initTests()
{

}

void FrameworkTests::runtimeTests()
{

}