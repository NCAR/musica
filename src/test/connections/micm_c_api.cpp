#include <musica/micm.hpp>
#include <gtest/gtest.h>

// Test fixture for the MICM C API
class MicmCApiTest : public ::testing::Test {
protected:
    void* micm;
    int error_code;

    void SetUp() override {
        micm = nullptr;
        error_code = 0;
        create_micm(&micm, error_code);
    }

    void TearDown() override {
        delete_micm(&micm);
    }
};

// Test case for creating the MICM instance
TEST_F(MicmCApiTest, CreateMicmInstance) {
    ASSERT_EQ(error_code, 0);
    ASSERT_NE(micm, nullptr);
}

// Test case for creating the MICM solver
TEST_F(MicmCApiTest, CreateMicmSolver) {
    const char* config_path = "configs/chapman";
    int solver_creation_status = micm_create_solver(&micm, config_path);
    ASSERT_EQ(solver_creation_status, 0);
}

// Test case for solving the MICM instance
TEST_F(MicmCApiTest, SolveMicmInstance) {
    const char* config_path = "configs/chapman";
    micm_create_solver(&micm, config_path);

    double time_step = 200.0;
    double temperature = 272.5;
    double pressure = 101253.3;
    int num_concentrations = 5;
    double concentrations[] = {0.75, 0.4, 0.8, 0.01, 0.02};

    micm_solve(&micm, time_step, temperature, pressure, num_concentrations, concentrations);

    // Add assertions to check the solved concentrations
    ASSERT_EQ(concentrations[0], 0.75);
    ASSERT_NE(concentrations[1], 0.4);
    ASSERT_NE(concentrations[2], 0.8);
    ASSERT_NE(concentrations[3], 0.01);
    ASSERT_NE(concentrations[4], 0.02);
}