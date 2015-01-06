#pragma once

#include "dojo_common_header.h"

#include "Array.h"

namespace Dojo
{
	class InputDeviceListener;

	///a generic interface over the actual device implementations
	class InputDevice
	{
	public:

		const static int BUTTON_MAX = 16;

		enum class Type {
			Keyboard,
			Mouse,
			Xbox,
		};

		//! each pad has a number of analog inputs; each one is a float
		enum Axis
		{
			AI_LX,
			AI_LY,
			AI_LZ,

			AI_RX,
			AI_RY,
			AI_RZ,

			AI_SLIDER1,
			AI_SLIDER2,

			_AI_COUNT
		};

		const Type type;

		///Creates a new InputDevice of the given type, bound to the ID slot, supporting "buttonNumber" buttons and "axisNumber" axes
		InputDevice( Type type, int ID, int buttonNumber, int axisNumber );

		///returns if the given action is pressed
		virtual bool isKeyDown( KeyCode key );

		bool isKeyDown(int action);
 
		///returns the instant state of this axis
		virtual float getAxis( Axis axis );

		///returns true if this device supports the requested axis
		virtual bool hasAxis(Axis axis) const = 0;

		///returns the bound slot for this Device
		/*
		for example, 0..3 for XBox controllers, or 0..n for each control method mapped to the keyboard
		*/
		int getID()
		{
			return mID;
		}

		void addListener( InputDeviceListener* l );

		void removeListener( InputDeviceListener* l );

		///Adds an "Action Binding" to this device
		/**
		InputDevice::Listener will only receive events in terms of Actions;
		the same action can be bound to more than one KeyCode to allow for easy input configuration.
		\remark the default Action for a key is the key number itself
		*/
		void addBinding( int action, KeyCode key );

		///returns the action bound to this KeyCode
		/** 
		/remark the default action for unassigned keys is the key number itself */
		int getActionForKey( KeyCode key );

		///each device can be polled each frame if needed
		virtual void poll( float dt );

		///internal
		void _notifyButtonState( KeyCode key, bool pressed );

		///internal
		void _notifyAxis( Axis a, float state);

		///internal
		void _fireDisconnected();

	protected:

		typedef Array< InputDeviceListener* > ListenerList;
		typedef std::unordered_map< KeyCode, int, std::hash<int> > KeyActionMap;
		typedef std::unordered_multimap< int, KeyCode > ActionKeyMap;
		typedef std::unordered_map< KeyCode, bool, std::hash<int> > KeyPressedMap;
		typedef Array< float > FloatList;

		int mAxisNumber, mButtonNumber;

		ListenerList pListeners;

		KeyPressedMap mButton;
		FloatList mAxis, mDeadZone;

		KeyActionMap mBindings;
		ActionKeyMap mInverseBindings;

		int mID;
	};
}

