//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include "audio.h"
#include "audiopa.h"
#include "audioqt.h"
#include "QDebug"

Audio::Audio(QObject *parent) : QObject(parent)
{
}

Audio::~Audio()
{
}

Audio *Audio::Factory(CB_AudioProducer cb, quint16 framesPerBuffer)
{
#ifdef USE_QT_AUDIO
	qDebug()<<"Using QTAudio";
	return new AudioQT(cb, framesPerBuffer);
#endif
#ifdef USE_PORT_AUDIO
	qDebug()<<"Using PortAudio";
	return new AudioPA(cb, framesPerBuffer);
#endif
	qDebug()<<"Unknown audio configuration";
	return NULL;
}


QStringList Audio::InputDeviceList()
{
#ifdef USE_QT_AUDIO
		return AudioQT::InputDeviceList();
#endif
#ifdef USE_PORT_AUDIO
		return AudioPA::InputDeviceList();
#endif
	qDebug()<<"Unknown audio configuration";
	return QStringList();
}
QStringList Audio::OutputDeviceList()
{
#ifdef USE_QT_AUDIO
		return AudioQT::OutputDeviceList();
#endif
#ifdef USE_PORT_AUDIO
		return AudioPA::OutputDeviceList();
#endif
	qDebug()<<"Unknown audio configuration";
	return QStringList();
}

void Audio::OutputTimedOut()
{
	hasOutputTimedOut = true;
}

