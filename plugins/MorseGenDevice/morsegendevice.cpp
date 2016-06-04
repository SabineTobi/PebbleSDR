#include "morsegendevice.h"
#include "db.h"

//Plugin constructors are called indirectly when the plugin is loaded in Receiver
//Be careful not to access objects that are not initialized yet, do that in Initialize()
MorseGenDevice::MorseGenDevice():DeviceInterfaceBase()
{
	initSettings("ExampleSDR");
	optionUi = NULL;
}

//Called when the plugins object is deleted in the ~Receiver()
//Be careful not to access objects that may already be destroyed
MorseGenDevice::~MorseGenDevice()
{

}

bool MorseGenDevice::initialize(CB_ProcessIQData _callback,
								  CB_ProcessBandscopeData _callbackBandscope,
								  CB_ProcessAudioData _callbackAudio, quint16 _framesPerBuffer)
{
	DeviceInterfaceBase::initialize(_callback, _callbackBandscope, _callbackAudio, _framesPerBuffer);
	m_numProducerBuffers = 50;
	producerFreeBufPtr = NULL;
#if 1
	//Remove if producer/consumer buffers are not used
	//This is set so we always get framesPerBuffer samples (factor in any necessary decimation)
	//ProducerConsumer allocates as array of bytes, so factor in size of sample data
	quint16 sampleDataSize = sizeof(double);
	m_readBufferSize = framesPerBuffer * sampleDataSize * 2; //2 samples per frame (I/Q)

	m_producerConsumer.Initialize(std::bind(&MorseGenDevice::producerWorker, this, std::placeholders::_1),
		std::bind(&MorseGenDevice::consumerWorker, this, std::placeholders::_1),m_numProducerBuffers, m_readBufferSize);
	//Must be called after Initialize
	m_producerConsumer.SetProducerInterval(m_deviceSampleRate,framesPerBuffer);
	m_producerConsumer.SetConsumerInterval(m_deviceSampleRate,framesPerBuffer);

	//Start this immediately, before connect, so we don't miss any data
	m_producerConsumer.Start(true,true);

#endif

	return true;
}

void MorseGenDevice::readSettings()
{
	// +/- db gain required to normalize to fixed level input
	// Default is 0db gain, or a factor of 1.0
	m_normalizeIQGain = DB::dBToAmplitude(0);

	//Set defaults before calling DeviceInterfaceBase
	DeviceInterfaceBase::readSettings();
}

void MorseGenDevice::writeSettings()
{
	DeviceInterfaceBase::writeSettings();
}

bool MorseGenDevice::command(DeviceInterface::StandardCommands _cmd, QVariant _arg)
{
	switch (_cmd) {
		case Cmd_Connect:
			DeviceInterfaceBase::connectDevice();
			//Device specific code follows
			return true;

		case Cmd_Disconnect:
			DeviceInterfaceBase::disconnectDevice();
			//Device specific code follows
			return true;

		case Cmd_Start:
			DeviceInterfaceBase::startDevice();
			//Device specific code follows

			//How often do we need to read samples from files to get framesPerBuffer at sampleRate
			nsPerBuffer = (1000000000.0 / m_deviceSampleRate) * framesPerBuffer;
			//qDebug()<<"nsPerBuffer"<<nsPerBuffer;
			elapsedTimer.start();

			return true;

		case Cmd_Stop:
			DeviceInterfaceBase::stopDevice();
			//Device specific code follows
			return true;

		case Cmd_ReadSettings:
			DeviceInterfaceBase::readSettings();
			//Device specific settings follow
			readSettings();
			return true;

		case Cmd_WriteSettings:
			DeviceInterfaceBase::writeSettings();
			//Device specific settings follow
			writeSettings();
			return true;

		case Cmd_DisplayOptionUi: {
			//Add ui header file include
			//Add private uiOptions

			//Arg is QWidget *parent
			//Use QVariant::fromValue() to pass, and value<type passed>() to get back
			QWidget *parent = _arg.value<QWidget*>();
			if (optionUi != NULL)
				delete optionUi;

			//Change .h and this to correct class name for ui
			optionUi = new Ui::MorseGenOptions();
			optionUi->setupUi(parent);
			parent->setVisible(true);
			return true;
		}
		default:
			return false;
	}
}

