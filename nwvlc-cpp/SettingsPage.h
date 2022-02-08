#pragma once
#include <QtCore>
#include <QtWidgets>

class SettingsPage: public QMainWindow
{
public:
	SettingsPage(QMainWindow* pwindow);
	~SettingsPage();

	void show();
private:
	QMainWindow* parent_window = nullptr;
	QWidget* cwidget = nullptr;
	QVBoxLayout* cvboxlayout = nullptr;

	QLabel* mname_txt = nullptr;
	QLineEdit* mname_box = nullptr;
	QHBoxLayout* mname_lay = nullptr;

	QLabel* uname_txt = nullptr;
	QLineEdit* uname_box = nullptr;
	QHBoxLayout* uname_lay = nullptr;

	QLabel* url_txt = nullptr;
	QLineEdit* url_box = nullptr;
	QHBoxLayout* url_lay = nullptr;

	QLabel* poll_txt = nullptr;
	QSpinBox* poll_box = nullptr;
	QHBoxLayout* poll_lay = nullptr;

	QPushButton* ok_btn = nullptr;
	QLabel* ok_txt = nullptr;
	QHBoxLayout* ok_lay = nullptr;

	void create_ui();
	void apply_settings();
};

