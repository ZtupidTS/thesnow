#include <Foundation/Foundation.h>
#include <IOKit/hid/IOHIDLib.h>

#include "../ControllerInterface.h"
#include "OSXMouse.h"

namespace ciface
{
namespace OSX
{

extern void DeviceElementDebugPrint(const void*, void*);

Mouse::Mouse(IOHIDDeviceRef device)
	: m_device(device)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	m_device_name = [(NSString *)IOHIDDeviceGetProperty(m_device,
		CFSTR(kIOHIDProductKey)) UTF8String];

	// Buttons
	NSDictionary *buttonDict =
	 [NSDictionary dictionaryWithObjectsAndKeys:
	  [NSNumber numberWithInteger:kIOHIDElementTypeInput_Button],
		@kIOHIDElementTypeKey,
	  [NSNumber numberWithInteger:kHIDPage_Button],
		@kIOHIDElementUsagePageKey,
	  nil];

	CFArrayRef buttons = IOHIDDeviceCopyMatchingElements(m_device,
		(CFDictionaryRef)buttonDict, kIOHIDOptionsTypeNone);

	if (buttons)
	{
		for (int i = 0; i < CFArrayGetCount(buttons); i++)
		{
			IOHIDElementRef e =
			(IOHIDElementRef)CFArrayGetValueAtIndex(buttons, i);
			//DeviceElementDebugPrint(e, NULL);

			AddInput(new Button(e));
		}
		CFRelease(buttons);
	}

	// Axes
	NSDictionary *axisDict =
	[NSDictionary dictionaryWithObjectsAndKeys:
	 [NSNumber numberWithInteger:kIOHIDElementTypeInput_Misc],
		@kIOHIDElementTypeKey,
	 nil];

	CFArrayRef axes = IOHIDDeviceCopyMatchingElements(m_device,
		(CFDictionaryRef)axisDict, kIOHIDOptionsTypeNone);

	if (axes)
	{
		for (int i = 0; i < CFArrayGetCount(axes); i++)
		{
			IOHIDElementRef e =
			(IOHIDElementRef)CFArrayGetValueAtIndex(axes, i);
			//DeviceElementDebugPrint(e, NULL);

			AddInput(new Axis(e, Axis::negative));
			AddInput(new Axis(e, Axis::positive));
		}
		CFRelease(axes);
	}

	[pool release];
}

ControlState Mouse::GetInputState(
	const ControllerInterface::Device::Input* const input) const
{
	return ((Input*)input)->GetState(m_device);
}

void Mouse::SetOutputState(
	const ControllerInterface::Device::Output* const output,
	const ControlState state)
{
}

bool Mouse::UpdateInput()
{
	return true;
}

bool Mouse::UpdateOutput()
{
	return true;
}

std::string Mouse::GetName() const
{
	return m_device_name;
}

std::string Mouse::GetSource() const
{
	return "HID";
}

int Mouse::GetId() const
{
	// Overload the "id" to identify devices by HID type when names collide
	return kHIDUsage_GD_Mouse;
}


Mouse::Button::Button(IOHIDElementRef element)
	: m_element(element)
{
	std::ostringstream s;
	s << IOHIDElementGetUsage(m_element);
	m_name = std::string("Click ") + s.str();
}

ControlState Mouse::Button::GetState(IOHIDDeviceRef device) const
{
	IOHIDValueRef value;
	if (IOHIDDeviceGetValue(device, m_element, &value) == kIOReturnSuccess)
		return IOHIDValueGetIntegerValue(value) > 0;

	return false;
}

std::string Mouse::Button::GetName() const
{
	return m_name;
}


Mouse::Axis::Axis(IOHIDElementRef element, direction dir)
	: m_element(element)
	, m_direction(dir)
{
	// Need to parse the element a bit first
	std::string description("unk");

	switch (IOHIDElementGetUsage(m_element)) {
	default:
		NSLog(@"Unknown axis type 0x%x, using it anyway...",
			IOHIDElementGetUsage(m_element));
		break;
	case kHIDUsage_GD_X:
		description = "X";
		break;
	case kHIDUsage_GD_Y:
		description = "Y";
		break;
	case kHIDUsage_GD_Wheel:
		description = "Wheel";
		break;
	case kHIDUsage_Csmr_ACPan:
		description = "Pan";
		break;
	}

	m_name = std::string("Axis ") + description;
	m_name.append((m_direction == positive) ? "+" : "-");

	// yeah, that factor is completely random :/
	m_range = (float)IOHIDElementGetLogicalMax(m_element) / 1000.;
}

ControlState Mouse::Axis::GetState(IOHIDDeviceRef device) const
{
	IOHIDValueRef value;
	if (IOHIDDeviceGetValue(device, m_element, &value) == kIOReturnSuccess)
	{
		int int_value = IOHIDValueGetIntegerValue(value);

		if (((int_value < 0) && (m_direction == positive)) ||
			((int_value > 0) && (m_direction == negative)) ||
			!int_value)
			return false;

		float actual_value = abs(int_value) / m_range;

		//NSLog(@"%s %i %f", m_name.c_str(), int_value, actual_value);

		return actual_value;
	}

	return false;
}

std::string Mouse::Axis::GetName() const
{
	return m_name;
}


}
}
