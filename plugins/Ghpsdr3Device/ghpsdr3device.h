#ifndef GHPSDR3DEVICE_H
#define GHPSDR3DEVICE_H

//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include <QObject>
#include "deviceinterfacebase.h"
#include "producerconsumer.h"

class Ghpsdr3Device : public QObject, public DeviceInterfaceBase
{
	Q_OBJECT

	//Exports, FILE is optional
	//IID must be same that caller is looking for, defined in interfaces file
	Q_PLUGIN_METADATA(IID DeviceInterface_iid)
	//Let Qt meta-object know about our interface
	Q_INTERFACES(DeviceInterface)

public:
	Ghpsdr3Device();
	~Ghpsdr3Device();

	//Required
	bool Initialize(cbProcessIQData _callback,
					cbProcessBandscopeData _callbackSpectrum,
					cbProcessAudioData _callbackAudio,
					quint16 _framesPerBuffer);
	bool Connect();
	bool Disconnect();
	void Start();
	void Stop();
	void ReadSettings();
	void WriteSettings();
	QVariant Get(STANDARD_KEYS _key, quint16 _option = 0);
	bool Set(STANDARD_KEYS _key, QVariant _value, quint16 _option = 0);
	//Display device option widget in settings dialog
	void SetupOptionUi(QWidget *parent);

private:
	void producerWorker(cbProducerConsumerEvents _event);
	void consumerWorker(cbProducerConsumerEvents _event);
	ProducerConsumer producerConsumer;

};
#endif // GHPSDR3DEVICE_H