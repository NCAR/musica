#include <musica/micm.hpp>
#include <gtest/gtest.h>

// Test fixture for the MICM C API
class MicmCApiTest : public ::testing::Test {
protected:
    MICM* micm;
    int error_code;
    const char* config_path = "configs/chapman";

    void SetUp() override {
        micm = nullptr;
        error_code = 0;
        micm = create_micm(config_path, &error_code);
    }

    void TearDown() override {
        delete_micm(micm);
    }
};

// Test case for creating the MICM instance
TEST_F(MicmCApiTest, CreateMicmInstance) {
    ASSERT_EQ(error_code, 0);
    ASSERT_NE(micm, nullptr);
}

// Test case for solving the MICM instance
TEST_F(MicmCApiTest, SolveMicmInstance) {
    double time_step = 200.0;
    double temperature = 272.5;
    double pressure = 101253.3;
    int num_concentrations = 5;
    double concentrations[] = {0.75, 0.4, 0.8, 0.01, 0.02};

    auto ordering = micm->get_user_defined_reaction_rates_ordering();
    int num_custom_rate_parameters = ordering.size();
    std::vector<double> custom_rate_parameters(num_custom_rate_parameters, 0.0);
    for(auto& entry : ordering) {
        custom_rate_parameters[entry.second] = 0.0;
    }

    micm_solve(micm, time_step, temperature, pressure, num_concentrations, concentrations, custom_rate_parameters.size(), custom_rate_parameters.data());

    // Add assertions to check the solved concentrations
    ASSERT_EQ(concentrations[0], 0.75);
    ASSERT_NE(concentrations[1], 0.4);
    ASSERT_NE(concentrations[2], 0.8);
    ASSERT_NE(concentrations[3], 0.01);
    ASSERT_NE(concentrations[4], 0.02);
}

// Test case for getting species properties
TEST_F(MicmCApiTest, GetSpeciesProperty) {
    String string_value;
    string_value = get_species_property_string(micm, "O3", "__long name");
    ASSERT_STREQ(string_value.value_, "ozone");
    DeleteString(string_value);
    ASSERT_EQ(get_species_property_double(micm, "O3", "molecular weight [kg mol-1]"), 0.048);
    ASSERT_TRUE(get_species_property_bool(micm, "O3", "__do advect"));
    ASSERT_EQ(get_species_property_int(micm, "O3", "__atoms"), 3);
// these exceptions are not caught by ASSERT_THROW when using clang-cl
#ifndef MUSICA_USING_CLANGCL
    ASSERT_THROW({
        try {
            get_species_property_bool(micm, "bad species", "__is gas");
        } catch (const std::runtime_error& e) {
            ASSERT_STREQ(e.what(), "Species 'bad species' not found");
            throw;
        }}, std::runtime_error);
    EXPECT_ANY_THROW(
        get_species_property_double(micm, "O3", "bad property")
    );
#endif
}