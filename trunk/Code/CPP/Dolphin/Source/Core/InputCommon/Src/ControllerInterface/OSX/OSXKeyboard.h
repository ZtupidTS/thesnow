#include <IOKit/hid/IOHIDLib.h>

#include "../ControllerInterface.h"

namespace ciface
{
namespace OSX
{

class Keyboard : public ControllerInterface::Device
{
	friend class ControllerInterface;
	friend class ControllerInterface::ControlReference;

protected:
	class Input : public ControllerInterface::Device::Input
	{
		friend class Keyboard;
	protected:
		virtual ControlState GetState(IOHIDDeviceRef device) const = 0;
	};

	class Key : public Input
	{
		friend class Keyboard;
	public:
		std::string GetName() const;
	protected:
		Key(IOHIDElementRef element);
		ControlState GetState(IOHIDDeviceRef device) const;
	private:
		IOHIDElementRef		m_element;
		std::string		m_name;
	};

	bool UpdateInput();
	bool UpdateOutput();

	ControlState GetInputState(
		const ControllerInterface::Device::Input* const input) const;
	void SetOutputState(
		const ControllerInterface::Device::Output* const output,
		const ControlState state);

public:
	Keyboard(IOHIDDeviceRef device, std::string name, int index);

	std::string GetName() const;
	std::string GetSource() const;
	int GetId() const;

private:
	IOHIDDeviceRef	m_device;
	std::string	m_device_name;
	int		m_index;
};

}
}
