1. Content of Each File:
	main.cpp: This file simulates the complete process of Dynamic Supply Task.
	parameters.h:
		1) Defines three objects in the dynamic supply task: vehicles, machines, and products.
		2) Initialization function "initObjects": Initializes vehicles, machines, and products to be processed.
	multiOpt_TSEA.h: The dynamic path planning module in MADS.
	TSEA_operators.h: The crossover and mutation operators in multiOpt_TSEA.
	pathplanning.h: Some utility functions require by the MADS.
	supplyAmount.h: A utility function require by the dynamic path planning module.
	stationAssignment.h: The dynamic machine assignment module in MADS.
	vehicleWork.h: Four working states of the vehicle (MV, UL, RL, BK).
	tools.h: Some utility functions.

2. Relationships Between Files:
	Before the dynamic supply task begins, main.cpp first calls parameters.h for initialization.
	During the dynamic supply task, main.cpp calls vehicleWork.cpp to manage the vehicle's operations in different states.
	In vehicleWork.cpp, when the unloading (UL) and loading (RL) states are completed, multiOpt_TSEA.h is called to dynamically adjust the vehicle’s subsequent supply routes. 
	During the dynamic supply task, main.cpp also calls stationAssignment.h to dynamically adjust the machine areas each vehicle is responsible for.