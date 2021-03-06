#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include "Circuit.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <unordered_map>


void Circuit::draw(){
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        std::cout << it->first << ":\n";
        std::cout << "Shapes:\n";
        it->second.print_shapes(1);
        std::cout << "Obstacles:\n";
        it->second.print_shapes(0);
        std::cout << "Vias:\n";
        it->second.print_vias();
        std::cout << "\n";
    }
}


void Circuit::input_to_objects(string case_path){
    std::cout << "Generating objects from input\n";
    std::ifstream fs(case_path);
    std::string line,  one, two, three, four;
    std::vector<std::string> input;
    std::vector<std::string> strs;

    while (std::getline(fs, line))
        input.push_back(line);


    boost::split(strs, input.at(0), boost::is_any_of("\t "));
    this->ViaCost = boost::lexical_cast<int>(strs.at(2));

    boost::split(strs, input.at(1), boost::is_any_of("\t "));
    this->Spacing = boost::lexical_cast<int>(strs.at(2));

    boost::split(strs, input.at(3), boost::is_any_of("\t "));
    this->N_MetalLayers = boost::lexical_cast<int>(strs.at(2));

    boost::split(strs, input.at(4), boost::is_any_of("\t "));
    this->N_RoutedShapes = boost::lexical_cast<int>(strs.at(2));

    boost::split(strs, input.at(5), boost::is_any_of("\t "));
    this->N_RoutedVias = boost::lexical_cast<int>(strs.at(2));

    boost::split(strs, input.at(6), boost::is_any_of("\t "));
    this->N_Obstacles = boost::lexical_cast<int>(strs.at(2));


    //  Cria as camadas no circuito (ainda vazias)
    for (int i = 1; i <= this->N_MetalLayers; i++) {
        Layer l;
        this->Layers[i] = l;
    }

    //  Instanciação dos objetos das Layers
    for (int i = 0; i < N_RoutedShapes; i++) {
        boost::split(strs, input.at(i + 7), boost::is_any_of("\t "));
        Layers[strs.at(1)[1] - 48].add_shape(strs);
    }

    int j = N_RoutedShapes + 7;
    for (int i = 0; i < N_RoutedVias; i++) {
        boost::split(strs, input.at(j + i), boost::is_any_of("\t "));
        Layers[strs.at(1)[1] - 48].add_via(strs);

        Layers[strs.at(1)[1] - 47].add_via(strs);
    }

    j = N_RoutedVias + N_RoutedShapes + 7;
    for (int i = 0; i < N_Obstacles; i++) {
        boost::split(strs, input.at(j + i), boost::is_any_of("\t "));
        Layers[strs.at(1)[1] - 48].add_shape(strs);

    }
    fs.close();
}


void Circuit::move_obstacles_points() {
    std::cout << "Moving obstacle points\n";
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        it->second.move_obstacles_points();
    }
}


bool point_collide_rect(Vertex v, Shape s) {
    return ((v[0] > s.A[0] && v[0] < s.B[0]) && (v[1] > s.D[1] && v[1] < s.A[1]));
}


