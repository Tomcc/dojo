#include "stdafx.h"

#include "InputSystem.h"

#include "GameState.h"
#include "Platform.h"
#include "InputDevice.h"

using namespace Dojo;

InputSystem::InputSystem( bool enable ) :
enabled(enable)
{
	Platform::getSingleton()->addApplicationListener( this );
}

void InputSystem::poll(float dt)
{
    for( auto device : mDeviceList )
        device->poll( dt );
};

void InputSystem::_fireTouchBeginEvent( const Vector& point )
{
    if( enabled )
    {
        //create a new Touch
        Touch* t = _registertouch( point );
        
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onTouchBegan( *t );
    }
}

void InputSystem::_fireTouchMoveEvent( const Vector& currentPos, const Vector& prevPos )
{
    if( enabled )
    {
        Touch* t = _getExistingTouch( prevPos );
        
        t->point = currentPos;
        t->speed = prevPos - currentPos; //get translation
        
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onTouchMove( *t );
    }
}

void InputSystem::_fireTouchEndEvent( const Vector& point )
{
    if( enabled )
    {
        Touch* t = _popExistingTouch( point );
        t->point = point;
        
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onTouchEnd( *t );
        
        delete t;
    }
}

void InputSystem::_fireMouseMoveEvent( const Vector& currentPos, const Vector& prevPos )
{
    if( enabled )
    {				
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onMouseMove( currentPos, prevPos );
    }
}

void InputSystem::_fireScrollWheelEvent( float scroll )
{
    if( !enabled )	return;
    
    for( int i = 0; i < listeners.size(); ++i )
        listeners[i]->onScrollWheel( scroll );
}

void InputSystem::_fireShakeEvent()
{
    if( enabled )	
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onShake();
}

void InputSystem::_fireAccelerationEvent( const Dojo::Vector& accel, float roll )
{
    if( enabled )
    {				
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onAcceleration( accel,roll );
    }
}

void InputSystem::_fireDeviceConnected( Dojo::InputDevice* j )
{
	//notify listeners
	for( Listener* l : listeners )
		l->onDeviceConnected( j );

	DEBUG_MESSAGE( "Connected a " << j->getType().ASCII().c_str() << "!" );
}

void InputSystem::_fireDeviceDisconnected( Dojo::InputDevice* j )
{
	DEBUG_ASSERT( mDeviceList.exists(j) );

	//first notify this to all the listeners
	for( Listener* l : listeners )
		l->onDeviceDisconnected( j );
}