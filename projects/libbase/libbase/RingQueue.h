#pragma once
#include <mutex>
#include "ringbuffer.h"

template <class T>
class RingQueue
{
	RingBuffer<T> _buffer;
	std::mutex _lock;

public:
	RingQueue(int size)
		: _buffer(size), _lock()
	{}

	~RingQueue()
	{}

	T get()
	{
		std::lock_guard<std::mutex> l(_lock);
		while (_buffer.is_empty())
		{
			return T();
		}
		return _buffer.read();
	}

	void put(T data)
	{
		std::lock_guard<std::mutex> l(_lock);
		_buffer.write(data);
	}
};