void Circuit::generate_hanan_grid(bool gen_img) {
    std::cout << "Generating hanan grid\n";

    std::set<int> X;
    std::set<int> Y;
    std::set<int> Z;

    std::vector<Vertex> vertices;
    std::set<Edge> grid;

    int z_coord = 1;

    std::cout << "  Getting coordinates\n";
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        for (Shape c : it->second.Components) {
            X.insert(c.A[0]);
            Y.insert(c.A[1]);
            X.insert(c.B[0]);
            Y.insert(c.B[1]);
            X.insert(c.C[0]);
            Y.insert(c.C[1]);
            X.insert(c.D[0]);
            Y.insert(c.D[1]);
        }
        for (Shape o : it->second.Obstacles) {
            X.insert(o.A[0]);
            Y.insert(o.A[1]);
            X.insert(o.B[0]);
            Y.insert(o.B[1]);
            X.insert(o.C[0]);
            Y.insert(o.C[1]);
            X.insert(o.D[0]);
            Y.insert(o.D[1]);
        }
        for (Via v : it->second.Vias) {
            X.insert(v.point[0]);
            Y.insert(v.point[1]);
        }
        Z.insert(z_coord);
        z_coord++;
    }

    this->XY = Set_Pair(X, Y);
    g.g = Graph(X.size() * Y.size() * Z.size());

    std::cout << "  Creating vertices\n";
    int v_num = 0;
    //Vertices
    for (std::set<int>::iterator x = X.begin(); x != X.end(); ++x) {
        for (std::set<int>::iterator y = Y.begin(); y != Y.end(); ++y) {
            for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
                bool flag = false;
                for (Shape o : Layers[*z].Obstacles) {
                    if (point_collide_rect({*x,*y,*z}, o)) {
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    V vd = v_num;
                    v_num++;
                    rev_map[{*x,*y,*z}] = vd;
                    ver_map[vd] = {*x,*y,*z};
                }
            }
        }
    }

    std::cout << "  Creating edges\n";
    //Arestas
    for (std::set<int>::iterator x = X.begin(); x != X.end(); ++x) {
        for (std::set<int>::iterator y = Y.begin(); y != Y.end(); ++y) {
            for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
                std::set<int>::iterator xp = x;
                std::set<int>::iterator yp = y;
                std::set<int>::iterator zp = z;

                ++xp;
                ++yp;
                ++zp;

                if (xp != X.end()){
                    if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*xp,*y,*z}) != rev_map.end()))
                        boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*xp,*y,*z}], euclidian_dist({*x,*y,*z}, {*xp,*y,*z}), g.g);
                }
                if (yp != Y.end()){
                    if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*x,*yp,*z}) != rev_map.end()))
                        boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*x,*yp,*z}], euclidian_dist({*x,*y,*z}, {*x,*yp,*z}), g.g);
                }
                if (zp != Z.end()){
                    if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*x,*y,*zp}) != rev_map.end()))
                        boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*x,*y,*zp}], euclidian_dist({*x,*y,*z}, {*x,*y,*zp}), g.g);
                }
            }
        }
    }


    /*
        A-----B
        |     |
        |     |
        P1----P2   <-- Remove arestas desse tipo
        |     |
        |     |
        D-----C
    */
    std::cout << "removing side-to-side edges from obstacles\n";
    for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
        for (Shape o : Layers[*z].Obstacles) {
            auto x1 = X.find(o.A[0]);
            auto x2 = X.find(o.B[0]);
            ++x1;

            auto y1 = Y.find(o.A[1]);
            auto y2 = Y.find(o.C[1]);
            --y1;

            if (*x1 == o.B[0]) {
                auto yy = y1;
                while (*yy != *y2) {
                    auto nu = rev_map[{o.A[0], *yy, o.A[2]}];
                    auto nv = rev_map[{o.B[0], *yy, o.A[2]}];
                    boost::remove_edge(nu, nv, g.g);
                    boost::remove_edge(nv, nu, g.g);
                    --yy;
                }
            }

            if (*y1 == o.C[1]) {
                auto xx = x1;
                while (*xx != *x2) {
                    auto nu = rev_map[{*xx, o.A[1], o.A[2]}];
                    auto nv = rev_map[{*xx, o.C[1], o.A[2]}];
                    boost::remove_edge(nu, nv, g.g);
                    boost::remove_edge(nv, nu, g.g);
                    ++xx;
                }
            }
        }
    }

    if (gen_img) to_dot(g.g);
}








void Circuit::add_to_subgrids(Vertex p) {
    for (auto x_it = this->subgradeX.begin(); x_it != this->subgradeX.end(); ++x_it) {
        std::set<int>::iterator auxX = x_it;
        if (++auxX == this->subgradeX.end()) {
            break;
        }
        for (auto y_it = this->subgradeY.begin(); y_it != this->subgradeY.end(); ++y_it) {
            std::set<int>::iterator auxY = y_it;
            if (++auxY == this->subgradeY.end()) {
                break;
            }

            if (p[0] >= *x_it && p[0] <= *auxX and
                p[1] >= *y_it && p[1] <= *auxY) {
                std::pair<Vertex, Vertex> subG ({*x_it, *auxY, 0}, {*auxX, *y_it, 0});
                this->subgrades[subG].insert({p[0], p[1], 0});
            }
        }
    }
}



