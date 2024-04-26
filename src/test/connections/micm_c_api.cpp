#include <micm/util/error.hpp>
#include <musica/micm.hpp>
#include <musica/util.hpp>
#include <gtest/gtest.h>

// Test fixture for the MICM C API
class MicmCApiTest : public ::testing::Test {
protected:
    MICM* micm;
    const char* config_path = "configs/chapman";

    void SetUp() override {
        micm = nullptr;
        Error error;
        micm = create_micm(config_path, &error);
        ASSERT_EQ(error, NoError());
    }

    void TearDown() override {
        Error error;
        delete_micm(micm, &error);
        ASSERT_EQ(error, NoError());
    }
};

// Test case for bad configuration file path
TEST_F(MicmCApiTest, BadConfigurationFilePath) {
    Error error = NoError();
    auto micm_bad_config = create_micm("bad config path", &error);
    ASSERT_EQ(micm_bad_config, nullptr);
    ASSERT_EQ(error, ToError(MICM_ERROR_CATEGORY_CONFIGURATION,
                             MICM_CONFIGURATION_ERROR_CODE_INVALID_FILE_PATH));
}

// Test case for missing species property
TEST_F(MicmCApiTest, MissingSpeciesProperty) {
    Error error = NoError();
    String string_value;
    string_value = get_species_property_string(micm, "O3", "bad property", &error);
    ASSERT_EQ(error, ToError(MICM_ERROR_CATEGORY_SPECIES,
                             MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
    ASSERT_STREQ(string_value.value_, "");
    DeleteString(string_value);
    error = NoError();
    ASSERT_EQ(get_species_property_double(micm, "O3", "bad property", &error), 0.0);
    ASSERT_EQ(error, ToError(MICM_ERROR_CATEGORY_SPECIES,
                             MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
    error = NoError();
    ASSERT_EQ(get_species_property_int(micm, "O3", "bad property", &error), 0);
    ASSERT_EQ(error, ToError(MICM_ERROR_CATEGORY_SPECIES,
                             MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
    error = NoError();
    ASSERT_FALSE(get_species_property_bool(micm, "O3", "bad property", &error));
    ASSERT_EQ(error, ToError(MICM_ERROR_CATEGORY_SPECIES,
                             MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
}

// Test case for creating the MICM instance
TEST_F(MicmCApiTest, CreateMicmInstance) {
    ASSERT_NE(micm, nullptr);
}

// Test case for getting species ordering
TEST_F(MicmCApiTest, GetSpeciesOrdering)
{
    Error error;
    size_t array_size;
    Mapping* species_ordering = get_species_ordering(micm, &array_size, &error);
    ASSERT_EQ(error, NoError());
    ASSERT_EQ(array_size, 5);
    bool found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(species_ordering[i].name, "O3") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(species_ordering[i].name, "O") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(species_ordering[i].name, "O2") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(species_ordering[i].name, "M") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(species_ordering[i].name, "O1D") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    delete[] species_ordering;
}

// Test case for getting user-defined reaction rates ordering
TEST_F(MicmCApiTest, GetUserDefinedReactionRatesOrdering)
{
    Error error;
    size_t array_size;
    Mapping* reaction_rates_ordering = get_user_defined_reaction_rates_ordering(micm, &array_size, &error);
    ASSERT_EQ(error, NoError());
    ASSERT_EQ(array_size, 3);
    bool found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(reaction_rates_ordering[i].name, "PHOTO.R1") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(reaction_rates_ordering[i].name, "PHOTO.R3") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    found = false;
    for (size_t i = 0; i < array_size; i++) {
        if (strcmp(reaction_rates_ordering[i].name, "PHOTO.R5") == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    delete[] reaction_rates_ordering;
}

// Test case for solving the MICM instance
TEST_F(MicmCApiTest, SolveMicmInstance) {
    Error error;
    double time_step = 200.0;
    double temperature = 272.5;
    double pressure = 101253.3;
    int num_concentrations = 5;
    double concentrations[] = {0.75, 0.4, 0.8, 0.01, 0.02};

    auto ordering = micm->get_user_defined_reaction_rates_ordering(&error);
    ASSERT_EQ(error, NoError());

    int num_custom_rate_parameters = ordering.size();
    std::vector<double> custom_rate_parameters(num_custom_rate_parameters, 0.0);
    for(auto& entry : ordering) {
        custom_rate_parameters[entry.second] = 0.0;
    }

    micm_solve(micm, time_step, temperature, pressure, num_concentrations, concentrations, custom_rate_parameters.size(), custom_rate_parameters.data(), &error);
    ASSERT_EQ(error, NoError());

    // Add assertions to check the solved concentrations
    ASSERT_EQ(concentrations[0], 0.75);
    ASSERT_NE(concentrations[1], 0.4);
    ASSERT_NE(concentrations[2], 0.8);
    ASSERT_NE(concentrations[3], 0.01);
    ASSERT_NE(concentrations[4], 0.02);
}

// Test case for getting species properties
TEST_F(MicmCApiTest, GetSpeciesProperty) {
    Error error;
    String string_value;
    string_value = get_species_property_string(micm, "O3", "__long name", &error);
    ASSERT_EQ(error, NoError());
    ASSERT_STREQ(string_value.value_, "ozone");
    DeleteString(string_value);
    ASSERT_EQ(get_species_property_double(micm, "O3", "molecular weight [kg mol-1]", &error), 0.048);
    ASSERT_EQ(error, NoError());
    ASSERT_TRUE(get_species_property_bool(micm, "O3", "__do advect", &error));
    ASSERT_EQ(error, NoError());
    ASSERT_EQ(get_species_property_int(micm, "O3", "__atoms", &error), 3);
    ASSERT_EQ(error, NoError());
    get_species_property_bool(micm, "bad species", "__is gas", &error);
    ASSERT_EQ(error, ToError(MUSICA_ERROR_CATEGORY,
                             MUSICA_ERROR_CODE_SPECIES_NOT_FOUND));
    get_species_property_double(micm, "O3", "bad property", &error);
    ASSERT_EQ(error, ToError(MICM_ERROR_CATEGORY_SPECIES,
                             MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
}