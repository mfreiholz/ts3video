#ifndef STARTUPLOGIC_H
#define STARTUPLOGIC_H

class QApplication;
class QString;


class AbstractStartupLogic
{
public:
	AbstractStartupLogic(QApplication* a);
	virtual ~AbstractStartupLogic();
	QApplication* qapp() const;
	QString configFilePath() const;
	virtual int exec();

private:
	void initApplication();
	void initLogging();
	void initStyleSheet();

private:
	QApplication* _qapp;
};


#endif