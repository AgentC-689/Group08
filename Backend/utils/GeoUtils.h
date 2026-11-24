#pragma once

// Part 4 (Member 4): Geospatial distance utility for FR15 & FR16 GPS verification
namespace GeoUtils {
    // Computes great-circle distance in meters using the Haversine formula
    double calculateDistanceMeters(double lat1, double lon1, double lat2, double lon2);
    // Returns true if student device location is within allowed radius
    bool isWithinAllowedRadius(double classLat, double classLon, double deviceLat, double deviceLon, double allowedMeters);
}
