#include "PlayerScreen.h"
#include <iostream>
#include "common.h"


void VFrame::mouseDoubleClickEvent(QMouseEvent* e)
{
	QFrame::mouseDoubleClickEvent(e);
	emit doubleClicked();
}

PlayerScreen::PlayerScreen(): QMainWindow()
{
	refresh_configs();
	this->setWindowIcon(QIcon(":/nwvlclog.png"));
	this->setWindowTitle("NWVLC Player");
	
	cwidget = new QWidget(this);
	this->setCentralWidget(cwidget);

	cvboxlayout = new QVBoxLayout();
	cwidget->setLayout(cvboxlayout);
	
	vinstance = VLC::Instance(0, nullptr);
	vmediaplayer = VLC::MediaPlayer(vinstance);
	vmediaplayer.setKeyInput(false);
	vmediaplayer.setMouseInput(false);

	connect(this, &PlayerScreen::edit_controls, this, &PlayerScreen::edit_control_states);

	create_ui();
}

void PlayerScreen::refresh_configs()
{
	n_lock.lock();
	try {
		json sjson;
		sjson["media_name"] = media_name;
		sjson["user"] = user_name;
		sjson["current_ts"] = 0;
		sjson["action"] = "stop";
		cpr::Response res = cpr::Post(
			cpr::Url{ nw_url },
			cpr::Body{ sjson.dump()},
			cpr::Header{ {"Content-Type", "application/json"} }
		);
	}
	catch (int err) {
		//std::cout << err << std::endl;
	}

	json rjson = read_config_njson();
	nw_url = rjson["url"];
	media_name = rjson["media_name"];
	user_name = rjson["uname"];
	nw_poll_interval = rjson["nwpoll_interval_ms"];

	if (is_url_invalid) {
		cvboxlayout->removeWidget(etextview);
		cvboxlayout->removeWidget(settings_btn);
		create_ui();
	}
	n_lock.unlock();
}

void PlayerScreen::create_ui()
{
	if (settings_ui == nullptr) settings_ui = new SettingsPage(this);

	if (settings_btn == nullptr) {
		settings_btn = new QPushButton("Settings");
		connect(settings_btn, &QPushButton::clicked, this, &PlayerScreen::create_settings_ui);
	}

	if (!check_nurl_valid(nw_url)) {
		is_url_invalid = true;
		etextview = new QLabel();
		etextview->setAlignment(Qt::AlignmentFlag::AlignCenter);
		etextview->setText(("Cannot reach the URL " + nw_url).c_str());
		cvboxlayout->addWidget(etextview);
		cvboxlayout->addWidget(settings_btn);
		return;
	}

	is_url_invalid = false;
	vframe = new VFrame();
	vpalette = vframe->palette();
	vpalette.setColor(QPalette::ColorRole::Window, QColor(0, 0, 0));
	vframe->setPalette(vpalette);
	vframe->setAutoFillBackground(true);
	connect(vframe, &VFrame::doubleClicked, this, &PlayerScreen::toggle_fscreen);

	pos_slider = new QSlider(Qt::Orientation::Horizontal, this);
	pos_slider->setToolTip("Position");
	pos_slider->setMaximum(2147483647);
	connect(pos_slider, &QSlider::sliderMoved, this, &PlayerScreen::set_position);
	connect(pos_slider, &QSlider::sliderPressed, this, &PlayerScreen::set_position);

	btn_box = new QHBoxLayout();

	play_btn = new QPushButton("Play");
	connect(play_btn, &QPushButton::clicked, this, &PlayerScreen::vplay_pause);
	btn_box->addWidget(play_btn);

	stop_btn = new QPushButton("Stop");
	connect(stop_btn, &QPushButton::clicked, this, &PlayerScreen::vstop);
	btn_box->addWidget(stop_btn);

	btn_box->addWidget(settings_btn);

	btn_box->addStretch(1);

	vol_slider = new QSlider(Qt::Orientation::Horizontal, this);
	vol_slider->setToolTip("Volume");
	vol_slider->setMaximum(100);
	vol_slider->setValue(vmediaplayer.volume());
	connect(vol_slider, &QSlider::valueChanged, this, &PlayerScreen::set_volume);
	btn_box->addWidget(vol_slider);

	cvboxlayout->setContentsMargins(5, 5, 5, 5);
	cvboxlayout->addWidget(vframe);
	cvboxlayout->addWidget(pos_slider);
	cvboxlayout->addLayout(btn_box);

	menubar = menuBar();

	file_menu = menubar->addMenu("File");
	aud_menu = menubar->addMenu("Audio");
	sub_menu = menubar->addMenu("Subtitles");

	at_group = new QActionGroup(this);
	st_group = new QActionGroup(this);

	aud_menu->menuAction()->setVisible(false);
	sub_menu->menuAction()->setVisible(false);

	load_act = new QAction("Load Video", this);
	quit_act = new QAction("Close App", this);
	connect(load_act, &QAction::triggered, this, &PlayerScreen::open_file);
	connect(quit_act, &QAction::triggered, this, &PlayerScreen::close);
	file_menu->addAction(load_act);
	file_menu->addAction(quit_act);

	u_timer = new QTimer();
	u_timer->setInterval(100);
	connect(u_timer, &QTimer::timeout, this, &PlayerScreen::spawn_uui_thread,Qt::DirectConnection);

	n_timer = new QTimer();
	n_timer->setInterval(nw_poll_interval);
	connect(n_timer, &QTimer::timeout, this, &PlayerScreen::spawn_nthread, Qt::DirectConnection);
}