void Circuit::generate_spanning_grid(bool gen_img) {
    std::cout << "Generating spanning grid\n";

    std::set<int> X;
    std::set<int> Y;
    std::set<int> Z;

    std::vector<Vertex> vertices;
    std::set<Edge> grid;

    std::set<Vertex> vertices_set;

    int z_coord = 1;

    std::set<int> subGridX;
    std::set<int> subGridY;
    std::set<int> subGridZ;


    std::cout << "  Getting coordinates\n";
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        for (Shape c : it->second.Components) {
            X.insert(c.A[0]);
            Y.insert(c.A[1]);
            X.insert(c.B[0]);
            Y.insert(c.B[1]);
            X.insert(c.C[0]);
            Y.insert(c.C[1]);
            X.insert(c.D[0]);
            Y.insert(c.D[1]);
        }
        for (Shape o : it->second.Obstacles) {
            X.insert(o.A[0]);
            Y.insert(o.A[1]);
            X.insert(o.B[0]);
            Y.insert(o.B[1]);
            X.insert(o.C[0]);
            Y.insert(o.C[1]);
            X.insert(o.D[0]);
            Y.insert(o.D[1]);
        }
        for (Via v : it->second.Vias) {
            X.insert(v.point[0]);
            Y.insert(v.point[1]);
        }
        Z.insert(z_coord);
        z_coord++;
    }

    //this->XY = Set_Pair(X, Y);
    //g.g = Graph(X.size() * Y.size() * Z.size());

    std::cout << "  Creating vertices\n";

    this->sub_grid_size = sqrt( (this->N_RoutedShapes + this->N_Obstacles) * 4 + this->N_RoutedVias );
    std::cout << "( " << this->sub_grid_size << " )\n";

    int i, j, k;
    for (k = 0; k < (int)Z.size(); k++) {
        for (i = 0; i < (int)X.size() - 1; i += this->sub_grid_size - 1) {
            for (j = 0; j < (int)Y.size() - 1; j += this->sub_grid_size - 1) {
                int iMais = (i + this->sub_grid_size - 1);
                int jMais = (j + this->sub_grid_size - 1);

                if ((i + this->sub_grid_size - 1) > ((int)X.size() - 1)) {
                    iMais = ((int)X.size() - 1);
                }
                if ((j + this->sub_grid_size - 1) > ((int)Y.size() - 1)) {
                    jMais = ((int)Y.size() - 1);
                }

                std::set<int>::iterator it_x, it_y;
                int xx1, xx2, yy1, yy2;

                it_x = X.begin();
                std::advance(it_x, i);
                xx1 = *it_x;

                it_x = X.begin();
                std::advance(it_x, iMais);
                xx2 = *it_x;

                it_y = Y.begin();
                std::advance(it_y, j);
                yy1 = *it_y;

                it_y = Y.begin();
                std::advance(it_y, jMais);
                yy2 = *it_y;

                std::set<Vertex> subset;

                subset.insert({xx1, yy1, 0});
                subset.insert({xx2, yy1, 0});
                subset.insert({xx1, yy2, 0});
                subset.insert({xx2, yy2, 0});
                std::pair<Vertex, Vertex> key ({xx1, yy2, 0}, {xx2, yy1, 0});
                this->subgrades[key] = subset;
                this->subgradeX.insert({xx1, xx2});
                this->subgradeY.insert({yy1, yy2});
            }
        }
    }


    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        for (Shape c : it->second.Components) {
            add_to_subgrids(c.A);
            add_to_subgrids(c.B);
            add_to_subgrids(c.C);
            add_to_subgrids(c.D);
        }
        for (Shape o : it->second.Obstacles) {
            add_to_subgrids(o.A);
            add_to_subgrids(o.B);
            add_to_subgrids(o.C);
            add_to_subgrids(o.D);
        }
        for (Via v : it->second.Vias) {
            add_to_subgrids(v.point);
        }
    }


    std::set<Vertex> allVertices;
    for (std::map<std::pair<Vertex, Vertex>, std::set<Vertex>>::iterator it = subgrades.begin(); it != subgrades.end(); ++it) {
        std::set<int> subX;
        std::set<int> subY;

        for (std::set<Vertex>::iterator set_it = it->second.begin(); set_it != it->second.end(); ++set_it) {
            subX.insert((*set_it)[0]);
            subY.insert((*set_it)[1]);
        }

        //this->XY = Set_Pair(X, Y);
        //g.g = Graph(X.size() * Y.size() * Z.size());

        //std::cout << "  Creating vertices\n";
        //Vertices
        for (std::set<int>::iterator x = subX.begin(); x != subX.end(); ++x) {
            for (std::set<int>::iterator y = subY.begin(); y != subY.end(); ++y) {
                bool flag = false;
                for (std::map<int, Layer>::iterator l_it = Layers.begin(); l_it != Layers.end(); ++l_it) {
                    for (Shape o : l_it->second.Obstacles) {
                        if (point_collide_rect({*x,*y, 0}, o)) {
                            flag = true;
                            break;
                        }
                    }
                    if (flag) { break; }
                }

                if (!flag) {
                    allVertices.insert({*x,*y,0});
                }
            }
        }
    }
    this->XY = Set_Pair(X, Y);
    g.g = Graph(X.size() * Y.size() * Z.size());
    int v_num = 0;

    std::cout << "Vertices\n";
    for (auto i = allVertices.begin(); i != allVertices.end(); ++i) {
        //iterar todas as camadas
        for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
            //adicionar os pontos
            V vd = v_num;
            v_num++;
            rev_map[{(*i)[0],(*i)[1],*z}] = vd;
            ver_map[vd] = {(*i)[0],(*i)[1],*z};
            //print_v({(*i)[0],(*i)[1],*z});
        }
    }






    std::cout << "  Creating edges\n";
    for (std::map<std::pair<Vertex, Vertex>, std::set<Vertex>>::iterator it = subgrades.begin(); it != subgrades.end(); ++it) {
        std::set<int> sgX, sgY;
        for (std::set<Vertex>::iterator set_it = it->second.begin(); set_it != it->second.end(); ++set_it) {
            sgX.insert((*set_it)[0]);
            sgY.insert((*set_it)[1]);
        }

        // Itera em Z
        for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
            for (std::set<int>::iterator x = sgX.begin(); x != sgX.end(); ++x) {
                for (std::set<int>::iterator y = sgY.begin(); y != sgY.end(); ++y) {
                    std::set<int>::iterator xp = x;
                    std::set<int>::iterator yp = y;
                    std::set<int>::iterator zp = z;

                    ++xp;
                    ++yp;
                    ++zp;

                    if (xp != sgX.end()){
                        if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*xp,*y,*z}) != rev_map.end())) {
                            boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*xp,*y,*z}], euclidian_dist({*x,*y,*z}, {*xp,*y,*z}), g.g);
                            //print_v({*x,*y,*z});
                            //print_v({*xp,*y,*z});
                            //std::cout << "\n";
                        }
                    }
                    if (yp != sgY.end()){
                        if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*x,*yp,*z}) != rev_map.end())) {
                            boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*x,*yp,*z}], euclidian_dist({*x,*y,*z}, {*x,*yp,*z}), g.g);
                            //print_v({*x,*y,*z});
                            //print_v({*x,*yp,*z});
                            //std::cout << "\n";
                        }
                    }
                    if (zp != Z.end()){
                        if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*x,*y,*zp}) != rev_map.end())){
                            boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*x,*y,*zp}], euclidian_dist({*x,*y,*z}, {*x,*y,*zp}), g.g);
                            //print_v({*x,*y,*z});
                            //print_v({*x,*y,*zp});
                            //std::cout << "\n";
                        }
                    }
                }
            }
        }
    }





    /*
    //Arestas
    for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
        for (std::set<int>::iterator x = X.begin(); x != X.end(); ++x) {
            for (std::set<int>::iterator y = Y.begin(); y != Y.end(); ++y) {
                std::set<int>::iterator xp = x;
                std::set<int>::iterator yp = y;
                std::set<int>::iterator zp = z;

                ++xp;
                ++yp;
                ++zp;

                if (xp != X.end()){
                    if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*xp,*y,*z}) != rev_map.end())) {
                        boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*xp,*y,*z}], euclidian_dist({*x,*y,*z}, {*xp,*y,*z}), g.g);
                        //print_v({*x,*y,*z});
                        //print_v({*xp,*y,*z});
                        //std::cout << "\n";
                    }
                }
                if (yp != Y.end()){
                    if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*x,*yp,*z}) != rev_map.end())) {
                        boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*x,*yp,*z}], euclidian_dist({*x,*y,*z}, {*x,*yp,*z}), g.g);
                        //print_v({*x,*y,*z});
                        //print_v({*x,*yp,*z});
                        //std::cout << "\n";
                    }
                }
                if (zp != Z.end()){
                    if ((rev_map.find({*x,*y,*z}) != rev_map.end()) && (rev_map.find({*x,*y,*zp}) != rev_map.end())){
                        boost::add_edge(rev_map[{*x,*y,*z}], rev_map[{*x,*y,*zp}], euclidian_dist({*x,*y,*z}, {*x,*y,*zp}), g.g);
                        //print_v({*x,*y,*z});
                        //print_v({*x,*y,*zp});
                        //std::cout << "\n";
                    }
                }
            }
        }
    }

*/

