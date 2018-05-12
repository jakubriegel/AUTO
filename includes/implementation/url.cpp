#include "../project.hpp"

Route * url::getRoute(const Position & a, const Position & b) {
    std::stringstream route;
    std::string url = "https://maps.googleapis.com/maps/api/directions/json?origin=" + std::to_string(a.getLat()) + "," + std::to_string(a.getLng()) + "&destination=" + std::to_string(b.getLat()) + "," + std::to_string(b.getLng()) + "&language=en&key=AIzaSyBfTruPldaz027vUHZKmRvRaAD-VCrdmIk";
    try
    {
        curlpp::Cleanup myCleanup;

        curlpp::Easy myRequest;
        myRequest.setOpt(curlpp::options::WriteStream(&route));
        myRequest.setOpt<curlpp::options::Url>(url);
        myRequest.perform();
    }

    catch (curlpp::RuntimeError & e)
    {
        std::cout << e.what() << std::endl;
    }

    catch (curlpp::LogicError & e)
    {
        std::cout << e.what() << std::endl;
    }

    auto jRoute = nlohmann::json::parse(route);
    return new Route(jRoute);
}

const Position url::getCoords(const std::string & location) {
    std::stringstream data;
    std::string url = "https://maps.googleapis.com/maps/api/geocode/json?address=" + location + "&key=AIzaSyBfTruPldaz027vUHZKmRvRaAD-VCrdmIk";
    try
    {
        curlpp::Cleanup myCleanup;

        curlpp::Easy myRequest;
        myRequest.setOpt(curlpp::options::WriteStream(&data));
        myRequest.setOpt<curlpp::options::Url>(url);
        myRequest.perform();
    }

    catch (curlpp::RuntimeError & e)
    {
        std::cout << e.what() << std::endl;
    }

    catch (curlpp::LogicError & e)
    {
        std::cout << e.what() << std::endl;
    }

    auto jData = nlohmann::json::parse(data);

    const double lat = jData["/results/0/geometry/location/lat"_json_pointer].get<double>();
    const double lng = jData["/results/0/geometry/location/lng"_json_pointer].get<double>();

    return Position(lat, lng);
}