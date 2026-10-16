// Unit tests for Module 2 (Resource & Infrastructure Management) - Manan Kumar
#include "../include/resource.hpp"
#include "test_framework.hpp"

int main() {
    ResourceManager rm;
    rm.addResource({1, "Traffic Team", "Clock Tower", "Available", 5});
    rm.addResource({2, "Ambulance", "Ballupur", "Available", 2});
    rm.addResource({3, "Waste Vehicle", "Prem Nagar", "Available", 1});
    rm.addResource({4, "Maintenance Team", "Rajpur Road", "Available", 6});
    rm.addResource({5, "Ambulance", "Sahastradhara", "Busy", 2});

    CHECK(rm.size() == 5, "Five resources stored");
    CHECK(ResourceManager::requiredResourceType("Emergency") == "Ambulance", "Emergency needs Ambulance");
    CHECK(ResourceManager::requiredResourceType("Waste") == "Waste Vehicle", "Waste needs Waste Vehicle");
    CHECK(ResourceManager::requiredResourceType("Unknown").empty(), "Unknown incident type has no resource");

    // Filtering: busy ambulance R5 must be excluded
    auto amb = rm.suitableAvailable("Emergency");
    CHECK(amb.size() == 1 && amb[0].id == 2, "Only available ambulance R2 is suitable");
    CHECK(rm.suitableAvailable("Unknown").empty(), "No candidates for unknown type");

    // Nearest selection using a fake distance function
    ResourceManager rm2;
    rm2.addResource({10, "Ambulance", "FarPlace", "Available", 2});
    rm2.addResource({11, "Ambulance", "NearPlace", "Available", 2});
    auto dist = [](const std::string& loc) { return loc == "NearPlace" ? 3 : 20; };
    const Resource* best = rm2.selectNearest("Emergency", dist);
    CHECK(best && best->id == 11, "Nearest ambulance (R11) selected");

    // Unreachable resources are skipped
    auto unreachable = [](const std::string&) { return -1; };
    CHECK(rm2.selectNearest("Emergency", unreachable) == nullptr, "Unreachable resources give no selection");

    // All busy -> incident stays pending
    rm2.setBusy(10); rm2.setBusy(11);
    CHECK(rm2.selectNearest("Emergency", dist) == nullptr, "All busy -> nullptr (incident pending)");

    // Release makes it available again
    rm2.release(11);
    best = rm2.selectNearest("Emergency", dist);
    CHECK(best && best->id == 11, "Released resource can be selected again");
    CHECK(rm2.countAvailable() == 1, "Available count is 1 after release");

    // Maintenance status
    rm2.setStatus(11, "Under Maintenance");
    CHECK(rm2.selectNearest("Emergency", dist) == nullptr, "Under Maintenance resource is not selected");

    // Validation
    CHECK_THROWS(rm.addResource({1, "Ambulance", "X", "Available", 1}), "Duplicate resource ID rejected");
    CHECK_THROWS(rm.addResource({99, "Ambulance", "X", "Flying", 1}), "Invalid status rejected");
    CHECK_THROWS(rm.addResource({98, "Ambulance", "X", "Available", -1}), "Negative capacity rejected");
    CHECK_THROWS(rm.setBusy(12345), "Unknown resource ID throws");

    // File handling
    rm.saveToFile("/tmp/test_resources.csv");
    std::ifstream in("/tmp/test_resources.csv");
    std::string header; std::getline(in, header);
    CHECK(header == "ID,Type,Location,Status,Capacity", "Resource CSV header correct");
    CHECK_THROWS(rm.saveToFile("/nonexistent_dir/r.csv"), "Saving to invalid path throws");

    return TEST_SUMMARY();
}