void PlayerScreen::toggle_fscreen()
{
	if (isFullScreen()) {
		cvboxlayout->setContentsMargins(5, 5, 5, 5);
		showNormal();
		menubar->show();
		pos_slider->show();
		vol_slider->show();
		stop_btn->show();
		play_btn->show();
		settings_btn->show();
	}
	else {
		menubar->hide();
		pos_slider->hide();
		vol_slider->hide();
		stop_btn->hide();
		play_btn->hide();
		settings_btn->hide();
		cvboxlayout->setContentsMargins(0, 0, 0, 0);
		showFullScreen();
	}
}

void PlayerScreen::refresh_aud_sub_tracks()
{
	if (vmedias.size() == 0) return;

	while (aud_tracks.size() == 0 || sub_tracks.size() == 0) {
		if (vmediaplayer.isPlaying()) {
			// Add Sub track options
			if (vmediaplayer.spuCount() > 0 && sub_tracks.size() == 0) {
				sub_tracks = vmediaplayer.spuDescription();
				int selected_sub_track = vmediaplayer.spu();
				sub_menu->clear();
				st_acts.clear();
				for (VLC::TrackDescription st : sub_tracks) {
					QAction* mitem = new QAction(st.name().c_str(), st_group);
					mitem->setCheckable(true);
					mitem->setData(st.id());
					sub_menu->addAction(mitem);
					st_acts.push_back(mitem);
					if (st.id() == selected_sub_track) mitem->setChecked(true);
					connect(mitem, &QAction::triggered, this, &PlayerScreen::set_sub_track);
				}
				sub_menu->menuAction()->setVisible(true);
			}
			// Add Sub track options
			if (vmediaplayer.audioTrackCount() > 0 && aud_tracks.size() == 0) {
				aud_tracks = vmediaplayer.audioTrackDescription();
				int selected_aud_track = vmediaplayer.audioTrack();
				aud_menu->clear();
				at_acts.clear();
				for (VLC::TrackDescription at : aud_tracks) {
					QAction* mitem = new QAction(at.name().c_str(), at_group);
					mitem->setCheckable(true);
					mitem->setData(at.id());
					aud_menu->addAction(mitem);
					at_acts.push_back(mitem);
					if (at.id() == selected_aud_track) mitem->setChecked(true);
					connect(mitem, &QAction::triggered, this, &PlayerScreen::set_aud_track);
				}
				aud_menu->menuAction()->setVisible(true);
			}
			load_subtitle_action = new QAction("Load Subtitle", st_group);
			sub_menu->addAction(load_subtitle_action);
			connect(load_subtitle_action, &QAction::triggered, this, &PlayerScreen::open_subtitle_file);
		}
	}
}

