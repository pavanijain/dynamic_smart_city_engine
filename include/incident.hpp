#ifndef INCIDENT_HPP
#define INCIDENT_HPP
// Module 1 - Incident & Dynamic Priority Engine (owner: Pavani Jain)
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

// -----------------------------
// Common incident model / OOP
// -----------------------------
class CityEvent {
protected:
    int id;
    string type;
    string location;
    int severity;       // 1-10
    int impact;         // people affected, capped to a practical score
    int waitingTime;    // minutes
    string status;
    double priority;

public:
    CityEvent(int id, string type, string location, int severity,
              int impact, int waitingTime)
        : id(id), type(std::move(type)), location(std::move(location)),
          severity(severity), impact(impact), waitingTime(waitingTime),
          status("Reported"), priority(0.0) {
        if (severity < 1 || severity > 10)
            throw invalid_argument("Severity must be between 1 and 10.");
        if (impact < 0 || waitingTime < 0)
            throw invalid_argument("Impact and waiting time cannot be negative.");
    }

    virtual ~CityEvent() = default;

    int getId() const { return id; }
    const string& getType() const { return type; }
    const string& getLocation() const { return location; }
    int getSeverity() const { return severity; }
    int getImpact() const { return impact; }
    int getWaitingTime() const { return waitingTime; }
    const string& getStatus() const { return status; }
    double getPriority() const { return priority; }

    void setStatus(const string& s) { status = s; }
    void addWaitingTime(int minutes) { waitingTime += max(0, minutes); }

    // Impact is normalised to a 0-10 range for the weighted score.
    double normalisedImpact() const {
        return min(10.0, impact / 10.0);
    }

    virtual double calculatePriority() = 0;
    virtual string categoryDescription() const = 0;

    void updatePriority() { priority = calculatePriority(); }

    virtual void print() const {
        cout << left << setw(6) << id
             << setw(17) << type
             << setw(18) << location
             << setw(9) << severity
             << setw(9) << impact
             << setw(9) << waitingTime
             << setw(11) << fixed << setprecision(2) << priority
             << setw(14) << status << '\n';
    }
};

class TrafficEvent : public CityEvent {
public:
    TrafficEvent(int id, const string& location, int severity, int impact, int wait)
        : CityEvent(id, "Traffic", location, severity, impact, wait) {}
    double calculatePriority() override {
        return 0.5 * severity + 0.3 * normalisedImpact() + 0.2 * min(10, waitingTime / 5);
    }
    string categoryDescription() const override { return "Traffic-related incident"; }
};

class WasteEvent : public CityEvent {
public:
    WasteEvent(int id, const string& location, int severity, int impact, int wait)
        : CityEvent(id, "Waste", location, severity, impact, wait) {}
    double calculatePriority() override {
        return 0.45 * severity + 0.30 * normalisedImpact() + 0.25 * min(10, waitingTime / 5);
    }
    string categoryDescription() const override { return "Waste collection incident"; }
};

class InfrastructureEvent : public CityEvent {
public:
    InfrastructureEvent(int id, const string& location, int severity, int impact, int wait)
        : CityEvent(id, "Infrastructure", location, severity, impact, wait) {}
    double calculatePriority() override {
        return 0.55 * severity + 0.25 * normalisedImpact() + 0.20 * min(10, waitingTime / 5);
    }
    string categoryDescription() const override { return "Infrastructure incident"; }
};

class EmergencyEvent : public CityEvent {
public:
    EmergencyEvent(int id, const string& location, int severity, int impact, int wait)
        : CityEvent(id, "Emergency", location, severity, impact, wait) {}
    double calculatePriority() override {
        return 0.60 * severity + 0.30 * normalisedImpact() + 0.10 * min(10, waitingTime / 5);
    }
    string categoryDescription() const override { return "Emergency incident"; }
};

// Factory function: creates the appropriate incident dynamically.
// The returned unique_ptr makes ownership explicit and safe.
inline unique_ptr<CityEvent> makeCityEvent(
    const string& type, int id, const string& location,
    int severity, int impact, int waitingTime) {
    if (type == "Traffic")
        return make_unique<TrafficEvent>(id, location, severity, impact, waitingTime);
    if (type == "Waste")
        return make_unique<WasteEvent>(id, location, severity, impact, waitingTime);
    if (type == "Infrastructure")
        return make_unique<InfrastructureEvent>(id, location, severity, impact, waitingTime);
    if (type == "Emergency")
        return make_unique<EmergencyEvent>(id, location, severity, impact, waitingTime);
    throw invalid_argument("Unknown incident type: " + type);
}

// -----------------------------
// Simple BST for ID indexing
// -----------------------------
struct BSTNode {
    int key;
    BSTNode* left;
    BSTNode* right;
    explicit BSTNode(int k) : key(k), left(nullptr), right(nullptr) {}
};

class IncidentBST {
    BSTNode* root = nullptr;

    BSTNode* insert(BSTNode* node, int key) {
        if (!node) return new BSTNode(key);
        if (key < node->key) node->left = insert(node->left, key);
        else if (key > node->key) node->right = insert(node->right, key);
        return node;
    }

    bool search(BSTNode* node, int key) const {
        if (!node) return false;
        if (node->key == key) return true;
        return key < node->key ? search(node->left, key) : search(node->right, key);
    }

