#include "cpuTemperatureReader.hpp"

CpuTemperatureReader::CpuTemperatureReader() {
    initializeSensors();
}

CpuTemperatureReader::~CpuTemperatureReader() {
    cleanupSensors();
}

void CpuTemperatureReader::initializeSensors() {
    // Initialize the sensors library
    if (sensors_init(NULL) != 0) {
        std::cerr << "Failed to initialize sensors library" << std::endl;
        exit(1);
    }
}

void CpuTemperatureReader::cleanupSensors() {
    sensors_cleanup();
}

void CpuTemperatureReader::readTemperatures() {
    // Get the list of detected sensor chips
    const sensors_chip_name *chip;
    int chip_nr = 0;
    while ((chip = sensors_get_detected_chips(NULL, &chip_nr)) != NULL) {
        std::cout << "Chip: " << chip->prefix << std::endl;

        // Get the list of features for this chip
        const sensors_feature *feature;
        int feature_nr = 0;
        while ((feature = sensors_get_features(chip, &feature_nr)) != NULL) {
            std::cout << "  Feature: " << sensors_get_label(chip, feature) << std::endl;

            // Get the list of sub-features (temperature readings, etc.) for this feature
            const sensors_subfeature *subfeature;
            int subfeature_nr = 0;
            while ((subfeature = sensors_get_all_subfeatures(chip, feature, &subfeature_nr)) != NULL) {
                if (subfeature->type == SENSORS_SUBFEATURE_TEMP_INPUT) {
                    double temp;
                    if (sensors_get_value(chip, subfeature->number, &temp) == 0) {
                        std::cout << "    Temperature: " << temp << "°C" << std::endl;
                    } else {
                        std::cerr << "    Failed to read temperature" << std::endl;
                    }
                }
            }
        }
    }
}

