#include <musica/tuvx.hpp>

#include <gtest/gtest.h>

using namespace musica;

// Test fixture for the TUVX C API
class TuvxCApiTest : public ::testing::Test {
protected:
    TUVX* tuvx;
    const char* config_path;

    // the function that google test actually calls before each test
    void SetUp() override {
        tuvx = nullptr;
    }

    void SetUp(const char* configPath) {
        Error error;
        tuvx = nullptr;
        config_path = configPath; // Set the config path based on the parameter
        tuvx = CreateTuvx(config_path, &error);
        if (!IsSuccess(error)) {
            std::cerr << "Error creating TUVX instance: " << error.message_.value_ << std::endl;
        }
        ASSERT_TRUE(IsSuccess(error));
        DeleteError(&error);
    }

    void TearDown() override {
        if (tuvx == nullptr) {
            return;
        }
        Error error;
        DeleteTuvx(tuvx, &error);
        ASSERT_TRUE(IsSuccess(error));
        DeleteError(&error);
        tuvx = nullptr;
    }
};


TEST_F(TuvxCApiTest, CreateTuvxInstanceWithYamlConfig) {
    const char* yaml_config_path = "examples/ts1_tsmlt.yml";
    SetUp(yaml_config_path);
    ASSERT_NE(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, CreateTuvxInstanceWithJsonConfig) {
    const char* json_config_path = "examples/ts1_tsmlt.json";
    SetUp(json_config_path);
    ASSERT_NE(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, DetectsNonexistentConfigFile) {
    const char* config_path = "nonexisting.yml";
    Error error;
    TUVX* tuvx = CreateTuvx(config_path, &error);
    ASSERT_FALSE(IsSuccess(error));
    DeleteError(&error);
}


TEST_F(TuvxCApiTest, CanCallRun) {
    const char* yaml_config_path = "examples/ts1_tsmlt.yml";
    SetUp(yaml_config_path);
    ASSERT_NE(tuvx, nullptr);
    Error error = NoError();
    run_tuvx(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
}
