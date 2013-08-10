#pragma once
//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include "signalprocessing.h"

class NoiseFilter :
	public SignalProcessing
{
public:
	NoiseFilter(int sampleRate, int nSamples);
	~NoiseFilter(void);
	CPX * ProcessBlock(CPX *in);
	void setAnfEnabled(bool b);

private:
	//ANF Delay line, todo: make DelayLine class
	bool anfEnabled;
	int anfAdaptiveFilterSize;
	float anfAdaptationRate;
	float anfLeakage;
	int anfDelaySize; //Size of delay buffer
	DelayLine *anfDelay;
	//ANF filter coefficients
    double *anfCoeff;

};