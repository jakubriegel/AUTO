#include "../project.hpp"

Route * url::getRoute(const Position & a, const Position & b) {
    std::stringstream route;
    std::string url = "https://maps.googleapis.com/maps/api/directions/json?origin=" + std::to_string(a.getLat()) + "," + std::to_string(a.getLng()) + "&destination=" + std::to_string(b.getLat()) + "," + std::to_string(b.getLng()) + "&language=en&key=" + Const::getMapsApiKey();
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