QVariant MorseGenDevice::get(DeviceInterface::StandardKeys _key, QVariant _option)
{
	Q_UNUSED(_option);

	switch (_key) {
		case Key_PluginName:
			return "Morse Generator";
			break;
		case Key_PluginDescription:
			return "Generates Morse Code";
			break;
		case Key_DeviceName:
			return "MorseGenDevice";
		case Key_DeviceType:
			return DeviceInterfaceBase::DT_AUDIO_IQ_DEVICE;
		default:
			return DeviceInterfaceBase::get(_key, _option);
	}
}

bool MorseGenDevice::set(DeviceInterface::StandardKeys _key, QVariant _value, QVariant _option)
{
	Q_UNUSED(_option);

	switch (_key) {
		case Key_DeviceFrequency:
			return true; //Must be handled by device

		default:
			return DeviceInterfaceBase::set(_key, _value, _option);
	}
}

void MorseGenDevice::producerWorker(cbProducerConsumerEvents _event)
{
#if 0
	//For verifying device data format min/max so we can normalize later
	static short maxSample = 0;
	static short minSample = 0;
#endif
	timespec req, rem;
	qint64 nsRemaining = nsPerBuffer - elapsedTimer.nsecsElapsed();

	switch (_event) {
		case cbProducerConsumerEvents::Start:
			break;
		case cbProducerConsumerEvents::Run:
			//Govern output rate to match sampleRate, same as if actual device was outputing samples
			if (nsRemaining > 0) {
				req.tv_sec = 0;
				//We want to get close to exact time, but not go over
				//So we sleep for 1/2 of the remaining time each sequence
				req.tv_nsec = nsRemaining;
				if (nanosleep(&req,&rem) < 0) {
					qDebug()<<"nanosleep failed";
				}
			}
			elapsedTimer.start(); //Restart elapsed timer

			if ((producerFreeBufPtr = (CPX*)m_producerConsumer.AcquireFreeBuffer()) == NULL)
				return;
#if 0
			while (running) {
				//This ignores producer thread slices and runs as fast as possible to get samples
				//May be used for sample rates where thread slice is less than 1ms
				//Get data from device and put into producerFreeBufPtr
			}
#else
			//Get data from device and put into producerFreeBufPtr
			//Return and wait for next producer time slice
#endif
#if 0
			//For testing device sample format
			if (producerIBuf[i] > maxSample) {
				maxSample = producerIBuf[i];
				qDebug()<<"New Max sample "<<maxSample;
			}
			if (producerQBuf[i] > maxSample) {
				maxSample = producerQBuf[i];
				qDebug()<<"New Max sample "<<maxSample;
			}
			if (producerIBuf[i] < minSample) {
				minSample = producerIBuf[i];
				qDebug()<<"New Min sample "<<minSample;
			}
			if (producerQBuf[i] < minSample) {
				minSample = producerQBuf[i];
				qDebug()<<"New Min sample "<<minSample;
			}
#endif

			m_producerConsumer.ReleaseFilledBuffer();
			return;

			break;
		case cbProducerConsumerEvents::Stop:
			break;
	}
}

void MorseGenDevice::consumerWorker(cbProducerConsumerEvents _event)
{
	unsigned char *consumerFilledBufferPtr;

	switch (_event) {
		case cbProducerConsumerEvents::Start:
			break;
		case cbProducerConsumerEvents::Run:
			//We always want to consume everything we have, producer will eventually block if we're not consuming fast enough
			while (m_producerConsumer.GetNumFilledBufs() > 0) {
				//Wait for data to be available from producer
				if ((consumerFilledBufferPtr = m_producerConsumer.AcquireFilledBuffer()) == NULL) {
					//qDebug()<<"No filled buffer available";
					return;
				}

				//Process data in filled buffer and convert to Pebble format in consumerBuffer

				//perform.StartPerformance("ProcessIQ");
				processIQData(consumerBuffer,framesPerBuffer);
				//perform.StopPerformance(1000);
				//We don't release a free buffer until ProcessIQData returns because that would also allow inBuffer to be reused
				m_producerConsumer.ReleaseFreeBuffer();

			}
			break;
		case cbProducerConsumerEvents::Stop:
			break;
	}
}