std::cout << "removing side-to-side edges from obstacles\n";
for (std::set<int>::iterator z = Z.begin(); z != Z.end(); ++z) {
    for (Shape o : Layers[*z].Obstacles) {
        auto x1 = X.find(o.A[0]);
        auto x2 = X.find(o.B[0]);
        ++x1;

        auto y1 = Y.find(o.A[1]);
        auto y2 = Y.find(o.C[1]);
        --y1;

        if (*x1 == o.B[0]) {
            auto yy = y1;
            while (*yy != *y2) {
                auto nu = rev_map[{o.A[0], *yy, o.A[2]}];
                auto nv = rev_map[{o.B[0], *yy, o.A[2]}];
                boost::remove_edge(nu, nv, g.g);
                boost::remove_edge(nv, nu, g.g);
                --yy;
            }
        }

        if (*y1 == o.C[1]) {
            auto xx = x1;
            while (*xx != *x2) {
                auto nu = rev_map[{*xx, o.A[1], o.A[2]}];
                auto nv = rev_map[{*xx, o.C[1], o.A[2]}];
                boost::remove_edge(nu, nv, g.g);
                boost::remove_edge(nv, nu, g.g);
                ++xx;
            }
        }
    }
}
    //for (auto i = allVertices.begin(); i != allVertices.end(); ++i) {
        //std::cout << (*i)[0] << ", " << (*i)[1] << "\n";
    //}
}


