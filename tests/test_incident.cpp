// Unit tests for Module 1 (Incident & Dynamic Priority Engine) - Pavani Jain
#include <cmath>
#include "../include/incident.hpp"
#include "test_framework.hpp"

int main() {
    // Priority formulas (polymorphism): severity 9, impact 50, wait 5
    EmergencyEvent em(1, "ISBT", 9, 50, 5);
    em.updatePriority();
    CHECK(fabs(em.getPriority() - 7.00) < 1e-9, "Emergency priority = 0.6*9 + 0.3*5 + 0.1*1 = 7.00");

    TrafficEvent tr(2, "Rajpur Road", 7, 80, 15);
    tr.updatePriority();
    CHECK(fabs(tr.getPriority() - 6.50) < 1e-9, "Traffic priority = 6.50");

    // Waiting-time component is capped at 10
    WasteEvent w(3, "Prem Nagar", 5, 120, 1000);
    w.updatePriority();
    CHECK(fabs(w.getPriority() - (0.45 * 5 + 0.30 * 10 + 0.25 * 10)) < 1e-9, "Waiting-time score capped at 10");

    // Longer waiting raises priority
    InfrastructureEvent i1(4, "A", 5, 50, 0), i2(5, "A", 5, 50, 50);
    i1.updatePriority(); i2.updatePriority();
    CHECK(i2.getPriority() > i1.getPriority(), "Higher waiting time gives higher priority");

    // Validation
    CHECK_THROWS(TrafficEvent(9, "X", 11, 10, 0), "Severity above 10 rejected");
    CHECK_THROWS(TrafficEvent(9, "X", 0, 10, 0), "Severity below 1 rejected");
    CHECK_THROWS(TrafficEvent(9, "X", 5, -1, 0), "Negative impact rejected");

    // Engine
    IncidentEngine eng;
    eng.addIncident(new TrafficEvent(101, "Rajpur Road", 7, 80, 15));
    eng.addIncident(new EmergencyEvent(102, "ISBT", 9, 50, 5));
    eng.addIncident(new WasteEvent(103, "Prem Nagar", 5, 120, 30));
    eng.addIncident(new InfrastructureEvent(104, "Sahastradhara", 8, 35, 10));

    CHECK(eng.size() == 4, "Four incidents stored in linked list");
    CHECK(eng.nextPriorityId() == 102, "Priority queue top is incident 102");
    CHECK(eng.findById(103) != nullptr && eng.findById(999) == nullptr, "Hash lookup finds 103, not 999");
    CHECK(eng.treeContains(104) && !eng.treeContains(999), "BST index contains 104, not 999");
    CHECK_THROWS(eng.addIncident(new TrafficEvent(101, "Dup", 5, 5, 5)), "Duplicate ID rejected");

    auto sorted = eng.sortedByPriority();
    bool ordered = true;
    for (size_t k = 1; k < sorted.size(); ++k)
        if (sorted[k - 1]->getPriority() < sorted[k]->getPriority()) ordered = false;
    CHECK(ordered && sorted.front()->getId() == 102, "Insertion sort gives descending priority");

    // Status history (stack) and pop of pending incidents
    size_t before = eng.historySize();
    eng.changeStatus(102, "Assigned");
    CHECK(eng.historySize() == before + 1, "Status change pushed on history stack");
    CHECK(eng.popNextPendingId() == 103, "Assigned incident 102 skipped; next pending is 103");
    CHECK_THROWS(eng.changeStatus(555, "Resolved"), "Status change on unknown ID throws");

    // File handling
    eng.saveToFile("/tmp/test_incidents.csv");
    std::ifstream in("/tmp/test_incidents.csv");
    std::string header; std::getline(in, header);
    CHECK(header.rfind("ID,Type,Location", 0) == 0, "CSV file written with header");
    CHECK_THROWS(eng.saveToFile("/nonexistent_dir/x.csv"), "Saving to invalid path throws");

    return TEST_SUMMARY();
}
