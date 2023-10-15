#pragma once

#include <vector>
#include <iostream>

#include <sstream>

namespace phst {

    class Af_Point2d
    {
    public:
        Af_Point2d(double x, double y) {
            this->x = x;
            this->y = y;
        }
        Af_Point2d() :x(), y() { x = 0; y = 0; };
        Af_Point2d operator+(const Af_Point2d& p1)const
        {
            Af_Point2d p;
            p.x = x + p1.x;
            p.y = y + p1.y;
            return p;
        }
        Af_Point2d operator-(const Af_Point2d& p1)const
        {
            Af_Point2d p;
            p.x = x - p1.x;
            p.y = y - p1.y;
            return p;
        }
        template<class T> Af_Point2d operator*(T i)const
        {
            Af_Point2d p;
            p.x = x * i;
            p.y = y * i;
            return p;
        }
        template<class T> Af_Point2d operator/(T i)const
        {
            Af_Point2d p;
            p.x = x / i;
            p.y = y / i;
            return p;
        }
        template<class T> Af_Point2d operator+(T i)const
        {
            Af_Point2d p;
            p.x = x + i;
            p.y = y + i;
            return p;
        }
        template<class T> Af_Point2d operator-(T i)const
        {
            Af_Point2d p;
            p.x = x - i;
            p.y = y - i;
            return p;
        }
        // 距離の二乗
        double distance()const
        {
            return x * x + y * y;
        }
        std::string first_line()const { return "#px py"; }
        std::string first_line_csv()const { return "px,py"; }
        std::string to_string()const
        {
            std::stringstream ss;
            ss << x << " " << y;
            return ss.str();
        }
        std::string to_string_csv()const
        {
            std::stringstream ss;
            ss << x << "," << y;
            return ss.str();
        }
        double x;
        double y;
        friend std::ostream& operator<<(std::ostream& stream, Af_Point2d ob);
    };

    namespace base {

        // アフィン変換
        Af_Point2d af_trans(const double aff_coef[6], const Af_Point2d& orig);
        // アフィン逆変換
        Af_Point2d af_intrans(const double aff_coef[6], const Af_Point2d& orig);


        // 回転変換
        Af_Point2d af_trans_rotation(const double aff_coef[4], const Af_Point2d& orig);
        // 回転逆変換
        Af_Point2d af_intrans_rotation(const double aff_coef[4], const Af_Point2d& orig);

        //スケールを計算
        double af_calc_scaling(const double aff_coef[4]);

        // 2個の点列の組からアフィンパラメータを計算
        void af_calc_coef(const std::vector<Af_Point2d>& orig, const std::vector<Af_Point2d>& trans, double aff_coef[6], double& RMS);

        // 2個の点列の組からシフト量を固定したアフィンパラメータを計算
        void af_calc_coef_without_shift(const std::vector<Af_Point2d>& orig, const std::vector<Af_Point2d>& trans, double aff_coef[6], double& RMS);

        // 2個の点列の組から回転とシフト量によるアフィンパラメータを計算
        void af_calc_coef_rot_shift(const std::vector<Af_Point2d>& orig, const std::vector<Af_Point2d>& trans, double aff_coef[6], double& RMS);


        void af_combine(double aff_coef_left[6], double aff_coef_right[6], double aff_coef_combine[6]);


        // flag=0 first, flag=1 second
        std::vector< std::pair<Af_Point2d, Af_Point2d>> sort_pair_points(const std::vector< std::pair<Af_Point2d, Af_Point2d>>& pairs, Af_Point2d xy);

        // flag=0 first, flag=1 second
        void get_affine(const std::vector< std::pair<Af_Point2d, Af_Point2d>>& pairs, Af_Point2d xy, double aff_coef[6]);
    }

}