void Circuit::components_edges(vector<nEdge>* edges) {
    EI ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(this->g.g); ei != ei_end; ++ei) {
        Edge ne;
        ne.u = ver_map[source(*ei, this->g.g)];
        ne.v = ver_map[target(*ei, this->g.g)];

        bool flag = false;
        for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
            for (Shape c : it->second.Components) {
                if (c.collide_with_edge(ne)) {
                    flag = true;
                    break;
                }
            }
            if (flag) break;
        }

        if (flag) {
            edges->emplace_back(source(*ei, this->g.g), target(*ei, this->g.g), boost::get(edge_weight, this->g.g, *ei));
        }
    }
}


void Circuit::componentEdgesSpanning(Vertex A, Vertex B, Vertex C, vector<nEdge>* edges) {
    std::set<int> X, Y;

    // subX
    for (std::set<int>::iterator it = XY.first.find(A[0]); it != XY.first.find(B[0]); ++it) {
        X.insert(*it);
    }
    X.insert(B[0]);
    // subY
    for (std::set<int>::iterator it = XY.second.find(C[1]); it != XY.second.find(B[1]); ++it) {
        Y.insert(*it);
    }
    Y.insert(B[1]);


    // Adiciona as arestas verticais
    for (std::set<int>::iterator it_x = X.begin(); it_x != X.end(); ++it_x) {
        for (std::set<int>::iterator it_y = Y.begin(); it_y != Y.end(); ++it_y) {

            // Testar se o primeiro ponto da aresta existe
            while (rev_map.find({*it_x, *it_y, A[2]}) == rev_map.end() && it_y != Y.end()) {
                ++it_y;
            }
            if (it_y == Y.end()) {
                break;
            }
            std::set<int>::iterator it_yp = it_y;
            ++it_yp;


            // Testar se o segundo ponto da aresta existe
            while (rev_map.find({*it_x, *it_yp, A[2]}) == rev_map.end() && it_yp != Y.end()) {
                ++it_yp;
            }
            if (it_y == Y.end()) {
                break;
            }

            if (it_yp == Y.end()) break;

            Vertex u = {*it_x, *it_y, A[2]};
            Vertex v = {*it_x, *it_yp, A[2]};

            nEdge ne = nEdge(this->rev_map[u], this->rev_map[v], euclidian_dist(u, v));
            if(std::find(edges->begin(), edges->end(), ne) == edges->end())
                edges->push_back(ne);
        }
    }

    // Adiciona as arestas horizontais
    for (auto it_y = Y.begin(); it_y != Y.end(); ++it_y) {
        for (auto it_x = X.begin(); it_x != X.end(); ++it_x) {
            // Testar se o primeiro ponto da aresta existe
            while (rev_map.find({*it_x, *it_y, A[2]}) == rev_map.end() && it_x != X.end()) {
                ++it_x;
            }
            if (it_x == X.end()) {
                break;
            }
            std::set<int>::iterator it_xp = it_x;
            ++it_xp;

            // Testar se o segundo ponto da aresta existe
            while (rev_map.find({*it_xp, *it_y, A[2]}) == rev_map.end() && it_xp != X.end()) {
                ++it_xp;
            }
            if (it_x == X.end()) {
                break;
            }

            if (it_xp == X.end()) break;

            Vertex u = {*it_x, *it_y, A[2]};
            Vertex v = {*it_xp, *it_y, A[2]};

            nEdge ne = nEdge(this->rev_map[u], this->rev_map[v], euclidian_dist(u, v));
            if(std::find(edges->begin(), edges->end(), ne) == edges->end())
                edges->push_back(ne);
        }
    }
}


void Circuit::componentEdges(Vertex A, Vertex B, Vertex C, vector<nEdge>* edges) {
    std::set<int> X, Y;

    // subX
    for (std::set<int>::iterator it = XY.first.find(A[0]); it != XY.first.find(B[0]); ++it) {
        X.insert(*it);
    }
    X.insert(B[0]);
    // subY
    for (std::set<int>::iterator it = XY.second.find(C[1]); it != XY.second.find(B[1]); ++it) {
        Y.insert(*it);
    }
    Y.insert(B[1]);


    // Adiciona as arestas verticais
    for (std::set<int>::iterator it_x = X.begin(); it_x != X.end(); ++it_x) {
        for (std::set<int>::iterator it_y = Y.begin(); it_y != Y.end(); ++it_y) {
            std::set<int>::iterator it_yp = it_y;
            ++it_yp;
            if (it_yp == Y.end()) break;

            Vertex u = {*it_x, *it_y, A[2]};
            Vertex v = {*it_x, *it_yp, A[2]};
            nEdge ne = nEdge(this->rev_map[u], this->rev_map[v], euclidian_dist(u, v));
            if(std::find(edges->begin(), edges->end(), ne) == edges->end())
                edges->push_back(ne);
        }
    }

    // Adiciona as arestas horizontais
    for (auto it_y = Y.begin(); it_y != Y.end(); ++it_y) {
        for (auto it_x = X.begin(); it_x != X.end(); ++it_x) {
            auto it_xp = it_x;
            ++it_xp;
            if (it_xp == X.end()) break;

            Vertex u = {*it_x, *it_y, A[2]};
            Vertex v = {*it_xp, *it_y, A[2]};

            nEdge ne = nEdge(this->rev_map[u], this->rev_map[v], euclidian_dist(u, v));
            if(std::find(edges->begin(), edges->end(), ne) == edges->end())
                edges->push_back(ne);
        }
    }
}


