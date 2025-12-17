/*############################################################################
# Copyright (c) 2020 Source Simian  :  https://github.com/sourcesimian/uICAL #
############################################################################*/
#include "uICAL/cppstl.h"
#include "uICAL/types.h"
#include "uICAL/util.h"
#include "uICAL/error.h"
#include "uICAL/counter.h"
#include "uICAL/logging.h"
#include "uICAL/cascade.h"

namespace uICAL {
    // Cascade::Cascade() {}

    void Cascade::add(Counter_ptr counter) {
        this->counters.push_back(counter);
    }

    void Cascade::add(std::vector<Counter_ptr> counters) {
        for (Counter_ptr counter : counters) {
            this->counters.push_back(counter);
        }
    }

    size_t Cascade::size() const {
        return this->counters.size();
    }

    bool Cascade::reset(const DateStamp& base) {
        std::ignore = base;  // TODO
        return false;
    }

    DateStamp Cascade::value() const {
        return this->counters.front()->value();
    }

    bool Cascade::syncLock(const DateStamp& from, const DateStamp& now) const {
        std::ignore = from;  // TODO
        std::ignore = now;  // TODO
        return true;
    };

    const string Cascade::name() const {
        Joiner names(',');
        for (auto counter : this->counters) {
            counter->str(names.out());
            names.next();
        }
        ostream out;
        names.write(out);
        return out.str();
    }

    void Cascade::wrap() {
    }

    bool Cascade::initCounters(const DateStamp& base, const DateStamp& begin) {
        counters_t::iterator it = this->counters.end();
        -- it;
        if(!(*it)->reset(base)) {
            return false;
        }

        // needsAdvance is set when a counter exhausts its span while seeking
        // This signals that we should advance via the cascade's next() mechanism
        bool needsAdvance = false;

        bool result = this->resetCascade(it, [&](counters_t::iterator it) {
            while (!(*it)->syncLock(begin, (*it)->value())) {
                if (!(*it)->next()) {
                    // Counter exhausted its current span while seeking to begin.
                    // This happens when DTSTART doesn't fall on a valid BYDAY, etc.
                    // Signal that we need to advance to next period.
                    needsAdvance = true;
                    return;  // Exit the sync lambda, let cascade handle it
                }
            }
        });

        if (!result) {
            return false;
        }

        // If any counter exhausted while seeking, advance cascade to find first valid occurrence
        if (needsAdvance) {
            // Keep calling next() until we find a valid occurrence or exhaust
            int maxIterations = 1000;  // Safety limit
            while (maxIterations-- > 0) {
                if (!this->next()) {
                    return false;
                }
                // Check if current value is >= begin
                DateStamp current = this->value();
                if (begin <= current) {
                    return true;
                }
            }
            return false;  // Exceeded iteration limit
        }

        return true;
    }

    bool Cascade::resetCascade(counters_t::iterator it, sync_f sync) {
        DateStamp base = (*it)->value();
        for (;;) {
            if (it == this->counters.begin())
                break;
            -- it;
            if (!(*it)->reset(base)) {
                ++ it;
                if(!(*it)->next()) {
                    ++ it;
                    if (it != this->counters.end()) {
                        throw ImplementationError("No next available from upper level counter");
                    }
                    -- it;
                }
            }

            if (sync != nullptr) {
                sync(it);
            }

            if (it == this->counters.begin())
                break;
            base = (*it)->value();
            // if (this->expired(base)) {
            //     return false;
            // }
        }
        return true;
    }

    bool Cascade::next() {
        counters_t::iterator it = this->counters.begin();

        if(!(*it)->next()) {
            for (;;) {
                ++ it;
                if (it == this->counters.end()) {
                    -- it;
                    if (it == this->counters.begin())
                        return true;
                    break;
                }
                if ((*it)->next())
                    break;
            }
            if (!this->resetCascade(it, nullptr)) {
                return false;
            }
        }
        return true;
    }

    void Cascade::str(ostream& out) const {
        Joiner counters('+');
        for (auto counter : this->counters) {
            counter->str(counters.out());
            counters.next();
        }
        counters.str(out);
    }
}
