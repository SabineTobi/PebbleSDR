//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include "nco.h"

NCO::NCO(quint32 _sampleRate, quint32 _bufferSize) :
	ProcessStep(_sampleRate,_bufferSize)
{
	m_sweepInitialized = false;
}

NCO::~NCO(void)
{
}
void NCO::setFrequency(double f)
{
	//Eveeything related to frequency has to be set together, no interupts that could change freq 
	//before step is set
	m_mutex.lock();
	//Saftey net, make sure freq is less than nyquist limit
	int nyquist = (sampleRate / 2 ) -1;
	if (f < - nyquist)
		f = -nyquist;
	else if (f > nyquist)
		f = nyquist;

	m_frequency = f;
	//Everytime freq changes, we restart synchronized NCO
	//step is how much we have to increment each sample in sin(sample) to get a waveform that
	//is perfectly synchronized to samplerate
	//w = 2 * PI * f where f is normalized (f/sampleRate) and w is angular frequency
	//x[t] = cos(wt)
	//NOTE: The only difference between a negative freq and a positive is the direction of the 'rotation'
	//ie angularIncrement will be neg for neg freq and pos for pos freq

	//Only need to calculate these when freq changes, not in main generate loop
	m_oscInc = TWOPI * m_frequency / sampleRate;
	m_oscCos = cos(m_oscInc);
	m_oscSin = sin(m_oscInc);
	m_lastOsc.re = 1.0;
	m_lastOsc.im = 0.0;
	m_oscTime = 0.0;
	m_mutex.unlock();
}

/*
From Lynn 2nd ed pg 20
x[n] = sin(n * 2 * Pi * f / fs)
f = desired frequency
fs = sampling frequency or 1/sample rate

We calculate 2Pif/fs once and save it as step
so
x[n] = sin(n * stepTone)
----------------
The number of samples per cycle is fixed, but the samples will be taken at different points in
each cycle.  We need to make sure that the spacing between each sample is consistent with the
sample rate and frequency.
A 10hz signal completes a full cycle ever 1/10 sec (1/Freg)
At 48000 sps, this means we need 4800 samples for a complete cytle (sampleRate / freq)
So a 24khz signal will have 2 samples per cycle (48000/24000), which is the Nyquist limit
-----------------
*/
void NCO::genSingle(CPX *_in, quint32 _numSamples, double _dbGain, bool _mix)
{
	for (int i = 0; i < _numSamples; i++) {
		nextSample(_in[i]);
	}

}

//Algorithm from http://dspguru.com/dsp/howtos/how-to-generate-white-gaussian-noise
//Uses Box-Muller Method, optimized to Polar Method [Ros88] A First Course on Probability
//Also http://www.dspguide.com/ch2/6.htm
//https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform R Knop algorithm
void NCO::genNoise(CPX *_in, quint32 _numSamples, double _dbGain, bool _mix)
{
	double u1;
	double u2;
	double s;
	double rad;
	double x;
	double y;

	//R Knop
	for (int i=0; i<_numSamples; i++) {
		do {
			u1 = 1.0 - 2.0 * (double)rand()/(double)RAND_MAX;
			u2 = 1.0 - 2.0 * (double)rand()/(double)RAND_MAX;
			//Collapsing steps
			s = u1 * u1 + u2 * u2;
		} while (s >= 1.0 || s == 0.0);
		// 0 < s < 1
		rad = sqrt(-2.0 * log(s) / s);
		x = _dbGain * u1 * rad;
		y = _dbGain * u2 * rad;
		if (_mix) {
			_in[i].re += x;
			_in[i].im += y;
		} else {
			_in[i].re = x;
			_in[i].im = y;
		}
	}
}

//Resets sweep generator, must be called before genSweep
void NCO::initSweep(double _sweepStartFreq, double _sweepStopFreq, double _sweepRate,
	double _pulseWidth, double _pulsePeriod, SweepType _sweepType)
{
	m_sweepStartFreq = _sweepStartFreq;
	m_sweepStopFreq = _sweepStopFreq;
	m_sweepFreq = m_sweepStartFreq;
	m_sweepRate = _sweepRate;
	m_sweepAcc = 0.0;
	m_sweepRateInc = m_sweepRate / sampleRate;
	m_sweepFreqNorm = TWOPI / sampleRate;
	m_sweepPulseWidth = _pulseWidth;
	m_sweepPulsePeriod = _pulsePeriod;
	m_sweepPulseTimer = 0;
	m_sweepType = _sweepType;
	m_sweepInitialized = true;
}

//Modifies _in
void NCO::genSweep(CPX *_in, quint32 _numSamples, double _dbGain, bool _mix)
{
	if (!m_sweepInitialized)
		return; //initSweep not called

	double amp; //Used to toggle amplitude for pulses

	for (int i=0; i<_numSamples; i++) {
		amp = _dbGain;
		if(m_sweepPulseWidth > 0.0) {
			//if pulse width is >0 create pulse modulation
			m_sweepPulseTimer += (1.0 / sampleRate);
			if(m_sweepPulseTimer > m_sweepPulsePeriod)
				m_sweepPulseTimer = 0.0;
			if(m_sweepPulseTimer > m_sweepPulseWidth)
				amp = 0.0; //Between pulses
		}

		//create complex sin/cos signal
		if (_mix) {
			//Add signal to incoming
			_in[i].re += amp * cos(m_sweepAcc);
			_in[i].im += amp * sin(m_sweepAcc);
		} else {
			//Replace incoming signal with generator
			_in[i].re = amp * cos(m_sweepAcc);
			_in[i].im = amp * sin(m_sweepAcc);
		}
		//inc phase accummulator with normalized freqeuency step
		m_sweepAcc += ( m_sweepFreq * m_sweepFreqNorm );
		m_sweepFreq += m_sweepRateInc;	//inc sweep frequency
		if(m_sweepFreq >= m_sweepStopFreq) {	//reached end of sweep?
			switch (m_sweepType) {
				//3 choices: stop, start over, return in opposite direction
				case SINGLE:
					m_sweepRateInc = 0.0; //stop sweep when end is reached
					break;
				case REPEAT:
					m_sweepFreq = m_sweepStartFreq;	//restart sweep when end is reached
					break;
				case REPEAT_REVERSE:
					//Todo: Implement reverse sweep
					break;
			}

		}
	}
	m_sweepAcc = (double)fmod((double)m_sweepAcc, TWOPI);	//keep radian counter bounded
}
