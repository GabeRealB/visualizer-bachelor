#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <visualizer/AlignedMemory.hpp>
#include <visualizer/Texture.hpp>
#include <visualizer/UniqueTypes.hpp>

namespace Visualizer {

using namespace std::literals;

enum class ShaderType { VertexShader, FragmentShader };
enum class ParameterQualifier : std::size_t { Program = 0b0001, Material = 0b0010 };
enum class ParameterType : std::size_t {
    Bool = 0,
    Int = 1,
    UInt = 2,
    Float = 3,
    BVec2 = 4,
    BVec3 = 5,
    BVec4 = 6,
    IVec2 = 7,
    IVec3 = 8,
    IVec4 = 9,
    UVec2 = 10,
    UVec3 = 11,
    UVec4 = 12,
    Vec2 = 13,
    Vec3 = 14,
    Vec4 = 15,
    Mat2x2 = 16,
    Mat2x3 = 17,
    Mat2x4 = 18,
    Mat3x2 = 19,
    Mat3x3 = 20,
    Mat3x4 = 21,
    Mat4x2 = 22,
    Mat4x3 = 23,
    Mat4x4 = 24,
    Sampler2D = 25,
    MaxIndex = Sampler2D
};

template <typename T> struct ShaderTypeMapping {
    static constexpr bool hasMapping{ false };
};

template <> struct ShaderTypeMapping<GLboolean> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Bool };
};

template <> struct ShaderTypeMapping<GLint> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Int };
};

template <> struct ShaderTypeMapping<GLuint> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::UInt };
};

template <> struct ShaderTypeMapping<GLfloat> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Float };
};

template <> struct ShaderTypeMapping<glm::bvec2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::BVec2 };
};

template <> struct ShaderTypeMapping<glm::bvec3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::BVec3 };
};

template <> struct ShaderTypeMapping<glm::bvec4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::BVec4 };
};

template <> struct ShaderTypeMapping<glm::ivec2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::IVec2 };
};

template <> struct ShaderTypeMapping<glm::ivec3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::IVec3 };
};

template <> struct ShaderTypeMapping<glm::ivec4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::IVec4 };
};

template <> struct ShaderTypeMapping<glm::uvec2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::UVec2 };
};

template <> struct ShaderTypeMapping<glm::uvec3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::UVec3 };
};

template <> struct ShaderTypeMapping<glm::uvec4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::UVec4 };
};

template <> struct ShaderTypeMapping<glm::vec2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Vec2 };
};

template <> struct ShaderTypeMapping<glm::vec3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Vec3 };
};

template <> struct ShaderTypeMapping<glm::vec4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Vec4 };
};

template <> struct ShaderTypeMapping<glm::mat2x2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat2x2 };
};

template <> struct ShaderTypeMapping<glm::mat2x3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat2x3 };
};

template <> struct ShaderTypeMapping<glm::mat2x4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat2x4 };
};

template <> struct ShaderTypeMapping<glm::mat3x2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat3x2 };
};

template <> struct ShaderTypeMapping<glm::mat3x3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat3x3 };
};

template <> struct ShaderTypeMapping<glm::mat3x4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat3x4 };
};

template <> struct ShaderTypeMapping<glm::mat4x2> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat4x2 };
};

template <> struct ShaderTypeMapping<glm::mat4x3> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat4x3 };
};

template <> struct ShaderTypeMapping<glm::mat4x4> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Mat4x4 };
};

template <> struct ShaderTypeMapping<TextureSampler<Texture2D>> {
    static constexpr bool hasMapping{ true };
    static constexpr ParameterType mappedType{ ParameterType::Sampler2D };
};

using ParameterDeclaration = std::tuple<ParameterQualifier, ParameterType, std::size_t, std::string>;

class Shader {
public:
    Shader(const Shader& other) = delete;
    Shader(Shader&& other) noexcept;
    ~Shader();

    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(Shader&& other) noexcept;

    GLuint shader() const;
    ShaderType shaderType() const;
    std::span<const ParameterDeclaration> parameters() const;

    static std::optional<Shader> create(const std::filesystem::path& shaderPath, ShaderType shaderType);

private:
    Shader(const std::filesystem::path& shaderPath, ShaderType shaderType);

    GLuint m_shader;
    ShaderType m_shaderType;
    std::vector<ParameterDeclaration> m_parameters;
};

class ShaderProgram;

class ShaderEnvironment {
public:
    ShaderEnvironment() = default;
    ShaderEnvironment(ShaderProgram& program, ParameterQualifier filter);
    ShaderEnvironment(const ShaderEnvironment& other);
    ShaderEnvironment(ShaderEnvironment&& other) noexcept = default;

    ShaderEnvironment& operator=(const ShaderEnvironment& other);
    ShaderEnvironment& operator=(ShaderEnvironment&& other) noexcept = default;

    std::span<std::string_view> parameters() const;

