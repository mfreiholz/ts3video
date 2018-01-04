#pragma once

template <class T>
class RingBuffer
{
	T* _buffer;
	int _size;
	int _start;
	int _end;

public:
	RingBuffer(int size = 4096)
		: _buffer(0), _size(0), _start(0), _end(0)
	{
		_size = size + 1;
		_buffer = new T[size + 1];
	}

	~RingBuffer()
	{
		if (_buffer)
			delete[] _buffer;
		_buffer = 0;
	}

	void write(T data)
	{
		_buffer[_end] = data;
		_end = (_end + 1) % _size;
		if (_end == _start)
			_start = (_start + 1) % _size;
	}

	T read()
	{
		T data = _buffer[_start];
		_start = (_start + 1) % _size;
		return data;
	}

	bool is_full() const
	{
		return ((_end + 1) % _size) == _start;
	}

	bool is_empty() const
	{
		return _end == _start;
	}
};
