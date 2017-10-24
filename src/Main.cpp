#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "Circuit.h"


int main(int argc, char* argv[]){
    std::pair<std::set<int>, std::set<int>> XY;
    Circuit C;


    time_t start, end;
    time(&start);
    //std::cout << start << "\n";


    C.input_to_objects(argv[1]);
    C.move_obstacles_points();

    XY = C.generate_hanan_grid();

    C.add_zero_edges_to_components(XY);

    C.convert_to_boost();


    string p = "print";
    if (argv[2] == p) {
        for (std::map<string, Layer>::iterator it = C.Layers.begin(); it != C.Layers.end(); ++it) {
            it->second.g.print_Vertex_map();
            it->second.g.print_edges();
        }
    }


    time(&end);
    double dif = difftime(end, start);
    std::cout << "\n\nTempo de execução: " << dif << " segundos\n";



    return 0;
}