void PlayerScreen::open_file()
{
	QString fdout = QFileDialog::getOpenFileName(this, "Choose Media File", ".");
	std::string filename = fdout.toStdString();

	if (filename == "" || !std::filesystem::exists(filename)) {
		play_btn->setEnabled(true);
		stop_btn->setEnabled(true);
		pos_slider->setEnabled(true);
		return;
	}

	std::replace(filename.begin(), filename.end(), '/', '\\');
	VLC::Media vmed = VLC::Media(vinstance, filename, VLC::Media::FromType::FromPath);
	vmedias.push_back(vmed);

	vmed.parseWithOptions(VLC::Media::ParseFlags::Local, 10000);

	VLC::Media::ParsedStatus pstat = vmed.parsedStatus();
	while (pstat != VLC::Media::ParsedStatus::Timeout && pstat != VLC::Media::ParsedStatus::Done && pstat != VLC::Media::ParsedStatus::Failed) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pstat = vmed.parsedStatus();
	}

	if (pstat != VLC::Media::ParsedStatus::Done) {
		vmedias.pop_back();
	}

	vmediaplayer.setMedia(vmed);
	this->setWindowTitle(QString(vmed.meta(libvlc_meta_Title).c_str()));

	vmediaplayer.setHwnd((void*)vframe->winId());

	vplay_pause();

	aud_tracks.clear();
	sub_tracks.clear();

	refresh_aud_sub_tracks();

	action = "none";

	should_stop_u = false;
	u_timer->start();

	play_btn->setEnabled(false);
	stop_btn->setEnabled(false);
	pos_slider->setEnabled(false);

	should_stop_n = false;
	n_timer->start();
}

void PlayerScreen::locked_vplay_pause()
{
	n_lock.lock();
	play_btn->setEnabled(false);
	stop_btn->setEnabled(false);
	pos_slider->setEnabled(false);
	vplay_pause();
	n_lock.unlock();
}

void PlayerScreen::vplay_pause()
{
	if (vmedias.size() == 0) {
		open_file();
		return;
	}
	if (vmediaplayer.isPlaying()) {
		vmediaplayer.pause();
		play_btn->setText("Play");
		is_vpaused = true;
		action = "pause";
	}
	else {
		vmediaplayer.play();
		play_btn->setText("Pause");
		is_vpaused = false;
		action = 'play';
	}
}

void PlayerScreen::vstop() {
	vmediaplayer.stop();
	vmedias.clear();
	aud_tracks.clear();
	sub_tracks.clear();
	if (aud_menu != nullptr) aud_menu->menuAction()->setVisible(false);
	if (sub_menu != nullptr) sub_menu->menuAction()->setVisible(false);
	action = "stop";
	should_stop_u = true;
	should_stop_n = true;
	if (play_btn != nullptr) play_btn->setText("Play");
}

void PlayerScreen::create_settings_ui()
{
	settings_ui->show();
}

void PlayerScreen::set_volume(int volume) {
	vmediaplayer.setVolume(volume);
}

void PlayerScreen::set_position() {
	u_lock.lock();
	int curr_pos = pos_slider->value();
	vmediaplayer.setPosition(float(curr_pos) / 2147483647.0);
	u_lock.unlock();
}

void PlayerScreen::set_sub_track()
{
	vmediaplayer.setSpu(st_group->checkedAction()->data().toInt());
}

void PlayerScreen::set_aud_track()
{
	vmediaplayer.setAudioTrack(at_group->checkedAction()->data().toInt());
}

void PlayerScreen::open_subtitle_file()
{
	QString fdout = QFileDialog::getOpenFileName(this, "Choose Subtitle File", ".", { "Subtitle file (*.srt)" });
	std::string filename = fdout.toStdString();

	if (filename == "" || !std::filesystem::exists(filename)) {
		return;
	}
	std::replace(filename.begin(), filename.end(), '/', '\\');
	vmediaplayer.addSlave(VLC::MediaSlave::Type::Subtitle, filename, true);
}

void PlayerScreen::spawn_uui_thread() {
	if (should_stop_u) {
		last_uthread = new std::thread(&PlayerScreen::update_ui, this);
		u_timer->stop();
	}
	else {
		std::thread uthread = std::thread(&PlayerScreen::update_ui, this);
		uthread.detach();
	}
}

