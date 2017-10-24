#include <iostream>
#include <string>
#include "Layer.h"


using namespace std;

struct Circuit {
    int ViaCost, Spacing, N_MetalLayers, N_RoutedShapes, N_RoutedVias, N_Obstacles;
    G g; // Grafo
    int Boundary; // Retângulo
    std::map<string, Layer> Layers;


    void input_to_objects(string);                  // Done
    void move_obstacles_points();                   // Done
    Set_Pair generate_hanan_grid();                 // Done
    void add_zero_edges_to_components(Set_Pair);    // Done
    void convert_to_boost();
    void remove_collided_points_with_obstacles();
    void connect_all_components();
    void remove_one_degree_vertices();
    void remove_zero_edges();                       // WTF???
    void generate_output();
    void draw();
};
