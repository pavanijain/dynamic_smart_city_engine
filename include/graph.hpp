```cpp
#ifndef GRAPH_HPP
#define GRAPH_HPP

// Module 3 - City Graph & Routing Engine
// Owner: Karan Babbar

#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

class CityGraph {
private:
    vector<string>* names;
    unordered_map<string, int>* index;
    vector<vector<pair<int, int>>>* adj;

public:
    // Constructor: dynamically allocate data structures
    CityGraph() {
        names = new vector<string>();
        index = new unordered_map<string, int>();
        adj = new vector<vector<pair<int, int>>>();
    }

    // Prevent unsafe copying of pointers
    CityGraph(const CityGraph&) = delete;
    CityGraph& operator=(const CityGraph&) = delete;

    // Destructor: release allocated memory
    ~CityGraph() {
        delete names;
        delete index;
        delete adj;
    }

    void addLocation(const string& name) {
        if (index->count(name)) return;

        (*index)[name] = static_cast<int>(names->size());
        names->push_back(name);
        adj->push_back({});
    }

    void addRoad(const string& a, const string& b, int distance) {
        if (distance < 0)
            throw invalid_argument("Road distance cannot be negative.");

        if (!index->count(a) || !index->count(b))
            throw invalid_argument("Unknown graph location.");

        int u = (*index)[a];
        int v = (*index)[b];

        (*adj)[u].push 
