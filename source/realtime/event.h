//
// MIT License
//
// Copyright (c) 2024 Elias Engelbert Plank
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef REALTIME_EVENT_H
#define REALTIME_EVENT_H

#include <functional>
#include "realtime.h"

namespace rt {

// A list of all event types
enum class EventType {
    Scroll,
    Cursor
};

// Represents an event
struct Event {
    /// Retrieves the type of the event
    virtual EventType type() const = 0;
};

// Represents a scroll event
struct ScrollEvent : public Event {
    f64 x;
    f64 y;

    /// Creates a new scroll event
    /// @param x The scroll in the x direction
    /// @param y The scroll in the y direction
    ScrollEvent(f64 x, f64 y);

    /// Retrieves the type of the event
    EventType type() const override;
};

struct CursorEvent : public Event {
    f64 x;
    f64 y;

    /// Creates a new cursor event
    /// @param x The movement in the x direction
    /// @param y The movement in the y direction
    CursorEvent(f64 x, f64 y);

    /// Retrieves the type of the event
    EventType type() const override;
};

struct EventListener {
    EventType type;
    std::function<void(const Event &)> handler;
};

}// namespace rt

#endif// REALTIME_EVENT_H
