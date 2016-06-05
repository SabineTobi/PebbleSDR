#include "morsegendevice.h"
#include "db.h"

//Plugin constructors are called indirectly when the plugin is loaded in Receiver
//Be careful not to access objects that are not initialized yet, do that in Initialize()
MorseGenDevice::MorseGenDevice():DeviceInterfaceBase()
{
	initSettings("morsegen");
	m_optionUi = NULL;
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

	quint32 tcwMs = MorseCode::wpmToTcwMs(20);
	quint32 maxSymbolMs = MorseCode::c_maxTcwPerSymbol * tcwMs;
	quint32 outBufLen = maxSymbolMs / (1000.0/m_deviceSampleRate);
	m_outBuf = CPX::memalign(outBufLen);
	m_outBuf1 = CPX::memalign(outBufLen);
	m_outBuf2 = CPX::memalign(outBufLen);

	//Sample rate must be same for all gen and same as wav file
	m_morseGen1 = new MorseGen(m_deviceSampleRate);
	m_morseGen2 = new MorseGen(m_deviceSampleRate);
	m_morseGen3 = new MorseGen(m_deviceSampleRate);
	m_morseGen4 = new MorseGen(m_deviceSampleRate);
	m_morseGen5 = new MorseGen(m_deviceSampleRate);


	//Trailing '=' is morse <BT> for new paragraph
	m_sampleText[0] = "=The quick brown fox jumped over the lazy dog 1,2,3,4,5,6,7,8,9,0 times=";
	m_sampleText[1] = "=Now is the time for all good men to come to the aid of their country=";
	m_sampleText[2] = "=Now is the time for all good men to come to the aid of their country=";
	m_sampleText[3] = "=Now is the time for all good men to come to the aid of their country=";
	//m_sampleText[4] = "=Now is the time for all good men to come to the aid of their country=";

	//Build text for all characters
	MorseSymbol *symbol;
	m_sampleText[4] = "=";
	for (quint32 i=0; i<MorseCode::c_morseTableSize; i++) {
		symbol = &MorseCode::m_morseTable[i];
		if (symbol->ascii == 0x00) {
			break; //End of table
		}
		m_sampleText[4].append(symbol->ascii);
	}
	m_sampleText[4].append("=");

	m_numProducerBuffers = 50;
	m_producerFreeBufPtr = NULL;
	//This is set so we always get framesPerBuffer samples (factor in any necessary decimation)
	//ProducerConsumer allocates as array of bytes, so factor in size of sample data
	quint16 sampleDataSize = sizeof(double);
	m_readBufferSize = m_framesPerBuffer * sampleDataSize * 2; //2 samples per frame (I/Q)

	m_producerConsumer.Initialize(std::bind(&MorseGenDevice::producerWorker, this, std::placeholders::_1),
		std::bind(&MorseGenDevice::consumerWorker, this, std::placeholders::_1),m_numProducerBuffers, m_readBufferSize);
	//Must be called after Initialize
	//m_producerConsumer.SetProducerInterval(m_deviceSampleRate,m_framesPerBuffer);
	//m_producerConsumer.SetConsumerInterval(m_deviceSampleRate,m_framesPerBuffer);

	return true;
}

