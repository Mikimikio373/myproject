#pragma once
#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Affine.hpp"
#if _WIN64
#include <Poco/Format.h>
#endif

/// hitの情報
class TrackHit {
public:
	int16_t px;  // X座標 7層目(0はじまり)が基準
	int16_t py;  // Y座標 7層目(0はじまり)が基準
	uint8_t ax;  // X傾き
	uint8_t ay;  // Y傾き
	uint16_t nh; // hit数
private:
	static const int angle_origin = 120; // ax,ayの原点
	static const int angle_divided = 1; // 傾きの重み係数の分母
	static const int angle_divider = 5; // 傾きの重み係数の分子
public:
	TrackHit() {};
	TrackHit(int px, int py, int ax, int ay, int nh) :px(px), py(py), ax(ax), ay(ay), nh(nh) {};
	static std::string first_line() { return "#px[pix] py[pix] ax[pix/15layer] ay[pix/15payer] number_of_hit"; }
#if _WIN64
	friend std::ostream& operator<<(std::ostream& os, const TrackHit& data) {
		std::string str = Poco::format("%?i %?i %?i %?i %?i",
			data.px, data.py, TrackHit::a2p(data.ax), TrackHit::a2p(data.ay), data.nh);
		os << str;
		return os;
	}
#endif
	static double a2p(double a) { return (a - angle_origin) * angle_divided * 15 / angle_divider; } //TrackHit angle to pixel
	static int a2p(uint8_t a) { return (int(a) - angle_origin) * angle_divided * 15 / angle_divider; } //TrackHit angle to pixel
	int Ax() const { return ((int)ax - 120) * 3; }
	int Ay() const { return ((int)ay - 120) * 3; }
	int Ph()const { return (int)nh; }
};


namespace phst {

	class ClusteredTrack
	{
	public:
		uint16_t px; ///X座標 pixel * 16
		uint16_t py; /// Y座標 pixel * 16
		uint8_t ax; /// X傾き, 0pix=120, 3pix=121, -3pix=119 / 15layers
		uint8_t ay; /// Y傾き
		uint16_t ph; /// hit数の最大値
		uint16_t num; /// hitしたtrackの所属する数
		uint16_t vol; /// hit数の合計
		static std::string first_line() { return "#px[pix] py[pix] ax[pix/15layer] ay[pix/15layer] ph num vol"; }
		int Ax() const { return ((int)ax - 120) * 3; }
		int Ay() const { return ((int)ay - 120) * 3; }
#if _WIN64
		friend std::ostream& operator<<(std::ostream& os, const ClusteredTrack& data) {
			std::string str = Poco::format("%?i %?i %?i %?i %?i %?i %?i",
				data.px, data.py, TrackHit::a2p(data.ax), TrackHit::a2p(data.ay), data.ph, data.num, data.vol);
			os << str;
			return os;
		}
#endif
	};
	class ClusterdMicroTrack // 24byte
	{
	public:
		float px; // pixel center is the 7th layer(0, 1, ...) :4
		float py; // pixel center is the 7th layer(0, 1, ...) :4
		float ax; // pixel / 15 layer :4
		float ay; // pixel / 15 layer :4
		uint16_t ph; /// hit数の最大値 :2
		uint16_t num; /// 空 :2
		uint16_t vol; /// hit数の合計 :2
		uint16_t vola; /// 空
		ClusterdMicroTrack() :px(0), py(0), ax(0), ay(0), ph(0), num(0), vol(0), vola(0) {};
		ClusterdMicroTrack(int px, int py, int ax, int ay) :
			px(static_cast<float>(px)),
			py(static_cast<float>(py)),
			ax(static_cast<float>(ax)),
			ay(static_cast<float>(ay)),
			ph(1),
			num(0),
			vol(1),
			vola(0) {}
		ClusterdMicroTrack(double px_sum, double py_sum, int ax_sum, int ay_sum, int ph, int vol, int vol_sum, int num, int vola) :
			px(static_cast<float>(px_sum / vol_sum)),
			py(static_cast<float>(py_sum / vol_sum)),
			ax(static_cast<float>(TrackHit::a2p(double(ax_sum) / vol_sum))),
			ay(static_cast<float>(TrackHit::a2p(double(ay_sum) / vol_sum))),
			ph(ph),
			num(num),
			vol(vol),
			vola(vola) {}
		std::string object()const { return "px/F:py:ax:ay:ph/s:num:vol:vola"; }
		std::string name()const { return "ClusterdMicroTrack"; }
		static std::string first_line() { return "#px[pix] py[pix] ax[pix/15pic] ay[pix/15pic] ph num vol vola"; }
		int pc_vol()const { return vola; }
		int pc_ph()const { return num & 0x1f; }
		int pc_area()const { return num >> 5; }
		void get_rotation(double rotation[2])const
		{
			const double rad = -atan2(ay, ax);
			rotation[0] = cos(rad);
			rotation[1] = sin(rad);
		}
		bool operator==(const ClusterdMicroTrack& lhs) const {
			if (lhs.px != this->px)return false;
			if (lhs.py != this->py)return false;
			if (lhs.ax != this->ax)return false;
			if (lhs.ay != this->ay)return false;
			if (lhs.ph != this->ph)return false;
			if (lhs.num != this->num)return false;
			if (lhs.vol != this->vol)return false;
			if (lhs.vola != this->vola)return false;
			return true;
		}
		//radial方向(飛跡をX軸方向に向けたときのX方向)
		double rax(double rotation[])const
		{
			return ax * rotation[0] + ay * -rotation[1];
		}
		//lateral方向(飛跡をX軸方向に向けたときのX方向)
		double ray(double rotation[])const
		{
			return ax * rotation[1] + ay * rotation[0];
		}
		//radial方向(飛跡をX軸方向に向けたときのX方向)
		double rpx(double px, double py, double rotation[])const
		{
			return (this->px - px) * rotation[0] + (this->py - py) * -rotation[1];
		}
		//lateral方向(飛跡をX軸方向に向けたときのX方向)
		double rpy(double px, double py, double rotation[])const
		{
			return (this->px - px) * rotation[1] + (this->py - py) * rotation[0];
		}
		static void rl2xy(const double& r, const double& l, double rotation[2], double& x, double& y) {
			x = r * rotation[0] * l * rotation[1];
			y = r * -rotation[1] * l * rotation[0];
		}
#if _WIN64
		friend std::ostream& operator<<(std::ostream& os, const ClusterdMicroTrack& data) {
			std::string str = Poco::format("%9.4hf %9.4hf %9.4hf %9.4hf %2hu %5hu %5hu %5hu", data.px, data.py, data.ax, data.ay, data.ph, data.num, data.vol, data.vola);
			os << str;
			return os;
		}
#endif
	};

