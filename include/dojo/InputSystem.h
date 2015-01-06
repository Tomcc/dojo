/*
 *  TouchListener.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/26/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Array.h"
#include "Vector.h"
#include "ApplicationListener.h"
#include "Touch.h"

namespace Dojo 
{
	class Renderable;
	class InputDevice;
	class InputSystemListener;
	
	///InputSystem manages the input at the lower level in Dojo
	/**
	Listening to the InputSystem allows to be updated about 
	-Touches on the touch screen, 
	-Accelerations on the gyroscope, 
	-mouse position and clicks
	-InputDevice connection and disconnection
	*/
	class InputSystem : public ApplicationListener
	{
	public:

		typedef Array< Touch* > TouchList;
		typedef Array< InputDevice* > DeviceList;
		typedef Array<InputSystemListener*> ListenerList;
		
		InputSystem( bool enable = true );
		
		virtual ~InputSystem();

		///registers a new device to this InputSystem
		/** 
		and sends an event about its connection to the listeners */
		void addDevice( InputDevice* device );

		///unregisters a new device to this InputSystem
		/** 
		and sends an event about its disconnection to the listeners */
		void removeDevice( InputDevice* device );
		
		void addListener( InputSystemListener* l );
		
		void removeListener( InputSystemListener* l );
        
        ///polls all the registered devices
        void poll( float dt );
        
		///enables or disables the whole input
		void setEnabled( bool e )
		{
			enabled = e;
		}
		
		///returns a list of the touches that existed in the last frame (that either began, or were kept still)
		const TouchList& getTouchList()
		{
			return mTouchList;
		}

		///returns a list of the already connected devices
		const DeviceList& getDeviceList() const
		{
			return mDeviceList;
		}

		virtual void onApplicationFocusGained()
		{
			//buffered states such as key presses etc could have changed while the app was out of focus!
			//TODO
		}

		void _fireTouchBeginEvent(const Vector& point, Touch::Type type);
		void _fireTouchMoveEvent(const Vector& currentPos, const Vector& prevPos, Touch::Type type);
		void _fireTouchEndEvent(const Vector& point, Touch::Type type);
		
		void _fireMouseMoveEvent( const Vector& currentPos, const Vector& prevPos );		
		void _fireScrollWheelEvent( float scroll );		
		
		void _fireShakeEvent();		
		void _fireAccelerationEvent( const Vector& accel, float roll );

	protected:
		
		bool enabled;
		
		ListenerList listeners;
		
		TouchList mTouchList;        
		size_t mAssignedTouches = 0;

		DeviceList mDeviceList;
				
		Touch* _registertouch( const Vector& point, Touch::Type type );
		
		int _getExistingTouchID(const Vector& point, Touch::Type type);
		
		Touch* _getExistingTouch(const Vector& point, Touch::Type type);
		
		Touch* _popExistingTouch(const Vector& point, Touch::Type type);

		void _fireDeviceConnected( InputDevice* j );
		void _fireDeviceDisconnected( InputDevice* j );

	};
}

