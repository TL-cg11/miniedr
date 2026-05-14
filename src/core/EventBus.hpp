#pragma once

#include "Event.hpp"	// Event 구조체
#include <functional>	// std::function
#include <map>			// std::map
#include <vector>		// std::vector

using EventHandler = std::function<void(const Event&)>;

class EventBus {
public:
	void subscribe(EventType type, EventHandler handler);
	void publish(const Event& event);

private:
	std::map<EventType, std::vector<EventHandler>> m_subscribers;
};