	class MicroTrack // 32byte
	{
	public:
		double px; /// mm :8 Z中心の座標
		double py; /// mm :8 Z中心の座標
		float ax; /// tan theta :4
		float ay; /// tan theta :4
		uint16_t ph; /// hit数の最大値 :2
		uint16_t num; /// hitしたtrackの所属する数 :2
		uint16_t vol; /// hit数の合計 :2
		uint16_t vola; /// angle空間のhit数の合計
		MicroTrack() :px(0), py(0), ax(0), ay(0), ph(0), num(0), vol(0), vola(0) {};
		MicroTrack(const MicroTrack& t, const double aff_coef[6])
		{
			Af_Point2d position = base::af_trans(aff_coef, Af_Point2d(t.px, t.py));
			Af_Point2d angle = base::af_trans_rotation(aff_coef, Af_Point2d(t.ax, t.ay));
			px = position.x;
			py = position.y;
			ax = static_cast<float>(angle.x);
			ay = static_cast<float>(angle.y);
			ph = t.ph;
			num = t.num;
			vol = t.vol;
			vola = t.vola;
		}
		void get_rotation(double rotation[2])const
		{
			rotation[0] = cos(-atan2(ay, ax));
			rotation[1] = sin(-atan2(ay, ax));
		}
		double rax(double rotation[])const
		{
			return ax * rotation[0] + ay * -rotation[1];
		}
		double ray(double rotation[])const
		{
			return ax * rotation[1] + ay * rotation[0];
		}
		double rx(const MicroTrack& mt, double rotation[])const
		{
			return (mt.px - this->px) * rotation[0] + (mt.py - this->py) * -rotation[1];
		}
		double ry(const MicroTrack& mt, double rotation[])const
		{
			return (mt.px - this->px) * rotation[1] + (mt.py - this->py) * rotation[0];
		}
		void diff(double bangle, double bax, double bay, double& dra, double& dla)const {
			dra = bangle - (bax * ax + bay * ay) / bangle;
			dla = -(-bay * ax + bax * ay) / bangle;
		}
		std::string object()const { return "px/D:py:ax/F:ay:ph/s:num:vol:vola"; }
		std::string name()const { return "MicroTrack"; }
		static std::string first_line() { return "#px[mm] py[mm] ax[mm] ay[mm] ph vol num vola"; }
#if _WIN64
		friend std::ostream& operator<<(std::ostream& os, const MicroTrack& data) {
			std::string str = Poco::format(" %9.4f %9.4f %9.4hf %9.4hf %2hu %3hu %5hd %5hd", data.px, data.py, data.ax, data.ay, data.ph, data.vol, int16_t(data.num), int16_t(data.vola));
			os << str;
			return os;
		}
#endif
		const int phvol() const
		{
			return ph * 10000 + vol;
		}
	};
	class PairdMicroTrack // 32byte
	{
	public:
		MicroTrack mt1, mt2;
		std::string object()const { return "px1/D:py1:ax1/F:ay1:ph1/s:num1:vol1:vola1:px2/D:py2:ax2/F:ay2:ph2/s:num2:vol2:vola2"; }
		std::string name()const { return "PairdMicroTrack"; }
	};
#pragma pack(push,1)
	class FullMicroTrack : public MicroTrack //50byte
	{
	public:
		//MicroTrack mt; ///32 byte
		int32_t shot; ///4 byte
		int32_t trackid; ///4 byte
		int16_t view; //2byte
		int16_t stage_x, stage_y; ///2+2 byte 0.1ミリ単位
		int8_t imager, layer; ///1+1 byte
		int16_t flag; ///2 byte
		std::string name()const { return "FullMicroTrack"; }
		std::string pairdname()const { return "PairdFullMicroTrack"; }
		FullMicroTrack(const MicroTrack& t) :shot(0), trackid(0), view(0), stage_x(0), stage_y(0), imager(0), layer(0), flag(0) {
			this->px = t.px;
			this->py = t.py;
			this->ax = t.ax;
			this->ay = t.ay;
			this->ph = t.ph;
			this->num = t.num;
			this->vol = t.vol;
			this->vola = t.vola;
		}
		//mm単位
		void z0(double z0) {
			shot = std::round(z0 * 100000);
		}
		void z1(double z1) {
			trackid = std::round(z1 * 100000);
		}
		double z0()const { return shot * 0.00001; }
		double z1()const { return trackid * 0.00001; }
		double x0()const { return px - ax * (z1() - z0()) * 0.5; }
		double x1()const { return px + ax * (z1() - z0()) * 0.5; }
		double y0()const { return py - ay * (z1() - z0()) * 0.5; }
		double y1()const { return py + ay * (z1() - z0()) * 0.5; }
		void id32(uint64_t id) {
			if (id > 0xffffffff) { throw std::exception("ID overflow"); }
			stage_x = (int16_t)(id & 0xffff);
			stage_y = (int16_t)((id & 0xffff0000) >> 16);
		}
		uint64_t id32()const {
			return (uint32_t)(uint16_t)stage_x | (((uint32_t)(uint16_t)stage_y) << 16);
		}
		FullMicroTrack() :shot(0), trackid(0), view(0), stage_x(0), stage_y(0), imager(0), layer(0), flag(0) {};
	};
#pragma pack(pop)

#pragma pack(push,1)
	class ClusterdMicroTrackBeta // 12byte
	{
	protected:
		uint16_t px_; // std::floor(pixel * 16)
		uint16_t py_; // std::floor(pixel * 16)
		int16_t ax_; // std::floor(pixel * 16)
		int16_t ay_; // std::floor(pixel * 16)
		uint16_t ph_;  // ph は下位5bit(0x1f, 0-31), 
		uint16_t vol_; // vol は下位14bit
		static const int pos_offset = 256;
		static const int pos_min = 0 - pos_offset;
		static const int pos_max = 2048 + pos_offset;
		static const int angle_min = -1024;
		static const int angle_max = 1024;
		static const int ph_max = 32;
		static const int ph_offset = 1;
		static const int vol_max = 9999;
	public:
		static const int size = 12;
		static std::string object() { return "px/s:py:ax/S:ay:ph/s:vol"; }
		static std::string name() { return "ClusterdMicroTrackBeta1.0"; }
		static bool out_of_range(const ClusterdMicroTrack& t)
		{
			if (t.px <= pos_min || t.px >= pos_max) { return true; }
			if (t.py <= pos_min || t.py >= pos_max) { return true; }
			if (t.ax <= angle_min || t.ax >= angle_max) { return true; }
			if (t.ay <= angle_min || t.ay >= angle_max) { return true; }
			if (t.ph > ph_max) { return true; }
			return false;
		}
		ClusterdMicroTrackBeta(const ClusterdMicroTrack& t)
		{
			if (t.px <= pos_min || t.px >= pos_max) { std::cerr << "out_of_range px = " << t.px << std::endl; throw std::out_of_range("px"); }
			if (t.py <= pos_min || t.py >= pos_max) { std::cerr << "out_of_range py =" << t.py << std::endl; throw std::out_of_range("py"); }
			if (t.ax <= angle_min || t.ax >= angle_max) { std::cerr << "out_of_range ax = " << t.ax << std::endl; throw std::out_of_range("ax"); }
			if (t.ay <= angle_min || t.ay >= angle_max) { std::cerr << "out_of_range ay = " << t.ay << std::endl; throw std::out_of_range("ay"); }
			if (t.ph > ph_max) { throw std::out_of_range("ph > 32"); }
			px_ = static_cast<uint16_t>(std::floor((t.px + pos_offset) * 16));
			py_ = static_cast<uint16_t>(std::floor((t.py + pos_offset) * 16));
			ax_ = static_cast<int16_t>(std::floor(t.ax * 16));
			ay_ = static_cast<int16_t>(std::floor(t.ay * 16));
			ph_ = t.ph - ph_offset;
			vol_ = ((t.vol > vol_max) ? vol_max : t.vol);
		}
		ClusterdMicroTrackBeta() {}
		ClusterdMicroTrack get_clusterd_microtrack()const {
			phst::ClusterdMicroTrack t;
			t.px = px();
			t.py = py();
			t.ax = ax();
			t.ay = ay();
			t.ph = ph();
			t.vol = vol();
			return t;
		}
		uint16_t get_px_()const { return px_; }
		uint16_t get_py_()const { return py_; }
		int16_t get_ax_()const { return ax_; }
		int16_t get_ay_()const { return ay_; }
		uint16_t get_ph_()const { return ph_; }
		uint16_t get_vol_()const { return vol_; }

