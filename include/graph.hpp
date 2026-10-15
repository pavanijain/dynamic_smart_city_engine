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
    CityGraph()
        : names(new vector<string>()),
          index(new unordered_map<string, int>()),
          adj(new vector<vector<pair<int, int>>>()) {}

    // Prevent unsafe copying of owned pointers
    CityGraph(const CityGraph&) = delete;
    CityGraph& operator=(const CityGraph&) = delete;

    // Destructor: release allocated memory
    ~CityGraph() {
        delete names;
        delete index;
        delete adj;
    }

    // Add a location
    void addLocation(const string& name) {
        if (index->count(name))
            return;

        int id = static_cast<int>(names->size());

        (*index)[name] = id;
        names->push_back(name);
        adj->push_back({});
    }

    // Add a two-way road
    void addRoad(const string& a, const string& b, int distance) {
        if (distance < 0)
            throw invalid_argument(
                "Road distance cannot be negative.");

        if (!index->count(a) || !index->count(b))
            throw invalid_argument(
                "Unknown graph location.");

        int u = (*index).at(a);
        int v = (*index).at(b);

        (*adj)[u].push_back({v, distance});
        (*adj)[v].push_back({u, distance});
    }

    // Get the index of a location
    int getIndex(const string& name) const {
        auto it = index->find(name);

        if (it == index->end())
            throw invalid_argument(
                "Location not found: " + name);

        return it->second;
    }

    const vector<string>& getNames() const {
        return *names;
    }

    size_t size() const {
        return names->size();
    }

    bool hasLocation(const string& name) const {
        return index->count(name) > 0;
    }

    // Breadth-First Search
    vector<int> bfs(const string& start) const {
        vector<int> order;
        vector<bool> visited(names->size(), false);
        queue<int> q;

        int s = getIndex(start);
        visited[s] = true;
        q.push(s);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            order.push_back(u);

            for (auto [v, w] : (*adj)[u]) {
                (void)w;

                if (!visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }

        return order;
    }

    // Depth-First Search
    vector<int> dfs(const string& start) const {
        vector<int> order;
        vector<bool> visited(names->size(), false);
        stack<int> st;

        st.push(getIndex(start));

        while (!st.empty()) {
            int u = st.top();
            st.pop();

            if (visited[u])
                continue;

            visited[u] = true;
            order.push_back(u);

            for (auto it = (*adj)[u].rbegin();
                 it != (*adj)[u].rend(); ++it) {
                if (!visited[it->first])
                    st.push(it->first);
            }
        }

        return order;
    }

    // Dijkstra: returns distances and parent indices
    pair<vector<int>, vector<int>>
    dijkstra(const string& source) const {
        const int INF = numeric_limits<int>::max() / 4;

        vector<int> dist(names->size(), INF);
        vector<int> parent(names->size(), -1);

        priority_queue<
            pair<int, int>,
            vector<pair<int, int>>,
            greater<pair<int, int>>
        > pq;

        int s = getIndex(source);
        dist[s] = 0;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d != dist[u])
                continue;

            for (auto [v, w] : (*adj)[u]) {
                // Avoid integer overflow
                if (w >= INF - d)
                    continue;

                int newDist = d + w;

                if (dist[v] > newDist) {
                    dist[v] = newDist;
                    parent[v] = u;
                    pq.push({newDist, v});
                }
            }
        }

        return {dist, parent};
    }

    // Shortest distance; -1 if unreachable
    int shortestDistance(const string& source,
                         const string& target) const {
        auto result = dijkstra(source);
        int d = result.first[getIndex(target)];

        const int INF = numeric_limits<int>::max() / 4;

        return d >= INF ? -1 : d;
    }

    // Check whether all locations are reachable
    bool isConnected() const {
        if (names->empty())
            return true;

        return bfs((*names)[0]).size() == names->size();
    }

    // Reconstruct a path using Dijkstra's parent array
    vector<string> reconstructPath(
        const vector<int>& parent,
        const string& source,
        const string& target) const {

        int s = getIndex(source);
        int t = getIndex(target);

        vector<string> path;

        if (parent.size() != names->size())
            throw invalid_argument(
                "Invalid parent array.");

        if (s != t && parent[t] == -1)
            return path;

        int cur = t;

        while (cur != -1) {
            path.push_back((*names)[cur]);

            if (cur == s)
                break;

            cur = parent[cur];
        }

        reverse(path.begin(), path.end());

        if (path.empty() || path.front() != source)
            path.clear();

        return path;
    }
};

// Print traversal order
inline void printTraversal(
    const CityGraph& g,
    const vector<int>& order) {

    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0)
            cout << " -> ";

        cout << g.getNames()[order[i]];
    }

    cout << '\n';
}

#endif // GRAPH_HPP
```
