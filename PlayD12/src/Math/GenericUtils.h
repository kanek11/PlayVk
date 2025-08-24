#pragma once


struct BoolEdgeDetector {

    bool previous = false;
    bool risingEdge = false;
    bool fallingEdge = false;

    void Update(bool current) {
        risingEdge = (!previous && current);
        fallingEdge = (previous && !current);
        previous = current;
    }

    bool Get() const { return previous; }
    operator bool() const { return previous; }
};