void MorseGenDevice::readSettings()
{
	// +/- db gain required to normalize to fixed level input
	// Default is 0db gain, or a factor of 1.0
	m_normalizeIQGain = DB::dBToAmplitude(0);

	//Set defaults before calling DeviceInterfaceBase
	DeviceInterfaceBase::readSettings();

	m_dbNoiseAmp = m_qSettings->value("DbNoiseAmp",-60).toDouble();
	m_gen1Enabled = m_qSettings->value("Gen1Enabled",true).toBool();
	m_gen1Freq = m_qSettings->value("Gen1Freq",1000).toDouble();
	m_gen1Amp = m_qSettings->value("Gen1Amp",-50).toDouble();
	m_gen1Wpm = m_qSettings->value("Gen1Wpm",20).toUInt();
	m_gen1Rise = m_qSettings->value("Gen1Rise",5).toUInt();
	m_gen1Text = m_qSettings->value("Gen1Text",0).toUInt();
	m_gen1Fade = m_qSettings->value("Gen1Fade",false).toBool();
	m_gen1Fist = m_qSettings->value("Gen1Fist",false).toBool();

	m_gen2Enabled = m_qSettings->value("Gen2Enabled",true).toBool();
	m_gen2Freq = m_qSettings->value("Gen2Freq",1500).toDouble();
	m_gen2Amp = m_qSettings->value("Gen2Amp",-50).toDouble();
	m_gen2Wpm = m_qSettings->value("Gen2Wpm",30).toUInt();
	m_gen2Rise = m_qSettings->value("Gen2Rise",5).toUInt();
	m_gen2Text = m_qSettings->value("Gen2Text",0).toUInt();
	m_gen2Fade = m_qSettings->value("Gen2Fade",false).toBool();
	m_gen2Fist = m_qSettings->value("Gen2Fist",false).toBool();

	m_gen3Enabled = m_qSettings->value("Gen3Enabled",true).toBool();
	m_gen3Freq = m_qSettings->value("Gen3Freq",2000).toDouble();
	m_gen3Amp = m_qSettings->value("Gen3Amp",-50).toDouble();
	m_gen3Wpm = m_qSettings->value("Gen3Wpm",40).toUInt();
	m_gen3Rise = m_qSettings->value("Gen3Rise",5).toUInt();
	m_gen3Text = m_qSettings->value("Gen3Text",0).toUInt();
	m_gen3Fade = m_qSettings->value("Gen3Fade",false).toBool();
	m_gen3Fist = m_qSettings->value("Gen3Fist",false).toBool();

	m_gen4Enabled = m_qSettings->value("Gen4Enabled",true).toBool();
	m_gen4Freq = m_qSettings->value("Gen4Freq",2500).toDouble();
	m_gen4Amp = m_qSettings->value("Gen4Amp",-50).toDouble();
	m_gen4Wpm = m_qSettings->value("Gen4Wpm",50).toUInt();
	m_gen4Rise = m_qSettings->value("Gen4Rise",5).toUInt();
	m_gen4Text = m_qSettings->value("Gen4Text",0).toUInt();
	m_gen4Fade = m_qSettings->value("Gen4Fade",false).toBool();
	m_gen4Fist = m_qSettings->value("Gen4Fist",false).toBool();

	m_gen5Enabled = m_qSettings->value("Gen5Enabled",true).toBool();
	m_gen5Freq = m_qSettings->value("Gen5Freq",2500).toDouble();
	m_gen5Amp = m_qSettings->value("Gen5Amp",-50).toDouble();
	m_gen5Wpm = m_qSettings->value("Gen5Wpm",60).toUInt();
	m_gen5Rise = m_qSettings->value("Gen5Rise",5).toUInt();
	m_gen5Text = m_qSettings->value("Gen5Text",0).toUInt();
	m_gen5Fade = m_qSettings->value("Gen5Fade",false).toBool();
	m_gen5Fist = m_qSettings->value("Gen5Fist",false).toBool();
}

