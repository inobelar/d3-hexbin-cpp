#ifndef D3__HEXBIN__HEXBIN_HPP
#define D3__HEXBIN__HEXBIN_HPP

#include <cmath>      // for M_PI, std::round(), std::sin(), std::cos()

#include <array>      // for std::array<T, N>
#include <vector>     // for std::vector<T>
#include <map>        // for std::map<K,V>

#include <functional> // for std::function<R(T)>
#include <string>     // for std::string

#include <sstream> // for std::ostringstream
#include <numeric> // for std::accumulate()

#include <type_traits> // for std::enable_if()
#include <limits>      // for std::numeric_limits<T>::...

namespace d3_hexbin {

namespace detail {

// -----------------------------------------------------------------------------
// Constants

constexpr double thirdPi
    = M_PI / 3;
constexpr std::array<double, 6>
    angles = {0, thirdPi, 2 * thirdPi, 3 * thirdPi, 4 * thirdPi, 5 * thirdPi};


// -----------------------------------------------------------------------------
// Subscription operator detection (aka square brackets [])

namespace detect {

/*
    References:
        - https://stackoverflow.com/a/45231946
        additional:
            - https://stackoverflow.com/a/4434734
            - https://stackoverflow.com/a/31306194
            - https://stackoverflow.com/a/56694704
*/

// in C++17 std::void_t
template < typename... >
using void_t = void;


template < typename T, typename Index >
using subscript_t = decltype(std::declval<T>()[std::declval<Index>()]);

template < typename, typename Index = size_t, typename = void_t<> >
struct has_subscript : std::false_type {};

template < typename T, typename Index >
struct has_subscript< T, Index, void_t< subscript_t<T,Index> > > : std::true_type {};

} // namespace detect

// -----------------------------------------------------------------------------
// Default converters (Datum -> Point)

/**
    Description:

    Direct naive conversion from js into c++ may be like this:

    @code{.cpp}
    template <typename T, typename NumberT>
    inline NumberT pointX(const T& d) { return d[0]; }
    @endcode

    But this not enough, to implement original js behavior: if type NOT contains
    '[] operator', return null/undefined/NaN. That's why below implemented
    HUGE & COMPLICATED overloaded functions (pointX() and PointY() ), which
    determines in compile-time - is type contains 'subscript operator' - then
    uses it, otherwise (if not) returns NaN.
 */

template <typename DatumT, typename NumberT>
inline
    typename std::enable_if< detect::has_subscript<DatumT>::value == false, NumberT >::type
pointX(const DatumT& /*d*/) {
    static_assert(std::numeric_limits<NumberT>::has_quiet_NaN == true, "NumberT doesn't have NaN support");
    return std::numeric_limits<NumberT>::quiet_NaN();
}

template <typename DatumT, typename NumberT>
inline
    typename std::enable_if< detect::has_subscript<DatumT>::value == true, NumberT >::type
pointX(const DatumT& d) {
    return d[0];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename DatumT, typename NumberT>
inline
    typename std::enable_if< detect::has_subscript<DatumT>::value == false, NumberT >::type
pointY(const DatumT& /*d*/) {
    static_assert(std::numeric_limits<NumberT>::has_quiet_NaN == true, "NumberT doesn't have NaN support");
    return std::numeric_limits<NumberT>::quiet_NaN();
}

template <typename DatumT, typename NumberT>
inline
    typename std::enable_if< detect::has_subscript<DatumT>::value == true, NumberT >::type
pointY(const DatumT& d) {
    return d[1];
}

// -----------------------------------------------------------------------------

} // namespace detail

// Based on: https://github.com/DefinitelyTyped/DefinitelyTyped/blob/master/types/d3-hexbin/index.d.ts#L11

template <typename T, typename NumberT>
struct HexbinBin : public std::vector<T>
{
    using number_t = NumberT;

    /**
     * The x-coordinate of the center of the associated bin’s hexagon.
     */
    number_t x;

    /**
     * The y-coordinate of the center of the associated bin’s hexagon.
     */
    number_t y;

    HexbinBin(const T& d)
        : std::vector<T>{d}
        , x(0)
        , y(0)
    {}
};

template <typename T, typename number_t, typename PointT = std::array<number_t, 2> >
class Hexbin {
public:

    using component_func_t = std::function< number_t (const T& ) >;

    using extent_t = std::array<PointT, 2>;

private:
    number_t x0 = 0;
    number_t y0 = 0;
    number_t x1 = 1;
    number_t y1 = 1;
    component_func_t _x = detail::pointX<T, number_t>;
    component_func_t _y = detail::pointY<T, number_t>;
    number_t r;
    number_t dx;
    number_t dy;

    // -------------------------------------------------------------------------
protected:

    // NOTICE: that this method returns RELATIVE coordinates (dx, dy),
    // not absolute coords.
    static std::array<PointT, 6> _hexagon(number_t radius) {
        number_t x0 = 0, y0 = 0;

        std::array<PointT, 6> result;
        for(std::size_t i = 0; i < detail::angles.size(); ++i)
        {
            const auto& angle = detail::angles[i];

            const number_t
                    x1 =  std::sin(angle) * radius,
                    y1 = -std::cos(angle) * radius,
                    dx = x1 - x0,
                    dy = y1 - y0;
            x0 = x1; y0 = y1;
            result[i] = {dx, dy};
        }
        return result;
    }

    // -------------------------------------------------------------------------
    // Utils (stringification)

    template <typename ValueT>
    static inline std::string _to_str(const ValueT& val) {
        std::ostringstream out;
        out << val;
        return out.str();
    }

    static inline std::string _p_to_str(const PointT& p) {
        return _to_str(p[0]) + "," + _to_str(p[1]);
    }

    static inline std::string _join(const std::vector<std::string>& list, const std::string& delim) {
        return std::accumulate(list.begin(), list.end(), std::string(), // via: https://stackoverflow.com/a/12155571
                   [&delim](const std::string& a, const std::string& b) -> std::string {
                       return a + (a.length() > 0 ? delim : "") + b;
               });
    }

    // -------------------------------------------------------------------------

    static std::vector<std::string> _hexagon_str(number_t radius) {
        std::vector<std::string> strings;
        const auto points = _hexagon(radius);
        for(const PointT& point : points) {
            strings.push_back( _p_to_str(point) );
        }
        return strings;
    }

public:

    Hexbin()
    {
        radius(1);
    }


    std::vector<HexbinBin<T, number_t>> operator () (const std::vector<T>& points)
    {
        using bin_t = HexbinBin<T, number_t>;

        std::map<std::string, bin_t> binsById = {};
        std::vector< bin_t > bins = {};
        std::size_t i;
        const std::size_t n = points.size();

        for (i = 0; i < n; ++i)
        {
            const T& point = points[i];

            number_t px;
            number_t py;
            if (std::isnan(px = _x(point /*, i, points*/))
             || std::isnan(py = _y(point /*, i, points*/))) continue;

            int pj = std::round(py = py / dy);
            int pi = std::round(px = (px / dx - (pj & 1) / 2.0) +0.00001); /// '2.0' instead of '2' for float division (non-integer), for same result as in js
            const number_t py1 = py - pj;

            if (std::abs(py1) * 3 > 1) {
                const number_t
                        px1 = px - pi,
                        pi2 = pi + (px < pi ? -1 : 1) / 2,
                        pj2 = pj + (py < pj ? -1 : 1),
                        px2 = px - pi2,
                        py2 = py - pj2;
                if (px1 * px1 + py1 * py1 > px2 * px2 + py2 * py2) { pi = pi2 + (pj & 1 ? 1 : -1) / 2; pj = pj2; }
            }

            const std::string id = _to_str(pi) + "-" + _to_str(pj);
            const auto bin_it = binsById.find(id); // In js it's: `bin = binsById[id]`
            if (bin_it != binsById.end()) // found
                bin_it->second.push_back(point);
            else { // not found

                // In js next 3 lines of code it's: `bin = binsById[id] = [point];`
                const auto result = binsById.insert({ id, bin_t(point) });
                const auto bin_it_new = result.first;
                bin_t& bin = (bin_it_new->second);

                bin.x = (pi + (pj & 1) / 2.0) * dx; /// '2.0' instead '2' important here too
                bin.y = pj * dy;
                bins.push_back( bin );
            }
        }

        // FIX for c++
        bins.clear();
        for(const auto& kv : binsById)
            bins.push_back(kv.second);

        return bins;
    }

