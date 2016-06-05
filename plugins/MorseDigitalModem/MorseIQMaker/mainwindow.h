#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "wavfile.h"
#include "morsegen.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void setupUi();
private slots:
	void generateButtonClicked(bool clicked);
	void resetButtonClicked(bool Clicked);

private:
	Ui::MainWindow *ui;

	static const quint32 c_dbFadeRange = 20; //For random fade generation
	quint32 m_sampleRate;

	QString m_outFileName;
	WavFile m_wavOutFile;
	CPX *m_outBuf;

	CPX *m_outBuf1;
	CPX *m_outBuf2;
	CPX *m_outBuf3;
	CPX *m_outBuf4;
	CPX *m_outBuf5;

	MorseGen *m_morseGen1;
	MorseGen *m_morseGen2;
	MorseGen *m_morseGen3;
	MorseGen *m_morseGen4;
	MorseGen *m_morseGen5;

	QString m_sampleText[5];
	CPX nextNoiseSample(double _dbGain);
};

#endif // MAINWINDOW_H