#include "cpuTemperatureReader.hpp"
namespace CpuTemperatureReader
{
std::vector<double> readTemperatures()
{
    std::vector<double> temperatures;
    // Get the list of detected sensor chips
    const sensors_chip_name* chip;
    int chip_nr = 0;
    while ((chip = sensors_get_detected_chips(NULL, &chip_nr)) != NULL)
    {
        // Get the list of features for this chip
        const sensors_feature* feature;
        int feature_nr = 0;
        while ((feature = sensors_get_features(chip, &feature_nr)) != NULL)
        {
            // Get the list of sub-features (temperature readings, etc.) for this feature
            const sensors_subfeature* subfeature;
            int subfeature_nr = 0;
            while ((subfeature = sensors_get_all_subfeatures(chip, feature, &subfeature_nr)) != NULL)
            {
                if (subfeature->type == SENSORS_SUBFEATURE_TEMP_INPUT)
                {
                    double temp;
                    if (sensors_get_value(chip, subfeature->number, &temp) == 0)
                    {
                        temperatures.push_back(temp);
                    }
                    else
                    {
                        std::cerr << "Failed to read temperature" << std::endl;
                    }
                }
            }
        }
    }

    // Return the last four readings they are the CPU core temperatures
    if (temperatures.size() >= 4)
    {
        return std::vector<double>(temperatures.end() - 4, temperatures.end());
    }
    else
    {
        std::cerr << "Not enough temperature readings found" << std::endl;
        return {};
    }
}
}    // namespace CpuTemperatureReader