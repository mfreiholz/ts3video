#ifndef STARTUPLOGIC_H
#define STARTUPLOGIC_H

class QApplication;
class QString;


class AbstractStartupLogic
{
	QApplication* _qapp;

public:
	AbstractStartupLogic(QApplication* a);
	virtual ~AbstractStartupLogic();
	QApplication* qapp() const;
	QString configFilePath() const;
	virtual int exec();

private:
	void initLogging();
	void initStyleSheet();
};


#endif