		void set_px_(uint16_t px) { this->px_ = px; }
		void set_py_(uint16_t py) { this->py_ = py; }
		void set_ax_(int16_t ax) { this->ax_ = ax; }
		void set_ay_(int16_t ay) { this->ay_ = ay; }
		void set_ph_(uint16_t ph) { this->ph_ = ph; }
		void set_vol_(uint16_t vol) { this->vol_ = vol; }

		double px()const { return px_ / 16.0 - pos_offset; }
		double py()const { return py_ / 16.0 - pos_offset; }
		double ax()const { return ax_ / 16.0; }
		double ay()const { return ay_ / 16.0; }
		int ph()const { return (ph_ & 0x001f) + ph_offset; }
		int vol()const { return (vol_ & 0x3fff); }
		void set_vol(int vol) { vol_ = ((vol > vol_max) ? vol_max : vol); }
		// 0 は全体のフラグ 15bit目
		// 1 はfilterによるフラグ  14bit目
		// 2 はcutによるフラグ 13bit目
		// 3 はdeadpixelによるフラグ 12bit目
		// 4-7は未定義 8-11bit目
		int get_flag(int i)const {
			if (i < 0 || i >= 8) { throw std::exception("out of range of flag"); }
			return (ph_ & (0x0001 << (15 - i))) >> (15 - i);
		}
		int get_flag()const {
			return (ph_ & 0xff00) >> 8;
		}
	private:
		int get_flag_impl()const {
			return (ph_ & 0x7f00) >> 8;
		}
		void set_flag_impl(int i, const bool b) {
			if (b) { ph_ = ph_ | (0x0001 << (15 - i)); }
			else { ph_ = ph_ & (~(0x0001 << (15 - i))); }
		}
	public:
		void set_flag(int i, const bool b) {
			if (i < 1 || i >= 8) { throw std::exception("out of range of flag"); }
			set_flag_impl(i, b);
			if (b) { set_flag_impl(0, true); }
			else {
				if (get_flag_impl() > 0) { set_flag_impl(0, true); }
				else { set_flag_impl(0, false); }
			}
		}
		void reset_flag() { ph_ = ph_ & ~0xff00; }
		void reset_flag(int i) {
			if (i < 1 || i >= 8) { throw std::exception("out of range of flag"); }
			set_flag_impl(i, false);
			if (get_flag_impl() == 0) { set_flag_impl(0, false); }
		}
#if _WIN64
		friend std::ostream& operator<<(std::ostream& os, const ClusterdMicroTrackBeta& data) {
			std::string str = Poco::format("%9.4f %9.4f %9.4f %9.4f %2d %3d", data.px(), data.py(), data.ax(), data.ay(), data.ph(), data.vol());
			os << str;
			return os;
		}
#endif
	};

