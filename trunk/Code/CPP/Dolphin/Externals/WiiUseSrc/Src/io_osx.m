/*
 *	wiiuse
 *
 *	Written By:
 *		Michael Laforest	< para >
 *		Email: < thepara (--AT--) g m a i l [--DOT--] com >
 *
 *	Copyright 2006-2007
 *
 *	This file is part of wiiuse.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	$Header$
 *
 */

/**
 *	@file
 *	@brief Handles device I/O for *nix.
 */
#define BLUETOOTH_VERSION_USE_CURRENT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//osx inlcude
#import <Foundation/NSObject.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#import <IOBluetooth/objc/IOBluetoothL2CAPChannel.h>
#import <IOBluetooth/objc/IOBluetoothHostController.h>

IOBluetoothDeviceInquiry *bti;
IOBluetoothDevice * btd;
IOBluetoothL2CAPChannel * _ichan;
IOBluetoothL2CAPChannel * _cchan;

#include "definitions.h"
#include "wiiuse_internal.h"
#include "io.h"

byte DataFromWiimote[MAX_PAYLOAD];
NSUInteger g_length;

static int wiiuse_connect_single(struct wiimote_t* wm, char* address);

@interface SearchBT: NSObject {}
-(void) deviceInquiryComplete: (IOBluetoothDeviceInquiry*) sender
                            error: (IOReturn) error
                            aborted: (BOOL) aborted;
-(void) deviceInquiryDeviceFound: (IOBluetoothDeviceInquiry*) sender
                            device: (IOBluetoothDevice*) device;

@end

@interface ConnectBT: NSObject {}

-(void) connectToWiimotes;

- (IOBluetoothL2CAPChannel *) openL2CAPChannelWithPSM:(BluetoothL2CAPPSM) psm delegate:(id) delegate;
@end

@implementation ConnectBT

-(void)writeToWiimote: (void *)data length:(UInt16)length
{

        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        [_cchan writeSync:data length:length];
        usleep(10000);
        [pool release];

}


#pragma mark -
#pragma mark Bluetooth

- (void) l2capChannelOpenComplete:(IOBluetoothL2CAPChannel*) l2capChannel status:(IOReturn) error {
	//channel opened
	//something to do here ?
}


- (void)l2capChannelData:(IOBluetoothL2CAPChannel *) l2capChannel data:(byte *)BtData length:(NSUInteger)length {
		
	//here we got data from wiimote
	memcpy(DataFromWiimote, BtData, MAX_PAYLOAD);
	g_length = length;
	
	//stop the main loop after reading
	CFRunLoopStop( CFRunLoopGetCurrent() );
}



- (void) l2capChannelClosed:(IOBluetoothL2CAPChannel*) l2capChannel {
        //channel closed
        printf("Bt channel closed\n");
        if (l2capChannel == _cchan)
                _cchan = nil;

        if (l2capChannel == _ichan)
                _ichan = nil;
}

#pragma mark -
- (IOBluetoothL2CAPChannel *) openL2CAPChannelWithPSM:(BluetoothL2CAPPSM) psm delegate:(id) delegate {
        IOBluetoothL2CAPChannel * channel = nil;
        IOReturn ret = kIOReturnSuccess;

        WIIUSE_INFO("Open channel (PSM:%i) ...", psm);
        if ((ret = (IOReturn)[btd openL2CAPChannelSync:&channel withPSM:psm delegate:delegate]) != kIOReturnSuccess) {
                WIIUSE_INFO("Could not open L2CAP channel (psm:%i)", psm);
                channel = nil;
                                //TODO : close the connection
        }

        return channel;
}


-(void) connectToWiimotes {

        NSEnumerator * en = [[bti foundDevices] objectEnumerator];

	id device = nil;
	while ((device = [en nextObject]) != nil) {
                btd = device;

                [btd openL2CAPChannelSync:&_cchan withPSM:kBluetoothL2CAPPSMHIDControl delegate:self];

        	if (!_cchan)
                	WIIUSE_INFO("error when initialised cchan");

        	usleep (20000);

		[btd openL2CAPChannelSync:&_ichan withPSM:kBluetoothL2CAPPSMHIDInterrupt delegate:self];

        	if (!_ichan)
                	WIIUSE_INFO("error when initialised ichan");

        	usleep (20000);

	}

}

@end

@implementation SearchBT


-(void) deviceInquiryComplete: (IOBluetoothDeviceInquiry*) sender
                            error: (IOReturn) error
                            aborted: (BOOL) aborted
{
        //int founded = [[bti foundDevices] count];
        CFRunLoopStop( CFRunLoopGetCurrent() );
}
-(void) deviceInquiryDeviceFound: (IOBluetoothDeviceInquiry*) sender
                            device: (IOBluetoothDevice*) device
{
    WIIUSE_INFO("discovered one wiimote: %s", [[device getAddressString] UTF8String]);
        [bti stop];
}



@end

SearchBT *sbt;
ConnectBT *cbt;

