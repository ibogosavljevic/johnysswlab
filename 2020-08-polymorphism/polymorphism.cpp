
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include "measure_time.h"
#include <algorithm>
#include "objects.h"
#include "polymorphic_vector.h"


int main(int argc, char* argv[]) {

    constexpr int arr_len = 10000000;
    bitmap b;

    if (false) {
        polymorphic_vector<object, 56> v;
        v.reserve(4 * arr_len);

        for (int i = 0; i < arr_len; i++) {
            v.emplace_back<circle>();
            v.emplace_back<line>();
            v.emplace_back<rectangle>();
            v.emplace_back<monster>();
        }

        //v.shuffle();


        measure_time m("Polymorphic vector"); 
        {
            for (int i = 0; i < arr_len; i++) {
                v.get(i)->draw(b);
            }
        }

        for (int i = 0; i < 4; i++) {
            std::cout << v.get(i)->to_string();
        }

    } else if (false) {
        std::vector<circle> v1(arr_len);
        std::vector<line> v2(arr_len);
        std::vector<rectangle> v3(arr_len);
        std::vector<monster> v4(arr_len);


        for (int i = 0; i < arr_len; i++) {
            v1.emplace_back();
            v2.emplace_back();
            v3.emplace_back();
            v4.emplace_back();
        }

        //std::random_shuffle(v1.begin(), v1.end());
        //std::random_shuffle(v2.begin(), v2.end());
        //std::random_shuffle(v3.begin(), v3.end());
        //std::random_shuffle(v4.begin(), v4.end());

        measure_time m("DOO vector"); 
        {
            for (int i = 0; i < arr_len; i++) {
                v1[i].draw(b);
            }
            for (int i = 0; i < arr_len; i++) {
                v2[i].draw(b);
            }
            for (int i = 0; i < arr_len; i++) {
                v3[i].draw(b);
            }
            for (int i = 0; i < arr_len; i++) {
                v4[i].draw(b);
            }
        

        }

        std::cout << v1[0].to_string();
        std::cout << v2[0].to_string();
        std::cout << v3[0].to_string();
        std::cout << v4[0].to_string();
    } else {
        std::vector<object*> q;

        q.reserve(arr_len * 4);

        for (int i = 0; i < arr_len; i++) {
            q.push_back(new circle());
            q.push_back(new line());
            q.push_back(new rectangle());
            q.push_back(new monster());
        }

        //std::random_shuffle(q.begin(), q.end());

        measure_time m("Array of pointers"); 
        {
            for (int i = 0; i < arr_len * 4; i++) {
                q[i]->draw(b);
            }
        }

        for (int i = 0; i < 4; i++) {
            std::cout << q[i]->to_string();
        }
    }


    return 0;
}