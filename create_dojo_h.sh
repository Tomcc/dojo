#!/bin/bash

function create_h {
	
	echo "#ifndef DOJO_H"
	echo "#define DOJO_H"
	echo

#for each folder into include/dojo

	cd include/dojo

	for HEADER in *.h
	do
		echo "#include <dojo/$HEADER>"
	done

	echo
	echo "#endif"
}

create_h > include/dojo.h