void PlayerScreen::update_ui()
{
	u_lock.lock();
	if (vmediaplayer.isValid() && (vmediaplayer.isPlaying() || is_vpaused)) {
		int media_pos = int(vmediaplayer.position() * 2147483647);
		pos_slider->setValue(media_pos);
	}
	u_lock.unlock();

	n_lock.lock();
	for (NWResponse ev : action_queue) {
		execute_action(ev);
	}
	action_queue.clear();
	n_lock.unlock();

	u_lock.lock();
	if (vmediaplayer.isValid() && !vmediaplayer.isPlaying()) {
		if (!is_vpaused) {
			vstop();
		}
	}
	u_lock.unlock();
}

void PlayerScreen::edit_control_states(bool state)
{
	play_btn->setEnabled(state);
	stop_btn->setEnabled(state);
	pos_slider->setEnabled(state);
}

void PlayerScreen::execute_action(NWResponse sdata)
{
	try {
		emit edit_controls(sdata.synced);

		if (sdata.action == "play") {
			//std::cout << "signal recieved to play at: " << sdata.current_ts;
			if (!vmediaplayer.isPlaying()) {
				vplay_pause();
				action = "none";
			}
			u_lock.lock();
			pos_slider->setValue(sdata.current_ts);
			u_lock.unlock();
			set_position();
			return;
		}
		if (sdata.action == "pause") {
			//std::cout << "signal recieved to pause at: " << sdata.current_ts;
			if (vmediaplayer.isPlaying()) {
				vplay_pause();
				action = "none";
			}
			u_lock.lock();
			pos_slider->setValue(sdata.current_ts);
			u_lock.unlock();
			set_position();
			return;
		}
		if (sdata.action == "seek") {
			//std::cout << "signal recieved to seek at: " << sdata.current_ts;
			u_lock.lock();
			pos_slider->setValue(sdata.current_ts);
			u_lock.unlock();
			set_position();
			return;
		}
	}
	catch (int eno) {
		//std::cout << eno << std::endl;
	}
}

void PlayerScreen::spawn_nthread()
{
	if (should_stop_n) {		
		last_nthread = new std::thread(&PlayerScreen::send_status, this);
		n_timer->stop();
	}
	else {
		std::thread nthread = std::thread(&PlayerScreen::send_status, this);
		nthread.detach();
	}
}

void PlayerScreen::send_status()
{
	n_lock.lock();
	json sjson;
	sjson["media_name"] = media_name;
	sjson["user"] = user_name;
	if (pos_slider != nullptr) sjson["current_ts"] = pos_slider->value();
	else sjson["current_ts"] = 0;
	sjson["action"] = action;
	try {
		std::string lol = sjson.dump();
		cpr::Response res = cpr::Post(
			cpr::Url{ nw_url },
			cpr::Header{ {"Content-Type", "application/json"} },
			cpr::Body{ lol }
		);
		json resjson = json::parse(res.text);
		NWResponse tmp;
		tmp.action = resjson["action"].get<std::string>();
		tmp.current_ts = resjson["current_ts"].get<int>();
		tmp.synced = resjson["synced"].get<bool>();
		action_queue.push_back(tmp);
		action = "none";
	}
	catch (int eno) {
		//std::cout << eno << std::endl;
	}
	n_lock.unlock();
}

PlayerScreen::~PlayerScreen()
{
	if (!is_url_invalid) vstop();
	
	if (etextview == nullptr){ delete etextview; }
	
	if (!is_url_invalid) {
		if (last_nthread != nullptr && last_nthread->joinable()) {
			last_nthread->join();
			delete last_nthread;
		}
		if (last_uthread != nullptr && last_uthread->joinable()) {
			last_uthread->join();
			delete last_uthread;
		}
		// Send stop cmd once more just in case
		action = "stop";
		should_stop_u = true;
		should_stop_n = true;
		spawn_uui_thread();
		spawn_nthread();
		if (last_nthread->joinable()) {
			last_nthread->join();
			delete last_nthread;
		}
		if (last_uthread->joinable()) {
			last_uthread->join();
			delete last_uthread;
		}

		delete pos_slider;
		delete play_btn;
		delete stop_btn;
		delete settings_btn;
		delete btn_box;
		vmediaplayer.~MediaPlayer();
		vinstance.~Instance();
		delete vframe;
		delete cvboxlayout;
		delete cwidget;
	}
}