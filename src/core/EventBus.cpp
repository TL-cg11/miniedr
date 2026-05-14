#include "EventBus.hpp"

void EventBus::subscribe(EventType type, EventHandler handler) {
	m_subscribers[type].push_back(handler);
}

void EventBus::publish(const Event& event) {
	auto it = m_subscribers.find(event.type);
	if (it != m_subscribers.end()) {			// 있으면 end()는 없음
		for (const auto& handler : it->second) {
			handler(event);
		}
	}
}