void MorseGenDevice::writeSettings()
{
	DeviceInterfaceBase::writeSettings();

	m_qSettings->setValue("DbNoiseAmp", m_dbNoiseAmp);
	m_qSettings->setValue("Gen1Enabled", m_gen1Enabled);
	m_qSettings->setValue("Gen1Freq",m_gen1Freq);
	m_qSettings->setValue("Gen1Amp",m_gen1Amp);
	m_qSettings->setValue("Gen1Wpm",m_gen1Wpm);
	m_qSettings->setValue("Gen1Rise",m_gen1Rise);
	m_qSettings->setValue("Gen1Text",m_gen1Text);
	m_qSettings->setValue("Gen1Fade",m_gen1Fade);
	m_qSettings->setValue("Gen1Fist",m_gen1Fist);

	m_qSettings->setValue("Gen2Enabled", m_gen2Enabled);
	m_qSettings->setValue("Gen2Freq",m_gen2Freq);
	m_qSettings->setValue("Gen2Amp",m_gen2Amp);
	m_qSettings->setValue("Gen2Wpm",m_gen2Wpm);
	m_qSettings->setValue("Gen2Rise",m_gen2Rise);
	m_qSettings->setValue("Gen2Text",m_gen2Text);
	m_qSettings->setValue("Gen2Fade",m_gen2Fade);
	m_qSettings->setValue("Gen2Fist",m_gen2Fist);

	m_qSettings->setValue("Gen3Enabled", m_gen3Enabled);
	m_qSettings->setValue("Gen3Freq",m_gen3Freq);
	m_qSettings->setValue("Gen3Amp",m_gen3Amp);
	m_qSettings->setValue("Gen3Wpm",m_gen3Wpm);
	m_qSettings->setValue("Gen3Rise",m_gen3Rise);
	m_qSettings->setValue("Gen3Text",m_gen3Text);
	m_qSettings->setValue("Gen3Fade",m_gen3Fade);
	m_qSettings->setValue("Gen3Fist",m_gen3Fist);

	m_qSettings->setValue("Gen4Enabled", m_gen4Enabled);
	m_qSettings->setValue("Gen4Freq",m_gen4Freq);
	m_qSettings->setValue("Gen4Amp",m_gen4Amp);
	m_qSettings->setValue("Gen4Wpm",m_gen4Wpm);
	m_qSettings->setValue("Gen4Rise",m_gen4Rise);
	m_qSettings->setValue("Gen4Text",m_gen4Text);
	m_qSettings->setValue("Gen4Fade",m_gen4Fade);
	m_qSettings->setValue("Gen4Fist",m_gen4Fist);

	m_qSettings->setValue("Gen5Enabled", m_gen5Enabled);
	m_qSettings->setValue("Gen5Freq",m_gen5Freq);
	m_qSettings->setValue("Gen5Amp",m_gen5Amp);
	m_qSettings->setValue("Gen5Wpm",m_gen5Wpm);
	m_qSettings->setValue("Gen5Rise",m_gen5Rise);
	m_qSettings->setValue("Gen5Text",m_gen5Text);
	m_qSettings->setValue("Gen5Fade",m_gen5Fade);
	m_qSettings->setValue("Gen5Fist",m_gen5Fist);
}

void MorseGenDevice::updateGenerators()
{
	m_morseGen1->setParams(m_gen1Freq,m_gen1Amp,m_gen1Wpm,m_gen1Rise);
	m_morseGen1->setTextOut(m_sampleText[m_gen1Text]);
	m_morseGen2->setParams(m_gen2Freq,m_gen2Amp,m_gen2Wpm,m_gen2Rise);
	m_morseGen2->setTextOut(m_sampleText[m_gen2Text]);
	m_morseGen3->setParams(m_gen3Freq,m_gen3Amp,m_gen3Wpm,m_gen3Rise);
	m_morseGen3->setTextOut(m_sampleText[m_gen3Text]);
	m_morseGen4->setParams(m_gen4Freq,m_gen4Amp,m_gen4Wpm,m_gen4Rise);
	m_morseGen4->setTextOut(m_sampleText[m_gen4Text]);
	m_morseGen5->setParams(m_gen5Freq,m_gen5Amp,m_gen5Wpm,m_gen5Rise);
	m_morseGen5->setTextOut(m_sampleText[m_gen5Text]);
	//Because noise is averaged, fudget +30db so it matches with generator values
	//ie -30db gen and -30db noise should be 0snr
	m_noiseAmp = DB::dBToAmplitude(m_dbNoiseAmp+20);

}