	class ClusterdMicroTrackBeta1 : public ClusterdMicroTrackBeta // 16byte
	{
	public:
		uint16_t value1; /// 値1
		uint16_t value2; /// 値2
		static std::string object() { return "px/s:py:ax/S:ay:ph/s:vol:value1:value2"; }
		static std::string name() { return "ClusterdMicroTrackBeta1_1.0"; }
		static const int size = 16;
		ClusterdMicroTrackBeta1() :ClusterdMicroTrackBeta() {}
		ClusterdMicroTrackBeta1(const ClusterdMicroTrack& t) :ClusterdMicroTrackBeta(t)
		{
			value1 = t.num;
			value2 = t.vola;
		}
		ClusterdMicroTrack get_clusterd_microtrack()const {
			phst::ClusterdMicroTrack t;
			t.px = px();
			t.py = py();
			t.ax = ax();
			t.ay = ay();
			t.ph = ph();
			t.vol = vol();
			t.num = value1;
			t.vola = value2;
			return t;
		}

	};
#pragma pack(pop)
#pragma pack(push,1)
#undef min
	class MiniPhVol {
		uint8_t ph12;
		uint8_t _vol1;
		uint8_t _vol2;
		static const int ph_max = 0x010;
		static const int ph_offset = 0x001;
		static const int vol_max = 0x100;
		static const int vol_offset = 0x001;
	public:
		MiniPhVol(int phvol1, int phvol2) : MiniPhVol(phvol1 / 10000, phvol1 % 10000, phvol2 / 10000, phvol2 % 10000) {}
		MiniPhVol(int ph1, int vol1, int ph2, int vol2) {
			if (ph1 > ph_max) { throw std::exception(); }
			if (ph1 < ph_offset) { throw std::exception(); }
			if (ph2 > ph_max) { throw std::exception(); }
			if (ph2 < ph_offset) { throw std::exception(); }
			if (vol1 < vol_offset) { throw std::exception(); }
			if (vol2 < vol_offset) { throw std::exception(); }
			vol1 = std::min(vol1, vol_max);
			vol2 = std::min(vol2, vol_max);
			ph12 = (uint8_t)(ph1 - ph_offset) << 4 | (uint8_t)(ph2 - ph_offset);
			_vol1 = (uint8_t)(vol1 - vol_offset);
			_vol2 = (uint8_t)(vol2 - vol_offset);
		}
		MiniPhVol() :ph12(0), _vol1(0), _vol2(0) { throw std::exception(); }
		int ph1()const { return (uint16_t)(ph12 >> 4) + ph_offset; }
		int ph2()const { return (uint16_t)(ph12 & 0x0f) + ph_offset; }
		int vol1()const { return (uint16_t)_vol1 + vol_offset; }
		int vol2()const { return (uint16_t)_vol2 + vol_offset; }
		int phvol1()const { return ph1() * 10000 + vol1(); }
		int phvol2()const { return ph2() * 10000 + vol2(); }
		int ph()const { return ph1() + ph2(); }
		int vol()const { return vol1() + vol2(); }
#if _WIN64
		friend std::ostream& operator<<(std::ostream& os, const MiniPhVol& data) {
			std::string str = Poco::format("%?i %?i %?i %?i", data.ph1(), data.vol1(), data.ph2(), data.vol2());
			os << str;
			return os;
		}
#endif
	};
#pragma pack(pop)
}