void Circuit::components_edges2(vector<nEdge>* edges) {
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        for (Shape c : it->second.Components) {
            componentEdgesSpanning(c.A, c.B, c.C, edges);
        }
    }
}


void Circuit::spanning_tree(bool gen_img) {
    std::cout << "Spanning tree\n";
    std::vector <E> spanning_tree;
    int num_edges = boost::num_edges(this->g.g);
    MST mst(num_edges, &this->g.g);
    vector<nEdge> components;
    vector<nEdge> components2;

    std::cout << "  Getting component edges\n";
    components_edges(&components2);
    components_edges2(&components);

    std::cout << components.size() << " - " << components2.size() << "\n";


    for (unsigned i = 0; i < components.size(); i++) {
        std::cout << components[i].u  << "<->" << components[i].v << ": " << components[i].w << " / " << components2[i].u  << "<->" << components2[i].v << ": " << components2[i].w << "\n";
    }
    /*
    */
    std::cout << "  Finding spanning tree\n";
    vector<nEdge> kruskal = mst.compute(1, components);
    std::cout << "  Done finding spanning tree\n";

    spanning = Graph(boost::num_vertices(this->g.g));

    for (nEdge e : kruskal) {
        boost::add_edge(e.u, e.v, e.w, spanning);
    }
    //to_dot(spanning);


    if (gen_img){
        EI ei, ei_end;
        std::string out;
        std::ofstream myfile;
        myfile.open("out/teste.dot");
        myfile << "graph {\n";

        for (boost::tie(ei, ei_end) = boost::edges(this->spanning); ei != ei_end; ++ei) {
            Edge ne;
            ne.u = ver_map[source(*ei, this->spanning)];
            ne.v = ver_map[target(*ei, this->spanning)];

            bool flag = false;
            for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
                for (Shape c : it->second.Components) {
                    if (c.collide_with_edge(ne)) {
                        flag = true;
                        break;
                    }
                }
                if (flag) break;
            }

            if (!flag) {
                myfile << source(*ei, this->spanning) << " -- " << target(*ei, this->spanning) << "\n";
            }
            else {
                myfile << source(*ei, this->spanning) << " -- " << target(*ei, this->spanning) << " [color=\"red\"]\n";
            }
        }

        myfile << "}";
        myfile.close();
        std::cout << out;
    }

}


std::set<Vertex> Circuit::component_vertices (Vertex A, Vertex B, Vertex C) {
    std::set<int> X, Y;
    std::set<Vertex> subV;

    // subX
    for (std::set<int>::iterator it = XY.first.find(A[0]); it != XY.first.find(B[0]); ++it) {
        X.insert(*it);
    }
    X.insert(B[0]);
    // subY
    for (std::set<int>::iterator it = XY.second.find(C[1]); it != XY.second.find(B[1]); ++it) {
        Y.insert(*it);
    }
    Y.insert(B[1]);


    // Adiciona as arestas verticais
    for (auto x : X) {
        for (auto y : Y) {
            subV.insert({x, y, A[2]});
        }
    }
    return subV;
}


void Circuit::components_vertices(std::set<Vertex> * c_vertices) {
    //std::cout << "components_verticeSJHAGJDGHKJASDHs\n";
    // std::set<Vertex> c_vertices;
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        for (Shape c : it->second.Components) {
            for (auto v : component_vertices(c.A, c.B, c.C)) {
                c_vertices->insert(v);
                //std::cout << "perdi";
            }
        }
    }
    //std::cout << c_vertices.size() << "\n";
    // return c_vertices;
}


