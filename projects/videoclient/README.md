# Memory critical places
List here all places, where it might be possible that memory could stack up.

	- All encoding- and decoding threads (audio + video)
		The internal queue may get very big, if it does not get processed fast enougth.
		Slow CPU?


# Reminder for performance improvements

	- AudioDecodingThread
		Use OpusFrame* instead of OpusFrameRefPtr (see VideoDecodingThread)
	
		The following code could be reduced by one mutex-lock action by simply
		checking the _queue.empty() directlry after .wait() again.

```cpp
		QMutexLocker l(&_m);
		if (_queue.isEmpty())
		{
			_queueCond.wait(&_m);
			continue;
		}
		auto item = _queue.dequeue();
		l.unlock();
```