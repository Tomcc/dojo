#include "stdafx.h"

#include "FrameworkTests.h"

#include "TestUnit.h"
#include "Table.h"

using namespace Dojo;
using namespace std;

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

		}
		
		for( int i = 0; i < 5; ++i )
		{
			Table* a = d.createTable();
			a->set( "a", 1 );
			
			u.test( a->getInt( "a" ) == 1 )	<< "int get broken";
			
			a = d.getTable( d.autoMember(i) );
			
			u.test( a->getInt( "a" ) == 1 ) << "table get broken";
			
			u.test( a->getInt( "a" ) == 1 )	<< "int get broken";
			u.test( a->getInt( "b" ) == 0 ) << "int get broken";
			
			a->set( "b", 2 );
			a->set( "c", 3 );
			
			u.test( a->getInt( "c" ) == 3 )	<< "table structure broken";
			
			a->set( "d", "lol" );
			a->set( "e", Vector::UNIT_X );
			a->set( "f", true );
			
			u.test( a->getString( "d" ) == String( "lol" ) ) << "string get";
			u.test( a->getVector( "e" ) == Vector::UNIT_X ) << "vector get";
			u.test( a->getBool( "f" ) ) << "bool get";
		}
	}
}

void FrameworkTests::initTests()
{

}

void FrameworkTests::runtimeTests()
{

}