void Circuit::close_component(Vertex A, Vertex B, Vertex C) {
    std::set<int> X, Y;

    // subX
    for (std::set<int>::iterator it = XY.first.find(A[0]); it != XY.first.find(B[0]); ++it) {
        X.insert(*it);
    }
    X.insert(B[0]);
    // subY
    for (std::set<int>::iterator it = XY.second.find(C[1]); it != XY.second.find(B[1]); ++it) {
        Y.insert(*it);
    }
    Y.insert(B[1]);


    // Adiciona as arestas verticais
    for (std::set<int>::iterator it_x = X.begin(); it_x != X.end(); ++it_x) {
        for (std::set<int>::iterator it_y = Y.begin(); it_y != Y.end(); ++it_y) {
            std::set<int>::iterator it_yp = it_y;
            ++it_yp;
            if (it_yp == Y.end()) break;

            Vertex u = {*it_x, *it_y, A[2]};
            Vertex v = {*it_x, *it_yp, A[2]};

            boost::add_edge(this->rev_map[u], this->rev_map[v], 0, this->spanning);
        }
    }

    // Adiciona as arestas horizontais
    for (std::set<int>::iterator it_y = Y.begin(); it_y != Y.end(); ++it_y) {
        for (auto it_x = X.begin(); it_x != X.end(); ++it_x) {
            auto it_xp = it_x;
            ++it_xp;
            if (it_xp == X.end()) break;

            Vertex u = {*it_x, *it_y, A[2]};
            Vertex v = {*it_xp, *it_y, A[2]};

            boost::add_edge(this->rev_map[u], this->rev_map[v], 0, this->spanning);
        }
    }
}


void Circuit::close_components_cycles() {
    std::cout << "close_components_cycles\n";

    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        for (Shape c : it->second.Components) {
            close_component(c.A, c.B, c.C);
        }
    }
}


string v_string(Vertex v) {
    return string("(" + std::to_string(v[0]) + ", " + std::to_string(v[1]) + ", " + std::to_string(v[2]) + ")\n");
}

/*
struct less_than_key {
    inline bool operator() (const MyStruct& struct1, const MyStruct& struct2) {
        return (struct1.key < struct2.key);
    }
};
*/

std::vector<bool> visited;
std::vector<bool> alive;
std::set<Vertex> comp_vertices;

bool Circuit::isComponent (V n) {
    return comp_vertices.find(ver_map[n]) != comp_vertices.end();
}


bool Circuit::remove (int n) {
    //std::cout << "remove\n";
    if (visited[n]) {
        return alive[n];
    }

    visited[n] = true;
    //std::cout << n << " - ";
    if (isComponent(n)) {
        alive[n] = true;
    }

    for (auto v : boost::make_iterator_range(boost::adjacent_vertices(n, this->spanning))) {
        //std::cout << "v/" << n << "-" << v << ":" << visited[v] << "\n";
        if (!visited[v]) {
            alive[n] = remove(v) || alive[n];
        }
    }
    return alive[n];
}

void Circuit::remove_one_degree_vertices() {
    std::cout << "remove_one_degree_vertices\n";
    //bool flag = true;

    long num = num_vertices(this->spanning);
    std::cout << "  Number of vertices: " << num << "\n";
    std::vector<bool> visited1(num, false);
    std::vector<bool> alive1(num, false);
    std::set<Vertex> comp_vertices1;
    components_vertices(&comp_vertices1);
    // std::cout << comp_vertices1.size() << "\n\n\n";
    visited = visited1;
    alive = alive1;
    comp_vertices = comp_vertices1;
    Vertex start = *comp_vertices1.begin();
    auto perdi = this->rev_map[start];
    this->remove(perdi);

    // int c = 0;
    // for (auto i : alive) {
    //     if (i) {
    //         std::cout << c << "-";
    //         print_v(this->ver_map[c]);
    //     }
    //     // std::cout << "\n";
    //     c++;
    // }
    /*
    while (flag) {
        flag = false;
        for (std::pair<VI, VI> vi = boost::vertices(this->spanning); vi.first != vi.second; ++vi.first) {
            if (!visited[*vi.first] && boost::degree(*vi.first, this->spanning) < 2) {
                flag = true;
                visited[*vi.first] = true;
                boost::clear_vertex(*vi.first, this->spanning);
                break;
            }
        }
    }
    //to_dot(this->spanning);
*/
    int soma = 0;
    for (int i : alive) {
        if (i) soma++;
    }
    std::cout << "Result size (vertices): " << soma << "\n";
/*
    std::cout << "Generating result file\n\n";
    EI ei, ei_end;
    std::ofstream myfile, result;
    myfile.open("out/grau1.dot");
    result.open("out/result.out");
    myfile << "graph {\n";

    for (boost::tie(ei, ei_end) = boost::edges(this->spanning); ei != ei_end; ++ei) {
        Edge ne;
        ne.u = ver_map[source(*ei, this->spanning)];
        ne.v = ver_map[target(*ei, this->spanning)];

        // VVV Isso tudo é pra colocar cor diferente nas arestas dos componentes VVV
        // bool flag = false;
        // for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
        //     for (Shape c : it->second.Components) {
        //         if (c.collide_with_edge(ne)) {
        //             flag = true;
        //             break;
        //         }
        //     }
        //     if (flag) break;
        // }

        // RESULTADO
        // if () {
        //     myfile << source(*ei, this->spanning) << " -- " << target(*ei, this->spanning) << "\n";
        //     result << v_string(ver_map[source(*ei, this->spanning)]);
        //     result << v_string(ver_map[target(*ei, this->spanning)]);
        //     result << "\n";
        //
        // }
        // COMPONENTES
        if (alive[source(*ei, this->spanning)] and alive[target(*ei, this->spanning)]) {
            myfile << source(*ei, this->spanning) << " -- " << target(*ei, this->spanning) << " [color=\"blue\"]\n";
            result << v_string(ver_map[source(*ei, this->spanning)]);
            result << v_string(ver_map[target(*ei, this->spanning)]);
            result << "\n";
        }
        // AAA Isso tudo é pra colocar cor diferente nas arestas dos componentes AAA

    }
    myfile << "}";
    myfile.close();
    */
}


