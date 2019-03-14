#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cassert>
#include <cstddef>

#include <iostream>


namespace circ {

//==================================================================================================
template<typename T>
struct basic_string_ref
{
	using value_type = T;

	constexpr basic_string_ref() :
		b{nullptr},
		e{nullptr} {}

	constexpr basic_string_ref(value_type *b, value_type *e) :
		b{b},
		e{e} {}

	constexpr basic_string_ref(value_type *s) :
		b{s},
		e{s + std::strlen(s)} {}

	template<typename U>
	constexpr basic_string_ref(basic_string_ref<U> const &rhs) :
		b{rhs.b},
		e{rhs.e} {}

	bool empty() const { return b == e; }
	size_t size() const { return e - b; }

	value_type *b, *e;
};

using string_ref = basic_string_ref<char>;
using cstring_ref = basic_string_ref<char const>;


template<typename T>
bool operator == (basic_string_ref<T> const &a, basic_string_ref<T> const &b)
{
	if(a.size() != b.size())
		return false;

	auto cur_a = a.b;
	auto cur_b = b.b;
	while(cur_a != a.e)
	{
		if(*cur_a++ != *cur_b++)
			return false;
	}

	return true;
}

template<typename T>
std::ostream& operator << (std::ostream &os, basic_string_ref<T> a)
{
	return os.write(a.b, a.size());
}

template<typename T, typename Str>
bool operator == (basic_string_ref<T> const &a, Str const *str)
{
	return a == basic_string_ref<Str const>{str};
}

template<typename T>
std::basic_string<typename std::remove_const<T>::type> to_str(basic_string_ref<T> const &ref)
{
	return {ref.b, ref.e};
}

template<typename T>
bool starts_with(basic_string_ref<T> const &ref, typename std::remove_const<T>::type const *str)
{
	auto cur = ref.b;
	while(cur != ref.e && *str)
	{
		if(*cur++ != *str++)
			return false;
	}

	return *str == 0;
}

template<typename T>
bool starts_with(basic_string_ref<T> a, basic_string_ref<T> b)
{
	if(a.size() < b.size())
		return false;

	while(a.b != a.e && b.b != b.e)
	{
		if(*a.b++ != *b.b++)
			return false;
	}

	return true;
}

template<typename OutIt, typename T>
OutIt iota_n(OutIt it, size_t n, T value)
{
	while(n--)
	{
		*it++ = value;
		++value;
	}

	return it;
}


//==================================================================================================
// All bits in `val` above `width` must be zero.
template<typename T>
T sign_extend(T val, int width)
{
	// See http://graphics.stanford.edu/~seander/bithacks.html
	T m = T{1} << (width - 1);
	return (val ^ m) - m;
}

template<typename T>
bool is_power_of_two(T t)
{
	return t != 0 && !(t & (t - 1));
}

template<typename T>
T set_leading_ones(int count)
{
	if(count == sizeof(T) * 8)
		return -1;
	else
		return (T{1} << count) - 1;
}

template<typename T>
T extract_bits(T val, int start, int count)
{
	assert(start >= 0 && start + count <= (int)sizeof(T) * 8);

	return (val >> start) & set_leading_ones<T>(count);
}

inline uint64_t extract_bits(std::vector<uint8_t> const &data, int start, int count)
{
	assert(count <= 64);

	auto byte_idx = start / 8;
	auto num_bits_read = 0;

	uint64_t val = 0;
	int num_bits_in_first_byte = std::min(count, 8 - start % 8);
	val |= extract_bits(data[byte_idx++], start % 8, num_bits_in_first_byte);
	num_bits_read += num_bits_in_first_byte;

	while(count - num_bits_read >= 8)
	{
		val |= (uint64_t)data[byte_idx++] << num_bits_read;
		num_bits_read += 8;
	}

	auto remaining_bits = count - num_bits_read;
	if(remaining_bits)
		val |= uint64_t(data[byte_idx] & set_leading_ones<uint8_t>(remaining_bits)) << num_bits_read;

	assert(num_bits_read + remaining_bits == count);

	return val;
}


// Quick and dirty implementation of optional<> to stay compatible with C++11
//==================================================================================================
struct nullopt_t {};
constexpr nullopt_t nullopt;

template<typename T>
class optional
{
public:
	using value_type = T;

	optional() :
		m_init{false} {}

	optional(nullopt_t) :
		m_init{false} {}

	optional(T const &v)
	{
		new (&m_value) T(v);
		m_init = true;
	}

	optional(T &&v)
	{
		new (&m_value) T(std::move(v));
		m_init = true;
	}

	~optional() { clear(); }

	optional& operator = (T const &v)
	{
		if(m_init)
			value_unchecked() = v;
		else
		{
			new (&m_value) T(v);
			m_init = true;
		}

		return *this;
	}

	optional& operator = (T &&v)
	{
		if(m_init)
			value_unchecked() = std::move(v);
		else
		{
			new (&m_value) T(std::move(v));
			m_init = true;
		}

		return *this;
	}

