#include <doctest/doctest.h>
#include <filesystem>
#include <random>

#include <iostream>
#include <visualizer/VisualizerConfiguration.hpp>

TEST_SUITE("VisualizerConfiguration")
{
    Visualizer::InnerCube generateRandomInnerCube(std::size_t numChildren)
    {
        std::random_device rd{};
        std::mt19937 gen{ rd() };

        std::uniform_int_distribution<std::uint32_t> colorDistribution{ 0, 255 };
        std::uniform_int_distribution<std::uint32_t> tilingInfoDistribution{ 0, 255 };
        std::uniform_int_distribution<std::uint32_t> traversalOrderDistribution{ 0, 6 };
        std::uniform_int_distribution<std::size_t> numChildrenDistribution{ 0, numChildren - 1 };

        Visualizer::Color color{ static_cast<std::uint8_t>(colorDistribution(gen)),
            static_cast<std::uint8_t>(colorDistribution(gen)),
            static_cast<std::uint8_t>(colorDistribution(gen)) };

        Visualizer::TilingInfo tilingInfo{ tilingInfoDistribution(gen), tilingInfoDistribution(gen),
            tilingInfoDistribution(gen) };

        Visualizer::TraversalOrder traversalOrder{};

        switch (traversalOrderDistribution(gen)) {
        case 1:
            traversalOrder = Visualizer::TraversalOrder::XYZ;
            break;
        case 2:
            traversalOrder = Visualizer::TraversalOrder::XZY;
            break;
        case 3:
            traversalOrder = Visualizer::TraversalOrder::YXZ;
            break;
        case 4:
            traversalOrder = Visualizer::TraversalOrder::YZX;
            break;
        case 5:
            traversalOrder = Visualizer::TraversalOrder::ZXY;
            break;
        case 6:
            traversalOrder = Visualizer::TraversalOrder::ZYX;
            break;
        default:
            traversalOrder = Visualizer::TraversalOrder::XYZ;
            break;
        }

        std::shared_ptr<Visualizer::InnerCube> child{ nullptr };
        if (numChildren > 0) {
            child = std::make_shared<Visualizer::InnerCube>(
                generateRandomInnerCube(numChildrenDistribution(gen)));
        }

        return { color, tilingInfo, traversalOrder, std::move(child) };
    }

    Visualizer::OuterCube generateRandomOuterCube(std::size_t numChildren)
    {
        std::random_device rd{};
        std::mt19937 gen{ rd() };

        std::uniform_int_distribution<std::int32_t> positionDistribution{
            std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::max()
        };

        Visualizer::Position position{ positionDistribution(gen), positionDistribution(gen),
            positionDistribution(gen) };

        auto innerCube{ generateRandomInnerCube(numChildren) };
        return { position, innerCube.color, innerCube.tiling, innerCube.traversalOrder,
            std::move(innerCube.innerCube) };
    }

    Visualizer::VisualizerConfiguration generateRandomConfig()
    {
        std::random_device rd{};
        std::mt19937 gen{ rd() };
        std::uniform_int_distribution<std::uint32_t> resolutionWidthDistribution{
            std::numeric_limits<std::uint32_t>::min(), std::numeric_limits<std::uint32_t>::max()
        };
        std::uniform_int_distribution<std::uint32_t> resolutionHeightDistribution{
            std::numeric_limits<std::uint32_t>::min(), std::numeric_limits<std::uint32_t>::max()
        };
        std::uniform_int_distribution<std::uint32_t> fullscreenDistribution{ 0, 1 };
        std::uniform_int_distribution<std::size_t> numOuterCubesDistribution{ 0, 3 };
        std::uniform_int_distribution<std::size_t> numChildrenDistribution{ 0, 8 };

        Visualizer::VisualizerConfiguration config{};

        config.resolution = { resolutionWidthDistribution(gen), resolutionHeightDistribution(gen) };
        config.fullscreen = fullscreenDistribution(gen) == 1;
        config.cubes.reserve(numOuterCubesDistribution(gen));
        for (std::size_t i = 0; i < config.cubes.size(); ++i) {
            config.cubes.push_back(generateRandomOuterCube(numChildrenDistribution(gen)));
        }

        return config;
    }

    Visualizer::VisualizerConfiguration generateValidConfig()
    {
        Visualizer::VisualizerConfiguration config{};

        config.resolution = { 1920, 1080 };
        config.fullscreen = false;

        Visualizer::Position position{ 1, 1, 1 };
        Visualizer::TilingInfo tilingInfo{ 2, 2, 2 };
        Visualizer::Color color{ 255, 255, 255 };
        Visualizer::TraversalOrder traversalOrder{ Visualizer::TraversalOrder::YXZ };

        Visualizer::TilingInfo innerCubeTilingInfo{ 2, 2, 2 };
        Visualizer::Color innerCubeColor{ 255, 255, 255 };
        Visualizer::TraversalOrder innerCubeTraversalOrder{ Visualizer::TraversalOrder::YXZ };

        auto child{ std::make_shared<Visualizer::InnerCube>(
            innerCubeColor, innerCubeTilingInfo, innerCubeTraversalOrder, nullptr) };

        config.cubes.emplace_back(position, color, tilingInfo, traversalOrder, std::move(child));

        return config;
    }

    TEST_CASE("Can load and save the config")
    {
        auto tmpDir{ std::filesystem::temp_directory_path() };
        auto visualizerTmpDir{ tmpDir / "visualizer_tests" };
        REQUIRE_EQ(std::filesystem::exists(visualizerTmpDir), false);

        std::filesystem::create_directory(visualizerTmpDir);

        using namespace Visualizer;

        auto invalidConfigPath{ visualizerTmpDir / "invalid.json" };
        REQUIRE_EQ(loadConfig(invalidConfigPath).has_value(), false);

        VisualizerConfiguration testConfig{ generateRandomConfig() };

        auto testConfigPath{ visualizerTmpDir / "config.json" };
        REQUIRE_EQ(saveConfig(testConfigPath, testConfig), true);

        auto optConfig{ loadConfig(testConfigPath) };
        REQUIRE_EQ(optConfig.has_value(), true);
        CHECK_EQ(testConfig, *optConfig);

        std::filesystem::remove_all(visualizerTmpDir);
    }

    TEST_CASE("Load a valid config")
    {
        auto configFilePath{ "config.json" };
        REQUIRE_EQ(std::filesystem::exists(configFilePath), true);

        using namespace Visualizer;

        auto testConfig{ generateValidConfig() };
        auto loadedConfig{ loadConfig(configFilePath) };

        REQUIRE_EQ(loadedConfig.has_value(), true);
        CHECK_EQ(testConfig, *loadedConfig);
    }

    TEST_CASE("Load a valid config and save it again")
    {
        auto configFilePath{ "config.json" };
        auto tmpDir{ std::filesystem::temp_directory_path() };
        auto visualizerTmpDir{ tmpDir / "visualizer_tests" };

        REQUIRE_EQ(std::filesystem::exists(configFilePath), true);
        REQUIRE_EQ(std::filesystem::exists(visualizerTmpDir), false);

        std::filesystem::create_directory(visualizerTmpDir);

        using namespace Visualizer;

        auto loadedConfig{ loadConfig(configFilePath) };

        // save the loaded file again to check if it saves correct
        auto testConfigPath{ visualizerTmpDir / "load_and_save_valid_config.json" };
        REQUIRE_EQ(saveConfig(testConfigPath, *loadedConfig), true);

        auto testConfig{ loadConfig(testConfigPath) };

        REQUIRE_EQ(loadedConfig.has_value(), true);
        REQUIRE_EQ(testConfig.has_value(), true);
        CHECK_EQ(*testConfig, *loadedConfig);

        std::filesystem::remove_all(visualizerTmpDir);
    }

    TEST_CASE("Save and load valid config")
    {
        auto tmpDir{ std::filesystem::temp_directory_path() };
        auto visualizerTmpDir{ tmpDir / "visualizer_tests" };
        REQUIRE_EQ(std::filesystem::exists(visualizerTmpDir), false);

        std::filesystem::create_directory(visualizerTmpDir);

        using namespace Visualizer;

        VisualizerConfiguration testConfig{ generateValidConfig() };

        auto testConfigPath{ visualizerTmpDir / "valid_config.json" };
        REQUIRE_EQ(saveConfig(testConfigPath, testConfig), true);

        auto optConfig{ loadConfig(testConfigPath) };
        REQUIRE_EQ(optConfig.has_value(), true);
        CHECK_EQ(testConfig, *optConfig);

        std::filesystem::remove_all(visualizerTmpDir);
    }

    TEST_CASE("The key 'traversal_order' is optional")
    {
        using namespace Visualizer;

        nlohmann::json json{ { InnerCube::colorJSONKey, Position{} },
            { InnerCube::tilingJSONKey, TilingInfo{} },
            { InnerCube::innerCubeJSONKey, generateRandomInnerCube(0) } };

        InnerCube cube{};
        json.get_to(cube);

        CHECK_EQ(cube.traversalOrder, TraversalOrder::XYZ);
    }
}