    // -------------------------------------------------------------------------

    std::string hexagon() const {
        return "m" + _join(_hexagon_str(r), "l") + "z";
    }

    static std::string hexagon(number_t radius_) {
        return "m" + _join(_hexagon_str(radius_), "l") + "z";
    }

    // -------------------------------------------------------------------------

    std::vector<PointT> centers() const {
        std::vector<PointT> centers = {};
              int j = std::round(y0 / dy);
        const int i = std::round(x0 / dx);
        for (number_t y = j * dy; y < y1 + r; y += dy, ++j) {
            for (number_t x = i * dx + (j & 1) * dx / 2; x < x1 + dx / 2; x += dx) {
                centers.push_back({x, y});
            }
        }
        return centers;
    }

    // -------------------------------------------------------------------------

    std::string mesh() const {
        auto hexagons = _hexagon_str(r);
        hexagons.resize(4); // same as: slice(0, 4)
        const auto fragment = _join(hexagons, "l");

        std::string result;
        for(const PointT& p : this->centers())
            result.append( "M" + _p_to_str(p) + "m" + fragment );
        return result;
    }

    // -------------------------------------------------------------------------

    Hexbin& x(const component_func_t& x_) {
        _x = x_;
        return *this;
    }

    component_func_t x() const {
        return _x;
    }

    // -------------------------------------------------------------------------

    Hexbin& y(const component_func_t& y_) {
        _y = y_;
        return *this;
    }

    component_func_t y() const {
        return _y;
    }

    // -------------------------------------------------------------------------

    Hexbin& radius(number_t radius_) {
        r = radius_; dx = r * 2 * std::sin(detail::thirdPi); dy = r * 1.5;
        return *this;
    }

    number_t radius() const {
        return r;
    }

    // -------------------------------------------------------------------------

    Hexbin& size(const PointT& size_) {
        x0 = y0 = 0; x1 = size_[0]; y1 = size_[1];
        return *this;
    }

    PointT size() const {
        return {x1 - x0, y1 - y0};
    }

    // -------------------------------------------------------------------------

    Hexbin& extent(const extent_t& extent_) {
        x0 = extent_[0][0]; y0 = extent_[0][1]; x1 = extent_[1][0]; y1 = extent_[1][1];
        return *this;
    }

    extent_t extent() const {
        return { PointT{x0, y0}, PointT{x1, y1} };
    }

    // =========================================================================
    // Non-standart EXPERIMENTAL API for direct drawing by using something like
    // d3-path-cpp PathInterface API

    template <typename PathInterface>
    static void draw_hexagon(PathInterface& path, number_t radius_) {
        const auto hex = _hexagon(radius_);

        PointT curr = hex[0];
        path.moveTo(curr[0], curr[1]);

        for(std::size_t i = 1; i < hex.size(); ++i)
        {
            // cur += hex[i];
            curr[0] = curr[0] + hex[i][0];
            curr[1] = curr[1] + hex[i][1];

            path.lineTo(curr[0], curr[1]);
        }

        path.closePath();
    }

    template <typename PathInterface>
    void draw_hexagon(PathInterface& path) {
        draw_hexagon(path, r);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    template <typename PathInterface>
    void draw_mesh(PathInterface& path) {
        const auto hexagons = _hexagon(r);

        for(const PointT& p : this->centers())
        {
            PointT curr;
            // curr = p + hexagons[0]
            curr[0] = p[0] + hexagons[0][0];
            curr[1] = p[1] + hexagons[0][1];

            path.moveTo(curr[0], curr[1]);

            for(std::size_t i = 1; i < 4; ++i)
            {
                // curr += hexagons[i]
                curr[0] = curr[0] + hexagons[i][0];
                curr[1] = curr[1] + hexagons[i][1];

                path.lineTo(curr[0], curr[1]);
            }
        }
    }

};

// TODO: typename number_t = remove_ref_and_const< decltype(PointT().operator[](0) >::type;
template <typename T, typename number_t, typename PointT = std::array<double, 2>>
inline Hexbin<T, number_t, PointT> hexbin() {
    return Hexbin<T, number_t, PointT>();
}

} // namespace d3_hexbin

#endif // D3__HEXBIN__HEXBIN_HPP