    template <typename T>
    requires ShaderTypeMapping<T>::hasMapping std::optional<T> get(std::string_view name, std::size_t idx = 0) const
    {
        if (auto pos{ m_parameterInfos.find(name) }; pos != m_parameterInfos.end()) {
            auto parameterInfo{ pos->second };
            if (idx >= parameterInfo.size || ShaderTypeMapping<T>::mappedType != parameterInfo.type) {
                return std::nullopt;
            }
            auto index{ parameterInfo.pos + (idx * sizeof(T)) };

            T tmp;
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(&tmp, &m_parameterData[index], sizeof(T));
            } else {
                tmp = *reinterpret_cast<T*>(const_cast<unsigned char*>(&m_parameterData[index]));
            }
            return tmp;
        } else {
            return std::nullopt;
        }
    }

    template <typename T, std::size_t Size>
    requires ShaderTypeMapping<T>::hasMapping std::optional<std::array<T, Size>> get(std::string_view name) const
    {
        auto dataPtr{ getPtr<T>(name, Size) };

        if (dataPtr == nullptr) {
            return std::nullopt;
        }

        std::array<T, Size> tmp;
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(&tmp, dataPtr, sizeof(T) * Size);
        } else {
            auto pos{ dataPtr };
            for (std::size_t i = 0; i < Size; i++, pos++) {
                tmp[i] = *pos;
            }
        }
        return tmp;
    }

    template <typename T>
    requires ShaderTypeMapping<T>::hasMapping T* getPtr(std::string_view name, std::size_t size) const
    {
        if (auto pos{ m_parameterInfos.find(name) }; pos != m_parameterInfos.end()) {
            auto parameterInfo{ pos->second };
            if (size != parameterInfo.size || ShaderTypeMapping<T>::mappedType != parameterInfo.type) {
                return nullptr;
            }
            auto index{ parameterInfo.pos };
            return reinterpret_cast<T*>(const_cast<unsigned char*>(&m_parameterData[index]));
        } else {
            return nullptr;
        }
    }

    template <typename T>
    requires ShaderTypeMapping<T>::hasMapping bool set(std::string_view name, const T& val, std::size_t idx = 0)
    {
        if (auto pos{ m_parameterInfos.find(name) }; pos != m_parameterInfos.end()) {
            auto parameterInfo{ pos->second };
            if (idx >= parameterInfo.size || ShaderTypeMapping<T>::mappedType != parameterInfo.type) {
                return false;
            }
            auto index{ parameterInfo.pos + (idx * sizeof(T)) };

            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(&m_parameterData[index], &val, sizeof(T));
            } else {
                *reinterpret_cast<T*>(const_cast<unsigned char*>(&m_parameterData[index])) = val;
            }

            return true;
        } else {
            return false;
        }
    }

    template <typename T>
    requires ShaderTypeMapping<T>::hasMapping bool setArray(std::string_view name, std::span<const T> values)
    {
        auto dataPtr{ getPtr<T>(name, values.size()) };

        if (dataPtr == nullptr) {
            return false;
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(dataPtr, values.data(), sizeof(T) * values.size());
        } else {
            auto pos{ dataPtr };
            for (std::size_t i = 0; i < values.size(); i++, pos++) {
                *pos = values[i];
            }
        }

        return true;
    }

private:
    struct StringCmp {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const { return a < b; }
    };

    struct ParameterInfo {
        std::size_t pos;
        std::size_t size;
        ParameterType type;
    };

    std::size_t m_dataSize;
    std::size_t m_dataAlignment;
    std::vector<std::string_view> m_parameterNames;
    std::map<std::string, ParameterInfo, StringCmp> m_parameterInfos;
    std::unique_ptr<unsigned char[], AlignedDeleter<unsigned char>> m_parameterData;
};

class ShaderProgram {
public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ~ShaderProgram();

    ShaderProgram& operator=(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    void bind();
    void unbind();

    void apply(const ShaderEnvironment& environment) const;
    std::span<const ParameterDeclaration> parameters() const;

    template <typename... Args>
    requires SameType<Shader, Args...>&& NoCVRefs<Args...> static std::shared_ptr<ShaderProgram> create(
        const Args&... args)
    {
        auto program{ std::make_shared<ShaderProgram>() };
        if (program->m_program == 0) {
            return nullptr;
        } else {
            std::array<GLuint, sizeof...(Args)> shaders{ static_cast<const Shader&>(args).shader()... };
            for (auto shader : shaders) {
                glAttachShader(program->m_program, shader);
            }
            glLinkProgram(program->m_program);

            GLint testVal;
            glGetProgramiv(program->m_program, GL_LINK_STATUS, &testVal);
            if (testVal == GL_FALSE) {
                char infoLog[1024];
                glGetProgramInfoLog(program->m_program, 1024, NULL, infoLog);
                std::cerr << "The program failed to compile with the error:" << std::endl << infoLog << std::endl;
                return nullptr;
            }

            std::array<std::span<const ParameterDeclaration>, sizeof...(Args)> declSpans{
                static_cast<const Shader&>(args).parameters()...
            };

            for (auto span : declSpans) {
                for (auto& decl : span) {
                    if (program->m_parameterLocations.contains(std::get<3>(decl))) {
                        continue;
                    }
                    program->m_parameters.push_back(decl);
                }
            }

            program->bind();
            std::size_t textures{ 0 };

            for (auto& parameter : program->m_parameters) {
                auto location{ glGetUniformLocation(program->m_program, std::get<3>(parameter).data()) };
                if (location == -1) {
                    program->unbind();
                    return nullptr;
                }
                program->m_parameterLocations.insert_or_assign(std::get<3>(parameter), location);

                if (std::get<1>(parameter) == ParameterType::Sampler2D) {
                    glUniform1i(location, textures++);
                }
            }
            program->unbind();
        }
        return program;
    }

private:
    bool m_bound;
    GLuint m_program;
    std::vector<ParameterDeclaration> m_parameters;
    std::unordered_map<std::string_view, GLuint> m_parameterLocations;
};

struct Material {
    ShaderEnvironment m_materialVariables;
    std::shared_ptr<ShaderProgram> m_shader;
};

}
