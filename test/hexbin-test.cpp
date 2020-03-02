#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include "./pathEqual.hpp"

#include "d3_hexbin/hexbin.hpp"

using point_t  = std::array<double, 2>;
using extent_t = std::array<point_t, 2>;
using points_t = std::vector<point_t>;

using datum_t = std::array<double, 2>;
using data_t  = std::vector<datum_t>;


// =============================================================================

template <typename T, typename number_t>
std::vector< std::vector<T> > noxy(const std::vector<d3_hexbin::HexbinBin<T, number_t>>& bins) {
    std::vector< std::vector<T> > result;
    for(const auto& bin : bins) {
        result.push_back(bin);
    }
    return result;
}

template <typename T>
struct PointXY {
    T x, y;
    PointXY() : x(0), y(0) {}
    PointXY(T x_, T y_) : x(x_), y(y_) {}

    bool operator == (const PointXY& other) const {
        return (x == other.x && y == other.y);
    }
    bool operator != (const PointXY& other) const {
        return !(*this == other);
    }

    const T& operator [](std::size_t i) const {
        switch (i) {
        case 0: return x;
        case 1: return y;
        default: return x; // suppress warnings
        }
    }
};


template <typename T, typename number_t>
std::vector<PointXY<number_t>> xy(const std::vector<d3_hexbin::HexbinBin<T, number_t>>& bins) {
    std::vector<PointXY<number_t>> result;
    for(const auto& bin : bins) {
        result.push_back( PointXY<number_t>(/*x: */bin.x, /*y: */bin.y) );
    }
    return result;
}

// =============================================================================

TEST_CASE("d3.hexbin() has the expected defaults") {
    const auto b = d3_hexbin::hexbin<datum_t, double, point_t>();
    REQUIRE( b.extent() == extent_t{{ {0, 0}, {1, 1} }});
    REQUIRE( b.size() == point_t{1, 1});
    REQUIRE( b.x()({41, 42}) == 41);
    REQUIRE( b.y()({41, 42}) == 42);
    REQUIRE( b.radius() == 1);
}

TEST_CASE("hexbin(points) bins the specified points into hexagonal bins") {
    const auto bins = d3_hexbin::hexbin<datum_t, double, point_t>()({
        {0, 0}, {0, 1}, {0, 2},
        {1, 0}, {1, 1}, {1, 2},
        {2, 0}, {2, 1}, {2, 2}
    });

    REQUIRE( noxy(bins) == std::vector<data_t>{
        {{0, 0}},
        {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
        {{1, 0}, {2, 0}},
        {{2, 1}, {2, 2}}
    });

    REQUIRE( xy(bins) == std::vector<PointXY<double>>{
        {/*x:*/ 0, /*y:*/ 0},
        {/*x:*/ 0.8660254037844386, /*y:*/ 1.5},
        {/*x:*/ 1.7320508075688772, /*y:*/ 0},
        {/*x:*/ 2.598076211353316,  /*y:*/ 1.5}
    });
}

TEST_CASE("hexbin(points) observes the current x- and y-accessors") {
    static const auto x = [](const PointXY<double>& d) { return d.x; };
    static const auto y = [](const PointXY<double>& d) { return d.y; };
    const auto bins = d3_hexbin::hexbin<PointXY<double>, double, point_t>().x(x).y(y)({
        {/*x:*/ 0, /*y:*/ 0}, {/*x:*/ 0, /*y:*/ 1}, {/*x:*/ 0, /*y:*/ 2},
        {/*x:*/ 1, /*y:*/ 0}, {/*x:*/ 1, /*y:*/ 1}, {/*x:*/ 1, /*y:*/ 2},
        {/*x:*/ 2, /*y:*/ 0}, {/*x:*/ 2, /*y:*/ 1}, {/*x:*/ 2, /*y:*/ 2}
    });

    REQUIRE( noxy(bins) == std::vector<std::vector<PointXY<double>>> {
        {{/*x:*/ 0, /*y:*/ 0}},
        {{/*x:*/ 0, /*y:*/ 1}, {/*x:*/ 0, /*y:*/ 2}, {/*x:*/ 1, /*y:*/ 1}, {/*x:*/ 1, /*y:*/ 2}},
        {{/*x:*/ 1, /*y:*/ 0}, {/*x:*/ 2, /*y:*/ 0}},
        {{/*x:*/ 2, /*y:*/ 1}, {/*x:*/ 2, /*y:*/ 2}}
    });

    REQUIRE( xy(bins) == std::vector<PointXY<double>> {
        {/*x:*/ 0, /*y:*/ 0},
        {/*x:*/ 0.8660254037844386, /*y:*/ 1.5},
        {/*x:*/ 1.7320508075688772, /*y:*/ 0},
        {/*x:*/ 2.598076211353316,  /*y:*/ 1.5}
    });
}

TEST_CASE("hexbin(points) observes the current radius") {
    const auto bins = d3_hexbin::hexbin<datum_t, double, point_t>().radius(2)({
        {0, 0}, {0, 1}, {0, 2},
        {1, 0}, {1, 1}, {1, 2},
        {2, 0}, {2, 1}, {2, 2}
    });

    REQUIRE( noxy(bins) == std::vector<data_t>{
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 2}, {1, 2}, {2, 2}},
        {{2, 0}, {2, 1}}
    });

    REQUIRE( xy(bins) == std::vector<PointXY<double>>{
        {/*x:*/ 0, /*y:*/ 0},
        {/*x:*/ 1.7320508075688772, /*y:*/ 3},
        {/*x:*/ 3.4641016151377544, /*y:*/ 0}
    });
}

