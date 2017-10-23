#include <iostream>
#include <set>
#include <array>


typedef std::array<int, 3> Vertex;

struct Edge {
    Vertex u;
    Vertex v;
    int w;

    bool operator< (const Edge& x) const {
        return u[0] != x.u[0] || u[1] != x.u[1] || u[2] != x.u[2] ||
               v[0] != x.v[0] || v[1] != x.v[1] || v[2] != x.v[2];
    }

    bool operator== (const Edge& x) const {
        return (u[0] == x.u[0] && u[1] == x.u[1] && u[2] == x.u[2] &&
               v[0] == x.v[0] && v[1] == x.v[1] && v[2] == x.v[2]) ||
               (u[0] == x.v[0] && u[1] == x.v[1] && u[2] == x.v[2] &&
                v[0] == x.u[0] && v[1] == x.u[1] && v[2] == x.u[2]);
    }
};

Edge create (Vertex u, Vertex v, int w) {
    Edge new_e;
    new_e.u = u;
    new_e.v = v;
    new_e.w = w;
    return new_e;
}


int main () {
    std::set<Edge> edges;

    Vertex a = {1, 1, 1};
    Vertex b = {2, 2, 2};
    Vertex c = {3, 3, 3};
    Vertex d = {4, 4, 4};


    edges.insert(create(a, b, 0));
    edges.insert(create(a, c, 0));
    edges.insert(create(a, b, 0));
    edges.insert(create(b, d, 0));
    edges.insert(create(c, d, 0));

    for (std::set<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        Edge e = *it;
        std::cout << "(" << e.u[0] << ", " << e.u[1] << ", " << e.u[2] << ") ("
                  << "(" << e.v[0] << ", " << e.v[1] << ", " << e.v[2] << ")\n";
    }

    if (create(c, b, 0) == create(b, a, 0)) std::cout << "igual\n";
    else std::cout << "diferente\n";

    return 0;
}
