/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Implements the service object that dispatches keyboard events to the system.
*/

//#include <os/log.h>
#include <DriverKit/IOUserServer.h>
#include <DriverKit/IOLib.h>
#include <HIDDriverKit/IOHIDInterface.h>
#include <DriverKit/OSCollections.h>
#include <HIDDriverKit/HIDDriverKit.h>
#include "HIDKeyboardDriver.h"


struct HIDKeyboardDriver_IVars
{
    OSArray *elements;
    
    struct {
        OSArray *elements;
    } keyboard;
};

#define _elements   ivars->elements
#define _keyboard   ivars->keyboard


bool HIDKeyboardDriver::init()
{
    os_log(OS_LOG_DEFAULT, "Initialization started.");
    
    if (!super::init()) {
        return false;
    }
    
    ivars = IONewZero(HIDKeyboardDriver_IVars, 1);
    if (!ivars) {
        return false;
    }
    
exit:
    return true;
}


void HIDKeyboardDriver::free()
{
    if (ivars) {
        OSSafeReleaseNULL(_elements);
        OSSafeReleaseNULL(_keyboard.elements);
    }
    
    IOSafeDeleteNULL(ivars, HIDKeyboardDriver_IVars, 1);
    super::free();
}


kern_return_t
IMPL(HIDKeyboardDriver, Start)
{
    kern_return_t ret;
    
    ret = Start(provider, SUPERDISPATCH);
    if (ret != kIOReturnSuccess) {
        Stop(provider, SUPERDISPATCH);
        return ret;
    }
    
    _elements = getElements();
    if (!_elements) {
        os_log(OS_LOG_DEFAULT, "Failed to get elements");
        Stop(provider, SUPERDISPATCH);
        return kIOReturnError;
    }
    
    _elements->retain();
    
    if (!parseElements(_elements)) {
        os_log(OS_LOG_DEFAULT, "No shift found");
        Stop(provider, SUPERDISPATCH);
        return kIOReturnUnsupported;
    }
    
    RegisterService();
    
    return ret;
}


bool HIDKeyboardDriver::parseElements(OSArray *elements)
{
    bool result = false;
    
    for (unsigned int i = 0; i < elements->getCount(); i++) {
        IOHIDElement *element = OSDynamicCast(IOHIDElement, elements->getObject(i));
        if (element && element->getUsagePage() == 0x07 && element->getUsage() == 0xE1) {
            result = true;
        }
        
        return result;
    }
}
    
    
void HIDKeyboardDriver::handleReport(uint64_t timestamp,
                                     uint8_t *report,
                                     uint32_t reportLength,
                                     IOHIDReportType type,
                                     uint32_t reportID)
{
    if (!_keyboard.elements) {
        return;
    }
    
    bool pedalPressed = CheckPedalStatus(report, reportLength);
    activateShiftKey(pedalPressed);
    
    for (unsigned int i = 0; i < _keyboard.elements->getCount(); i++) {
        IOHIDElement *element = NULL;
        uint64_t elementTimeStamp;
        uint32_t usagePage, usage, value, preValue;
        
        element = OSDynamicCast(IOHIDElement, _keyboard.elements->getObject(i));
        
        if (!element) {
            continue;
        }
        
        if (element->getReportID() != reportID) {
            continue;
        }
        
        elementTimeStamp = element->getTimeStamp();
        if (timestamp != elementTimeStamp) {
            continue;
        }
        
        preValue = element->getValue(kIOHIDValueOptionsFlagPrevious) != 0;
        value = element->getValue(0) != 0;
        
        if (value == preValue) {
            continue;
        }
        
        usagePage = element->getUsagePage();
        usage = element->getUsage();
        
        os_log(OS_LOG_DEFAULT,
               "Dispatching key with usagePage: 0x%02x usage: 0x%02x value: %d",
               usagePage, usage, value);
        dispatchKeyboardEvent(timestamp, usagePage, usage, value, 0, true);
    }
}
    
bool HIDKeyboardDriver::CheckPedalStatus(uint8_t *report, uint32_t reportLength)
{
    if (reportLength < 2) {
        return false;
    }
    
    return (report[1] == 0x01); // Pedal pressed
}

void HIDKeyboardDriver::activateShiftKey(bool pressed)
{
    if (!shiftKeyElement) return;
    
    IOHIDElementValue value;
    value.integerValue = pressed ? 1 : 0;
    updateElementValue(shiftKeyElement, &value);
}
