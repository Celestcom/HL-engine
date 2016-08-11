#include "StdAfx.h"
#include <iostream>
#include <stdio.h>

#include "SuitHardwareInterface.h"


#include "SerialAdapter.h"
#include "Enums.h"
#include "Parser.h"

#include "HapticDirectoryTools.h"
#include "DependencyResolver.h"

int main() {
	

	
	SuitHardwareInterface suit;
	std::shared_ptr<ICommunicationAdapter> adapter(new SerialAdapter());
	adapter->Connect();

	suit.SetAdapter(adapter);

	//Parser p;
	//p.SetBasePath("C:/Users/NullSpace Team/Documents/API_Environment/Assets/StreamingAssets");
	//p.EnumerateHapticFiles();

	DependencyResolver resolver("C:/Users/NullSpace Team/Documents/API_Environment/Assets/StreamingAssets");

	std::cin.get();

	
	
}
