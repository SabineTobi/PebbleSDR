#ifndef DEVICE_INTERFACES_H
#define DEVICE_INTERFACES_H
//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
/*
 * WIP to develop an extio.dll equivalent for Pebble, leveraging what we learned from digital modem plugins
 * First example will be to move File device  to this
 *
 * We want devices to be as ignorant as possible about caller so there is no tight coupling to Pebble Receiver class.
 * This allows devices to be used in other programs eventually.
 *
 * We want to support the producer/consumer thread model to support high sample rate devices
 *
 */
#include <QtCore>
#include "pebblelib_global.h"
//Don't use tr1, part of older pre c++ 11 standard
//Use of std::function requires CONFIG += c++11 in .pro file
#include <functional> //xcode header on mac, may be other include on Win

class CPX;

//using std::placeholders;
//ProcessIQData callback: Call with CPX buffer and number of samples
typedef std::function<void(CPX *, quint16)> cbProcessIQData;

class PEBBLELIBSHARED_EXPORT DeviceInterface
{
friend class SDR; //Temp while we're transitioning from internal to plugins to sdr has access to protected

public:
    typedef enum IQORDER {IQ,QI,IONLY,QONLY} IQORDER;
    typedef enum STARTUP {SETFREQ = 0, LASTFREQ, DEFAULTFREQ} STARTUP;
    enum SDRDEVICE {SR_LITE=1, SR_V9, SR_ENSEMBLE, SR_ENSEMBLE_2M,
        SR_ENSEMBLE_4M, SR_ENSEMBLE_6M, SR_ENSEMBLE_LF,
        ELEKTOR, ELEKTOR_PA, SDR_IQ_USB,
        HPSDR_USB, HPSDR_TCP, SPARE1, FUNCUBE,
        NOSDR, FILE, DVB_T, FUNCUBE_PLUS, SDR_IP_TCP, FiFi};

	//These enums can only be extended, not changed, once released.  Otherwise will break existing plugins that are not rebuilt
	enum STANDARD_KEYS {
		PluginName,				//QString Name of plugin device was found in
		PluginDescription,		//QString Description of plugin device was found in
		PluginNumDevices,		//How many unique devices does plugin support, see DeviceNumber for correlation
		DeviceName,				//QString Actual device name, may be more than one device per plugin
		DeviceDescription,		//QString Actual device description
		DeviceNumber,			//Optional index for plugins that support multiple devices
		HighFrequency,			//Highest frequency device supports
		LowFrequency,			//Lowest frequency device supports
		IQGain,					//double User adjustable to normalize levels among devices
		SampleRate,
		StartupMode,			//int (enum) Default mode for device if not otherwise specified
		StarupFrequency,		//Default frequency for device if not otherwise specified
		LastMode,				//int (enum) Mode in use when power off
		LastFrequency,			//Frequency displayed with power off
		LastSpectrumMode,		//int (enum) Last spectrum selection
		UserMode,				//int (enum) User specified startup mode
		UserFrequency,			//User specified startup frequency
		IQOrder,
		IQBalanceEnabled,
		IQBalanceGain,
		IQBalancePhase
	};

    DeviceInterface() {};
    virtual ~DeviceInterface() {};
    //Interface must be all pure virtual functions
    virtual bool Initialize(cbProcessIQData _callback, quint16 _framesPerBuffer) = 0;
    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;

    virtual double SetFrequency(double fRequested,double fCurrent) = 0;
    //Display device option widget in settings dialog
    virtual void SetupOptionUi(QWidget *parent) = 0;
    //Called by settings to write device options to ini file
    virtual void WriteOptionUi() {}

    //Device doesn't have to implement this.  Called by receiver to bring up SDR options dialog
    virtual void ShowSdrOptions(bool b) {b=true;}

    virtual void ReadSettings() = 0;
    virtual void WriteSettings() = 0;

    virtual double GetStartupFrequency() = 0;
    virtual int GetStartupMode() = 0;
    virtual double GetHighLimit() = 0;
    virtual double GetLowLimit() = 0;
    virtual double GetGain() {return iqGain;}
    virtual QString GetDeviceName() = 0;
    virtual int GetSampleRate() = 0;
    virtual int* GetSampleRates(int &len) = 0; //Returns array of allowable rates and length of array as ref
    virtual bool UsesAudioInput() = 0;
    virtual bool GetTestBenchChecked() {return isTestBenchChecked;}

    cbProcessIQData ProcessIQData;

	virtual void SetLastDisplayMode(int mode) {lastSpectrumMode = mode;}

    virtual void SetIQOrder(IQORDER o) {iqOrder = o;}

