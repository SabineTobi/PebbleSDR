//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include "processstep.h"
#include "string.h"

ProcessStep::ProcessStep(quint32 _sampleRate, quint32 _bufferSize)
{
	sampleRate = _sampleRate;
	numSamples = _bufferSize;
	bufferSize = _bufferSize; //Will eventually replace numSamples
	numSamplesX2 = _bufferSize*2; //Frequency used
	out = memalign(numSamples);
	enabled = false;
}

ProcessStep::~ProcessStep(void)
{
	if (out != NULL)
		free(out);
}

CPX *ProcessStep::process(CPX *in, quint32 _numSamples)
{
	Q_UNUSED(_numSamples);
	return in;
}

void ProcessStep::enableStep(bool _enableStep)
{
	enabled = _enableStep;
}

