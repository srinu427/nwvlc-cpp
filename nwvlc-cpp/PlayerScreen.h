#pragma once

#include <thread>
#include <chrono>
#include <mutex>
#include <QtWidgets>
#include <vlcpp/vlc.hpp>
#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include "SettingsPage.h"

using json = nlohmann::json;

#ifndef DOUBLECLICKEDWIDGET_H
#define DOUBLECLICKEDWIDGET_H
class VFrame : public QFrame {
	Q_OBJECT
signals:
	void doubleClicked();
protected:
	void mouseDoubleClickEvent(QMouseEvent* e);
};
#endif

struct NWResponse {
	std::string action;
	int current_ts;
	bool synced;
};

class PlayerScreen: public QMainWindow
{
	Q_OBJECT
public:
	PlayerScreen();
	~PlayerScreen();

	void refresh_configs();
signals:
	void edit_controls(bool state);

private:
	std::string media_name = "ex_media_name";
	std::string user_name = "ex_user_name";
	std::string nw_url = "http://127.0.0.1:4270/poll_status";
	int nw_poll_interval = 0;
	bool is_url_invalid = false;

	QTimer* u_timer = nullptr;
	bool should_stop_u = false;
	std::mutex u_lock;
	std::thread* last_uthread = nullptr;

	QTimer* n_timer = nullptr;
	bool should_stop_n = false;
	std::mutex n_lock;
	std::thread* last_nthread = nullptr;
	
	bool is_vpaused = false;

	std::string action = "none";
	std::vector<NWResponse> action_queue;

	SettingsPage* settings_ui = nullptr;

	VLC::Instance vinstance;
	VLC::MediaPlayer vmediaplayer;
	std::vector<VLC::Media> vmedias;

	std::vector<VLC::TrackDescription> sub_tracks;
	std::vector<VLC::TrackDescription> aud_tracks;

	QWidget* cwidget = nullptr;
	QVBoxLayout* cvboxlayout = nullptr;
	QMenuBar* menubar = nullptr;

	QLabel* etextview = nullptr;

	QMenu* file_menu = nullptr;
	QMenu* aud_menu = nullptr;
	QMenu* sub_menu = nullptr;

	QAction* load_act = nullptr;
	QAction* quit_act = nullptr;

	QActionGroup* st_group = nullptr;
	QActionGroup* at_group = nullptr;

	std::vector<QAction*> st_acts;
	std::vector<QAction*> at_acts;
	QAction* load_subtitle_action = nullptr;

	VFrame* vframe = nullptr;
	QPalette vpalette;

	QSlider* pos_slider = nullptr;

	QHBoxLayout* btn_box = nullptr;
	
	QPushButton* play_btn = nullptr;
	QPushButton* stop_btn = nullptr;
	QPushButton* settings_btn = nullptr;

	QSlider* vol_slider = nullptr;

	void create_ui();
	void toggle_fscreen();
	void refresh_aud_sub_tracks();
	void open_file();
	void locked_vplay_pause();
	void vplay_pause();
	void vstop();
	void create_settings_ui();
	void set_volume(int volume);
	void set_position();
	void set_sub_track();
	void set_aud_track();
	void open_subtitle_file();

	void spawn_uui_thread();
	void update_ui();
	void edit_control_states(bool state);
	void execute_action(NWResponse sdata);

	void spawn_nthread();
	void send_status();
};

