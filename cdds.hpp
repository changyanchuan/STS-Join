/*
 * CDDS -- Close-distance duration similarity of two trajectories
 * by: yanchuan (2020-11)
 */

#ifndef __CDDS_HPP__
#define __CDDS_HPP__

#include <vector>
#include <iostream>
#include <cmath>
#include <utility>


class Point {
public:
    Point(const float x, const float y, const long t) 
            :x(x), y(y), t(t) {
    }

    float x;
    float y;
    long t; // timestamp
};


typedef std::vector<Point> Points;
typedef std::pair<const Point*, const Point*> Edge; // read only

class Traj {
public:
    Traj() {}

    size_t length() const{
        return vec_points.size();
    }

    friend std::istream& operator>>(std::istream& in, Traj& traj);
    friend std::ostream& operator<<(std::ostream& out, Traj& traj);

    Points vec_points;
};


std::istream& operator>>(std::istream& in, Traj& traj) {
    // assume all inputted data are reliable
    in.precision(7);

    size_t num_points = 0;
    float x, y;
    long t;

    in >> num_points;
    for (size_t i = 0; i < num_points; ++i) {
        in >> x >> y >> t;
        traj.vec_points.push_back( Point(x, y, t) );
    }
    return in;
}


std::ostream& operator<<(std::ostream& out, Traj& traj) {
    out.precision(7);

    const Points& points = traj.vec_points;
    
    out << traj.length();
    for (size_t i = 0; i < traj.length(); ++i) {
        out << " (" << points[i].x << ", " 
                    << points[i].y << ", "
                    << points[i].t << ")";
    }
    return out;
}


void LineSlopeIntercept(const Edge& e, float& xk, float& xb, float& yk, float& yb) {
    yk = (e.second->y - e.first->y) / (e.second->t - e.first->t);
    yb = (e.first->y * e.second->t - e.second->y * e.first->t) / (e.second->t - e.first->t);

    xk = (e.second->x - e.first->x) / (e.second->t - e.first->t);
    xb = (e.first->x * e.second->t - e.second->x * e.first->t) / (e.second->t - e.first->t); 
}


float CDD(const Edge& e1, const Edge& e2, const float distance_thres) {

    if (e1.second->t <= e2.first->t || e2.second->t <= e1.first->t) {
        return 0.0;
    }

    float thres_dis_square = pow(distance_thres, 2);

    float e1_xk, e1_xb, e1_yk, e1_yb;
    float e2_xk, e2_xb, e2_yk, e2_yb;
    LineSlopeIntercept(e1, e1_xk, e1_xb, e1_yk, e1_yb);
    LineSlopeIntercept(e2, e2_xk, e2_xb, e2_yk, e2_yb);

    // distance^2 = _at^2 + _bt + _c (Euclidean Distance^2)
    float _a = pow(e1_xk - e2_xk, 2) + pow(e1_yk - e2_yk, 2);
    float _b = 2 * ((e1_xk - e2_xk) * (e1_xb - e2_xb) 
                + (e1_yk - e2_yk) * (e1_yb - e2_yb));
    float _c = pow(e1_xb - e2_xb, 2) + pow(e1_yb - e2_yb, 2);
    

    if (_a == 0 && _b == 0 && _c == 0) { // e1 and e2 are on the same line
        return std::min(e1.second->t, e2.second->t) - 
                std::max(e1.first->t, e2.first->t);
    }
    else if (_a == 0 && _b == 0) {
        if (_c <= thres_dis_square) {
            return std::min(e1.second->t, e2.second->t) - 
                    std::max(e1.first->t, e2.first->t);
        }
    }
    else if (_a == 0) {
        float _t1 = -1 * _c / _b;
        float _t2 = (thres_dis_square - _c) / _b;
        float t_start = std::max(e1.first->t, e2.first->t);
        float t_end = std::min(e1.second->t, e2.second->t);

        if (_t1 < _t2) {
            float t_min = std::max(_t1, t_start);
            float t_max = std::min(_t2, t_end);
            return t_max > t_min ? t_max - t_min : 0;
        }
        else if (_t2 < _t1) {
            float t_min = std::max(_t2, t_start);
            float t_max = std::min(_t1, t_end);
            return t_max > t_min ? t_max - t_min : 0;
        }
        // we dont care _t2 = _t1
    }
    else { // quatratic
        _c -= thres_dis_square;
        float theta = _b * _b - 4 * _a * _c;
        if (theta > 0) {
            float theta_sqrt = sqrt(theta);
            float _t1 = (-1 * theta_sqrt - _b) / 2 / _a;
            float _t2 = (theta_sqrt - _b) / 2 / _a;
            float t_min = std::max(e1.first->t, e2.first->t);
            float t_max = std::min(e1.second->t, e2.second->t);
            t_min = std::max(_t1, t_min);
            t_max = std::min(_t2, t_max);
            return t_max > t_min ? t_max - t_min : 0;
        }
    }
    return 0.0; 
}


// Given two trajectories t1 and t2, CDDS returns the close-distance duration 
// of the two trajectories (cf. Equation 4 in STS-Join paper).
float CDDS(const Traj& t1, const Traj& t2, const float distance_thres) {
    
    float duration_result =  0.0;
    size_t i = 0, j = 0;
    size_t t1_num_edges = t1.length() - 1;
    size_t t2_num_edges = t2.length() - 1;

    while( i < t1_num_edges && j < t2_num_edges ) {

            auto e1 = std::make_pair(&t1.vec_points[i], &t1.vec_points[i+1]);
            auto e2 = std::make_pair(&t2.vec_points[j], &t2.vec_points[j+1]);
            duration_result += CDD(e1, e2, distance_thres);

        if (t1.vec_points[i+1].t < t2.vec_points[j+1].t) {
            i += 1;
        }
        else if (t2.vec_points[j+1].t < t1.vec_points[i+1].t) {
            j += 1;
        }
        else {
            i += 1;
            j += 1;
        }
    }
    return duration_result;
}


#endif