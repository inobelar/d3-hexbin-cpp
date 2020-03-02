#ifndef D3__HEXBIN__TEST__PATH_EQUAL_HPP
#define D3__HEXBIN__TEST__PATH_EQUAL_HPP


// NOTE: This is pretty the same testing code, as in d3-path-cpp, except,
// 'expected' string NOT 'normalized'


#include "catch/catch.hpp"

#include "_regex_replace.hpp"
#include <sstream> // for std::ostringstream
#include <cmath>   // for std::abs(), std::round()
#include <iomanip> // for std::setprecision()

namespace detail {

inline std::string toFixed(double value, int n) {
    std::ostringstream out;
    out << std::setprecision(n)  << value;
    return out.str();
}

static const std::string reNumber = R"([-+]?(?:\d+\.\d+|\d+\.|\.\d+|\d+)(?:[eE][-]?\d+)?)";

std::string formatNumber(const std::smatch& match) {
    const double s = std::stod( match.str() );
    return (std::abs(s - std::round(s)) < 1e-6) ? std::to_string( std::round(s) ) : toFixed(s, 6);
}

std::string normalizePath(const std::string& path) {
    return utils::regex_replace(path, std::regex(reNumber), formatNumber);
}

} // namespace detail

class PathEqual : public Catch::MatcherBase<std::string>
{
    const std::string& m_str;

public:

    PathEqual(const std::string& str)
        : m_str(str)
    {}

    // Performs the test for this matcher
    bool match(const std::string& actual_path) const override
    {
        // const std::string path_str = path.toString();

        const std::string actual   = detail::normalizePath(actual_path);
        // const std::string expected = detail::normalizePath(m_str   );

        const std::string& expected = m_str; // detail::normalizePath(m_str   );

        return (actual == expected);
    }

    // Produces a string describing what this matcher does. It should include
    // any provided data and be written as if it were stating a fact (in the
    // output it will be preceded by the value under test).
    virtual std::string describe() const override {
        std::ostringstream ss;
        ss << "Should be equal to: " << m_str;
        return ss.str();
    }
};

// The builder function
inline PathEqual pathEqual(const std::string& str) {
    return PathEqual(str);
}

#endif // D3__HEXBIN__TEST__PATH_EQUAL_HPP
