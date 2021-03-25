#pragma once


#include <cstddef>
#include <utility>


namespace Util {

template<class T, class... TT>
struct over_all {
	using next = over_all<TT...>;
	static const constexpr std::size_t size = 1 + next::size;

	template<class C>
	inline constexpr static C for_each(C cbk, T && tval, TT &&... ttval){
		cbk(std::forward<T>(tval));
		next::for_each(cbk, std::forward<TT>(ttval)...);
		return cbk;
	}

	template<class C>
	inline constexpr C operator()(C cbk, T && tval, TT &&... ttval) const {
		return for_each(cbk, std::forward<T>(tval), std::forward<TT>(ttval)...);
	}
};

template<class T>
struct over_all<T> {
	static const constexpr std::size_t size = 1;

	template<class C>
	inline constexpr static C for_each(C cbk, T && tval) {
		cbk(std::forward<T>(tval));
		return cbk;
	}

	template<class C>
	inline constexpr C operator()(C cbk, T && tval) const {
		return for_each(cbk, std::forward<T>(tval));
	}
};

}