    virtual double GetFreqToSet() {
        //If freq is outside of mode we are in return default
        if (freqToSet > GetHighLimit() || freqToSet < GetLowLimit())
            return GetStartupFrequency();
        else
            return freqToSet;
    }

    virtual double GetLastFreq() {
        //If freq is outside of mode we are in return default
        if (lastFreq > GetHighLimit() || lastFreq < GetLowLimit())
            return GetStartupFrequency();
        else
            return lastFreq;
    }
    virtual void SetLastFreq(double f) {lastFreq = f;}

    virtual void SetLastMode(int mode) {lastMode = mode;}

    virtual STARTUP GetStartup() {return startup;}
    virtual QString GetInputDeviceName() {return inputDeviceName;}
    virtual QString GetOutputDeviceName() {return outputDeviceName;}

    virtual void SetIQGain(double g) {iqGain = g;}

    //These only apply to old internal
    SDRDEVICE GetSDRDevice() {return sdrDevice;}
    void SetSDRDevice(SDRDEVICE dev) {sdrDevice = dev;}

    virtual QSettings *GetQSettings() {return qSettings;}

    //Support for multiple devices in one plugin
    virtual void SetDeviceNumber(quint16 _deviceNumber) {deviceNumber = _deviceNumber;}
    virtual quint16 GetDeviceNumber() {return deviceNumber;}

    //Allows us to get/set any device specific data
    //Standard keys will be defined, but any key can be passed
	virtual QVariant Get(QString _key, quint16 _option = 0) {return QVariant();}
	virtual QVariant Get(STANDARD_KEYS _key, quint16 _option = 0) {
		switch (_key) {
			case PluginName:
				break;
			case PluginDescription:
				break;
			case PluginNumDevices:
				break;
			case DeviceName:
				break;
			case DeviceDescription:
				break;
			case DeviceNumber:
				break;
			case HighFrequency:
				break;
			case LowFrequency:
				break;
			case IQGain:
				break;
			case SampleRate:
				break;
			case StartupMode:
				break;
			case StarupFrequency:
				break;
			case LastMode:
				break;
			case LastFrequency:
				break;
			case UserMode:
				break;
			case UserFrequency:
				break;
			case IQOrder:
				break;
			case IQBalanceEnabled:
				break;
			case IQBalanceGain:
				break;
			case IQBalancePhase:
				break;
			default:
				break;
		}
		return QVariant();
	}

	virtual bool Set(STANDARD_KEYS _key, quint16 _option = 0) {
		switch (_key) {
			case PluginName:
				break;
			case PluginDescription:
				break;
			case PluginNumDevices:
				break;
			case DeviceName:
				break;
			case DeviceDescription:
				break;
			case DeviceNumber:
				break;
			case HighFrequency:
				break;
			case LowFrequency:
				break;
			case IQGain:
				break;
			case SampleRate:
				break;
			case StartupMode:
				break;
			case StarupFrequency:
				break;
			case LastMode:
				break;
			case LastFrequency:
				break;
			case UserMode:
				break;
			case UserFrequency:
				break;
			case IQOrder:
				break;
			case IQBalanceEnabled:
				break;
			case IQBalanceGain:
				break;
			case IQBalancePhase:
				break;
			default:
				break;
		}
	}
	virtual bool Set(QString _key, QVariant _value) {return false;}

    virtual quint16 GetFrequencyCorrection() {return 0;}
    virtual bool SetFrequencyCorrection(quint16 _correction) {return false;}

protected:
    //Todo: Flag which of these is just a convenience for Pebble, vs required for the interface
    quint16 framesPerBuffer;

	int lastSpectrumMode; //Spectrum, waterfall, etc
    STARTUP startup;
    double freqToSet;
    QString inputDeviceName;
    QString outputDeviceName;
    quint32 sampleRate;

    double iqGain; //Normalize device so incoming IQ levels are consistent
    IQORDER iqOrder;
    //Image rejection (iqbalance) factors for this device
    double iqBalanceGain;
    double iqBalancePhase;
    bool iqBalanceEnable;

    double lastFreq;
    int lastMode;

    //Device needs to manage QSettings since it knows its own settings file name
    QSettings *qSettings;

    bool isTestBenchChecked;

    SDRDEVICE sdrDevice;

    int deviceNumber; //For plugins that support multiple devices

};

//How best to encode version number in interface
#define DeviceInterface_iid "N1DDY.Pebble.DeviceInterface.V1"

//Creates cast macro for interface ie qobject_cast<DigitalModemInterface *>(plugin);
Q_DECLARE_INTERFACE(DeviceInterface, DeviceInterface_iid)


#endif // DEVICE_INTERFACES_H
