#include <visualizer/Shader.hpp>

#include <cassert>
#include <charconv>
#include <fstream>
#include <iostream>
#include <utility>

namespace Visualizer {

static std::vector<std::string_view> splitSV(std::string_view strv, std::string_view delims = " "sv)
{
    std::vector<std::string_view> output;
    size_t first = 0;

    while (first < strv.size()) {
        const auto second = strv.find_first_of(delims, first);

        if (first != second)
            output.emplace_back(strv.substr(first, second - first));

        if (second == std::string_view::npos)
            break;

        first = second + 1;
    }

    return output;
}

template <typename T> T fromStringView(std::string_view stringView)
{
    T tmp{};
    std::from_chars(stringView.data(), stringView.data() + stringView.size(), tmp);
    return tmp;
}

/**************************************************************************************************
 ********************************************* Shader *********************************************
 **************************************************************************************************/

constexpr std::array<std::string_view, 2> sParameterQualifierNames{ "@program"sv, "@material"sv };
constexpr std::array<ParameterQualifier, 2> sParameterQualifierMap{ ParameterQualifier::Program,
    ParameterQualifier::Material };

std::map<std::string_view, ParameterType> s_parameter_type_map{
    { "bool"sv, ParameterType::Bool },
    { "int"sv, ParameterType::Int },
    { "uint"sv, ParameterType::UInt },
    { "float"sv, ParameterType::Float },
    { "bvec2"sv, ParameterType::BVec2 },
    { "bvec3"sv, ParameterType::BVec3 },
    { "bvec4"sv, ParameterType::BVec4 },
    { "ivec2"sv, ParameterType::IVec2 },
    { "ivec3"sv, ParameterType::IVec3 },
    { "ivec4"sv, ParameterType::IVec4 },
    { "uvec2"sv, ParameterType::UVec2 },
    { "uvec3"sv, ParameterType::UVec3 },
    { "uvec4"sv, ParameterType::UVec4 },
    { "vec2"sv, ParameterType::Vec2 },
    { "vec3"sv, ParameterType::Vec3 },
    { "vec4"sv, ParameterType::Vec4 },
    { "mat2x2"sv, ParameterType::Mat2x2 },
    { "mat2x3"sv, ParameterType::Mat2x3 },
    { "mat2x4"sv, ParameterType::Mat2x4 },
    { "mat3x2"sv, ParameterType::Mat3x2 },
    { "mat3x3"sv, ParameterType::Mat3x3 },
    { "mat3x4"sv, ParameterType::Mat3x4 },
    { "mat4x2"sv, ParameterType::Mat4x2 },
    { "mat4x3"sv, ParameterType::Mat4x3 },
    { "mat4x4"sv, ParameterType::Mat4x4 },
    { "sampler2D"sv, ParameterType::Sampler2D },
    { "sampler2DMS"sv, ParameterType::Sampler2DMultisample },
    { "samplerBuffer"sv, ParameterType::SamplerBuffer },
    { "usampler2D"sv, ParameterType::Sampler2D },
    { "usampler2DMS"sv, ParameterType::Sampler2DMultisample },
    { "usamplerBuffer"sv, ParameterType::SamplerBuffer },
    { "image2D"sv, ParameterType::Image2D },
    { "image2DMS"sv, ParameterType::Image2DMultisample },
    { "imageBuffer"sv, ParameterType::ImageBuffer },
    { "uimage2D"sv, ParameterType::Image2D },
    { "uimage2DMS"sv, ParameterType::Image2DMultisample },
    { "uimageBuffer"sv, ParameterType::ImageBuffer },
};

Shader::Shader(const std::filesystem::path& shaderPath, ShaderType shaderType)
    : m_shader{ 0 }
    , m_shaderType{ shaderType }
    , m_parameters{}
{
    if (!std::filesystem::exists(shaderPath)) {
        return;
    }

    std::string shaderFile{};
    std::ifstream shaderStream{ shaderPath };
    shaderStream.seekg(0, std::ios::end);
    auto shaderSize{ static_cast<std::size_t>(shaderStream.tellg()) };
    shaderStream.seekg(0, std::ios::beg);
    shaderFile.reserve(shaderSize);

    std::string line{};
    while (std::getline(shaderStream, line)) {
        auto lineTokens{ splitSV({ line.data(), line.size() }, " ") };
        if (lineTokens.size() >= 4) {
            if (auto qualPos{
                    std::find(sParameterQualifierNames.begin(), sParameterQualifierNames.end(), lineTokens[0]) };
                qualPos != sParameterQualifierNames.end()) {
                auto qualifier{ sParameterQualifierMap[qualPos - sParameterQualifierNames.begin()] };

                if (auto type_pos{ s_parameter_type_map.find(lineTokens[1]) }; type_pos != s_parameter_type_map.end()) {
                    auto type{ type_pos->second };
                    auto arraySize{ fromStringView<std::size_t>(lineTokens[2]) };
                    auto parameterName{ lineTokens[3] };

                    m_parameters.emplace_back(
                        qualifier, type, arraySize, std::string{ parameterName.data(), parameterName.size() });
                    continue;
                }
            }
        }
        shaderFile.append("\n");
        shaderFile.append(line);
    }

    assert(glGetError() == GL_NO_ERROR);
    switch (m_shaderType) {
    case ShaderType::VertexShader:
        m_shader = glCreateShader(GL_VERTEX_SHADER);
        break;
    case ShaderType::FragmentShader:
        m_shader = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        m_parameters.clear();
        return;
    }
    assert(glGetError() == GL_NO_ERROR);

    auto shaderDataPtr{ shaderFile.data() };
    glShaderSource(m_shader, 1, &shaderDataPtr, nullptr);
    glCompileShader(m_shader);
    GLint testVal;
    glGetShaderiv(m_shader, GL_COMPILE_STATUS, &testVal);
    if (testVal == GL_FALSE) {
        char infoLog[1024];
        glGetShaderInfoLog(m_shader, 1024, NULL, infoLog);
        std::cerr << "The vertex shader failed to compile with the error:" << std::endl << infoLog << std::endl;
        glDeleteShader(m_shader);
        m_shader = 0;
        m_parameters.clear();
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_shader{ std::exchange(other.m_shader, 0) }
    , m_shaderType{ other.m_shaderType }
    , m_parameters{ std::exchange(other.m_parameters, {}) }
{
}

Shader::~Shader()
{
    if (m_shader != 0) {
        glDeleteShader(m_shader);
    }
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        if (m_shader != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteShader(m_shader);
            assert(glGetError() == GL_NO_ERROR);
        }

        m_shader = std::exchange(other.m_shader, 0);
        m_shaderType = other.m_shaderType;
        m_parameters = std::exchange(other.m_parameters, {});
    }
    return *this;
}

GLuint Shader::shader() const { return m_shader; }
ShaderType Shader::shaderType() const { return m_shaderType; }
std::span<const ParameterDeclaration> Shader::parameters() const
{
    return { m_parameters.data(), m_parameters.size() };
}

std::optional<Shader> Shader::create(const std::filesystem::path& shaderPath, ShaderType shaderType)
{
    Shader shader{ shaderPath, shaderType };
    if (shader.m_shader != 0) {
        return shader;
    } else {
        return std::nullopt;
    }
}

/**************************************************************************************************
 *************************************** ShaderEnvironment ***************************************
 **************************************************************************************************/

constexpr std::array<std::tuple<std::size_t, std::size_t>, 31> sTypeSizeAlignmentPairs{
    std::tuple<std::size_t, std::size_t>{ sizeof(GLboolean), alignof(GLboolean) },
    std::tuple<std::size_t, std::size_t>{ sizeof(GLint), alignof(GLint) },
    std::tuple<std::size_t, std::size_t>{ sizeof(GLuint), alignof(GLuint) },
    std::tuple<std::size_t, std::size_t>{ sizeof(GLfloat), alignof(GLfloat) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::bvec2), alignof(glm::bvec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::bvec3), alignof(glm::bvec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::bvec4), alignof(glm::bvec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::ivec2), alignof(glm::ivec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::ivec3), alignof(glm::ivec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::ivec4), alignof(glm::ivec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::uvec2), alignof(glm::uvec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::uvec3), alignof(glm::uvec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::uvec4), alignof(glm::uvec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::vec2), alignof(glm::vec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::vec3), alignof(glm::vec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::vec4), alignof(glm::vec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat2x2), alignof(glm::mat2x2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat2x3), alignof(glm::mat2x3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat2x4), alignof(glm::mat2x4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat3x2), alignof(glm::mat3x2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat3x3), alignof(glm::mat3x3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat3x4), alignof(glm::mat3x4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat4x2), alignof(glm::mat4x2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat4x3), alignof(glm::mat4x3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat4x4), alignof(glm::mat4x4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(TextureSampler<Texture2D>), alignof(TextureSampler<Texture2D>) },
    std::tuple<std::size_t, std::size_t>{
        sizeof(TextureSampler<Texture2DMultisample>), alignof(TextureSampler<Texture2DMultisample>) },
    std::tuple<std::size_t, std::size_t>{
        sizeof(TextureSampler<TextureBuffer>), alignof(TextureSampler<TextureBuffer>) },
    std::tuple<std::size_t, std::size_t>{ sizeof(TextureImage<Texture2D>), alignof(TextureImage<Texture2D>) },
    std::tuple<std::size_t, std::size_t>{
        sizeof(TextureImage<Texture2DMultisample>), alignof(TextureImage<Texture2DMultisample>) },
    std::tuple<std::size_t, std::size_t>{ sizeof(TextureImage<TextureBuffer>), alignof(TextureImage<TextureBuffer>) },
};

ShaderEnvironment::ShaderEnvironment(ShaderProgram& program, ParameterQualifier filter)
    : m_dataSize{ 0 }
    , m_dataAlignment{ 0 }
    , m_parameterNames{}
    , m_parameterInfos{}
    , m_parameterData{}
{
    std::size_t currentPos{ 0 };

    for (auto& parameter : program.parameters()) {
        auto qualifier{ std::get<0>(parameter) };
        auto type{ std::get<1>(parameter) };
        auto size{ std::get<2>(parameter) };
        auto& name{ std::get<3>(parameter) };

        if ((static_cast<std::size_t>(qualifier) & static_cast<std::size_t>(filter)) == 0) {
            continue;
        }

        auto it{ m_parameterInfos.insert_or_assign(
            name, ShaderEnvironment::ParameterInfo{ false, currentPos, size, type }) };
        m_parameterNames.emplace_back(it.first->first);

        if (static_cast<std::size_t>(type) <= static_cast<std::size_t>(ParameterType::MaxIndex)) {
            auto sizeAlignmentPair{ sTypeSizeAlignmentPairs[static_cast<std::size_t>(type)] };
            auto typeSize{ std::get<0>(sizeAlignmentPair) };
            auto typeAlignment{ std::get<1>(sizeAlignmentPair) };

            if (m_dataAlignment == 0) {
                m_dataAlignment = typeAlignment;
            }

            currentPos += (currentPos % typeAlignment) + (typeSize * size);
        }
    }

    m_dataSize = currentPos;

    m_parameterData
        = { AlignedDeleter<unsigned char>::allocate(m_dataAlignment, m_dataSize), AlignedDeleter<unsigned char>{} };
    assert(m_parameterData.get() != nullptr);
    std::memset(m_parameterData.get(), 0, m_dataSize);
}

ShaderEnvironment::ShaderEnvironment(const ShaderEnvironment& other)
    : m_dataSize{ other.m_dataSize }
    , m_dataAlignment{ other.m_dataAlignment }
    , m_parameterNames{}
    , m_parameterInfos{ other.m_parameterInfos }
    , m_parameterData{ AlignedDeleter<unsigned char>::allocate(other.m_dataAlignment, other.m_dataSize),
        AlignedDeleter<unsigned char>{} }
{
    m_parameterNames.reserve(other.m_parameterNames.size());
    for (auto& it : m_parameterInfos) {
        m_parameterNames.emplace_back(it.first);
    }
}

ShaderEnvironment& ShaderEnvironment::operator=(const ShaderEnvironment& other)
{
    if (this != &other) {
        if (other.m_dataSize == 0) {
            m_dataSize = 0;
            m_parameterInfos = {};
            m_dataAlignment = 0;
            m_parameterData = {};
            m_parameterNames = {};
        } else {
            m_dataSize = other.m_dataSize;
            m_parameterInfos = other.m_parameterInfos;
            m_dataAlignment = other.m_dataAlignment;
            m_parameterData = { AlignedDeleter<unsigned char>::allocate(other.m_dataAlignment, other.m_dataSize),
                AlignedDeleter<unsigned char>{} };
            std::memset(m_parameterData.get(), 0, m_dataSize);

            m_parameterNames.clear();
            m_parameterNames.resize(other.m_parameterNames.size());
            for (auto& it : m_parameterInfos) {
                m_parameterNames.emplace_back(it.first);
            }
        }
    }
    return *this;
}

std::span<std::string_view> ShaderEnvironment::parameters() const
{
    return { const_cast<std::string_view*>(m_parameterNames.data()), m_parameterNames.size() };
}

std::size_t ShaderEnvironment::parameter_length(std::string_view name) const
{
    if (auto pos{ m_parameterInfos.find(name) }; pos != m_parameterInfos.end()) {
        return pos->second.size;
    } else {
        assert(false);
        return 0;
    }
}

ParameterType ShaderEnvironment::parameter_type(std::string_view name) const
{
    if (auto pos{ m_parameterInfos.find(name) }; pos != m_parameterInfos.end()) {
        return pos->second.type;
    } else {
        assert(false);
        return ParameterType::MaxIndex;
    }
}

ShaderEnvironment::~ShaderEnvironment()
{
    /// TODO: Destruct inners
}

/**************************************************************************************************
 ***************************************** ShaderProgram *****************************************
 **************************************************************************************************/

constexpr std::array<void (*)(const ShaderEnvironment&, GLuint, std::string_view, std::size_t), 31> sTypeApplyFuncs{
    /**************************************** Scalars ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLboolean>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform1iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLint>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform1iv(location, static_cast<GLsizei>(size), val);
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLuint>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform1uiv(location, static_cast<GLsizei>(size), val);
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLfloat>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform1fv(location, static_cast<GLsizei>(size), val);
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** BVecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::bvec2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform2iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(glm::value_ptr(*val)));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::bvec3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform3iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(glm::value_ptr(*val)));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::bvec4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform4iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(glm::value_ptr(*val)));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** IVecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::ivec2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform2iv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::ivec3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform3iv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::ivec4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform4iv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** UVecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::uvec2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform2uiv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::uvec3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform3uiv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::uvec4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform4uiv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** VecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::vec2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform2fv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::vec3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform3fv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::vec4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniform4fv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** Mat2xN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat2x2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix2fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat2x3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix2x3fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat2x4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix2x4fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** Mat3xN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat3x2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix3x2fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat3x3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix3fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat3x4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix3x4fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** Mat4xN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat4x2>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix4x2fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat4x3>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix4x3fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat4x4>(name, size) };
        if (val != nullptr) {
            assert(glGetError() == GL_NO_ERROR);
            glUniformMatrix4fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
            assert(glGetError() == GL_NO_ERROR);
        }
    },
    /**************************************** SamplerN ****************************************/
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureSampler<Texture2D>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureSampler<Texture2DMultisample>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureSampler<TextureBuffer>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
    /**************************************** ImageN ****************************************/
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureImage<Texture2D>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureImage<Texture2DMultisample>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureImage<TextureBuffer>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
};

ShaderProgram::ShaderProgram()
    : m_bound{ false }
    , m_program{ glCreateProgram() }
    , m_parameters{}
    , m_parameterLocations{}
{
    assert(glGetError() == GL_NO_ERROR);
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : m_bound{ std::exchange(other.m_bound, false) }
    , m_program{ std::exchange(other.m_program, 0) }
    , m_parameters{ std::exchange(other.m_parameters, {}) }
    , m_parameterLocations{ std::exchange(other.m_parameterLocations, {}) }
{
    assert(glGetError() == GL_NO_ERROR);
}

ShaderProgram::~ShaderProgram()
{
    unbind();
    if (m_program != 0) {
        assert(glGetError() == GL_NO_ERROR);
        glDeleteProgram(m_program);
        assert(glGetError() == GL_NO_ERROR);
    }
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other) {
        unbind();
        if (m_program != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteProgram(m_program);
            assert(glGetError() == GL_NO_ERROR);
        }

        m_bound = std::exchange(other.m_bound, false);
        m_program = std::exchange(other.m_program, 0);
        m_parameters = std::exchange(other.m_parameters, {});
        m_parameterLocations = std::exchange(other.m_parameterLocations, {});
    }
    return *this;
}

void ShaderProgram::bind()
{
    if (!m_bound) {
        assert(glGetError() == GL_NO_ERROR);
        glUseProgram(m_program);
        assert(glGetError() == GL_NO_ERROR);
        m_bound = true;
    }
}
void ShaderProgram::unbind()
{
    if (m_bound) {
        assert(glGetError() == GL_NO_ERROR);
        glUseProgram(0);
        assert(glGetError() == GL_NO_ERROR);
        m_bound = false;
    }
}

void ShaderProgram::apply(const ShaderEnvironment& environment) const
{
    if (!m_bound) {
        return;
    }

    for (auto parameter : environment.parameters()) {
        auto declarationPos{ std::find_if(m_parameters.begin(), m_parameters.end(),
            [&](const ParameterDeclaration& decl) -> bool { return std::get<3>(decl) == parameter; }) };

        if (declarationPos == m_parameters.end()) {
            continue;
        }

        auto& declaration{ *declarationPos };
        auto parameterType{ std::get<1>(declaration) };
        auto parameterSize{ static_cast<GLsizei>(std::get<2>(declaration)) };
        auto parameterLocation{ m_parameterLocations.at(parameter) };

        if (static_cast<std::size_t>(parameterType) <= static_cast<std::size_t>(ParameterType::MaxIndex)) {
            auto applyFunc{ sTypeApplyFuncs[static_cast<std::size_t>(parameterType)] };
            applyFunc(environment, parameterLocation, parameter, parameterSize);
        }
    }
}

std::span<const ParameterDeclaration> ShaderProgram::parameters() const
{
    return { m_parameters.data(), m_parameters.size() };
}

}