/*
 * An example of CDDS computation 
 * by: yanchuan (2020-11)
 */

#include <string>
#include <iostream>
#include <sstream>

#include "cdds.hpp"

int main(int argc, char const *argv[]) {

    // str_t1 and str_t2 -> trajectory examples
    // See 'Traj::operator>>' for more details of the trajectory input format.
    // traj = (n, x1, y1, t1, x2, y2, t2, .... xn, yn, tn)
    std::string str_t1 = "4 0 0 0 10 0 10 20 0 20 30 0 30";  
    std::string str_t2 = "5 2 2 1 5 5 5 15 5 15 25 -5 28 35 5 35";
    std::istringstream iss_t1(str_t1);
    std::istringstream iss_t2(str_t2);

    Traj t1, t2;
    iss_t1 >> t1;
    iss_t2 >> t2;

    float distance_thres = 3.0;
    std::cout << CDDS(t1, t2, distance_thres) << std::endl;

    return 0;
}