    void destroy(BSTNode* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    ~IncidentBST() { destroy(root); }
    void insert(int key) { root = insert(root, key); }
    bool search(int key) const { return search(root, key); }
};

// -----------------------------
// Incident engine
// -----------------------------
class IncidentEngine {
    list<unique_ptr<CityEvent>> records;         // linked list owns dynamically allocated incidents
    queue<int> waitingQueue;                     // normal waiting order
    priority_queue<pair<double,int>> priorityQ;  // highest priority first
    unordered_map<int, CityEvent*> idIndex;      // hashing
    IncidentBST idTree;                          // tree-based ID index
    stack<string> history;                       // recent status history

public:
    // unique_ptr owns each dynamic incident and automatically deletes it.
    // The compiler-generated destructor is therefore sufficient.
    IncidentEngine() = default;
    IncidentEngine(const IncidentEngine&) = delete;
    IncidentEngine& operator=(const IncidentEngine&) = delete;

    void addIncident(unique_ptr<CityEvent> event) {
        if (!event) throw invalid_argument("Null incident object.");
        const int id = event->getId();
        if (idIndex.count(id)) throw invalid_argument("Incident ID already exists.");

        event->updatePriority();
        CityEvent* rawEvent = event.get();
        // Store ownership before publishing non-owning indexes.
        records.push_back(std::move(event));
        waitingQueue.push(id);
        priorityQ.push({rawEvent->getPriority(), id});
        idIndex[id] = rawEvent;
        idTree.insert(id);
        history.push("Incident " + to_string(id) + " created (Reported)");
    }

    // Backward-compatible overload for existing callers that use new.
    // Ownership transfers to the engine immediately, even if validation fails.
    void addIncident(CityEvent* event) {
        addIncident(unique_ptr<CityEvent>(event));
    }

    CityEvent* findById(int id) const {
        auto it = idIndex.find(id);
        return it == idIndex.end() ? nullptr : it->second;
    }

    bool treeContains(int id) const { return idTree.search(id); }

    int nextPriorityId() const {
        if (priorityQ.empty()) return -1;
        return priorityQ.top().second;
    }

    // Removes and returns the highest-priority incident that is still "Reported".
    // Incidents that were already assigned/resolved are skipped (lazy deletion).
    int popNextPendingId() {
        while (!priorityQ.empty()) {
            int id = priorityQ.top().second;
            priorityQ.pop();
            CityEvent* e = findById(id);
            if (e && e->getStatus() == "Reported") return id;
        }
        return -1;
    }

    size_t size() const { return records.size(); }

    vector<CityEvent*> sortedByPriority() const {
        vector<CityEvent*> v;
        v.reserve(records.size());
        for (const auto& event : records) v.push_back(event.get());
        // Insertion sort: complete ordered view for reporting.
        for (size_t i = 1; i < v.size(); ++i) {
            CityEvent* key = v[i];
            int j = static_cast<int>(i) - 1;
            while (j >= 0 && v[j]->getPriority() < key->getPriority()) {
                v[j + 1] = v[j];
                --j;
            }
            v[j + 1] = key;
        }
        return v;
    }

    void changeStatus(int id, const string& newStatus) {
        CityEvent* e = findById(id);
        if (!e) throw runtime_error("Incident ID not found.");
        string old = e->getStatus();
        e->setStatus(newStatus);
        history.push("Incident " + to_string(id) + ": " + old + " -> " + newStatus);
    }

    void printAll() const {
        cout << "\n" << left << setw(6) << "ID" << setw(17) << "Type"
             << setw(18) << "Location" << setw(9) << "Sev"
             << setw(9) << "Impact" << setw(9) << "Wait"
             << setw(11) << "Priority" << setw(14) << "Status" << '\n';
        cout << string(93, '-') << '\n';
        for (const auto& e : records) e->print();
    }

    void printSorted() const {
        cout << "\nIncidents sorted by priority (Insertion Sort):\n";
        cout << left << setw(6) << "ID" << setw(17) << "Type"
             << setw(18) << "Location" << setw(9) << "Sev"
             << setw(9) << "Impact" << setw(9) << "Wait"
             << setw(11) << "Priority" << setw(14) << "Status" << '\n';
        cout << string(93, '-') << '\n';
        for (auto* e : sortedByPriority()) e->print();
    }

    size_t historySize() const { return history.size(); }

    void printHistory() const {
        stack<string> copy = history;
        cout << "\nRecent status history (LIFO stack):\n";
        if (copy.empty()) { cout << "No history.\n"; return; }
        while (!copy.empty()) { cout << "- " << copy.top() << '\n'; copy.pop(); }
    }

    void printQueue() const {
        queue<int> copy = waitingQueue;
        cout << "\nWaiting queue (FIFO): ";
        while (!copy.empty()) { cout << copy.front() << ' '; copy.pop(); }
        cout << '\n';
    }

    void saveToFile(const string& filename) const {
        ofstream out(filename);
        if (!out) throw runtime_error("Unable to open incident output file.");
        out << "ID,Type,Location,Severity,Impact,WaitingTime,Priority,Status\n";
        for (const auto& ownedEvent : records) {
            const CityEvent* e = ownedEvent.get();
            out << e->getId() << ',' << e->getType() << ',' << e->getLocation() << ','
                << e->getSeverity() << ',' << e->getImpact() << ',' << e->getWaitingTime() << ','
                << fixed << setprecision(2) << e->getPriority() << ',' << e->getStatus() << '\n';
        }
    }
};


#endif // INCIDENT_HPP


    
