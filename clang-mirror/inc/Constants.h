#pragma once

#include <tuple>
#include <string>
#include <vector>
#include <chrono>
#include <string_view>

namespace clmirror {

	inline constexpr std::string_view RESET = "\033[0m";
	inline constexpr std::string_view DARK_RED = "\033[31m";
	inline constexpr std::string_view RED = "\033[38;2;231;72;86m";
	inline constexpr std::string_view GREEN = "\033[32m";
	inline constexpr std::string_view YELLOW = "\033[33m";
	inline constexpr std::string_view BLUE = "\033[34m";
	inline constexpr std::string_view MAGENTA = "\033[35m";
	inline constexpr std::string_view CYAN = "\033[38;2;97;214;214m";
	inline constexpr std::string_view WHITE = "\033[37m";
	inline constexpr std::string_view TEAL = "\033[38;2;0;128;128m";
	inline constexpr std::string_view GREY = "\033[38;2;118;118;118m";

	inline constexpr std::string_view CONST = "const";
	inline constexpr std::string_view ENUM = "enum";
	inline constexpr std::string_view CLASS = "class";
	inline constexpr std::string_view STRUCT = "struct";

	inline constexpr std::string_view INC_MANAGERS_DATA = "incManagersData.txt";
	
	inline constexpr std::string_view META_ID_HEADER = "rtcl_meta_ids.h";
	inline constexpr std::string_view REGISTRATION_HEADER = "rtcl_registration.h";

	inline constexpr std::string_view NS_TYPE = "type";
	inline constexpr std::string_view NS_METHOD = "method";
	inline constexpr std::string_view NS_FUNCTION = "function";
	inline constexpr std::string_view NS_REGISTRATION = "registration";
	inline constexpr std::string_view DECL_INIT_REGIS = "static void init(std::vector<rtl::Function>&);";
	inline constexpr std::string_view DEFN_INIT_REGIS = "static void init(std::vector<rtl::Function>& fns)";

	using ErrorTuple = std::tuple<std::string, std::string, std::string>;

	using Clock = std::chrono::high_resolution_clock;

	using Second = std::chrono::duration<double, std::ratio<1> >;
}


namespace clmirror 
{
	enum class MetaKind 
	{
		None,
		Ctor,
		NonMemberFn,
		MemberFnConst,
		MemberFnStatic,
		MemberFnNonConst
	};
}


#define GETTER_CREF(_varType, _name, _var)       \
inline constexpr const _varType& get##_name() const {  \
    return _var;                             \
}