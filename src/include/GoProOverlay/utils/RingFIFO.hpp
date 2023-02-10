#pragma once

namespace utils
{

	template <typename T>
	class RingFIFO
	{
	public:
		RingFIFO(
			T *buffer,
			size_t capacity)
		 : buffer_(buffer)
		 , capacity_(capacity)
		 , size_(0)
		 , head_(0)
		 , tail_(0)
		{
			if (capacity == 0)
			{
				throw std::runtime_error("capacity can't be zero");
			}
		}

		T front() const
		{
			return buffer_[head_];
		}

		T back() const
		{
			return buffer_[tail_];
		}

		bool empty() const
		{
			return size_ == 0;
		}

		size_t size() const
		{
			return size_;
		}

		size_t capacity() const
		{
			return capacity_;
		}

		void push(const T &value)
		{
			if (size_ == 0)
			{
				// buffer empty. safe to store at current head location.
				buffer_[head_] = value;
				size_ = 1;
			}
			else if (size_ == capacity_)
			{
				// buffer is full. move head and tail forward and store value in the
				// tail's previous location. effectively deletes oldest value.
				head_ = tail_;
				tail_ = nextIndex(tail_);
				buffer_[head_] = value;
			}
			else
			{
				// buffer not empty, but not full yet. jump to next head and store there.
				head_ = nextIndex(head_);
				buffer_[head_] = value;
				size_++;
			}
		}

		void pop()
		{
			if (size_ == 0)
			{
				// do nothing
			}
			else
			{
				tail_ = nextIndex(tail_);
				size_--;
			}
		}

	private:
		size_t nextIndex(size_t index) const
		{
			index++;
			if (index >= capacity_)
				return 0;
			return index;
		}

	private:
		T *buffer_;
		size_t capacity_;
		size_t size_;
		size_t head_;// front
		size_t tail_;// back

	};

}