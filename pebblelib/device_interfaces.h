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
//ProcessIQData callback: Call with CPX buffer of I/Q unprocessed samples and number of samples
typedef std::function<void(CPX *, quint16)> cbProcessIQData;

//ProcessBandscopeData callback: Call with buffer of spectrum values and number of data points
typedef std::function<void(quint8 *, quint16)> cbProcessBandscopeData;

//ProcessAudioData callback: Call with CPX buffer with left/right audio values and number of samples
typedef std::function<void(CPX *, quint16)> cbProcessAudioData;

class PEBBLELIBSHARED_EXPORT DeviceInterface
{
public:
    typedef enum IQORDER {IQ,QI,IONLY,QONLY} IQORDER;
	typedef enum STARTUP_TYPE {SETFREQ = 0, LASTFREQ, DEFAULTFREQ} STARTUP_TYPE;
	//Does device generate IQ, rely on Sound Card for IQ (not in plugin) or return processed audio
	typedef enum DEVICE_TYPE{INTERNAL_IQ, AUDIO_IQ, AUDIO_ONLY} DEVICE_TYPE;
	//These enums can only be extended, not changed, once released.  Otherwise will break existing plugins that are not rebuilt
	enum STANDARD_KEYS {
		PluginName = 0,				//RO QString Name of plugin device was found in
		PluginDescription,		//RO QString Description of plugin device was found in
		PluginNumDevices,		//RO How many unique devices does plugin support, see DeviceNumber for correlation
		DeviceName,				//RO QString Actual device name, may be more than one device per plugin
		DeviceDescription,		//RO QString Actual device description
		DeviceNumber,			//RW Optional index for plugins that support multiple devices
		DeviceType,				//RO int (enum DEVICE_TYPE)
		DeviceSampleRate,		//RW quint32
		DeviceSampleRates,		//RO QStringList Sample rates supported by device
		DeviceFrequency,		//RW double Device center (LO) frequency
		DeviceHealthValue,		//RO quin16 0-100 where 0 = throwing away data and 100 = within expected tollerances
		DeviceHealthString,		//RO QString explaining last DeviceHealth returned value
		InputDeviceName,		//RW QString Plugins manage settings - OS name for selected Audio input device, if any
		OutputDeviceName,		//RW QString
		HighFrequency,			//RO Highest frequency device supports
		LowFrequency,			//RO Lowest frequency device supports
		FrequencyCorrection,	//RW ???What's the universal format for this?  int ppm?
		IQGain,					//RW double User adjustable to normalize levels among devices
		StartupType,			//RW int (enum STARTUP_TYPE)
		StartupDemodMode,		//RO int (enum DEMODMODE) Default mode for device if not otherwise specified
		StartupSpectrumMode,	//RO Not used yet
		StartupFrequency,		//RO Default frequency for device if not otherwise specified
		LastDemodMode,			//RW int (enum) Mode in use when power off
		LastSpectrumMode,		//RW int (enum) Last spectrum selection
		LastFrequency,			//RW Frequency displayed with power off
		UserDemodMode,			// not used yet int (enum) User specified demod mode
		UserSpectrumMode,		//not used yet
		UserFrequency,			//User specified startup frequency
		IQOrder,				//Enum
		IQBalanceEnabled,		//bool
		IQBalanceGain,			//double
		IQBalancePhase,			//double
		//Expansion room if needed
		CustomKey1 = 100,		//Devices can implement custom keys, as long as they start with this
		CustomKey2,
		CustomKey3,
		CustomKey4,
		CustomKey5,
		CustomKey6,
		CustomKey7,
		CustomKey8,
		CustomKey9,
		CustomKey10,
	};

    //Interface must be all pure virtual functions
	virtual ~DeviceInterface() = 0;

	virtual bool Initialize(cbProcessIQData _callback,
							cbProcessBandscopeData _callbackBandscope,
							cbProcessAudioData _callbackAudio,
							quint16 _framesPerBuffer) = 0;
    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;

    //Display device option widget in settings dialog
    virtual void SetupOptionUi(QWidget *parent) = 0;

    virtual void ReadSettings() = 0;
    virtual void WriteSettings() = 0;

    //Allows us to get/set any device specific data
	virtual QVariant Get(STANDARD_KEYS _key, quint16 _option = 0) = 0;
	virtual bool Set(STANDARD_KEYS _key, QVariant _value, quint16 _option = 0) = 0;
};

//How best to encode version number in interface
#define DeviceInterface_iid "N1DDY.Pebble.DeviceInterface.V1"

//Creates cast macro for interface ie qobject_cast<DigitalModemInterface *>(plugin);
Q_DECLARE_INTERFACE(DeviceInterface, DeviceInterface_iid)


#endif // DEVICE_INTERFACES_H
