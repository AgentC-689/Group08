#include "utils/GeoUtils.h"
#include <cmath>

namespace GeoUtils {

static constexpr double PI = 3.14159265358979323846;
static constexpr double EARTH_RADIUS_METERS = 6371000.0;

static double toRadians(double degrees) {
    return degrees * PI / 180.0;
}

double calculateDistanceMeters(double lat1, double lon1, double lat2, double lon2) {
    const double phi1 = toRadians(lat1);
    const double phi2 = toRadians(lat2);
    const double dPhi = toRadians(lat2 - lat1);
    const double dLambda = toRadians(lon2 - lon1);

    const double a = std::sin(dPhi / 2.0) * std::sin(dPhi / 2.0) +
                     std::cos(phi1) * std::cos(phi2) *
                     std::sin(dLambda / 2.0) * std::sin(dLambda / 2.0);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_METERS * c;
}

bool isWithinAllowedRadius(double classLat, double classLon, double deviceLat, double deviceLon, double allowedMeters) {
    return calculateDistanceMeters(classLat, classLon, deviceLat, deviceLon) <= allowedMeters;
}

} // namespace GeoUtils