void MorseGenDevice::updateFields()
{
	//!!! Move to slots that handle changes in real time
	m_gen1Freq = m_optionUi->freqencyEdit_1->text().toDouble();
	m_gen1Amp = m_optionUi->dbBox_1->currentData().toDouble();
	m_gen1Wpm = m_optionUi->wpmBox_1->currentData().toUInt();
	m_gen1Rise = 5; //Add to UI?
	m_gen1Text = m_optionUi->sourceBox_1->currentData().toUInt();

	m_gen2Freq = m_optionUi->freqencyEdit_2->text().toDouble();
	m_gen2Amp = m_optionUi->dbBox_2->currentData().toDouble();
	m_gen2Wpm = m_optionUi->wpmBox_2->currentData().toUInt();
	m_gen2Rise = 5; //Add to UI?
	m_gen2Text = m_optionUi->sourceBox_2->currentData().toUInt();

	m_gen3Freq = m_optionUi->freqencyEdit_3->text().toDouble();
	m_gen3Amp = m_optionUi->dbBox_3->currentData().toDouble();
	m_gen3Wpm = m_optionUi->wpmBox_3->currentData().toUInt();
	m_gen3Rise = 5; //Add to UI?
	m_gen3Text = m_optionUi->sourceBox_3->currentData().toUInt();

	m_gen4Freq = m_optionUi->freqencyEdit_4->text().toDouble();
	m_gen4Amp = m_optionUi->dbBox_4->currentData().toDouble();
	m_gen4Wpm = m_optionUi->wpmBox_4->currentData().toUInt();
	m_gen4Rise = 5; //Add to UI?
	m_gen4Text = m_optionUi->sourceBox_4->currentData().toUInt();

	m_gen5Freq = m_optionUi->freqencyEdit_5->text().toDouble();
	m_gen5Amp = m_optionUi->dbBox_5->currentData().toDouble();
	m_gen5Wpm = m_optionUi->wpmBox_5->currentData().toUInt();
	m_gen5Rise = 5; //Add to UI?
	m_gen5Text = m_optionUi->sourceBox_5->currentData().toUInt();

	m_dbNoiseAmp = m_optionUi->noiseBox->currentData().toDouble();

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

			updateGenerators();

			//How often do we need to read samples from files to get framesPerBuffer at sampleRate
			m_nsPerBuffer = (1000000000.0 / m_deviceSampleRate) * m_framesPerBuffer;
			//qDebug()<<"nsPerBuffer"<<nsPerBuffer;
			m_producerConsumer.Start(true,true);
			m_elapsedTimer.start();
			return true;

		case Cmd_Stop:
			DeviceInterfaceBase::stopDevice();
			//Device specific code follows
			m_producerConsumer.Stop();

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
			//Use QVariant::fromValue() to pass, and value<type passed>() to get back
			this->setupOptionUi(_arg.value<QWidget*>());
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
			return DeviceInterfaceBase::DT_IQ_DEVICE;
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
	qint64 nsRemaining = m_nsPerBuffer - m_elapsedTimer.nsecsElapsed();

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
			m_elapsedTimer.start(); //Restart elapsed timer

			if ((m_producerFreeBufPtr = (CPX*)m_producerConsumer.AcquireFreeBuffer()) == NULL)
				return;
			//Get data from device and put into producerFreeBufPtr
			generate(m_producerFreeBufPtr);
			//Return and wait for next producer time slice
			m_producerConsumer.ReleaseFilledBuffer();
			return;

			break;
		case cbProducerConsumerEvents::Stop:
			break;
	}
}

void MorseGenDevice::consumerWorker(cbProducerConsumerEvents _event)
{
	switch (_event) {
		case cbProducerConsumerEvents::Start:
			break;
		case cbProducerConsumerEvents::Run:
			//We always want to consume everything we have, producer will eventually block if we're not consuming fast enough
			while (m_producerConsumer.GetNumFilledBufs() > 0) {
				//Wait for data to be available from producer
				if ((m_consumerFilledBufPtr = (CPX*)m_producerConsumer.AcquireFilledBuffer()) == NULL) {
					//qDebug()<<"No filled buffer available";
					return;
				}

				//Process data in filled buffer and convert to Pebble format in consumerBuffer

				//perform.StartPerformance("ProcessIQ");
				processIQData(m_consumerFilledBufPtr,m_framesPerBuffer);
				//perform.StopPerformance(1000);
				//We don't release a free buffer until ProcessIQData returns because that would also allow inBuffer to be reused
				m_producerConsumer.ReleaseFreeBuffer();

			}
			break;
		case cbProducerConsumerEvents::Stop:
			break;
	}
}

