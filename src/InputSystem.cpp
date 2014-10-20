#include "stdafx.h"

#include "InputSystem.h"

#include "GameState.h"
#include "Platform.h"
#include "InputDevice.h"
#include "InputSystemListener.h"

using namespace Dojo;

InputSystem::InputSystem( bool enable ) :
enabled(enable)
{
	Platform::singleton().addApplicationListener( this );
}

InputSystem::~InputSystem() {

}


void InputSystem::addDevice(InputDevice* device) {
	DEBUG_ASSERT(!mDeviceList.exists(device), "addDevice: this device was already registered");

	mDeviceList.add(device);

	_fireDeviceConnected(device);
}

void InputSystem::removeDevice(InputDevice* device) {
	DEBUG_ASSERT(mDeviceList.exists(device), "removeDevice: this device was not registered");

	_fireDeviceDisconnected(device);

	mDeviceList.remove(device);
}

void InputSystem::addListener(InputSystemListener* l) {
	DEBUG_ASSERT(l != nullptr, "addListener: null listener passed ");

	if (l->getSource() == NULL)
	{
		listeners.add(l);
		l->_notifySource(this);
	}
}

void InputSystem::removeListener(InputSystemListener* l) {
	DEBUG_ASSERT(l != nullptr, "removeListener: null listener passed ");

	if (l->getSource() == this)
	{
		listeners.remove(l);
		l->_notifySource(NULL);
	}
}

Touch* InputSystem::_registertouch(const Vector& point, Touch::Type type) {
	Touch* t = new Touch(mAssignedTouches++, point, type);

	mTouchList.add(t);

	return t;
}

int InputSystem::_getExistingTouchID(const Vector& point, Touch::Type type) {
	//find the nearest touch to this position
	float minDist = FLT_MAX;
	int nearest = -1;

	for (int i = 0; i < mTouchList.size(); ++i)
	{
		if (type != type)
			continue;

		float d = mTouchList[i]->point.distance(point);

		if (d < minDist)
		{
			nearest = i;
			minDist = d;
		}
	}

	return nearest;
}

Touch* InputSystem::_getExistingTouch(const Vector& point, Touch::Type type) {
	return mTouchList[_getExistingTouchID(point, type)];
}

Touch* InputSystem::_popExistingTouch(const Vector& point, Touch::Type type) {
	int idx = _getExistingTouchID(point, type);

	DEBUG_ASSERT(idx >= 0, "Needed to remove an existing touch but it was not found");

	Touch* t = mTouchList[idx];

	mTouchList.remove(idx);

	return t;
}

void InputSystem::poll(float dt)
{
	//update all the touches
	for( auto touch : mTouchList )
		touch->_update();

    for( auto device : mDeviceList )
        device->poll( dt );
};

void InputSystem::_fireTouchBeginEvent(const Vector& point, Touch::Type type)
{
    if( enabled )
    {
        //create a new Touch
        Touch* t = _registertouch( point, type );
        
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onTouchBegan( *t );
    }
}

void InputSystem::_fireTouchMoveEvent(const Vector& currentPos, const Vector& prevPos, Touch::Type type)
{
    if( enabled )
    {
        Touch* t = _getExistingTouch( prevPos, type );
        
        t->point = currentPos;
        t->speed = prevPos - currentPos; //get translation
        
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onTouchMove( *t );
    }
}

void InputSystem::_fireTouchEndEvent(const Vector& point, Touch::Type type)
{
    if( enabled )
    {
        Touch* t = _popExistingTouch( point, type );
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

void InputSystem::_fireAccelerationEvent( const Vector& accel, float roll )
{
    if( enabled )
    {				
        for( int i = 0; i < listeners.size(); ++i )
            listeners.at(i)->onAcceleration( accel,roll );
    }
}

void InputSystem::_fireDeviceConnected( InputDevice* j )
{
	//notify listeners
	for( InputSystemListener* l : listeners )
		l->onDeviceConnected(j);

	DEBUG_MESSAGE("Connected a new input device!");
}

void InputSystem::_fireDeviceDisconnected( InputDevice* j )
{
	DEBUG_ASSERT( mDeviceList.exists(j), "Tried to disconnect a non existing device" );

	//first notify this to all the listeners
	for( InputSystemListener* l : listeners )
		l->onDeviceDisconnected( j );
}

