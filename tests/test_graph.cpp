// Unit tests for Module 3 (City Graph & Routing Engine) - Karan Babbar
#include "../include/graph.hpp"
#include "test_framework.hpp"

static CityGraph demoGraph() {
    CityGraph g;
    for (const char* l : {"Clock Tower", "ISBT", "Rajpur Road", "Ballupur", "Sahastradhara", "Prem Nagar"})
        g.addLocation(l);
    g.addRoad("Clock Tower", "Rajpur Road", 4);
    g.addRoad("Clock Tower", "ISBT", 5);
    g.addRoad("Clock Tower", "Ballupur", 6);
    g.addRoad("Rajpur Road", "Sahastradhara", 7);
    g.addRoad("Rajpur Road", "Ballupur", 5);
    g.addRoad("Ballupur", "Prem Nagar", 6);
    g.addRoad("ISBT", "Prem Nagar", 8);
    g.addRoad("Sahastradhara", "ISBT", 9);
    return g;
}

int main() {
    CityGraph g = demoGraph();
    CHECK(g.size() == 6, "Graph has 6 locations");
    CHECK(g.hasLocation("ISBT") && !g.hasLocation("Mars"), "hasLocation works");

    // Dijkstra
    CHECK(g.shortestDistance("Ballupur", "ISBT") == 11, "Ballupur -> ISBT = 11");
    CHECK(g.shortestDistance("Clock Tower", "Clock Tower") == 0, "Distance to itself = 0");
    CHECK(g.shortestDistance("Prem Nagar", "Rajpur Road") == 11, "Prem Nagar -> Rajpur Road = 11");
    CHECK(g.shortestDistance("ISBT", "Ballupur") == g.shortestDistance("Ballupur", "ISBT"), "Roads are undirected (symmetry)");

    // Path reconstruction
    auto res = g.dijkstra("Ballupur");
    auto path = g.reconstructPath(res.second, "Ballupur", "ISBT");
    CHECK(path.size() == 3 && path[0] == "Ballupur" && path[1] == "Clock Tower" && path[2] == "ISBT",
          "Route is Ballupur -> Clock Tower -> ISBT");

    // BFS / DFS
    auto bfs = g.bfs("Clock Tower");
    CHECK(bfs.size() == 6 && g.getNames()[bfs[0]] == "Clock Tower", "BFS visits all 6 nodes from Clock Tower");
    auto dfs = g.dfs("Clock Tower");
    CHECK(dfs.size() == 6 && g.getNames()[dfs[0]] == "Clock Tower", "DFS visits all 6 nodes from Clock Tower");
    CHECK(g.getNames()[bfs[1]] == "Rajpur Road", "BFS visits nearest-by-edge neighbour first");
    CHECK(g.isConnected(), "Demo graph is connected");

    // Unreachable location
    CityGraph h = demoGraph();
    h.addLocation("Island");
    CHECK(h.shortestDistance("Clock Tower", "Island") == -1, "Unreachable location returns -1");
    CHECK(!h.isConnected(), "Graph with isolated node is not connected");
    auto r2 = h.dijkstra("Clock Tower");
    CHECK(h.reconstructPath(r2.second, "Clock Tower", "Island").empty(), "No path to unreachable node");

    // Validation / exceptions
    CHECK_THROWS(g.addRoad("ISBT", "Ballupur", -3), "Negative road weight rejected");
    CHECK_THROWS(g.addRoad("ISBT", "Nowhere", 3), "Road to unknown location rejected");
    CHECK_THROWS(g.shortestDistance("Nowhere", "ISBT"), "Unknown source throws");

    // Shorter alternative is chosen
    CityGraph k;
    for (const char* l : {"A", "B", "C"}) k.addLocation(l);
    k.addRoad("A", "C", 10);
    k.addRoad("A", "B", 3);
    k.addRoad("B", "C", 4);
    CHECK(k.shortestDistance("A", "C") == 7, "Dijkstra prefers A-B-C (7) over direct A-C (10)");

    return TEST_SUMMARY();
}