void MorseGenDevice::setupOptionUi(QWidget *parent)
{
	if (m_optionUi != NULL)
		delete m_optionUi;

	m_optionUi = new Ui::MorseGenOptions();
	m_optionUi->setupUi(parent);
	parent->setVisible(true);

	m_optionUi->noiseBox->addItem("-40db", -40);
	m_optionUi->noiseBox->addItem("-45db", -45);
	m_optionUi->noiseBox->addItem("-50db", -50);
	m_optionUi->noiseBox->addItem("-55db", -55);
	m_optionUi->noiseBox->addItem("-60db", -60);
	m_optionUi->noiseBox->addItem("-60db", -65);
	m_optionUi->noiseBox->addItem("-60db", -70);
	m_optionUi->noiseBox->addItem("-60db", -75);
	m_optionUi->noiseBox->addItem("-60db", -80);
	m_optionUi->noiseBox->setCurrentText("-60db");

	m_optionUi->sourceBox_1->addItem("Sample1",0);
	m_optionUi->sourceBox_1->addItem("Sample2",1);
	m_optionUi->sourceBox_1->addItem("Sample3",2);
	m_optionUi->sourceBox_1->addItem("Sample4",3);
	m_optionUi->sourceBox_1->addItem("Table",4);

	m_optionUi->wpmBox_1->addItem("5 wpm", 5);
	m_optionUi->wpmBox_1->addItem("10 wpm", 10);
	m_optionUi->wpmBox_1->addItem("15 wpm", 15);
	m_optionUi->wpmBox_1->addItem("20 wpm", 20);
	m_optionUi->wpmBox_1->addItem("30 wpm", 30);
	m_optionUi->wpmBox_1->addItem("40 wpm", 40);
	m_optionUi->wpmBox_1->addItem("50 wpm", 50);
	m_optionUi->wpmBox_1->addItem("60 wpm", 60);
	m_optionUi->wpmBox_1->addItem("70 wpm", 70);
	m_optionUi->wpmBox_1->addItem("80 wpm", 80);

	m_optionUi->dbBox_1->addItem("-30db", -30);
	m_optionUi->dbBox_1->addItem("-35db", -35);
	m_optionUi->dbBox_1->addItem("-40db", -40);
	m_optionUi->dbBox_1->addItem("-45db", -45);
	m_optionUi->dbBox_1->addItem("-50db", -50);
	m_optionUi->dbBox_1->addItem("-55db", -55);
	m_optionUi->dbBox_1->addItem("-60db", -60);

	//2
	m_optionUi->sourceBox_2->addItem("Sample1",0);
	m_optionUi->sourceBox_2->addItem("Sample2",1);
	m_optionUi->sourceBox_2->addItem("Sample3",2);
	m_optionUi->sourceBox_2->addItem("Sample4",3);
	m_optionUi->sourceBox_2->addItem("Table",4);

	m_optionUi->wpmBox_2->addItem("5 wpm", 5);
	m_optionUi->wpmBox_2->addItem("10 wpm", 10);
	m_optionUi->wpmBox_2->addItem("15 wpm", 15);
	m_optionUi->wpmBox_2->addItem("20 wpm", 20);
	m_optionUi->wpmBox_2->addItem("30 wpm", 30);
	m_optionUi->wpmBox_2->addItem("40 wpm", 40);
	m_optionUi->wpmBox_2->addItem("50 wpm", 50);
	m_optionUi->wpmBox_2->addItem("60 wpm", 60);
	m_optionUi->wpmBox_2->addItem("70 wpm", 70);
	m_optionUi->wpmBox_2->addItem("80 wpm", 80);

	m_optionUi->dbBox_2->addItem("-30db", -30);
	m_optionUi->dbBox_2->addItem("-35db", -35);
	m_optionUi->dbBox_2->addItem("-40db", -40);
	m_optionUi->dbBox_2->addItem("-45db", -45);
	m_optionUi->dbBox_2->addItem("-50db", -50);
	m_optionUi->dbBox_2->addItem("-55db", -55);
	m_optionUi->dbBox_2->addItem("-60db", -60);

	//3
	m_optionUi->sourceBox_3->addItem("Sample1",0);
	m_optionUi->sourceBox_3->addItem("Sample2",1);
	m_optionUi->sourceBox_3->addItem("Sample3",2);
	m_optionUi->sourceBox_3->addItem("Sample4",3);
	m_optionUi->sourceBox_3->addItem("Table",4);

	m_optionUi->wpmBox_3->addItem("5 wpm", 5);
	m_optionUi->wpmBox_3->addItem("10 wpm", 10);
	m_optionUi->wpmBox_3->addItem("15 wpm", 15);
	m_optionUi->wpmBox_3->addItem("20 wpm", 20);
	m_optionUi->wpmBox_3->addItem("30 wpm", 30);
	m_optionUi->wpmBox_3->addItem("40 wpm", 40);
	m_optionUi->wpmBox_3->addItem("50 wpm", 50);
	m_optionUi->wpmBox_3->addItem("60 wpm", 60);
	m_optionUi->wpmBox_3->addItem("70 wpm", 70);
	m_optionUi->wpmBox_3->addItem("80 wpm", 80);

	m_optionUi->dbBox_3->addItem("-30db", -30);
	m_optionUi->dbBox_3->addItem("-35db", -35);
	m_optionUi->dbBox_3->addItem("-40db", -40);
	m_optionUi->dbBox_3->addItem("-45db", -45);
	m_optionUi->dbBox_3->addItem("-50db", -50);
	m_optionUi->dbBox_3->addItem("-55db", -55);
	m_optionUi->dbBox_3->addItem("-60db", -60);

	//4
	m_optionUi->sourceBox_4->addItem("Sample1",0);
	m_optionUi->sourceBox_4->addItem("Sample2",1);
	m_optionUi->sourceBox_4->addItem("Sample3",2);
	m_optionUi->sourceBox_4->addItem("Sample4",3);
	m_optionUi->sourceBox_4->addItem("Table",4);

	m_optionUi->wpmBox_4->addItem("5 wpm", 5);
	m_optionUi->wpmBox_4->addItem("10 wpm", 10);
	m_optionUi->wpmBox_4->addItem("15 wpm", 15);
	m_optionUi->wpmBox_4->addItem("20 wpm", 20);
	m_optionUi->wpmBox_4->addItem("30 wpm", 30);
	m_optionUi->wpmBox_4->addItem("40 wpm", 40);
	m_optionUi->wpmBox_4->addItem("50 wpm", 50);
	m_optionUi->wpmBox_4->addItem("60 wpm", 60);
	m_optionUi->wpmBox_4->addItem("70 wpm", 70);
	m_optionUi->wpmBox_4->addItem("80 wpm", 80);

	m_optionUi->dbBox_4->addItem("-30db", -30);
	m_optionUi->dbBox_4->addItem("-35db", -35);
	m_optionUi->dbBox_4->addItem("-40db", -40);
	m_optionUi->dbBox_4->addItem("-45db", -45);
	m_optionUi->dbBox_4->addItem("-50db", -50);
	m_optionUi->dbBox_4->addItem("-55db", -55);
	m_optionUi->dbBox_4->addItem("-60db", -60);

	//5
	m_optionUi->sourceBox_5->addItem("Sample1",0);
	m_optionUi->sourceBox_5->addItem("Sample2",1);
	m_optionUi->sourceBox_5->addItem("Sample3",2);
	m_optionUi->sourceBox_5->addItem("Sample4",3);
	m_optionUi->sourceBox_5->addItem("Table",4);

	m_optionUi->wpmBox_5->addItem("5 wpm", 5);
	m_optionUi->wpmBox_5->addItem("10 wpm", 10);
	m_optionUi->wpmBox_5->addItem("15 wpm", 15);
	m_optionUi->wpmBox_5->addItem("20 wpm", 20);
	m_optionUi->wpmBox_5->addItem("30 wpm", 30);
	m_optionUi->wpmBox_5->addItem("40 wpm", 40);
	m_optionUi->wpmBox_5->addItem("50 wpm", 50);
	m_optionUi->wpmBox_5->addItem("60 wpm", 60);
	m_optionUi->wpmBox_5->addItem("70 wpm", 70);
	m_optionUi->wpmBox_5->addItem("80 wpm", 80);

	m_optionUi->dbBox_5->addItem("-30db", -30);
	m_optionUi->dbBox_5->addItem("-35db", -35);
	m_optionUi->dbBox_5->addItem("-40db", -40);
	m_optionUi->dbBox_5->addItem("-45db", -45);
	m_optionUi->dbBox_5->addItem("-50db", -50);
	m_optionUi->dbBox_5->addItem("-55db", -55);
	m_optionUi->dbBox_5->addItem("-60db", -60);

	connect(m_optionUi->resetButton,SIGNAL(clicked(bool)),this,SLOT(resetButtonClicked(bool)));

	resetButtonClicked(true);
}

