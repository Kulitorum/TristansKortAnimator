#include "geojsonparser.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

GeoJsonParser::GeoJsonParser(QObject* parent)
    : QObject(parent)
{
}

bool GeoJsonParser::loadFromResource(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit loadError(QString("Cannot open resource: %1").arg(resourcePath));
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit loadError(QString("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    m_features.clear();
    parseFeatureCollection(doc.object());
    emit loaded();
    return true;
}

bool GeoJsonParser::appendFromResource(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit loadError(QString("Cannot open resource: %1").arg(resourcePath));
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit loadError(QString("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    // Don't clear - append to existing features
    parseFeatureCollection(doc.object());
    emit loaded();
    return true;
}

bool GeoJsonParser::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit loadError(QString("Cannot open file: %1").arg(filePath));
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit loadError(QString("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    m_features.clear();
    parseFeatureCollection(doc.object());
    emit loaded();
    return true;
}

void GeoJsonParser::parseFeatureCollection(const QJsonObject& root) {
    if (root["type"].toString() != "FeatureCollection") {
        emit loadError("Not a FeatureCollection");
        return;
    }

    QJsonArray features = root["features"].toArray();
    for (const auto& featureVal : features) {
        parseFeature(featureVal.toObject());
    }
}

void GeoJsonParser::parseFeature(const QJsonObject& feature) {
    GeoFeature geoFeature;

    // Parse properties
    QJsonObject props = feature["properties"].toObject();

    // Get name - Natural Earth uses various fields
    geoFeature.name = props["NAME"].toString();
    if (geoFeature.name.isEmpty()) {
        geoFeature.name = props["name"].toString();
    }
    if (geoFeature.name.isEmpty()) {
        geoFeature.name = props["ADMIN"].toString();
    }

    // Get code - prefer ISO_A2 for countries, iso_3166_2 for states
    geoFeature.code = props["ISO_A2"].toString();
    if (geoFeature.code.isEmpty()) {
        geoFeature.code = props["iso_a2"].toString();
    }
    if (geoFeature.code.isEmpty()) {
        // For states/provinces, use the full ISO 3166-2 code (e.g., "US-CA")
        geoFeature.code = props["iso_3166_2"].toString();
    }
    if (geoFeature.code.isEmpty()) {
        geoFeature.code = props["ISO_A3"].toString();
    }
    if (geoFeature.code.isEmpty()) {
        geoFeature.code = props["adm1_code"].toString();
    }

    // Determine type based on Natural Earth field patterns
    if (props.contains("ADMIN") || props.contains("SOVEREIGNT") || props.contains("ADM0_A3")) {
        geoFeature.type = "country";
    } else if (props.contains("adm1_code") || props.contains("iso_3166_2") || props.contains("admin")) {
        geoFeature.type = "region";
        // Store parent country code for regions
        QString parentCode = props["iso_a2"].toString();
        if (!parentCode.isEmpty()) {
            geoFeature.properties["parentCountry"] = parentCode;
        }
    } else {
        geoFeature.type = "feature";
    }

    // Store all properties
    geoFeature.properties = props.toVariantMap();

    // Parse geometry
    QJsonObject geometry = feature["geometry"].toObject();
    QString geoType = geometry["type"].toString();

    if (geoType == "Polygon") {
        QJsonArray coords = geometry["coordinates"].toArray();
        if (!coords.isEmpty()) {
            geoFeature.polygons.append(parsePolygon(coords[0].toArray()));
        }
    } else if (geoType == "MultiPolygon") {
        QJsonArray multiCoords = geometry["coordinates"].toArray();
        for (const auto& polyVal : multiCoords) {
            QJsonArray poly = polyVal.toArray();
            if (!poly.isEmpty()) {
                geoFeature.polygons.append(parsePolygon(poly[0].toArray()));
            }
        }
    } else if (geoType == "Point") {
        QJsonArray coords = geometry["coordinates"].toArray();
        if (coords.size() >= 2) {
            geoFeature.centroid = QPointF(coords[1].toDouble(), coords[0].toDouble());
        }
        geoFeature.type = "city";
    }

    // Calculate centroid if not already set
    if (geoFeature.centroid.isNull() && !geoFeature.polygons.isEmpty()) {
        geoFeature.centroid = calculateCentroid(geoFeature.polygons);
    }

    m_features.append(geoFeature);
}

QPolygonF GeoJsonParser::parsePolygon(const QJsonArray& coords) {
    QPolygonF polygon;
    for (const auto& pointVal : coords) {
        QJsonArray point = pointVal.toArray();
        if (point.size() >= 2) {
            // GeoJSON is [lon, lat], we store as (lat, lon) for QPointF
            double lon = point[0].toDouble();
            double lat = point[1].toDouble();
            polygon.append(QPointF(lat, lon));
        }
    }
    return polygon;
}

QPointF GeoJsonParser::calculateCentroid(const QVector<QPolygonF>& polygons) {
    if (polygons.isEmpty()) return QPointF();

    double totalLat = 0, totalLon = 0;
    int count = 0;

    for (const auto& poly : polygons) {
        for (const auto& point : poly) {
            totalLat += point.x();
            totalLon += point.y();
            count++;
        }
    }

    if (count == 0) return QPointF();
    return QPointF(totalLat / count, totalLon / count);
}

QVariantList GeoJsonParser::countryList() const {
    QVariantList result;
    for (const auto& feature : m_features) {
        if (feature.type == "country" && !feature.name.isEmpty()) {
            QVariantMap item;
            item["name"] = feature.name;
            item["code"] = feature.code;
            result.append(item);
        }
    }
    return result;
}

QVariantList GeoJsonParser::regionList(const QString& countryCode) const {
    QVariantList result;
    for (const auto& feature : m_features) {
        if (feature.type == "region") {
            QString parentCode = feature.properties.value("iso_a2").toString();
            if (parentCode == countryCode) {
                QVariantMap item;
                item["name"] = feature.name;
                item["code"] = feature.code;
                result.append(item);
            }
        }
    }
    return result;
}

QVariantList GeoJsonParser::cityList() const {
    QVariantList result;
    for (const auto& feature : m_features) {
        if (feature.type == "city" && !feature.name.isEmpty()) {
            QVariantMap item;
            item["name"] = feature.name;
            item["lat"] = feature.centroid.x();
            item["lon"] = feature.centroid.y();
            result.append(item);
        }
    }
    return result;
}

const GeoFeature* GeoJsonParser::findByCode(const QString& code) const {
    for (const auto& feature : m_features) {
        if (feature.code == code) {
            return &feature;
        }
    }
    return nullptr;
}

const GeoFeature* GeoJsonParser::findByName(const QString& name) const {
    for (const auto& feature : m_features) {
        if (feature.name.compare(name, Qt::CaseInsensitive) == 0) {
            return &feature;
        }
    }
    return nullptr;
}

void GeoJsonParser::loadBuiltInCities() {
    // Major world cities with population data
    struct CityData {
        const char* name;
        double lat;
        double lon;
        int population;
        const char* country;
    };

    static const CityData cities[] = {
        // Europe
        {"London", 51.5074, -0.1278, 9000000, "GB"},
        {"Paris", 48.8566, 2.3522, 11000000, "FR"},
        {"Berlin", 52.5200, 13.4050, 3600000, "DE"},
        {"Madrid", 40.4168, -3.7038, 6600000, "ES"},
        {"Rome", 41.9028, 12.4964, 4300000, "IT"},
        {"Vienna", 48.2082, 16.3738, 1900000, "AT"},
        {"Amsterdam", 52.3676, 4.9041, 1100000, "NL"},
        {"Brussels", 50.8503, 4.3517, 1200000, "BE"},
        {"Warsaw", 52.2297, 21.0122, 1800000, "PL"},
        {"Prague", 50.0755, 14.4378, 1300000, "CZ"},
        {"Budapest", 47.4979, 19.0402, 1750000, "HU"},
        {"Stockholm", 59.3293, 18.0686, 1000000, "SE"},
        {"Oslo", 59.9139, 10.7522, 700000, "NO"},
        {"Copenhagen", 55.6761, 12.5683, 800000, "DK"},
        {"Helsinki", 60.1699, 24.9384, 650000, "FI"},
        {"Athens", 37.9838, 23.7275, 3100000, "GR"},
        {"Lisbon", 38.7223, -9.1393, 2900000, "PT"},
        {"Dublin", 53.3498, -6.2603, 1400000, "IE"},
        {"Zurich", 47.3769, 8.5417, 430000, "CH"},
        {"Munich", 48.1351, 11.5820, 1500000, "DE"},
        {"Milan", 45.4642, 9.1900, 3100000, "IT"},
        {"Barcelona", 41.3851, 2.1734, 5500000, "ES"},
        {"Kyiv", 50.4501, 30.5234, 2900000, "UA"},
        {"Moscow", 55.7558, 37.6173, 12500000, "RU"},
        {"St. Petersburg", 59.9311, 30.3609, 5400000, "RU"},
        {"Minsk", 53.9045, 27.5615, 2000000, "BY"},
        {"Bucharest", 44.4268, 26.1025, 1900000, "RO"},
        {"Sofia", 42.6977, 23.3219, 1300000, "BG"},
        {"Belgrade", 44.7866, 20.4489, 1400000, "RS"},
        {"Zagreb", 45.8150, 15.9819, 800000, "HR"},

        // Asia
        {"Tokyo", 35.6762, 139.6503, 37400000, "JP"},
        {"Beijing", 39.9042, 116.4074, 21500000, "CN"},
        {"Shanghai", 31.2304, 121.4737, 27000000, "CN"},
        {"Hong Kong", 22.3193, 114.1694, 7500000, "HK"},
        {"Seoul", 37.5665, 126.9780, 9800000, "KR"},
        {"Singapore", 1.3521, 103.8198, 5700000, "SG"},
        {"Bangkok", 13.7563, 100.5018, 10500000, "TH"},
        {"Mumbai", 19.0760, 72.8777, 21000000, "IN"},
        {"Delhi", 28.7041, 77.1025, 31000000, "IN"},
        {"Kolkata", 22.5726, 88.3639, 14700000, "IN"},
        {"Chennai", 13.0827, 80.2707, 11000000, "IN"},
        {"Bangalore", 12.9716, 77.5946, 12500000, "IN"},
        {"Jakarta", 6.2088, 106.8456, 34500000, "ID"},
        {"Manila", 14.5995, 120.9842, 14400000, "PH"},
        {"Hanoi", 21.0278, 105.8342, 8000000, "VN"},
        {"Ho Chi Minh City", 10.8231, 106.6297, 9000000, "VN"},
        {"Taipei", 25.0330, 121.5654, 7000000, "TW"},
        {"Osaka", 34.6937, 135.5023, 19200000, "JP"},
        {"Kuala Lumpur", 3.1390, 101.6869, 7800000, "MY"},
        {"Dubai", 25.2048, 55.2708, 3400000, "AE"},
        {"Tel Aviv", 32.0853, 34.7818, 4100000, "IL"},
        {"Istanbul", 41.0082, 28.9784, 15500000, "TR"},
        {"Ankara", 39.9334, 32.8597, 5700000, "TR"},
        {"Tehran", 35.6892, 51.3890, 9000000, "IR"},
        {"Riyadh", 24.7136, 46.6753, 7700000, "SA"},
        {"Karachi", 24.8607, 67.0011, 16000000, "PK"},
        {"Lahore", 31.5204, 74.3587, 13000000, "PK"},

        // Americas
        {"New York", 40.7128, -74.0060, 18800000, "US"},
        {"Los Angeles", 34.0522, -118.2437, 12500000, "US"},
        {"Chicago", 41.8781, -87.6298, 8900000, "US"},
        {"Houston", 29.7604, -95.3698, 6300000, "US"},
        {"Phoenix", 33.4484, -112.0740, 4900000, "US"},
        {"Philadelphia", 39.9526, -75.1652, 5700000, "US"},
        {"San Francisco", 37.7749, -122.4194, 4700000, "US"},
        {"Seattle", 47.6062, -122.3321, 4000000, "US"},
        {"Miami", 25.7617, -80.1918, 6200000, "US"},
        {"Washington", 38.9072, -77.0369, 6300000, "US"},
        {"Boston", 42.3601, -71.0589, 4900000, "US"},
        {"Atlanta", 33.7490, -84.3880, 6100000, "US"},
        {"Dallas", 32.7767, -96.7970, 7600000, "US"},
        {"Denver", 39.7392, -104.9903, 2900000, "US"},
        {"Toronto", 43.6532, -79.3832, 6200000, "CA"},
        {"Montreal", 45.5017, -73.5673, 4200000, "CA"},
        {"Vancouver", 49.2827, -123.1207, 2500000, "CA"},
        {"Mexico City", 19.4326, -99.1332, 21800000, "MX"},
        {"São Paulo", 23.5505, -46.6333, 22000000, "BR"},
        {"Rio de Janeiro", -22.9068, -43.1729, 13500000, "BR"},
        {"Buenos Aires", -34.6037, -58.3816, 15400000, "AR"},
        {"Lima", -12.0464, -77.0428, 10700000, "PE"},
        {"Bogotá", 4.7110, -74.0721, 11300000, "CO"},
        {"Santiago", -33.4489, -70.6693, 6800000, "CL"},
        {"Caracas", 10.4806, -66.9036, 2900000, "VE"},

        // Africa
        {"Cairo", 30.0444, 31.2357, 21300000, "EG"},
        {"Lagos", 6.5244, 3.3792, 15300000, "NG"},
        {"Johannesburg", -26.2041, 28.0473, 5800000, "ZA"},
        {"Cape Town", -33.9249, 18.4241, 4600000, "ZA"},
        {"Nairobi", -1.2921, 36.8219, 5000000, "KE"},
        {"Addis Ababa", 9.0320, 38.7469, 5000000, "ET"},
        {"Casablanca", 33.5731, -7.5898, 3700000, "MA"},
        {"Algiers", 36.7538, 3.0588, 3900000, "DZ"},
        {"Accra", 5.6037, -0.1870, 2500000, "GH"},
        {"Dar es Salaam", -6.7924, 39.2083, 7000000, "TZ"},
        {"Kinshasa", -4.4419, 15.2663, 15000000, "CD"},
        {"Luanda", -8.8390, 13.2894, 8300000, "AO"},

        // Oceania
        {"Sydney", -33.8688, 151.2093, 5300000, "AU"},
        {"Melbourne", -37.8136, 144.9631, 5000000, "AU"},
        {"Brisbane", -27.4698, 153.0251, 2500000, "AU"},
        {"Perth", -31.9505, 115.8605, 2100000, "AU"},
        {"Auckland", -36.8509, 174.7645, 1700000, "NZ"},
        {"Wellington", -41.2866, 174.7756, 420000, "NZ"},
    };

    for (const auto& city : cities) {
        GeoFeature feature;
        feature.type = "city";
        feature.name = QString::fromUtf8(city.name);
        feature.centroid = QPointF(city.lat, city.lon);
        feature.code = QString::fromUtf8(city.country);
        feature.properties["population"] = city.population;
        feature.properties["country"] = QString::fromUtf8(city.country);
        m_features.append(feature);
    }

    emit loaded();
}
