#ifndef GRAPH_HPP
#define GRAPH_HPP
// Module 3 - City Graph & Routing Engine (owner: Karan Babbar)
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

// -----------------------------
// City graph and routing
// -----------------------------
class CityGraph {
    vector<string> names;
    unordered_map<string, int> index;
    vector<vector<pair<int,int>>> adj;

public:
    void addLocation(const string& name) {
        if (index.count(name)) return;
        index[name] = static_cast<int>(names.size());
        names.push_back(name);
        adj.push_back({});
    }

    void addRoad(const string& a, const string& b, int distance) {
        if (distance < 0) throw invalid_argument("Road distance cannot be negative.");
        if (!index.count(a) || !index.count(b)) throw invalid_argument("Unknown graph location.");
        int u = index[a], v = index[b];
        adj[u].push_back({v, distance});
        adj[v].push_back({u, distance});
    }

    int getIndex(const string& name) const {
        auto it = index.find(name);
        if (it == index.end()) throw invalid_argument("Location not found: " + name);
        return it->second;
    }

    const vector<string>& getNames() const { return names; }

    vector<int> bfs(const string& start) const {
        vector<int> order;
        vector<bool> visited(names.size(), false);
        queue<int> q;
        int s = getIndex(start); visited[s] = true; q.push(s);
        while (!q.empty()) {
            int u = q.front(); q.pop(); order.push_back(u);
            for (auto [v, w] : adj[u]) {
                (void)w;
                if (!visited[v]) { visited[v] = true; q.push(v); }
            }
        }
        return order;
    }

    vector<int> dfs(const string& start) const {
        vector<int> order;
        vector<bool> visited(names.size(), false);
        stack<int> st;
        st.push(getIndex(start));
        while (!st.empty()) {
            int u = st.top(); st.pop();
            if (visited[u]) continue;
            visited[u] = true; order.push_back(u);
            for (auto it = adj[u].rbegin(); it != adj[u].rend(); ++it)
                if (!visited[it->first]) st.push(it->first);
        }
        return order;
    }

    pair<vector<int>, vector<int>> dijkstra(const string& source) const {
        const int INF = numeric_limits<int>::max() / 4;
        vector<int> dist(names.size(), INF), parent(names.size(), -1);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        int s = getIndex(source); dist[s] = 0; pq.push({0, s});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d != dist[u]) continue;
            for (auto [v, w] : adj[u]) {
                if (dist[v] > d + w) {
                    dist[v] = d + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        return {dist, parent};
    }

    size_t size() const { return names.size(); }
    bool hasLocation(const string& name) const { return index.count(name) > 0; }
    int countRoads() const {
    int total = 0;
    for (const auto& roads : adj)
        total += static_cast<int>(roads.size());
    return total / 2;
}



    // Shortest distance between two locations, or -1 if no route exists.
    int shortestDistance(const string& source, const string& target) const {
        auto result = dijkstra(source);
        int d = result.first[getIndex(target)];
        return d >= numeric_limits<int>::max() / 4 ? -1 : d;
    }

    // True if every location can be reached from the first one.
    bool isConnected() const {
        if (names.empty()) return true;
        return bfs(names[0]).size() == names.size();
    }

    vector<string> reconstructPath(const vector<int>& parent, const string& source,
                                    const string& target) const {
        int t = getIndex(target), s = getIndex(source);
        vector<string> path;
        if (t != s && parent[t] == -1) return path;
        for (int cur = t; cur != -1; cur = parent[cur]) {
            path.push_back(names[cur]);
            if (cur == s) break;
        }
        reverse(path.begin(), path.end());
        if (path.empty() || path.front() != source) path.clear();
        return path;
    }
};


inline void printTraversal(const CityGraph& g, const vector<int>& order) {
    for (size_t i = 0; i < order.size(); ++i) {
        if (i) cout << " -> ";
        cout << g.getNames()[order[i]];
    }
    cout << '\n';
}

#endif // GRAPH_HPP