TEST_CASE("hexbin.size() gets or sets the extent") {
    auto b = d3_hexbin::hexbin<datum_t, double, point_t>().size({2, 3});
    REQUIRE( b.extent() == extent_t{{ {0, 0}, {2, 3} }});
    b.extent({{ {1, 2}, {4, 8} }});
    REQUIRE(b.size() == point_t{3, 6});
}

/*

TEST_CASE("hexbin.x(x) sets the x-coordinate accessor") {
  var x = function(d) { return d.x; },
      b = d3.hexbin().x(x),
      bins = b([{x: 1, 1: 2}]);
  test.equal(b.x(), x);
  test.equal(bins.length, 1);
  test.equal(bins[0].x, 0.8660254037844386);
  test.equal(bins[0].y, 1.5);
  test.equal(bins[0].length, 1);
  test.deepEqual(bins[0][0], {x: 1, 1: 2});
}

TEST_CASE("hexbin.y(y) sets the y-coordinate accessor") {
  var y = function(d) { return d.y; },
      b = d3.hexbin().y(y),
      bins = b([{0: 1, y: 2}]);
  test.equal(b.y(), y);
  test.equal(bins.length, 1);
  test.equal(bins[0].x, 0.8660254037844386);
  test.equal(bins[0].y, 1.5);
  test.equal(bins[0].length, 1);
  test.deepEqual(bins[0][0], {0: 1, y: 2});
}

*/

TEST_CASE("hexbin.hexagon() returns the expected path") {
    const std::string path_str = d3_hexbin::hexbin<datum_t, double, point_t>().hexagon();
    REQUIRE_THAT(path_str, pathEqual("m0,-1l0.866025,0.500000l0,1l-0.866025,0.500000l-0.866025,-0.500000l0,-1z"));
}


TEST_CASE("hexbin.hexagon() observes the current bin radius") {
    const std::string path_2_str = d3_hexbin::hexbin<datum_t, double, point_t>().radius(2).hexagon();
    REQUIRE_THAT(path_2_str, pathEqual("m0,-2l1.732051,1l0,2l-1.732051,1l-1.732051,-1l0,-2z"));

    const std::string path_4_str = d3_hexbin::hexbin<datum_t, double, point_t>().radius(4).hexagon();
    REQUIRE_THAT(path_4_str, pathEqual("m0,-4l3.464102,2l0,4l-3.464102,2l-3.464102,-2l0,-4z"));
}

