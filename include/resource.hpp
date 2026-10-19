#ifndef RESOURCE_HPP
#define RESOURCE_HPP
// Module 2 - Resource & Infrastructure Management (owner: Manan Kumar)
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

struct Resource {
    int id;
    string type;
    string location;
    string status;       // Available / Busy / Under Maintenance
    int capacity;
};

class ResourceManager {
    vector<Resource> resources;

    static bool validStatus(const string& s) {
        return s == "Available" || s == "Busy" || s == "Under Maintenance";
    }

public:
    // Which resource type can handle which incident type.
    static string requiredResourceType(const string& incidentType) {
        if (incidentType == "Emergency") return "Ambulance";
        if (incidentType == "Traffic") return "Traffic Team";
        if (incidentType == "Waste") return "Waste Vehicle";
        if (incidentType == "Infrastructure") return "Maintenance Team";
        return "";
    }

    void addResource(const Resource& r) {
        if (!validStatus(r.status)) throw invalid_argument("Invalid resource status: " + r.status);
        if (r.capacity < 0) throw invalid_argument("Capacity cannot be negative.");
        for (const auto& x : resources)
            if (x.id == r.id) throw invalid_argument("Resource ID already exists.");
        resources.push_back(r);
    }

    size_t size() const { return resources.size(); }

    const Resource* findById(int id) const {
        for (const auto& r : resources) if (r.id == id) return &r;
        return nullptr;
    }

    // Step 1: match type.  Step 2: keep only "Available" resources.
    vector<Resource> suitableAvailable(const string& incidentType) const {
        vector<Resource> out;
        const string needed = requiredResourceType(incidentType);
        if (needed.empty()) return out;
        for (const auto& r : resources)
            if (r.type == needed && r.status == "Available") out.push_back(r);
        return out;
    }

    // Step 3: among the candidates pick the one with the smallest distance.
    // distanceTo(location) is supplied by the graph module; a negative value means unreachable.
    const Resource* selectNearest(const string& incidentType,
                                  const function<int(const string&)>& distanceTo) const {
        const Resource* best = nullptr;
        int bestDistance = numeric_limits<int>::max();
        const string needed = requiredResourceType(incidentType);
        if (needed.empty()) return nullptr;
        for (const auto& r : resources) {
            if (r.type != needed || r.status != "Available") continue;
            int d = distanceTo(r.location);
            if (d < 0) continue;                       // unreachable
            if (d < bestDistance) { bestDistance = d; best = &r; }
        }
        return best;
    }

    void setStatus(int id, const string& status) {
        if (!validStatus(status)) throw invalid_argument("Invalid resource status: " + status);
        for (auto& r : resources)
            if (r.id == id) { r.status = status; return; }
        throw runtime_error("Resource ID not found.");
    }

    void setBusy(int id) { setStatus(id, "Busy"); }
    void release(int id) { setStatus(id, "Available"); }

    size_t countAvailable() const {
        size_t c = 0;
        for (const auto& r : resources) if (r.status == "Available") ++c;
        return c;
    }

    void print() const {
        cout << "\nResources:\n";
        for (const auto& r : resources) {
            cout << "R" << r.id << " | " << r.type << " | " << r.location
                 << " | " << r.status << " | capacity=" << r.capacity << '\n';
        }
    }

    void saveToFile(const string& filename) const {
        ofstream out(filename);
        if (!out) throw runtime_error("Unable to open resource output file.");
        out << "ID,Type,Location,Status,Capacity\n";
        for (const auto& r : resources)
            out << r.id << ',' << r.type << ',' << r.location << ',' << r.status << ',' << r.capacity << '\n';
    }
};

#endif // RESOURCE_HPP