void detectWiimote(int timeout) {

    [bti setDelegate: sbt];
    [bti setInquiryLength:timeout];
    [bti setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
    [bti setUpdateNewDeviceNames:NO];

    IOReturn ret = [bti start];
    if (ret == kIOReturnSuccess)
            [bti retain];
    else
    {
            WIIUSE_INFO("error when detecting wiimote");
            [bti setDelegate: nil];
            bti = nil;
    }


}

/**
 *	@brief Find a wiimote or wiimotes.
 *
 *	@param wm			An array of wiimote_t structures.
 *	@param max_wiimotes	The number of wiimote structures in \a wm.
 *	@param timeout		The number of seconds before the search times out.
 *
 *	@return The number of wiimotes found.
 *
 *	@see wiimote_connect()
 *
 *	This function will only look for wiimote devices.						\n
 *	When a device is found the address in the structures will be set.		\n
 *	You can then call wiimote_connect() to connect to the found				\n
 *	devices.
 */
int wiiuse_find(struct wiimote_t** wm, int max_wiimotes, int timeout) {

	int found_devices;
	int found_wiimotes;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	IOBluetoothHostController *bth = [[IOBluetoothHostController alloc] init];
	if([bth addressAsString] == nil)
	{
		// Method addressAsString will return nil since it can't find a device
		WIIUSE_INFO("No BT device");
		[bth release];
		[pool release];
		return 0; // 0 Wiimotes found
	}
	sbt = [[SearchBT alloc] init];
	cbt = [[ConnectBT alloc] init];
	bti = [[IOBluetoothDeviceInquiry alloc] init];

	detectWiimote(timeout);

	CFRunLoopRun();

	found_wiimotes = 0;

	WIIUSE_INFO("Found %i bluetooth device(s).", found_devices);
	
	WIIMOTE_ENABLE_STATE(wm[found_wiimotes], WIIMOTE_STATE_DEV_FOUND);

	[bth release];
	[bti release];
	[sbt release];
	[cbt release];

	[pool release];


	return 1;


}


/**
 *	@brief Connect to a wiimote or wiimotes once an address is known.
 *
 *	@param wm			An array of wiimote_t structures.
 *	@param wiimotes		The number of wiimote structures in \a wm.
 *
 *	@return The number of wiimotes that successfully connected.
 *
 *	@see wiiuse_find()
 *	@see wiiuse_connect_single()
 *	@see wiiuse_disconnect()
 *
 *	Connect to a number of wiimotes when the address is already set
 *	in the wiimote_t structures.  These addresses are normally set
 *	by the wiiuse_find() function, but can also be set manually.
 */
int wiiuse_connect(struct wiimote_t** wm, int wiimotes) {
	int i = 0;

	/*for (; i < wiimotes; ++i) {
		if (!WIIMOTE_IS_SET(wm[i], WIIMOTE_STATE_DEV_FOUND))
			// if the device address is not set, skip it 
			continue;

		if (wiiuse_connect_single(wm[i], NULL))
			++connected;
	}*/
	wiiuse_connect_single(wm[i], NULL);

	//return connected;
	return 1;
}


/**
 *	@brief Connect to a wiimote with a known address.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param address	The address of the device to connect to.
 *					If NULL, use the address in the struct set by wiiuse_find().
 *
 *	@return 1 on success, 0 on failure
 */
static int wiiuse_connect_single(struct wiimote_t* wm, char* address) {

	if (!wm || WIIMOTE_IS_CONNECTED(wm))
		return 0;

	//connect to the wiimote
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	cbt = [[ConnectBT alloc] init];
	//start to connect to the wiimotes
	[cbt connectToWiimotes];
	
	usleep (200000); // Little delay, or else the device isn't ready!
	WIIUSE_INFO("Connected to wiimote [id %i].", wm->unid);

		/* do the handshake */
	WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_CONNECTED);
	wiiuse_handshake(wm, NULL, 0);

	wiiuse_set_report_type(wm);



	[pool release];

	return 1;
}


/**
 *	@brief Disconnect a wiimote.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *
 *	@see wiiuse_connect()
 *
 *	Note that this will not free the wiimote structure.
 */
void wiiuse_disconnect(struct wiimote_t* wm) {
	if (!wm || WIIMOTE_IS_CONNECTED(wm))
		return;

	//TODO

	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_CONNECTED);
	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_HANDSHAKE);
}

int wiiuse_io_read(struct wiimote_t* wm) {
        
	if (!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_CONNECTED))
                return 0;


	/* if this wiimote is not connected, skip it */
	if (!WIIMOTE_IS_CONNECTED(wm))
			return 0;

	//run the main loop to get bt data
	CFRunLoopRun();

	memcpy(wm->event_buf,DataFromWiimote, g_length);
	if(!g_length || !wm->event_buf[0]) // no packet
		return 0;
	
	wm->event_buf[0] = 0xa2; // Make sure it's 0xa2, just in case
	return 1;

}


int wiiuse_io_write(struct wiimote_t* wm, byte* buf, int len) {
	[cbt writeToWiimote:buf length:len];
	return 1;
}