TEST_CASE("hexbin.hexagon(radius) observes the specified radius") {
    const std::string path_2_str = d3_hexbin::hexbin<datum_t, double, point_t>().hexagon(2);
    REQUIRE_THAT(path_2_str, pathEqual("m0,-2l1.732051,1l0,2l-1.732051,1l-1.732051,-1l0,-2z"));

    const std::string path_4_str = d3_hexbin::hexbin<datum_t, double, point_t>().hexagon(4);
    REQUIRE_THAT(path_4_str, pathEqual("m0,-4l3.464102,2l0,4l-3.464102,2l-3.464102,-2l0,-4z"));
}

// NOTE: no sense in case of c++
TEST_CASE("hexbin.hexagon(radius) uses the current bin radius if radius is null") {
    const std::string path_null_str = d3_hexbin::hexbin<datum_t, double, point_t>().hexagon(/*null*/);
    REQUIRE_THAT(path_null_str, pathEqual("m0,-1l0.866025,0.500000l0,1l-0.866025,0.500000l-0.866025,-0.500000l0,-1z"));

    const std::string path_undefined_str = d3_hexbin::hexbin<datum_t, double, point_t>().hexagon(/*undefined*/);
    REQUIRE_THAT(path_undefined_str, pathEqual("m0,-1l0.866025,0.500000l0,1l-0.866025,0.500000l-0.866025,-0.500000l0,-1z"));
}

TEST_CASE("hexbin.centers() returns an array of bin centers") {
    REQUIRE( d3_hexbin::hexbin<datum_t, double, point_t>().centers() == points_t{
        {0, 0},
        {1.7320508075688772, 0},
        {0.8660254037844386, 1.5}
    });
}


TEST_CASE("hexbin.centers() observes the current bin radius") {
    REQUIRE( d3_hexbin::hexbin<datum_t, double, point_t>().radius(0.5).centers() == points_t{
        {0, 0},
        {0.8660254037844386, 0},
        {0.4330127018922193, 0.75},
        {1.299038105676658, 0.75}
    });
}


TEST_CASE("hexbin.centers() observes the current extent") {
    REQUIRE( d3_hexbin::hexbin<datum_t, double, point_t>().radius(0.5).extent({{ {-1.1, -1.1}, {1.1, 1.1} }}).centers() == points_t{
        {-0.4330127018922193, -0.75},
        {0.4330127018922193, -0.75},
        {1.299038105676658, -0.75},
        {-0.8660254037844386, 0},
        {0, 0},
        {0.8660254037844386, 0},
        {-0.4330127018922193, 0.75},
        {0.4330127018922193, 0.75},
        {1.299038105676658, 0.75},
        {-0.8660254037844386, 1.5},
        {0, 1.5},
        {0.8660254037844386, 1.5}
    });
}


TEST_CASE("hexbin.mesh() returns the expected path") {
    const std::string path_str = d3_hexbin::hexbin<datum_t, double, point_t>().mesh();
    REQUIRE_THAT(path_str, pathEqual("M0,0m0,-1l0.866025,0.500000l0,1l-0.866025,0.500000M1.732051,0m0,-1l0.866025,0.500000l0,1l-0.866025,0.500000M0.866025,1.500000m0,-1l0.866025,0.500000l0,1l-0.866025,0.500000"));
}

TEST_CASE("hexbin.mesh() observes the bin radius") {
    const std::string path_str = d3_hexbin::hexbin<datum_t, double, point_t>().radius(0.5).mesh();
    REQUIRE_THAT(path_str, pathEqual("M0,0m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0.866025,0m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0.433013,0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M1.299038,0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000"));
}

TEST_CASE("hexbin.mesh() observes the extent") {
    const std::string path_str = d3_hexbin::hexbin<datum_t, double, point_t>().radius(0.5).extent({{ {-1.1, -1.1}, {1.1, 1.1} }}).mesh();
    REQUIRE_THAT(path_str, pathEqual( "M-0.433013,-0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0.433013,-0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M1.299038,-0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M-0.866025,0m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0,0m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0.866025,0m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M-0.433013,0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0.433013,0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M1.299038,0.750000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M-0.866025,1.500000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0,1.500000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000M0.866025,1.500000m0,-0.500000l0.433013,0.250000l0,0.500000l-0.433013,0.250000") );
}
