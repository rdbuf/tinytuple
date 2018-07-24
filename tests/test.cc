#include "tinytuple.hpp"

#include <iostream>
#include <tuple>
#include <string>
#include <type_traits>
#include <utility>

static_assert(sizeof(tiny::tuple<bool, long, char, bool, int, bool>) == 16);
static_assert(sizeof(std::tuple<bool, long, char, bool, int, bool>) == 32);

int main() {
	using std::cout, std::endl, std::string;
	using std::is_same_v, std::decay_t;
	using std::get;

	const tiny::tuple<bool, long, char, bool, int, bool> x{};
	std::tuple<bool, long, char, bool, int, bool> y;
	cout << sizeof(x) << endl;
	cout << sizeof(y) << endl;

	tiny::tuple<string> z;
	get<0>(z) = "asdf";
	cout << get<0>(z) << endl;

	static_assert(is_same_v<decltype(get<1>(y)), long&>);
	static_assert(is_same_v<decltype(get<1>(x)), const long&>);

	static_assert(is_same_v<decltype(get<1>(std::move(y))), long&&>);
	static_assert(is_same_v<decltype(get<1>(std::move(x))), const long&&>);
}
