#include "fftw.h"

FFTfftw::FFTfftw() : FFT()
{
}

FFTfftw::~FFTfftw()
{
    fftw_destroy_plan(plan_fwd);
    fftw_destroy_plan(plan_rev);

    if (buf) CPXBuf::free(buf);
}

void FFTfftw::FFTParams(qint32 _size, bool _invert, double _dBCompensation, double _sampleRate)
{
    //Must call FFT base to properly init
    FFT::FFTParams(_size, _invert, _dBCompensation, _sampleRate);

    half_sz = fftSize / 2;
    plan_fwd = fftw_plan_dft_1d(fftSize , (double (*)[2])timeDomain, (double (*)[2])freqDomain, FFTW_FORWARD, FFTW_MEASURE);
    plan_rev = fftw_plan_dft_1d(fftSize , (double (*)[2])freqDomain, (double (*)[2])timeDomain, FFTW_BACKWARD, FFTW_MEASURE);
    buf = CPXBuf::malloc(fftSize);
    CPXBuf::clear(buf, fftSize);
}

//NOTE: size= # samples in 'in' buffer, 'out' must be == fftSize (set on construction) which is #bins
void FFTfftw::FFTForward(CPX * in, CPX * out, int size)
{
    if (!fftParamsSet)
        return;

    //If in==NULL, use whatever is in timeDomain buffer
    if (in != NULL ) {
        if (size < fftSize)
            //Make sure that buffer which does not have samples is zero'd out
            //We can pad samples in the time domain because it does not impact frequency results in FFT
            CPXBuf::clear(timeDomain,fftSize);

        //Put the data in properly aligned FFTW buffer
        CPXBuf::copy(timeDomain, in, size);
    }

    fftw_execute(plan_fwd);

    //If out == NULL, just leave result in freqDomain buffer and let caller get it
    if (out != NULL)
        CPXBuf::copy(out, freqDomain, fftSize);

    FFT::FFTForward(freqDomain, out, size);
}

//NOTE: size= # samples in 'in' buffer, 'out' must be == fftSize (set on construction) which is #bins
void FFTfftw::FFTInverse(CPX * in, CPX * out, int size)
{
    if (!fftParamsSet)
        return;

    //If in==NULL, use whatever is in freqDomain buffer
    if (in != NULL) {
        if (size < fftSize)
            //Make sure that buffer which does not have samples is zero'd out
            CPXBuf::clear(freqDomain,fftSize);

        CPXBuf::copy(freqDomain, in, size);
    }
    fftw_execute(plan_rev);

    if (out != NULL)
        CPXBuf::copy(out, timeDomain, fftSize);

}

//NOTE: size= # samples in 'in' buffer, 'out' must be == fftSize (set on construction) which is #bins
void FFTfftw::FFTMagnForward (CPX * in, int size, double baseline, double correction, double *fbr)
{
    if (!fftParamsSet)
        return;

    if (size < fftSize)
        //Make sure that buffer which does not have samples is zero'd out
        CPXBuf::clear(timeDomain, fftSize);

    CPXBuf::copy(timeDomain, in, size);
    // For Ref plan_fwd = fftwf_plan_dft_1d(size , (float (*)[2])timeDomain, (float (*)[2])freqDomain, FFTW_FORWARD, FFTW_MEASURE);
    fftw_execute(plan_fwd);
    //FFT output is now in freqDomain
    //FFTW does not appear to be in order as documented.  On-going mystery
    /*
     *From FFTW documentation
     *From above, an FFTW_FORWARD transform corresponds to a sign of -1 in the exponent of the DFT.
     *Note also that we use the standard “in-order” output ordering—the k-th output corresponds to the frequency k/n
     *(or k/T, where T is your total sampling period).
     *For those who like to think in terms of positive and negative frequencies,
     *this means that the positive frequencies are stored in the first half of the output
     *and the negative frequencies are stored in backwards order in the second half of the output.
     *(The frequency -k/n is the same as the frequency (n-k)/n.)
     */
    CPX temp;
    for (int i=0, j = size-1; i < size/2; i++, j--) {
        temp = freqDomain[i];
        freqDomain[i] = freqDomain[j];
        freqDomain[j] = temp;
    }

    FreqDomainToMagnitude(freqDomain, size, baseline, correction, fbr);
}

