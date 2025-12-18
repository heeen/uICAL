#include "../catch.hpp"

#include "uICAL.h"

#include "uICAL/vevent.h"

#include <fstream>

TEST_CASE("Calendar::basic", "[uICAL][Calendar]") {
    std::ifstream input(std::string("test/data/ical1.txt"));
    uICAL::istream_stl ical(input);

    uICAL::TZMap_ptr tzmap = uICAL::new_ptr<uICAL::TZMap>();
    auto cal = uICAL::Calendar::load(ical, tzmap);
    REQUIRE(cal->as_str() == "CALENDAR\n");

    auto calIt = uICAL::new_ptr<uICAL::CalendarIter>(cal,
        uICAL::DateTime("20191016T102000Z"),
        uICAL::DateTime("20191017T103000EST", tzmap));

    auto next = [=](){
        if(calIt->next()) {
            return calIt->current()->as_str();
        }
        else {
            return uICAL::string("END");
        }
    };

    REQUIRE(next() ==
        "Calendar EVENT: Irrigation Front\n"
        " - start: 20191017T100000EST\n"
        " - span: PT20M\n"
    );

    REQUIRE(next() ==
        "Calendar EVENT: Irrigation Back\n"
        " - start: 20191017T102000EST\n"
        " - span: PT5M\n"
    );

    REQUIRE(next() ==
        "Calendar EVENT: Irrigation Beds\n"
        " - start: 20191017T103000EST\n"
        " - span: PT10M\n"
    );

    REQUIRE(next() == "END");
}


TEST_CASE("Calendar::finite", "[uICAL][Calendar]") {
    std::ifstream input(std::string("test/data/ical1.txt"));
    uICAL::istream_stl ical(input);

    uICAL::TZMap_ptr tzmap = uICAL::new_ptr<uICAL::TZMap>();
    auto cal = uICAL::Calendar::load(ical, tzmap, [](const uICAL::VEvent& event) {
        if (event.summary == "Irrigation Back") return true;
        return false;
    });

    auto calIt = uICAL::new_ptr<uICAL::CalendarIter>(cal,
        uICAL::DateTime("20191016T102000Z"),
        uICAL::DateTime("20191017T103000Z"));
}

TEST_CASE("Calendar::folding", "[uICAL][Calendar]") {
    std::ifstream input(std::string("test/data/ical_folding.txt"));
    uICAL::istream_stl ical(input);

    uICAL::TZMap_ptr tzmap = uICAL::new_ptr<uICAL::TZMap>();
    auto cal = uICAL::Calendar::load(ical, tzmap);
    REQUIRE(cal->as_str() == "CALENDAR\n");

    auto calIt = uICAL::new_ptr<uICAL::CalendarIter>(
        cal,
        uICAL::DateTime("20191016T102000Z"),
        uICAL::DateTime("20191017T103000EST", tzmap));

    auto next = [=]() {
        if(calIt->next()) {
            return calIt->current()->as_str();
        } else {
            return uICAL::string("END");
        }
    };

    REQUIRE(next() ==
            "Calendar EVENT: This is a short line that is folded.\n"
            " - start: 20191017T102000EST\n"
            " - span: PT5M\n");

    REQUIRE(next() ==
            "Calendar EVENT: This is a long line which needs to wrap as it's "
            "longer than 75 characters. "
            "https://icalendar.org/iCalendar-RFC-5545/3-1-content-lines.html "
            "states that lines longer than 75 characters should be folded.\n"
            " - start: 20191017T103000EST\n"
            " - span: PT10M\n");

    REQUIRE(next() == "END");
}

TEST_CASE("Calendar::folding_escaped_multiline", "[uICAL][Calendar]") {
    // Test parsing of multi-line DESCRIPTION with:
    // - Escaped characters: \n (newline), \, (comma), \; (semicolon)
    // - UTF-8 characters (€)
    // - Line folding that breaks mid-word/number
    std::ifstream input(std::string("test/data/ical_folding.txt"));
    uICAL::istream_stl ical(input);

    uICAL::TZMap_ptr tzmap = uICAL::new_ptr<uICAL::TZMap>();
    auto cal = uICAL::Calendar::load(ical, tzmap);
    REQUIRE(cal->as_str() == "CALENDAR\n");

    // Query date range that includes the 2022 ski trip event
    auto calIt = uICAL::new_ptr<uICAL::CalendarIter>(
        cal,
        uICAL::DateTime("20220120T000000Z"),
        uICAL::DateTime("20220131T000000Z"));

    auto next = [=]() {
        if(calIt->next()) {
            return calIt->current()->as_str();
        } else {
            return uICAL::string("END");
        }
    };

    // Verify the event with escaped characters and UTF-8 parses correctly
    auto result = next();
    REQUIRE(result.find("Ski Trip with escaped chars") != uICAL::string::npos);

    REQUIRE(next() == "END");
}