void MorseGenDevice::resetButtonClicked(bool clicked)
{
	Q_UNUSED(clicked);

	m_optionUi->enabledBox_1->setChecked(true);
	m_optionUi->freqencyEdit_1->setText("1000");
	m_optionUi->sourceBox_1->setCurrentIndex(0);
	m_optionUi->wpmBox_1->setCurrentIndex(3);
	m_optionUi->dbBox_1->setCurrentIndex(2);

	m_optionUi->enabledBox_2->setChecked(true);
	m_optionUi->freqencyEdit_2->setText("1500");
	m_optionUi->sourceBox_2->setCurrentIndex(1);
	m_optionUi->wpmBox_2->setCurrentIndex(4);
	m_optionUi->dbBox_2->setCurrentIndex(2);

	m_optionUi->enabledBox_3->setChecked(true);
	m_optionUi->freqencyEdit_3->setText("2000");
	m_optionUi->sourceBox_3->setCurrentIndex(2);
	m_optionUi->wpmBox_3->setCurrentIndex(5);
	m_optionUi->dbBox_3->setCurrentIndex(2);

	m_optionUi->enabledBox_4->setChecked(true);
	m_optionUi->freqencyEdit_4->setText("2500");
	m_optionUi->sourceBox_4->setCurrentIndex(3);
	m_optionUi->wpmBox_4->setCurrentIndex(6);
	m_optionUi->dbBox_4->setCurrentIndex(2);

	m_optionUi->enabledBox_5->setChecked(true);
	m_optionUi->freqencyEdit_5->setText("3000");
	m_optionUi->sourceBox_5->setCurrentIndex(4);
	m_optionUi->wpmBox_5->setCurrentIndex(7);
	m_optionUi->dbBox_5->setCurrentIndex(2);

}

