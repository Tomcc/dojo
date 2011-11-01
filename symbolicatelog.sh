#!/bin/bash

#symbolicates a log, given an app
#usage:
#	./symbolicatelog file.crash file.app

#mostly an utility to remeber WTF symbolicatecrash is

/Developer/Platforms/iPhoneOS.platform/Developer/Library/PrivateFrameworks/DTDeviceKit.framework/Versions/A/Resources/symbolicatorcrash $1 $2 > $1.symbolicated