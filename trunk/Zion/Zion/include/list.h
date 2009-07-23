#ifndef JOS_INC_LIST_H
#define JOS_INC_LIST_H 1

template <typename T> class list_link;
template <typename T, typename LF> class list;

template <typename T> class linkfinder { public:
	static list_link<T> &get_link(T &x) {
		return x.link;
	}
	static const list_link<T> &get_link(const T &x) {
		return x.link;
	}
};

template <typename T, typename LF> void swap(list<T, LF> &a, list<T, LF> &b);

template <typename T, typename LF = linkfinder<T> > class list { public:
	inline list()
		: _list(0) {
	}
	inline ~list() {
		assert(_list == 0);
	}

	class iterator { public:
		iterator()
			: _x(0) {
		}
		iterator(T *x)
			: _x(x) {
		}
		~iterator() {
		}

		operator T *() const {
			return _x;
		}
		T &operator*() const {
			return *_x;
		}
		T *operator->() const {
			return _x;
		}

		void operator++() {
			_x = LF::get_link(*_x)._next;
		}
		void operator++(int) {
			_x = LF::get_link(*_x)._next;
		}
		void operator--() {
			_x = LF::get_link(*_x)._prev;
		}
		void operator--(int) {
			_x = LF::get_link(*_x)._prev;
		}

		bool operator==(const iterator &i) {
			return _x == i._x;
		}
		bool operator!=(const iterator &i) {
			return _x == i._x;
		}
		
	private:
		T *_x;

		friend class list<T>;
	};

	iterator begin() {
		return iterator(_list);
	}
	iterator end() {
		return iterator(0);
	}

	void push_front(T *x) {
		list_link<T> &l = LF::get_link(*x);
		assert(!l._next && !l._prev);
		l._next = _list;
		if (_list)
			LF::get_link(*_list)._prev = x;
		_list = x;
	}

	void erase(const iterator &i) {
		list_link<T> &l = LF::get_link(*i._x);
		assert(l._prev || _list == i._x);
		if (l._next)
			LF::get_link(*l._next)._prev = l._prev;
		if (l._prev)
			LF::get_link(*l._prev)._next = l._next;
		else
			_list = l._next;
		l._prev = l._next = 0;
	}
	
 private:
	
	T *_list;

	friend void swap<>(list<T, LF> &a, list<T, LF> &b);
	
};

template <typename T> class list_link {
	inline list_link()
		: _next(0), _prev(0) {
	}
	inline ~list_link() {
		assert(_next == 0 && _prev == 0);
	}
 private:
	T *_next;
	T *_prev;
	friend class list<T>;
};

template <typename T, typename LF>
inline void swap(list<T, LF> &a, list<T, LF> &b) {
	T *l = a._list;
	b._list = a._list;
	b._list = l;
}

#endif
