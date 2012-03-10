#pragma once
//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include "signalprocessing.h"

#define SPECDBMOFFSET 100.50

class SignalStrength :
	public SignalProcessing
{
public:
	SignalStrength(int sr, int ns);
	~SignalStrength(void);
	float instFValue();
	float avgFValue();
	char instCValue(); //Char representing db?
	char avgCValue();
	void setCorrection(const float value);
	CPX * ProcessBlock(CPX *in, int squelch);


private:
	float instValue; //Instantaneous value
    float avgValue; //90/10 weighted average
	float correction; //Constant value to make things fit

};
