#include "SettingsPage.h"
#include "PlayerScreen.h"
#include "common.h"


SettingsPage::SettingsPage(QMainWindow* pwindow): QMainWindow()
{
	setWindowTitle("Settings");
	setWindowFlags(windowFlags() | Qt::WindowType::CustomizeWindowHint);
	setWindowFlags(windowFlags() & ~Qt::WindowType::WindowMaximizeButtonHint);
	setMinimumSize(500, 200);
	setMaximumSize(800, 200);
	parent_window = pwindow;

	cwidget = new QWidget(this);
	setCentralWidget(cwidget);

	cvboxlayout = new QVBoxLayout();

	create_ui();
}

void SettingsPage::show()
{
	json cjdata = read_config_njson();
	mname_box->setText(cjdata["media_name"].get<std::string>().c_str());
	uname_box->setText(cjdata["uname"].get<std::string>().c_str());
	url_box->setText(cjdata["url"].get<std::string>().c_str());
	poll_box->setValue(cjdata["nwpoll_interval_ms"].get<int>());
	QMainWindow::show();
}

void SettingsPage::create_ui()
{
	cvboxlayout->addStretch(1);

	mname_txt = new QLabel("Media Name");
	mname_box = new QLineEdit();
	mname_lay = new QHBoxLayout();
	mname_lay->addWidget(mname_txt, 2);
	mname_lay->addStretch(1);
	mname_lay->addWidget(mname_box, 7);
	cvboxlayout->addLayout(mname_lay);

	uname_txt = new QLabel("User Name");
	uname_box = new QLineEdit();
	uname_lay = new QHBoxLayout();
	uname_lay->addWidget(uname_txt, 2);
	uname_lay->addStretch(1);
	uname_lay->addWidget(uname_box, 7);
	cvboxlayout->addLayout(uname_lay);

	url_txt = new QLabel("Server URL");
	url_box = new QLineEdit();
	url_lay = new QHBoxLayout();
	url_lay->addWidget(url_txt, 2);
	url_lay->addStretch(1);
	url_lay->addWidget(url_box, 7);
	cvboxlayout->addLayout(url_lay);

	poll_txt = new QLabel("Poll Interval");
	poll_box = new QSpinBox();
	poll_box->setMinimum(500);
	poll_box->setMaximum(2000);
	poll_lay = new QHBoxLayout();
	poll_lay->addWidget(poll_txt, 2);
	poll_lay->addStretch(6);
	poll_lay->addWidget(poll_box, 2);
	cvboxlayout->addLayout(poll_lay);

	ok_btn = new QPushButton("Ok");
	connect(ok_btn, &QPushButton::clicked, this, &SettingsPage::apply_settings);
	ok_txt = new QLabel("");
	ok_lay = new QHBoxLayout();
	ok_lay->addWidget(ok_btn, 2);
	ok_lay->addWidget(ok_txt, 8);
	cvboxlayout->addLayout(ok_lay);
	cvboxlayout->addStretch(1);
	cwidget->setLayout(cvboxlayout);
}

void SettingsPage::apply_settings()
{
	std::string nurl = url_box->text().toStdString();
	std::string mname = mname_box->text().toStdString();
	std::string uname = uname_box->text().toStdString();
	int nw_poll_interval = poll_box->value();

	if (mname.length() < 4 or uname.length() < 4) {
		ok_txt->setText("ERROR: Media name/User name should be atleast 4 char long");
		return;
	}
	if (!check_nurl_valid(nurl)) {
		ok_txt->setText("ERROR: Invalid URL given");
		return;
	}

	ok_txt->setText("");

	write_config_njson(nurl, mname, uname, nw_poll_interval);
	((PlayerScreen*)parent_window)->refresh_configs();
	hide();
}

SettingsPage::~SettingsPage()
{
	delete cwidget;
	delete cvboxlayout;

	delete mname_txt;
	delete mname_box;
	delete mname_lay;

	delete uname_txt;
	delete uname_box;
	delete uname_lay;

	delete url_txt;
	delete url_box;
	delete url_lay;

	delete poll_txt;
	delete poll_box;
	delete poll_lay;

	delete ok_btn;
	delete ok_txt;
	delete ok_lay;
}