void Circuit::generate_output() {
    // EI ei, ei_end;
    // std::vector<Edge> output;
    //
    // for (boost::tie(ei, ei_end) = boost::edges(this->spanning); ei != ei_end; ++ei) {
    //     Edge ne;
    //     ne.u = ver_map[source(*ei, this->spanning)];
    //     ne.v = ver_map[target(*ei, this->spanning)];
    //
    //     if(std::find(output.begin(), output.end(), ne) == output.end())
    //         output.push_back(ne);
    // }
    //
    // std::string out;
    // std::ofstream myfile;
    // myfile.open("out/saida.txt");
    //
    //
    // for (Edge e : output) {
    //     if (e.u[2] != e.v[2]) { // Via
    //         myfile << "Via V" << e.u[2] << " (" << e.u[0] << "," << e.u[1] << ")\n";
    //     }
    //     else if (e.u[0] == e.v[0]) { // Vertical
    //         myfile << "V-line M" << e.u[2] << " (" << e.u[0] << "," << e.u[1] << ") (" << e.v[0] << "," << e.v[1] << ")" << "\n";
    //     }
    //     else { // Horizontal
    //         myfile << "H-line M" << e.u[2] << " (" << e.u[0] << "," << e.u[1] << ") (" << e.v[0] << "," << e.v[1] << ")" << "\n";
    //     }
    // }
    std::string out;
    std::ofstream myfile;
    myfile.open("out/saida.txt");

    EI ei, ei_end;
    std::vector<Edge> output;

    for (boost::tie(ei, ei_end) = boost::edges(this->spanning); ei != ei_end; ++ei) {
        Edge ne;
        ne.u = ver_map[source(*ei, this->spanning)];
        ne.v = ver_map[target(*ei, this->spanning)];

        if(std::find(output.begin(), output.end(), ne) == output.end()) {
            output.push_back(ne);

            // Se os 2 pertencem ao mesmo componente, não desenhar
            //if (std::find(comp_vertices.begin(), comp_vertices.end(), source(*ei, this->spanning)) == comp_vertices.end()) {


                if (alive[source(*ei, this->spanning)] && alive[target(*ei, this->spanning)]){
                    if (ne.u[2] != ne.v[2]) { // Via
                        myfile << "Via V" << ne.u[2] << " (" << ne.u[0] << "," << ne.u[1] << ")\n";
                    }
                    else if (ne.u[0] == ne.v[0]) { // Vertical
                        myfile << "V-line M" << ne.u[2] << " (" << ne.u[0] << "," << ne.u[1] << ") (" << ne.v[0] << "," << ne.v[1] << ")" << "\n";
                    }
                    else { // Horizontal
                        myfile << "H-line M" << ne.u[2] << " (" << ne.u[0] << "," << ne.u[1] << ") (" << ne.v[0] << "," << ne.v[1] << ")" << "\n";
                    }
                }
            //}
        }
    }
}


void Circuit::connect_all_components() {
    /*
        grafo = Grafo()
        for i in componentes:
            for j in components[1:]:
                grafo.add(boost.astar(i, j, hanan))
    */
    Graph bi1s_n = Graph(boost::num_vertices(this->g.g));

    // Iterar nos componentes de todas as camadas
    for (std::map<int, Layer>::iterator it = Layers.begin(); it != Layers.end(); ++it) { // Itera nas camadas
        for (std::vector<Shape>::iterator si = it->second.Components.begin(); si != it->second.Components.end(); ++si) { // Itera nos shapes das camadas

            std::vector<Shape>::iterator siN = si;
            ++siN;
            for (std::map<int, Layer>::iterator jt = it; jt != Layers.end(); ++jt) {
                for (std::vector<Shape>::iterator sij = siN; sij != jt->second.Components.end(); ++sij) { // Itera nos shapes das camadas
                    // Adicionar caminho (si <--> sij)

                }
                ++jt;
                siN = jt->second.Components.begin();
                --jt;
            }
        }
    }
}












//