	void clear()
	{
		if(m_init)
        {
          value_unchecked().~T();
          m_init = false;
        }
	}

	T& operator * () { return value(); }
	T const& operator * () const { return value(); }

	T* operator -> () { return &value(); }
	T const* operator -> () const { return &value(); }

	explicit operator bool () const { return m_init; }

	T& value()
	{
		if(!m_init)
			throw std::runtime_error{"optional is not initialized"};

		return value_unchecked();
	}

	T const& value() const
	{
		if(!m_init)
			throw std::runtime_error{"optional is not initialized"};

		return value_unchecked();
	}

	T& value_unchecked()
	{
		return *reinterpret_cast<T*>(&m_value);
	}

	T const& value_unchecked() const
	{
		return *reinterpret_cast<T const*>(&m_value);
	}

private:
	bool m_init;
	typename std::aligned_storage<sizeof(T), alignof(T)>::type m_value;
};


//==================================================================================================
template<typename T>
struct IteratorRange
{
	using iterator = T;
	using value_type = typename std::iterator_traits<T>::value_type;

	IteratorRange() :
		b{nullptr},
		e{nullptr} {}

	IteratorRange(iterator begin, iterator end) :
		b{begin},
		e{end} {}

	bool empty() const { return b == e; }
	size_t size() const { return e - b; }

	value_type const& operator [] (ptrdiff_t idx) const { return b[idx]; }

	iterator b, e;
};

template<typename T>
T begin(IteratorRange<T> const &r)
{
	return r.b;
}

template<typename T>
T end(IteratorRange<T> const &r)
{
	return r.e;
}



template<typename It>
class PairSecondIterator
{
public:
	using traits = std::iterator_traits<It>;
	using PairType = typename traits::value_type;

	// Try to guess if the pair is const
	static constexpr bool is_const = std::is_const<typename std::remove_pointer<typename traits::pointer>::type>::value;

	using difference_type = typename traits::difference_type;
	using value_type = typename PairType::second_type;
	using pointer = typename std::conditional<is_const, value_type const*, value_type*>::type;
	using reference = typename std::conditional<is_const, value_type const&, value_type&>::type;
	using iterator_category = typename traits::iterator_category;

	PairSecondIterator(It it) :
		m_it{it} {}

	reference operator * () const { return m_it->second; }
	pointer operator -> () const { return &m_it->second; }

	reference operator ++ () { return (++m_it)->second;}
	value_type operator ++ (int) { return (m_it++)->second; }

	reference operator -- () { return (--m_it)->second; }
	value_type operator -- (int) { return(m_it--)->second; }

	friend bool operator == (PairSecondIterator a, PairSecondIterator b) { return a.m_it == b.m_it; }
	friend bool operator != (PairSecondIterator a, PairSecondIterator b) { return a.m_it != b.m_it; }

private:
	It m_it;
};


template<typename K, typename V, typename A>
IteratorRange<PairSecondIterator<typename std::map<K, V, A>::const_iterator>>
values(std::map<K, V, A> const &m)
{
	return {{m.begin()}, {m.end()}};
}


//==================================================================================================
// Taken from Boost
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}


//==================================================================================================
template<typename T>
void read_binary(std::istream &is, T &v)
{
	is.read((char*)&v, sizeof(v));
}

inline void read_binary(std::istream &is, std::string &v)
{
	std::getline(is, v, '\0');
}

struct RawReader
{
	std::istream &is;

	template<typename T>
	T read()
	{
		T v;
		read_binary(is, v);
		return v;
	}

	explicit operator bool () { return (bool)is; }
};

inline RawReader& operator >> (RawReader &rr, uint8_t &v)
{
	rr.is.read((char*)&v, sizeof(v));
	return rr;
}

inline RawReader& operator >> (RawReader &rr, uint32_t &v)
{
	rr.is.read((char*)&v, sizeof(v));
	return rr;
}

inline RawReader& operator >> (RawReader &rr, std::string &v)
{
	std::getline(rr.is, v, '\0');
	return rr;
}


//==================================================================================================
template<typename T, typename Tag>
struct TaggedValue
{
	using value_type = T;

	TaggedValue() = default;
	explicit TaggedValue(T value) :
		value{value} {}

	// To make a TaggedValue usable with std::iota()
	TaggedValue& operator ++ ()
	{
		++value;
		return *this;
	}

	value_type value;
};

template<typename T, typename Tag>
inline bool operator < (TaggedValue<T, Tag> const &a, TaggedValue<T, Tag> const &b)
{
	return a.value < b.value;
}

template<typename T, typename Tag>
inline bool operator <= (TaggedValue<T, Tag> const &a, TaggedValue<T, Tag> const &b)
{
	return a.value <= b.value;
}

}


namespace std {

template<typename T, typename Tag>
struct hash<::circ::TaggedValue<T, Tag>>
{
	using argument_type = ::circ::TaggedValue<T, Tag>;
	using result_type = size_t;

	result_type operator () (argument_type const &v) const
	{
		return hash<T>{}(v.value);
	}
};

}