void MorseGenDevice::generate(CPX *out)
{
	CPX cpx1, cpx2, cpx3, cpx4, cpx5, cpx6;

	for (quint32 i=0; i<m_framesPerBuffer; i++) {
		if (m_gen1Enabled) {
			cpx1 = m_morseGen1->nextOutputSample();
			if (m_gen1Fade) {
				//Fading
				double dbRand = -rand() % c_dbFadeRange;
				double ampRand = DB::dBToAmplitude(dbRand);
				cpx1 *= ampRand;
			}
		}
		if (m_gen2Enabled) {
			cpx2 = m_morseGen2->nextOutputSample();
			if (m_gen2Fade) {
				//Fading
				double dbRand = -rand() % c_dbFadeRange;
				double ampRand = DB::dBToAmplitude(dbRand);
				cpx2 *= ampRand;
			}
		}
		if (m_gen3Enabled) {
			cpx3 = m_morseGen3->nextOutputSample();
			if (m_gen3Fade) {
				//Fading
				double dbRand = -rand() % c_dbFadeRange;
				double ampRand = DB::dBToAmplitude(dbRand);
				cpx3 *= ampRand;
			}
		}
		if (m_gen4Enabled) {
			cpx4 = m_morseGen4->nextOutputSample();
			if (m_gen4Fade) {
				//Fading
				double dbRand = -rand() % c_dbFadeRange;
				double ampRand = DB::dBToAmplitude(dbRand);
				cpx4 *= ampRand;
			}
		}
		if (m_gen5Enabled) {
			cpx5 = m_morseGen5->nextOutputSample();
			if (m_gen5Fade) {
				//Fading
				double dbRand = -rand() % c_dbFadeRange;
				double ampRand = DB::dBToAmplitude(dbRand);
				cpx5 *= ampRand;
			}
		}
		cpx6 = nextNoiseSample(m_noiseAmp);
		out[i] = cpx1 + cpx2 + cpx3 + cpx4 + cpx5 + cpx6;
	}
}

//Derived from nco.cpp
//Algorithm from http://dspguru.com/dsp/howtos/how-to-generate-white-gaussian-noise
//Uses Box-Muller Method, optimized to Polar Method [Ros88] A First Course on Probability
//Also http://www.dspguide.com/ch2/6.htm
//https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform R Knop algorithm
CPX MorseGenDevice::nextNoiseSample(double _dbGain)
{
	double u1;
	double u2;
	double s;
	double rad;
	double x;
	double y;

	//R Knop
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
	return CPX(x,y);
}